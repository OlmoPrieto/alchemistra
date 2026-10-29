#include "game/potions_scene.h"

#include <cassert>

#include "game/game_manager.h"
#include "game/game.h"
#include "ui/ui.h"

// ------------------------

CPotionsScene::CPotionsScene()
{
  constexpr float fGridCellSizeScale = 2.0f;
  m_fCellSize     = CGame::sm_uElementPixelSize * fGridCellSizeScale;
  m_fGridWidth    = CGame::sm_uNumElementsPerTier * m_fCellSize;
  m_fGridHeight   = CGame::sm_uNumGroupsElements * m_fCellSize;
  m_fGridX        = (CGameManager::sm_uWindowWidth - m_fGridWidth) * 0.5f;
  m_fGridY        = (CGameManager::sm_uWindowHeight - m_fGridHeight) * 0.2f;
  m_fPadding      = (m_fCellSize - CGame::sm_uElementPixelSize) * 0.25f;
  m_oCauldronRect = { m_fGridX, m_fGridY + m_fGridHeight * 1.15f, m_fGridWidth, CGame::sm_uElementPixelSize * 3 };
  const CVector2D v2CraftButtonSize = CButton::CalculateSize("CRAFT");
  m_v2CraftButtonPos = { CGameManager::sm_uWindowWidth * 0.5f - v2CraftButtonSize.x * 0.5f, m_oCauldronRect.y + m_oCauldronRect.height * 1.15f };
  m_fPotionsGridX = m_fGridX;
  m_fPotionsGridY = m_v2CraftButtonPos.y * 1.125f;
  m_fPotionsGridWidth = m_fGridWidth;
  m_fPotionsGridHeight = m_fGridHeight;

  memset(m_auIngredients, 0, sizeof(uint8) * sm_uMaxIngredients);
}

// ------------------------

CPotionsScene::~CPotionsScene()
{

}

// ------------------------

void CPotionsScene::Init(const std::string& _sName)
{
  CScene::Init(_sName);
}

// ------------------------

void CPotionsScene::Update(float _fDt)
{
  const CVector2D v2MousePos = CGameManager::GetInstance().GetMousePos();
  const bool bIsLeftMouseButtonDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

  if (m_bIsDragging && bIsLeftMouseButtonDown == false)
  {
    // Drop ingredient into the cauldron
    if (v2MousePos.x >= m_oCauldronRect.x && v2MousePos.x <= m_oCauldronRect.x + m_oCauldronRect.width &&
      v2MousePos.y >= m_oCauldronRect.y && v2MousePos.y <= m_oCauldronRect.y + m_oCauldronRect.height)
    {
      if (m_uDraggedIngredient != 0u)
      {
        for (uint32 i = 0; i <= m_uCurrentNumIngredients; ++i)
        {
          if (m_auIngredients[i] == 0)
          {
            m_auIngredients[i] = m_uDraggedIngredient;
            ++m_uCurrentNumIngredients;
            break;
          }
        }
      }
    }
    else
    {
      if (m_uDraggedIngredientCauldron != 0u)
      {
        assert(m_uDraggedIngredientCauldronIndex != UINT8_MAX);
        m_auIngredients[m_uDraggedIngredientCauldronIndex] = 0u;
        m_uDraggedIngredientCauldronIndex = UINT8_MAX;
        m_uDraggedIngredientCauldron = 0u;
        --m_uCurrentNumIngredients;
      }
    }

    m_bIsDragging = false;
    m_uDraggedIngredient = 0;
    
    return;
  }
  else if (m_bIsDragging || bIsLeftMouseButtonDown == false)
  {
    return;
  }

  for (uint32 i = 0; i < CGame::sm_uNumGroupsElements; ++i)
  {
    for (uint32 j = 0; j < CGame::sm_uNumElementsPerTier; ++j)
    {
      const float fInventoryElementX = m_fGridX + j * m_fCellSize + m_fPadding + 1;
      const float fInventoryElementY = m_fGridY + i * m_fCellSize + m_fPadding;
      if (m_bIsDragging == false && bIsLeftMouseButtonDown &&
        v2MousePos.x >= fInventoryElementX && v2MousePos.x <= fInventoryElementX + CGame::sm_uElementPixelSize &&
        v2MousePos.y >= fInventoryElementY && v2MousePos.y <= fInventoryElementY + CGame::sm_uElementPixelSize)
      {
        m_bIsDragging = true;
        m_uDraggedIngredient = j + i * CGame::sm_uNumElementsPerTier + 1;
        return;
      }
    }
  }

  const uint32 uNumElementsHorizontal = m_oCauldronRect.width   / CGame::sm_uElementPixelSize;
  const uint32 uNumElementsVertical   = m_oCauldronRect.height  / CGame::sm_uElementPixelSize;

  for (uint32 i = 0; i < uNumElementsVertical; ++i)
  {
    for (uint32 j = 0; j < uNumElementsHorizontal; ++j)
    {
      const float fCauldronElementX = m_oCauldronRect.x + j * CGame::sm_uElementPixelSize;
      const float fCauldronElementY = m_oCauldronRect.y + i * CGame::sm_uElementPixelSize;
      const uint32 uIndex = i * uNumElementsHorizontal + j;
      if (m_bIsDragging == false && bIsLeftMouseButtonDown &&
        v2MousePos.x >= fCauldronElementX && v2MousePos.x <= fCauldronElementX + CGame::sm_uElementPixelSize &&
        v2MousePos.y >= fCauldronElementY && v2MousePos.y <= fCauldronElementY + CGame::sm_uElementPixelSize)
      {
        if (m_auIngredients[j + i * uNumElementsHorizontal] != 0)
        {
          m_bIsDragging = true;
          m_uDraggedIngredientCauldron = m_auIngredients[j + i * uNumElementsHorizontal];
          m_uDraggedIngredientCauldronIndex = j + i * uNumElementsHorizontal;
        }
      }
    }
  }
}

// ------------------------

void CPotionsScene::Draw()
{
  const CVector2D v2MousePos = CGameManager::GetInstance().GetMousePos();

  DrawText("POTIONS", CGameManager::sm_uWindowWidth * 0.5f - (GetFontDefault().baseSize * strlen("POTIONS")) * 0.65f,
    CGameManager::sm_uWindowHeight * 0.1f, GetFontDefault().baseSize * 2, WHITE);

  constexpr float fGridLineThickness = 1.0f;
  for (uint32 uColumn = 0; uColumn <= CGame::sm_uNumElementsPerTier; ++uColumn)
  {
    const float fX = m_fGridX + uColumn * m_fCellSize;
    DrawRectangleRec({ fX - fGridLineThickness * 0.5f, m_fGridY, fGridLineThickness, m_fGridHeight }, WHITE);
  }

  for (uint32 uRow = 0; uRow <= CGame::sm_uNumGroupsElements; ++uRow)
  {
    const float fY = m_fGridY + uRow * m_fCellSize;
    DrawRectangleRec({ m_fGridX, fY - fGridLineThickness * 0.5f, m_fGridWidth, fGridLineThickness }, WHITE);
  }

  const uint32* pInventory = CGameManager::GetInstance().GetInventory();
  std::pair<CGameManager::EPotion, uint8>* pPotions = CGameManager::GetInstance().GetPotions();
  for (uint32 i = 0; i < CGame::sm_uNumGroupsElements; ++i)
  {
    for (uint32 j = 0; j < CGame::sm_uNumElementsPerTier; ++j)
    {
      const float fElementX = m_fGridX + j * m_fCellSize + m_fPadding + 1;
      const float fElementY = m_fGridY + i * m_fCellSize + m_fPadding;
      DrawTexture(*CGameManager::GetInstance().GetTextureByElement(j + i * CGame::sm_uNumElementsPerTier + 1).get(),
        fElementX,
        fElementY,
        CGameManager::GetInstance().GetColorByElement(j + i * CGame::sm_uNumElementsPerTier + 1));

      char sBuffer[8] = { '\0' };
      sprintf(sBuffer, "x%u", pInventory[j + i * CGame::sm_uNumElementsPerTier]);

      DrawText(sBuffer,
        m_fGridX + j * m_fCellSize + m_fPadding + CGame::sm_uElementPixelSize * 1.1f, // Little offset in each coordinate
        m_fGridY + i * m_fCellSize + m_fPadding + CGame::sm_uElementPixelSize * 1.2f, // Little offset in each coordinate
        GetFontDefault().baseSize,
        WHITE);

      if (j + i * CGame::sm_uNumElementsPerTier < (uint8)CGameManager::EPotion::COUNT)
      {
        const float fPotionX = m_fPotionsGridX + j * m_fCellSize + m_fPadding + 1;
        const float fPotionY = m_fPotionsGridY + i * m_fCellSize + m_fPadding;
        DrawTexture(*CGameManager::GetInstance().GetPotionTexture().get(),
          fPotionX,
          fPotionY,
          CGameManager::GetInstance().GetPotionColor(j + i * CGame::sm_uNumElementsPerTier));

        sprintf(sBuffer, "x%u", pPotions[j + i * CGame::sm_uNumElementsPerTier].second);

        DrawText(sBuffer,
          m_fPotionsGridX + j * m_fCellSize + m_fPadding + CGame::sm_uElementPixelSize * 1.1f, // Little offset in each coordinate
          m_fPotionsGridY + i * m_fCellSize + m_fPadding + CGame::sm_uElementPixelSize * 1.2f, // Little offset in each coordinate
          GetFontDefault().baseSize,
          WHITE);
      }
    }
  }

  if (m_bIsDragging)
  {
    uint8 uIngredient = m_uDraggedIngredient != 0u ? m_uDraggedIngredient : (m_uDraggedIngredientCauldron != 0u ? m_uDraggedIngredientCauldron : 0u);
    DrawTexture(*CGameManager::GetInstance().GetTextureByElement(uIngredient).get(),
      v2MousePos.x - CGame::sm_uElementPixelSize * 0.5f,
      v2MousePos.y - CGame::sm_uElementPixelSize * 0.5f,
      CGameManager::GetInstance().GetColorByElement(uIngredient));
  }

  // Cauldron
  DrawLine(m_oCauldronRect.x,                         m_oCauldronRect.y,                          m_oCauldronRect.x + m_oCauldronRect.width,  m_oCauldronRect.y,                          SKYBLUE);
  DrawLine(m_oCauldronRect.x,                         m_oCauldronRect.y + m_oCauldronRect.height, m_oCauldronRect.x + m_oCauldronRect.width,  m_oCauldronRect.y + m_oCauldronRect.height, SKYBLUE);
  DrawLine(m_oCauldronRect.x,                         m_oCauldronRect.y,                          m_oCauldronRect.x,                          m_oCauldronRect.y + m_oCauldronRect.height, SKYBLUE);
  DrawLine(m_oCauldronRect.x + m_oCauldronRect.width, m_oCauldronRect.y,                          m_oCauldronRect.x + m_oCauldronRect.width,  m_oCauldronRect.y + m_oCauldronRect.height, SKYBLUE);

  const uint32 uNumElementsHorizontal  = m_oCauldronRect.width   / CGame::sm_uElementPixelSize;
  const uint32 uNumElementsVertical    = m_oCauldronRect.height  / CGame::sm_uElementPixelSize;

  //for (uint32 i = 0; i < sm_uMaxIngredients; ++i)
  for (uint32 i = 0; i < uNumElementsVertical; ++i)
  {
    for (uint32 j = 0; j < uNumElementsHorizontal; ++j)
    {
      const uint32 uIndex = i * uNumElementsHorizontal + j;
      if (m_auIngredients[uIndex] != 0)
      {
        DrawTexture(*CGameManager::GetInstance().GetTextureByElement(m_auIngredients[uIndex]).get(),
          m_oCauldronRect.x + j * CGame::sm_uElementPixelSize,
          m_oCauldronRect.y + i * CGame::sm_uElementPixelSize,
          CGameManager::GetInstance().GetColorByElement(m_auIngredients[uIndex]));
      }
    }
  }


  if (CButton("CRAFT", m_v2CraftButtonPos))
  {
    uint8 uPotion = CheckIngredientsForPotion();
    if (uPotion != UINT8_MAX)
    {
      printf("Crafted potion\n");
      pPotions[uPotion].second++;
    }
    else
    {
      printf("Wrong ingredients\n");
    }

    memset(m_auIngredients, 0, sizeof(uint8) * sm_uMaxIngredients);
    m_uCurrentNumIngredients = 0u;
  }


  // Potions inventory
  for (uint32 uColumn = 0; uColumn <= CGame::sm_uNumElementsPerTier; ++uColumn)
  {
    const float fX = m_fPotionsGridX + uColumn * m_fCellSize;
    DrawRectangleRec({ fX - fGridLineThickness * 0.5f, m_fPotionsGridY, fGridLineThickness, m_fPotionsGridHeight }, WHITE);
  }

  for (uint32 uRow = 0; uRow <= CGame::sm_uNumGroupsElements; ++uRow)
  {
    const float fY = m_fPotionsGridY + uRow * m_fCellSize;
    DrawRectangleRec({ m_fPotionsGridX, fY - fGridLineThickness * 0.5f, m_fPotionsGridWidth, fGridLineThickness }, WHITE);
  }
}                               

// ------------------------

bool CPotionsScene::CanBeChanged() const
{
  return true;
}

// ------------------------

uint8 CPotionsScene::CheckIngredientsForPotion()
{
  const std::vector<uint8>* pPotionsRecipes = CGameManager::GetInstance().GetPotionsRecipes();
  uint8 auIngredientsCopy[sm_uMaxIngredients];

  for (uint32 i = 0; i < (uint8)CGameManager::EPotion::COUNT; ++i)
  {
    uint8 uCurrentMatchingIngredients = 0u;
    // Check ingredients in each recipe
#if 0
    for (uint32 j = 0; j < pPotionsRecipes[i].size(); ++j)
    {
      const uint8 uPrevMatching = uCurrentMatchingIngredients;
      // Reset the ingredients copy
      memcpy(auIngredientsCopy, m_auIngredients, sizeof(uint8) * sm_uMaxIngredients);

      for (uint32 k = 0; k < m_uCurrentNumIngredients; ++k)
      {
        if (auIngredientsCopy[k] != 0 && auIngredientsCopy[k] == pPotionsRecipes[i][j])
        {
          ++uCurrentMatchingIngredients;
          auIngredientsCopy[k] = 0;
          break;
        }
      }

      if (uPrevMatching == uCurrentMatchingIngredients)
      {
        // There is a current ingredient that doesn't belong to the current potion, check next
        break;
      }
    }
#endif
    // --------------------
#if 1
    for (uint32 j = 0; j < m_uCurrentNumIngredients; ++j)
    {
      const uint8 uPrevMatching = uCurrentMatchingIngredients;
      // Reset the ingredients copy
      memcpy(auIngredientsCopy, m_auIngredients, sizeof(uint8) * sm_uMaxIngredients);

      for (uint32 k = 0; k < pPotionsRecipes[i].size(); ++k)
      {
        if (auIngredientsCopy[j] != 0 && auIngredientsCopy[j] == pPotionsRecipes[i][k])
        {
          ++uCurrentMatchingIngredients;
          auIngredientsCopy[j] = 0;
          break;
        }
      }

      if (uPrevMatching == uCurrentMatchingIngredients)
      {
        // There is a current ingredient that doesn't belong to the current potion, check next
        break;
      }
    }
#endif

    // For each recipe
    if (uCurrentMatchingIngredients == pPotionsRecipes[i].size())
    {
      printf("Matching ingredients for potion [%u]\n", i);
      // Need to check if there are remaining ingredients that doesn't belong to the current potion
      //  We allow to have more ingredients than necessary but they need to belong to the recipe
      bool bRecipeMatched = true;
      for (uint32 j = uCurrentMatchingIngredients; j < m_uCurrentNumIngredients; ++j)
      {
        const uint8 uPrevMatching = uCurrentMatchingIngredients;
        // Reset the ingredients copy
        memcpy(auIngredientsCopy, m_auIngredients, sizeof(uint8) * sm_uMaxIngredients);

        for (uint32 k = 0; k < pPotionsRecipes[i].size(); ++k)
        {
          if (auIngredientsCopy[j] != 0 && auIngredientsCopy[j] == pPotionsRecipes[i][k])
          {
            ++uCurrentMatchingIngredients;
            auIngredientsCopy[j] = 0;
            break;
          }
        }

        if (uPrevMatching == uCurrentMatchingIngredients)
        {
          // There is a current ingredient that doesn't belong to the current potion, check next
          printf("  But unfortunately you put ingredients that didn't belong to the recipe. Failed\n");
          bRecipeMatched = false;
          break;
        }
      }

      if (bRecipeMatched)
      {
        return i;
      }
    }
  }

  return UINT8_MAX;
}

// ------------------------
