#include "game/inventory_scene.h"

#include "game/game_manager.h"
#include "game/game.h"

// ------------------------

CInventoryScene::CInventoryScene()
{

}

// ------------------------

CInventoryScene::~CInventoryScene()
{

}

// ------------------------

void CInventoryScene::Init(const std::string& _sName)
{
  CScene::Init(_sName);
}

// ------------------------

void CInventoryScene::Update(float _fDt)
{

}

// ------------------------

void CInventoryScene::Draw()
{
  DrawText("Inventory", CGameManager::sm_uWindowWidth * 0.5f - (GetFontDefault().baseSize * strlen("Inventory")) * 0.5f, 
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

  const float fPadding = (fCellSize - CGame::sm_uElementPixelSize) * 0.25f;
  const uint32* pInventory = CGameManager::GetInstance().GetInventory();
  for (uint32 i = 0; i < CGame::sm_uNumGroupsElements; ++i)
  {
    for (uint32 j = 0; j < CGame::sm_uNumElementsPerTier; ++j)
    {
      DrawTexture(*CGameManager::GetInstance().GetTextureByElement(j + i * CGame::sm_uNumElementsPerTier + 1).get(), 
        fGridX + j * fCellSize + fPadding + 1, 
        fGridY + i * fCellSize + fPadding, 
        CGameManager::GetInstance().GetColorByElement(j + i * CGame::sm_uNumElementsPerTier + 1));

      char sBuffer[8] = { '\0' };
      sprintf(sBuffer, "x%u", pInventory[j + i * CGame::sm_uNumElementsPerTier]);

      DrawText(sBuffer, 
        fGridX + j * fCellSize + fPadding + CGame::sm_uElementPixelSize * 1.1f, 
        fGridY + i * fCellSize + fPadding + CGame::sm_uElementPixelSize * 1.2f, 
        GetFontDefault().baseSize, 
        WHITE);
    }
  }
}

// ------------------------
