#include "game/game_manager.h"

#include <cassert>
#include <filesystem>
#include <fstream>

#include "game/game.h"
#include "game/game_scene.h"
#include "game/inventory_scene.h"

// ------------------------

CGameManager::CGameManager()
{

}

// ------------------------

CGameManager::~CGameManager()
{
  
}

// ------------------------

void CGameManager::Init()
{
  InitWindow(CGameManager::sm_uVirtualWindowWidth, CGameManager::sm_uVirtualWindowHeight, "alchemistra");
  SetTargetFPS(60);

  m_oRenderTarget = LoadRenderTexture(CGameManager::sm_uWindowWidth, CGameManager::sm_uWindowHeight);
  SetTextureFilter(m_oRenderTarget.texture, TEXTURE_FILTER_POINT);

  m_spTriangle  = std::make_shared<Texture2D>(LoadTexture("assets/triangle.png"));
  m_spSquare    = std::make_shared<Texture2D>(LoadTexture("assets/square.png"));
  m_spPentagon  = std::make_shared<Texture2D>(LoadTexture("assets/pentagon.png"));
  m_spCircle    = std::make_shared<Texture2D>(LoadTexture("assets/circle.png"));

  m_aoScenes[0] = new CGameScene();
  m_aoScenes[0]->Init("Game");

  m_aoScenes[1] = new CInventoryScene();
  m_aoScenes[1]->Init("Inventory");

  m_uCurrentScene = 1;

  m_auInventory[0]  = 10;
  m_auInventory[1]  = 5;
  m_auInventory[2]  = 3;
  m_auInventory[3]  = 0;
  m_auInventory[4]  = 8;
  m_auInventory[5]  = 5;
  m_auInventory[6]  = 7;
  m_auInventory[7]  = 1;
  m_auInventory[8]  = 9;
  m_auInventory[9]  = 3;
  m_auInventory[10] = 3;
  m_auInventory[11] = 1;
}

// ------------------------

void CGameManager::Destroy()
{
  // TODO: destroy scenes?

  if (m_aoScenes[0] != nullptr)
  {
    delete m_aoScenes[0];
  }
  if (m_aoScenes[1] != nullptr)
  {
    delete m_aoScenes[1];
  }

  if (m_bWantClose)
  {
    CloseWindow();
  }
}

// ------------------------

void CGameManager::Loop()
{
  m_bWantClose = IsKeyPressed(KEY_ESCAPE);
  
  m_aoScenes[m_uCurrentScene]->Update(GetFrameTime());

  Draw();
}

// ------------------------

void CGameManager::Draw()
{
  BeginTextureMode(m_oRenderTarget);
  ClearBackground(BLACK);
  // ----
  m_aoScenes[m_uCurrentScene]->Draw();
  // ----
  EndTextureMode();

  // --------------

  BeginDrawing();
  ClearBackground(BLACK);

  // Flip texture on Y axis (raylib draws render textures inverted by default)
  Rectangle oSourceRect = { 0.0f, 0.0f, (float)m_oRenderTarget.texture.width, -(float)m_oRenderTarget.texture.height };

  // DestRect size will be the total size of the current physical window
  Rectangle oDestRect = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };

  // Draw virtual canvas stretching perfectly in the big window
  DrawTexturePro(m_oRenderTarget.texture, oSourceRect, oDestRect, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);

  // --------------
  EndDrawing();
}

// ------------------------

std::shared_ptr<Texture2D> CGameManager::GetTextureByElement(uint8 _uType)
{
  switch (_uType)
  {
    case 1:
    case 5:
    case 9:
    case 13:
    {
      return m_spTriangle;
    }
    case 2:
    case 6:
    case 10:
    case 14:
    {
      return m_spSquare;
    }
    case 3:
    case 7:
    case 11:
    case 15:
    {
      return m_spPentagon;
    }
    case 4:
    case 8:
    case 12:
    case 16:
    {
      return m_spCircle;
    }
    default:
    {

    }
  }

  return nullptr;
}

// ------------------------

Color CGameManager::GetColorByElement(uint8 _uType)
{
  switch (_uType)
  {
    case 1:
    case 2:
    case 3:
    case 4:
    {
      return RED;
    }
    case 5:
    case 6:
    case 7:
    case 8:
    {
      return GREEN;
    }
    case 9:
    case 10:
    case 11:
    case 12:
    {
      return BLUE;
    }
    case 13:
    case 14:
    case 15:
    case 16:
    {
      return YELLOW;
    }
    default:
    {

    }
  }

  return PURPLE;
}

// ------------------------

// ------------------------
// ------------------------
// ------------------------