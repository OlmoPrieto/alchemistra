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
    CGameManager::sm_uWindowHeight * 0.2f, GetFontDefault().baseSize * 2, WHITE);

  constexpr float fGridLineThickness = 1.0f;
  constexpr float fGridCellSizeScale = 1.1f;
  const float fCellSize = CGame::sm_uElementPixelSize * fGridCellSizeScale;
  const float fGridWidth = CGame::sm_uMaxHorizontalElements * fCellSize;
  const float fGridHeight = CGame::sm_uMaxVerticalElements * fCellSize;
  const float fGridX = (CGameManager::sm_uWindowWidth - fGridWidth) * 0.5f;
  const float fGridY = (CGameManager::sm_uWindowHeight - fGridHeight) * 0.5f;

  std::shared_ptr<Texture2D> spTriangle = CGameManager::GetInstance().GetTextureByElement(1);

  for (uint32 uColumn = 0; uColumn <= CGame::sm_uMaxHorizontalElements; ++uColumn)
  {
    const float fX = fGridX + uColumn * fCellSize;
    DrawRectangleRec({ fX - fGridLineThickness * 0.5f, fGridY, fGridLineThickness, fGridHeight }, WHITE);
  }

  for (uint32 uRow = 0; uRow <= CGame::sm_uMaxVerticalElements; ++uRow)
  {
    const float fY = fGridY + uRow * fCellSize;
    DrawRectangleRec({ fGridX, fY - fGridLineThickness * 0.5f, fGridWidth, fGridLineThickness }, WHITE);
  }

  const float fPadding = (fCellSize - spTriangle->width) * 0.5f;
  for (uint32 i = 0; i < CGame::sm_uNumGroupsElements; ++i)
  {
    for (uint32 j = 0; j < CGame::sm_uNumElementsPerTier; ++j)
    {
      if (i == 0 && j == 0)
      {
        DrawTexture(*spTriangle.get(), fGridX + fPadding + 1, fGridY + fPadding, WHITE);
      }
    }
  }
}

// ------------------------
