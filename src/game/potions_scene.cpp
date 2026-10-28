#include "game/potions_scene.h"

#include "game/game_manager.h"
#include "game/game.h"

// ------------------------

CPotionsScene::CPotionsScene()
{
  constexpr float fGridCellSizeScale = 2.0f;
  m_fCellSize     = CGame::sm_uElementPixelSize * fGridCellSizeScale;
  m_fGridWidth    = CGame::sm_uNumElementsPerTier * m_fCellSize;
  m_fGridHeight   = CGame::sm_uNumGroupsElements * m_fCellSize;
  m_fGridX        = (CGameManager::sm_uWindowWidth - m_fGridWidth) * 0.5f;
  m_fGridY        = (CGameManager::sm_uWindowHeight - m_fGridHeight) * 0.3f;
  m_fPadding      = (m_fCellSize - CGame::sm_uElementPixelSize) * 0.25f;
  m_oCauldronRect = { m_fGridX, m_fGridY + m_fGridHeight * 1.5f, m_fGridWidth, CGame::sm_uElementPixelSize * 3 };

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
    if (v2MousePos.x >= m_oCauldronRect.x && v2MousePos.x <= m_oCauldronRect.x + m_oCauldronRect.width &&
      v2MousePos.y >= m_oCauldronRect.y && v2MousePos.y <= m_oCauldronRect.y + m_oCauldronRect.height)
    {
      m_auIngredients[m_uCurrentNumIngredients++] = m_uDraggedElement;
    }

    m_bIsDragging = false;
    m_uDraggedElement = 0;
    
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
      const float fElementX = m_fGridX + j * m_fCellSize + m_fPadding + 1;
      const float fElementY = m_fGridY + i * m_fCellSize + m_fPadding;
      if (m_bIsDragging == false && bIsLeftMouseButtonDown &&
        v2MousePos.x >= fElementX && v2MousePos.x <= fElementX + CGame::sm_uElementPixelSize &&
        v2MousePos.y >= fElementY && v2MousePos.y <= fElementY + CGame::sm_uElementPixelSize)
      {
        m_bIsDragging = true;
        m_uDraggedElement = j + i * CGame::sm_uNumElementsPerTier + 1;
        return;
      }
    }
  }
}

// ------------------------

void CPotionsScene::Draw()
{
  const CVector2D v2MousePos = CGameManager::GetInstance().GetMousePos();

  DrawText("POTIONS", CGameManager::sm_uWindowWidth * 0.5f - (GetFontDefault().baseSize * strlen("POTIONS")) * 0.5f,
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
    }
  }

  if (m_bIsDragging && m_uDraggedElement > 0)
  {
    DrawTexture(*CGameManager::GetInstance().GetTextureByElement(m_uDraggedElement).get(), 
      v2MousePos.x - CGame::sm_uElementPixelSize * 0.5f,
      v2MousePos.y - CGame::sm_uElementPixelSize * 0.5f,
      CGameManager::GetInstance().GetColorByElement(m_uDraggedElement));
  }

  // Cauldron
  DrawLine(m_oCauldronRect.x,                         m_oCauldronRect.y,                          m_oCauldronRect.x + m_oCauldronRect.width,  m_oCauldronRect.y,                          SKYBLUE);
  DrawLine(m_oCauldronRect.x,                         m_oCauldronRect.y + m_oCauldronRect.height, m_oCauldronRect.x + m_oCauldronRect.width,  m_oCauldronRect.y + m_oCauldronRect.height, SKYBLUE);
  DrawLine(m_oCauldronRect.x,                         m_oCauldronRect.y,                          m_oCauldronRect.x,                          m_oCauldronRect.y + m_oCauldronRect.height, SKYBLUE);
  DrawLine(m_oCauldronRect.x + m_oCauldronRect.width, m_oCauldronRect.y,                          m_oCauldronRect.x + m_oCauldronRect.width,  m_oCauldronRect.y + m_oCauldronRect.height, SKYBLUE);

  for (uint32 i = 0; i < sm_uMaxIngredients; ++i)
  {
    if (m_auIngredients[i] != 0)
    {
      DrawTexture(*CGameManager::GetInstance().GetTextureByElement(m_auIngredients[i]).get(),
        m_oCauldronRect.x + i * CGame::sm_uElementPixelSize,
        m_oCauldronRect.y,
        CGameManager::GetInstance().GetColorByElement(m_auIngredients[i]));
    }
  }
}                               

// ------------------------

bool CPotionsScene::CanBeChanged() const
{
  return true;
}

// ------------------------
