#include "game/game_manager.h"

#include <cassert>
#include <filesystem>
#include <fstream>

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

  m_spTriangle = std::make_shared<Texture2D>(LoadTexture("assets/triangle.png"));
  m_spSquare = std::make_shared<Texture2D>(LoadTexture("assets/square.png"));
  m_spPentagon = std::make_shared<Texture2D>(LoadTexture("assets/pentagon.png"));
  m_spCircle = std::make_shared<Texture2D>(LoadTexture("assets/circle.png"));


  m_oGame.Init();
}

// ------------------------

void CGameManager::Destroy()
{
  SaveBackup();

  if (m_bWantClose)
  {
    CloseWindow();
  }
}

// ------------------------

void CGameManager::Loop()
{
  m_bWantClose = IsKeyPressed(KEY_ESCAPE);

  m_oGame.Update(0.0166666f);
  
  Draw();
}

// ------------------------

void CGameManager::Draw()
{
  BeginTextureMode(m_oRenderTarget);
  ClearBackground(BLACK);
  // ----
  m_oGame.Draw();
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

void CGameManager::SaveGame(const std::string& _sPath)
{
  std::filesystem::path oPath(_sPath);
  std::ofstream oFile(oPath);
  if (oFile.is_open() == false)
  {
    assert(false);
    return;
  }

  oFile << (uint32)sm_uSaveDataVersion << "\n";
  oFile << (uint32)CGame::sm_uMaxVerticalElements << "\n";
  oFile << (uint32)CGame::sm_uMaxHorizontalElements << "\n";
  oFile << (uint32)m_oGame.m_vctPreviousStates.size() << "\n";
  for (uint32 i = 0; i < m_oGame.m_vctPreviousStates.size(); ++i)
  {
    oFile << (uint32)m_oGame.m_vctPreviousStates[i].m_uPendingElementsState << "\n";
    oFile << (uint32)m_oGame.m_vctPreviousStates[i].m_uPendingElement0 << "\n";
    oFile << (uint32)m_oGame.m_vctPreviousStates[i].m_uPendingElement1 << "\n";
    for (uint32 j = 0; j < CGame::sm_uMaxVerticalElements; ++j)
    {
      for (uint32 k = 0; k < CGame::sm_uMaxHorizontalElements; ++k)
      {
        oFile << (uint32)m_oGame.m_vctPreviousStates[i].m_auPlacedElements[j][k] << " ";
      }
      oFile << "\n";
    }
    oFile << "\n";
  }

  SaveCurrentState();
  oFile << (uint32)m_oGame.m_oCurrentGameState.m_uPendingElementsState << "\n";
  oFile << (uint32)m_oGame.m_oCurrentGameState.m_uPendingElement0 << "\n";
  oFile << (uint32)m_oGame.m_oCurrentGameState.m_uPendingElement1 << "\n";
  for (uint32 j = 0; j < CGame::sm_uMaxVerticalElements; ++j)
  {
    for (uint32 k = 0; k < CGame::sm_uMaxHorizontalElements; ++k)
    {
      oFile << (uint32)m_oGame.m_oCurrentGameState.m_auPlacedElements[j][k] << " ";
    }
    oFile << "\n";
  }

  printf("File saved on [%s]\n", _sPath.c_str());
}

// ------------------------

void CGameManager::SaveCurrentState()
{
  m_oGame.m_oCurrentGameState.m_uPendingElementsState = (uint8)m_oGame.m_ePendingElementsState;
  m_oGame.m_oCurrentGameState.m_uPendingElement0 = m_oGame.m_oPendingElements.first;
  m_oGame.m_oCurrentGameState.m_uPendingElement1 = m_oGame.m_oPendingElements.second;
  memcpy(m_oGame.m_oCurrentGameState.m_auPlacedElements, m_oGame.m_auPlacedElements, sizeof(uint8) * CGame::sm_uMaxHorizontalElements * CGame::sm_uMaxVerticalElements);
}

// ------------------------

void CGameManager::LoadGame(const std::string& _sPath)
{
  std::filesystem::path oPath(_sPath);
  std::ifstream oFile(oPath);
  if (oFile.is_open() == false)
  {
    assert(false);
    return;
  }

  uint32 uSaveDataVersion = UINT32_MAX;
  uint32 uMaxVerticalElements = UINT32_MAX;
  uint32 uMaxHorizontalElements = UINT32_MAX;
  uint32 uPreviousStatesSize = UINT32_MAX;
  oFile >> uSaveDataVersion;        assert(uSaveDataVersion == sm_uSaveDataVersion);
  oFile >> uMaxVerticalElements;    assert(uMaxVerticalElements == CGame::sm_uMaxVerticalElements);
  oFile >> uMaxHorizontalElements;  assert(uMaxHorizontalElements == CGame::sm_uMaxHorizontalElements);
  oFile >> uPreviousStatesSize;
  m_oGame.m_vctPreviousStates.resize(uPreviousStatesSize);
  for (uint32 i = 0; i < m_oGame.m_vctPreviousStates.size(); ++i)
  {
    uint32 uTmp = UINT32_MAX;
    oFile >> uTmp;
    m_oGame.m_vctPreviousStates[i].m_uPendingElementsState = (uint8)uTmp;
    oFile >> uTmp;
    m_oGame.m_vctPreviousStates[i].m_uPendingElement0 = (uint8)uTmp;
    oFile >> uTmp;
    m_oGame.m_vctPreviousStates[i].m_uPendingElement1 = (uint8)uTmp;
    for (uint32 j = 0; j < CGame::sm_uMaxVerticalElements; ++j)
    {
      for (uint32 k = 0; k < CGame::sm_uMaxHorizontalElements; ++k)
      {
        oFile >> uTmp;
        m_oGame.m_vctPreviousStates[i].m_auPlacedElements[j][k] = (uint8)uTmp;
      }
    }
  }

  uint32 uTmp = UINT32_MAX;
  oFile >> uTmp;
  m_oGame.m_ePendingElementsState = (CGame::EPendingElementsState)uTmp;
  oFile >> uTmp;
  m_oGame.m_oPendingElements.first = (uint8)uTmp;
  oFile >> uTmp;
  m_oGame.m_oPendingElements.second = (uint8)uTmp;
  for (uint32 j = 0; j < CGame::sm_uMaxVerticalElements; ++j)
  {
    for (uint32 k = 0; k < CGame::sm_uMaxHorizontalElements; ++k)
    {
      oFile >> uTmp;
      m_oGame.m_auPlacedElements[j][k] = (uint8)uTmp;
    }
  }
}

// ------------------------

void CGameManager::SaveBackup()
{
  uint8 uCurrentBackup = 0;
  char sBuffer[128] = { '\0' };
  for (; uCurrentBackup < sm_uMaxBackups; ++uCurrentBackup)
  {
    sprintf(sBuffer, "%sbackup_%u.txt", sm_sBackupsPath, uCurrentBackup);
    if (std::filesystem::exists(sBuffer) == false)
    {
      break;
    }
  }

  if (uCurrentBackup < sm_uMaxBackups)
  {
    SaveGame(sBuffer);
  }
  else
  {
    // Need to overwrite the oldest one
    std::filesystem::file_time_type oCurrentTime = std::filesystem::file_time_type::clock::now();
    std::filesystem::file_time_type oOldestTime = oCurrentTime;
    uCurrentBackup = 0;
    uint8 uTargetBackup = 0;
    for (; uCurrentBackup < sm_uMaxBackups; ++uCurrentBackup)
    {
      sprintf(sBuffer, "%sbackup_%u.txt", sm_sBackupsPath, uCurrentBackup);
      std::filesystem::file_time_type oTime = std::filesystem::last_write_time(sBuffer);
      if (oTime < oOldestTime)
      {
        oOldestTime = oTime;
        uTargetBackup = uCurrentBackup;
      }
    }

    sprintf(sBuffer, "%sbackup_%u.txt", sm_sBackupsPath, uTargetBackup);
    SaveGame(sBuffer);
  }
}

// ------------------------
// ------------------------
// ------------------------