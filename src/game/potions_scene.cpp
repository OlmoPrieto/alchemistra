#include "game/potions_scene.h"

#include "game/game_manager.h"
#include "game/game.h"

// ------------------------

CPotionsScene::CPotionsScene()
{

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

}

// ------------------------

void CPotionsScene::Draw()
{
  DrawText("Potions", CGameManager::sm_uWindowWidth * 0.5f - (GetFontDefault().baseSize * strlen("Inventory")) * 0.5f,
    CGameManager::sm_uWindowHeight * 0.1f, GetFontDefault().baseSize * 2, WHITE);

  constexpr float fGridLineThickness = 1.0f;
  constexpr float fGridCellSizeScale = 2.0f;
  const float fCellSize = CGame::sm_uElementPixelSize * fGridCellSizeScale;
  const float fGridWidth = CGame::sm_uNumElementsPerTier * fCellSize;
  const float fGridHeight = CGame::sm_uNumGroupsElements * fCellSize;
  const float fGridX = (CGameManager::sm_uWindowWidth - fGridWidth) * 0.5f;
  const float fGridY = (CGameManager::sm_uWindowHeight - fGridHeight) * 0.3f;

  for (uint32 uColumn = 0; uColumn <= CGame::sm_uNumElementsPerTier; ++uColumn)
  {
    const float fX = fGridX + uColumn * fCellSize;
    DrawRectangleRec({ fX - fGridLineThickness * 0.5f, fGridY, fGridLineThickness, fGridHeight }, WHITE);
  }

  for (uint32 uRow = 0; uRow <= CGame::sm_uNumGroupsElements; ++uRow)
  {
    const float fY = fGridY + uRow * fCellSize;
    DrawRectangleRec({ fGridX, fY - fGridLineThickness * 0.5f, fGridWidth, fGridLineThickness }, WHITE);
  }

  const CVector2D v2MousePos = CGameManager::GetInstance().GetMousePos();
  const float fPadding = (fCellSize - CGame::sm_uElementPixelSize) * 0.25f;
  const uint32* pInventory = CGameManager::GetInstance().GetInventory();
  for (uint32 i = 0; i < CGame::sm_uNumGroupsElements; ++i)
  {
    for (uint32 j = 0; j < CGame::sm_uNumElementsPerTier; ++j)
    {
      const float fElementX = fGridX + j * fCellSize + fPadding + 1;
      const float fElementY = fGridY + i * fCellSize + fPadding;
      DrawTexture(*CGameManager::GetInstance().GetTextureByElement(j + i * CGame::sm_uNumElementsPerTier + 1).get(),
        fElementX,
        fElementY,
        CGameManager::GetInstance().GetColorByElement(j + i * CGame::sm_uNumElementsPerTier + 1));

      if (m_bIsDragging == false && IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        v2MousePos.x >= fElementX && v2MousePos.x <= fElementX + CGame::sm_uElementPixelSize &&
        v2MousePos.y >= fElementY && v2MousePos.y <= fElementY + CGame::sm_uElementPixelSize)
      {
        m_bIsDragging = true;
        m_uDraggedElement = j + i * CGame::sm_uNumElementsPerTier + 1;
      }
      else if (m_bIsDragging == true && IsMouseButtonDown(MOUSE_BUTTON_LEFT) == false)
      {
        m_bIsDragging = false;
        m_uDraggedElement = 0;
      }

      char sBuffer[8] = { '\0' };
      sprintf(sBuffer, "x%u", pInventory[j + i * CGame::sm_uNumElementsPerTier]);

      DrawText(sBuffer,
        fGridX + j * fCellSize + fPadding + CGame::sm_uElementPixelSize * 1.1f, // Little offset in each coordinate
        fGridY + i * fCellSize + fPadding + CGame::sm_uElementPixelSize * 1.2f, // Little offset in each coordinate
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

  Rectangle oCauldronRect{fGridX, fGridY + fGridHeight * 1.5f, fGridWidth, fGridHeight * 0.5f };

  /*DrawLine(fGridX, fGridY + fGridHeight * 1.5f, fGridX + fGridWidth, fGridY + fGridHeight * 1.5f, SKYBLUE);
  DrawLine(fGridX, fGridY + fGridHeight * 1.5f + fGridHeight * 0.5f, fGridX + fGridWidth, fGridY + fGridHeight * 1.5f + fGridHeight * 0.5f, SKYBLUE);
  DrawLine(fGridX, fGridY + fGridHeight * 1.5f, fGridX, fGridY + fGridHeight * 1.5f + fGridHeight * 0.5f, SKYBLUE);
  DrawLine(fGridX + fGridWidth, fGridY + fGridHeight * 1.5f, fGridX + fGridWidth, fGridY + fGridHeight * 1.5f + fGridHeight * 0.5f, SKYBLUE);*/
  
  DrawLine(oCauldronRect.x,                      oCauldronRect.y,                         oCauldronRect.x + oCauldronRect.width,  oCauldronRect.y,                        SKYBLUE);
  DrawLine(oCauldronRect.x,                      oCauldronRect.y + oCauldronRect.height,  oCauldronRect.x + oCauldronRect.width,  oCauldronRect.y + oCauldronRect.height, SKYBLUE);
  DrawLine(oCauldronRect.x,                      oCauldronRect.y,                         oCauldronRect.x,                        oCauldronRect.y + oCauldronRect.height, SKYBLUE);
  DrawLine(oCauldronRect.x + oCauldronRect.width,oCauldronRect.y,                         oCauldronRect.x + oCauldronRect.width,  oCauldronRect.y + oCauldronRect.height, SKYBLUE);
}                               

// ------------------------

bool CPotionsScene::CanBeChanged() const
{
  return true;
}

// ------------------------
