#include "game/gameclient.h"

#include <random>
#include <chrono>

#include "raylib/raylib.h"
#include "game/network_protocol.h"
#include "game/network_client.h"
#include "game/gameserver.h"
#include "ui/ui.h"

// ------------------------------------------------
// ------------------------------------------------

CClientBall::CClientBall()
{

}

// ------------------------

CClientBall::~CClientBall()
{
  
}

// ------------------------

void CClientBall::Init(bool _bLoadTextures /*= true*/)
{
  assert(!m_spTexture);

  if (_bLoadTextures && !m_spTexture)
  {
    // Image img = LoadImage("assets/ball_basic.png");
    // ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    // Texture2D ballTex = LoadTextureFromImage(img);
    // m_spTexture = std::make_shared<Texture2D>(ballTex);
    m_spTexture = std::make_shared<Texture2D>(LoadTexture("assets/ball_basic.png"));
    printf("Client ball creation w:%u h:%d\n", m_spTexture->width, m_spTexture->height);
  }
}

// ------------------------

void CClientBall::UnloadResources()
{
  if (m_spTexture.get())
  {
    UnloadTexture(*m_spTexture.get());
  }
}

// ------------------------

void CClientBall::Draw()
{
  if (m_spTexture)
  {
    DrawTextureV(*m_spTexture, Vector2{ m_v2Pos.x - m_spTexture->width * 0.5f,
      m_v2Pos.y - m_spTexture->height * 0.5f },
      WHITE);
  }
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

CGameClient::CGameClient(void)
  : m_oPlayerPaddle(this, m_bRightHanded ? sm_v2RightPlayerPos : sm_v2LeftPlayerPos , true)
  , m_oEnemyPaddle (this, m_bRightHanded ? sm_v2LeftPlayerPos  : sm_v2RightPlayerPos, true)
{
  for (uint32 i = 0; i < 1; ++i)
  {
    m_vctBalls.push_back(CClientBall());
  }

  m_oGameState.m_uNumBalls = 1;
  m_oGameState.m_vctBallsPos.push_back(CVector2D::v2ZERO);
  m_oGameState.m_vctBallsVel.push_back(CVector2D::v2ZERO);

  m_pfncLoop = &CGameClient::LoopConnecting;
}

// ------------------------

CGameClient::~CGameClient()
{
  for (uint32 i = 0; i < 1; ++i)
  {
    m_vctBalls[i].UnloadResources();
  }

  UnloadTexture(m_oConnectionTexture);

  if (m_bWantClose)
  {
    CloseWindow();
  }
}

// ------------------------

void CGameClient::Init(bool _bLocalGame /*= false*/)
{
  // Window creation
  //SetConfigFlags(FLAG_VSYNC_HINT);

  m_bLocalGame = _bLocalGame;

  if (_bLocalGame == false)
  {
    InitWindow(IGame::sm_uWindowWidth, IGame::sm_uWindowHeight, "pong-dx");
    SetTargetFPS(60);
    m_oConnectionTexture = LoadTexture("assets/connection_icon.png");
  }

  // For texture loading, raylib must be initialized
  m_oPlayerPaddle.Init(!_bLocalGame);
  m_oPlayerPaddle.m_aColor[0] = 0.3f;
  m_oPlayerPaddle.m_aColor[1] = 0.3f;
  m_oPlayerPaddle.m_aColor[2] = 1.0f;

  m_oEnemyPaddle.Init(!_bLocalGame);
  m_oEnemyPaddle.m_aColor[0] = 1.0f;
  m_oEnemyPaddle.m_aColor[1] = 0.3f;
  m_oEnemyPaddle.m_aColor[2] = 0.3f;

  for (uint32 i = 0; i < 1; ++i)
  {
    m_vctBalls[i].Init(!_bLocalGame);
  }

  if (!m_bRightHanded)
  {
    // Put player paddle on the left
    m_oPlayerPaddle.m_v2Pos = sm_v2LeftPlayerPos;
    m_oEnemyPaddle.m_v2Pos  = sm_v2RightPlayerPos;
  }
}

// ------------------------

void CGameClient::End()
{
  m_uClientId = UINT8_MAX;
  m_bGameStarted = false;
  m_bGameBegun = false;
  m_oPlayerPaddle.SetPlayerControlled(true);
  m_oEnemyPaddle.SetPlayerControlled(true);
  m_pfncLoop = &CGameClient::LoopMainMenu;
}

// ------------------------

void CGameClient::Begin(CNetworkClient* _pNetworkClient, uint8 _uPlayerId)
{
  m_pNetworkClient = _pNetworkClient;
  m_uClientId = _uPlayerId;
  assert(m_uClientId != UINT8_MAX);

  // HACK_TEMP
#if 1
  if (m_uClientId == 0u)
  {
    m_oPlayerPaddle.SetPlayerControlled(true);
    SetRightHanded(true);
  }
  if (m_uClientId == 1u)
  {
    m_oPlayerPaddle.SetPlayerControlled(false);
    SetRightHanded(false);
  }
#endif
  // HACK_TEMP

  if (IsWindowReady())
  {
    char sBuffer[16] = { '\0' };
    sprintf(sBuffer, "Player%u", m_uClientId);
    SetWindowTitle(sBuffer);
  }

  m_bGameStarted = true;
  m_pfncLoop = &CGameClient::LoopGame;
}

// ------------------------

void CGameClient::Pause()
{
  m_pfncLoop = &CGameClient::LoopPause;
}

// ------------------------

void CGameClient::Resume()
{
  m_pfncLoop = &CGameClient::LoopGame;
}

// ------------------------

void CGameClient::Loop()
{
  //m_bWantClose = WindowShouldClose();
  m_bWantClose = IsKeyPressed(KEY_ESCAPE);

  const bool bWindowReady = IsWindowReady() && m_bLocalGame == false;
  if (bWindowReady)
  {
    BeginDrawing();
    ClearBackground(BLACK);
  }

  (this->*m_pfncLoop)();

  if (bWindowReady)
  {
    if (m_pNetworkClient)
    {
      const float fPing = m_pNetworkClient->GetNetworkTime();
      const CVector2D v2Pos{ IGame::sm_uWindowWidth * 0.01f, IGame::sm_uWindowHeight * 0.01f };
      const float fScaleFactor = 0.05f;
      const CVector2D v2Scale{ m_oConnectionTexture.width * fScaleFactor, m_oConnectionTexture.height * fScaleFactor };
      DrawTextureEx(m_oConnectionTexture,
        Vector2{ v2Pos.x, v2Pos.y },
        0.0f, fScaleFactor, fPing < 60.0f ? GREEN : (fPing < 130.0f ? ORANGE : RED)
      );
      char sBuffer[8] = { '\0' };
      sprintf(sBuffer, "%u ms", (uint32)fPing);
      DrawText(sBuffer, 
        v2Pos.x + v2Scale.x * 1.1f, 
        v2Pos.y + v2Scale.y * 0.6f, 
        GetFontDefault().baseSize, WHITE);
    }

    EndDrawing();
  }
}

// ------------------------

void CGameClient::OnGameStateReceived(SNetStream* _pStream)
{
  m_oGameState.Read(_pStream);

  if (m_oGameState.m_uNumBalls != m_vctBalls.size())
  {
    assert(false);
    // TODO: need to create a new ball (??)
  }

  m_oGameState.m_oTimeReceived = std::chrono::high_resolution_clock::now();

  if (m_bGameBegun == true)
  {
    m_aGameStatesBuffer[m_uGameStatesBufferIndex] = m_oGameState;
    m_uGameStatesBufferIndex = (m_uGameStatesBufferIndex + 1) & (sm_uGameStatesBufferSize - 1);
  }

#if 0 // Now this is done in UpdateBalls() using the GameState provided in UpdateGameState()
  for (uint32 i = 0; i < m_vctBalls.size(); ++i)
  {
    m_vctBalls[i].m_v2Pos       = m_oGameState.m_vctBallsPos[i];
    m_vctBalls[i].m_v2Velocity  = m_oGameState.m_vctBallsVel[i];
  }

  if (m_bRightHanded == true && m_uClientId == 1u)
  {
    for (uint32 i = 0; i < m_vctBalls.size(); ++i)
    {
      m_vctBalls[i].m_v2Pos.x       = IGame::sm_uWindowWidth - m_oGameState.m_vctBallsPos[i].x;
      m_vctBalls[i].m_v2Velocity.x *= -1.0f;
    }
  }
#else
  // Only update velocity (??)
  for (uint32 i = 0; i < m_vctBalls.size(); ++i)
  {
    m_vctBalls[i].m_v2Velocity = m_oGameState.m_vctBallsVel[i];
  }

  if (m_bRightHanded == true && m_uClientId == 1u)
  {
    for (uint32 i = 0; i < m_vctBalls.size(); ++i)
    {
      m_vctBalls[i].m_v2Velocity.x *= -1.0f;
    }
  }
#endif
  m_oPlayerPaddle.m_v2Pos.y = m_oGameState.m_av2PlayersPos[m_uClientId].y;
  m_oEnemyPaddle.m_v2Pos.y  = m_oGameState.m_av2PlayersPos[(m_uClientId + 1) & 1].y;

  //printf("Ball pos: %.3f,%.3f\n", m_vctBalls[0].m_v2Pos.x, m_vctBalls[0].m_v2Pos.y);

  //printf("GameStateFrame: %llu | PacketFrame: %llu\n", m_uLogicTick, m_oGameState.m_uFrame);
}

// ------------------------

void CGameClient::ProcessInput()
{
  // Read input for current player
  m_oPlayerPaddle.Update(0.016f);

  // Do predictions based on received info and read info

  // Update enemy paddle
  // m_oEnemyPaddle.m_v2Pos = ServerEnemyPaddlePos();
  //  NOTE: right now is moved by AI, so just update here.
  //    Next step is to receive fake info from the server
  //    which will do de AI update of the paddle.

  // HACK
  if (m_oEnemyPaddle.GetPlayerControlled() == false)
  {
    float fY = m_oEnemyPaddle.m_v2Pos.y;
    m_oEnemyPaddle.Update(0.016f);
    m_oEnemyPaddle.m_v2Pos.y = fY;
  }
  // HACK

  if (IsKeyReleased(KEY_P))
  {
    // Pause request
    SNetStream oStream;
    byte pBuffer[NET_MAX_PACKET_SIZE];
    memset(pBuffer, 0, NET_MAX_PACKET_SIZE);
    InitNetStream(&oStream, pBuffer, NET_MAX_PACKET_SIZE);

    SPacketHeader oHeader;
    WriteHeader(&oStream, EMsgType::PAUSE_REQUEST);

    WriteUint8(&oStream, m_uClientId);

    m_pNetworkClient->Send(&oStream, EMsgPriority::HIGH);
  }
}

// ------------------------

void CGameClient::SendInput()
{
  SNetStream oStream;
  byte pBuffer[NET_MAX_PACKET_SIZE];
  memset(pBuffer, 0, NET_MAX_PACKET_SIZE);
  InitNetStream(&oStream, pBuffer, NET_MAX_PACKET_SIZE);

  SPacketHeader oHeader;
  WriteHeader(&oStream, EMsgType::PLAYER_INPUT);

  WriteUint8(&oStream, m_uClientId);
  WriteFloat32(&oStream, m_oPlayerPaddle.m_v2Pos.x);
  WriteFloat32(&oStream, m_oPlayerPaddle.m_v2Pos.y);

  m_pNetworkClient->Send(&oStream, EMsgPriority::LOW);
}

// ------------------------

void CGameClient::Update(float _fDt)
{
  // TODO: send info to server?

  UpdateBalls(_fDt);

  //UpdateGameState();

  ++m_uLogicTick;
}

// ------------------------

void CGameClient::Draw()
{
  m_oPlayerPaddle.Draw();
  m_oEnemyPaddle.Draw();
  for (uint32 i = 0; i < m_vctBalls.size(); ++i)
  {
    m_vctBalls[i].Draw();
  }
}

// ------------------------

bool CGameClient::WantClose()
{
  return m_bWantClose;
}

// ------------------------

void CGameClient::SetRightHanded(bool _bState)
{
  m_bRightHanded = _bState;
  if (m_bRightHanded)
  {
    m_oPlayerPaddle.m_v2Pos.x = IGame::sm_v2RightPlayerPos.x;
    m_oEnemyPaddle.m_v2Pos.x  = IGame::sm_v2LeftPlayerPos.x;
  }
  else
  {
    m_oPlayerPaddle.m_v2Pos.x = IGame::sm_v2LeftPlayerPos.x;
    m_oEnemyPaddle.m_v2Pos.x  = IGame::sm_v2RightPlayerPos.x;
  }
}

// ------------------------

void CGameClient::SetStartGameCurrentTime(float _fTime)
{
  m_fCountdownTimer = _fTime;

  if (_fTime <= 0.0f)
  {
    m_bGameBegun = true;
  }
}

// ------------------------

void CGameClient::UpdateBalls(float _fDt)
{
  /*if (!m_oPrevGameState.IsValid() || !m_oGameState.IsValid())
  {
    return;
  }*/
#define USE_INTERPOLATION 1
#if USE_INTERPOLATION
  if (!m_oGameState.IsValid())
  {
    return;
  }

  //const uint32 uPendingStates = (m_uGameStatesBufferIndex + sm_uGameStatesBufferSize - m_uLastProcessedGameStateIndex) & (sm_uGameStatesBufferSize - 1);
  const uint32 uPendingStates =
    (m_uGameStatesBufferIndex - m_uLastProcessedGameStateIndex)
    & (sm_uGameStatesBufferSize - 1);

  if (m_uClientId == 0)
  {
  printf(
    "Write: %u Processed: %u Pending: %u DeltaTime: %.5f\n",
    m_uGameStatesBufferIndex,
    m_uLastProcessedGameStateIndex,
    uPendingStates, _fDt);

#if 0
  if (uPendingStates > 2)
  {
    m_uLastProcessedGameStateIndex =
      (m_uGameStatesBufferIndex - 3) &
      (sm_uGameStatesBufferSize - 1);
  }
#endif

  if (uPendingStates > 1)
  {
    if (m_uClientId == 0)
    {
      //printf("Client tick: %llu\n", m_aGameStatesBuffer[m_uLastProcessedGameStateIndex].m_uFrame);
      //printf("  Ball x pos: %.3f\n", m_vctBalls[0].m_v2Pos.x);
    }

    // Can process game state
    // 
    //const float fTimeBetweenStates = std::chrono::duration<float>(m_aGameStatesBuffer[uNextIndex].m_oTimeReceived.time_since_epoch()
    // - m_aGameStatesBuffer[m_uLastProcessedGameStateIndex].m_oTimeReceived.time_since_epoch()).count();
    const float fTimeBetweenStates = 0.0166666f;

    m_fDeltaTimeBetweenGameStates += _fDt;

    while (m_fDeltaTimeBetweenGameStates >= fTimeBetweenStates)
    {
      m_fDeltaTimeBetweenGameStates -= fTimeBetweenStates;
      m_uLastProcessedGameStateIndex = (m_uLastProcessedGameStateIndex + 1) & (sm_uGameStatesBufferSize - 1);
      // Ensure we have 1 more game state to consume at least
      //assert(std::abs((int32)m_uGameStatesBufferIndex - (int32)m_uLastProcessedGameStateIndex) > 1);
    }

    const float fAlpha = m_fDeltaTimeBetweenGameStates / fTimeBetweenStates; // 0.0166666f
    const uint32 uNextIndex = (m_uLastProcessedGameStateIndex + 1) & (sm_uGameStatesBufferSize - 1);

    for (uint32 i = 0; i < m_vctBalls.size(); ++i)
    {
      m_vctBalls[i].m_v2Pos = HermiteInterpolation(
        m_aGameStatesBuffer[m_uLastProcessedGameStateIndex].m_vctBallsPos[i],
        m_aGameStatesBuffer[m_uLastProcessedGameStateIndex].m_vctBallsVel[i],
        m_aGameStatesBuffer[uNextIndex].m_vctBallsPos[i],
        m_aGameStatesBuffer[uNextIndex].m_vctBallsVel[i],
        fAlpha,
        fTimeBetweenStates
      );
    }

    if (m_bRightHanded == true && m_uClientId == 1u)
    {
      for (uint32 i = 0; i < m_vctBalls.size(); ++i)
      {
        m_vctBalls[i].m_v2Pos.x = IGame::sm_uWindowWidth - m_vctBalls[i].m_v2Pos.x;
      }
    }
  }
  }
#else
  for (uint32 i = 0; i < m_vctBalls.size(); ++i)
  {
    m_vctBalls[i].m_v2Pos = m_oGameState.m_vctBallsPos[i];
  }

  if (m_bRightHanded == true && m_uClientId == 1u)
  {
    for (uint32 i = 0; i < m_vctBalls.size(); ++i)
    {
      m_vctBalls[i].m_v2Pos.x = IGame::sm_uWindowWidth - m_oGameState.m_vctBallsPos[i].x;
    }
  }
#endif
}

// ------------------------

void CGameClient::UpdateGameState()
{
  m_oPrevGameState = m_oGameState;

  // If the writting index is equal to the processed index, we have received more states than processed ones.
  //assert((m_uGameStatesBufferIndex & (sm_uGameStatesBufferSize - 1)) != m_uLastProcessedGameStateIndex);
#if 0
  m_aGameStatesBuffer[m_uGameStatesBufferIndex] = m_oGameState;
  m_uGameStatesBufferIndex = (m_uGameStatesBufferIndex + 1) & (sm_uGameStatesBufferSize - 1);
#endif
}

// ------------------------

void CGameClient::LoopMainMenu()
{
  Rectangle oBounds;
  oBounds.width = IGame::sm_uWindowWidth * 0.2f;
  oBounds.height = IGame::sm_uWindowHeight * 0.7f;
  oBounds.x = IGame::sm_uWindowWidth * 0.5f - oBounds.width * 0.5f;
  oBounds.y = IGame::sm_uWindowHeight * 0.5f - oBounds.height * 0.5f;
  DrawRectangleRounded(oBounds, 0.05f, 8, Color{ 180u, 180u, 180u, 255u });

  CVector2D v2ButtonSize = CButton::CalculateSize("CONNECT");
  const CVector2D v2ButtonPos{ oBounds.x + oBounds.width * 0.5f,
    oBounds.y + oBounds.height * 0.5f };

  if (CButton("CONNECT",
        CVector2D{  v2ButtonPos.x - v2ButtonSize.x * 0.5f,
                    v2ButtonPos.y - v2ButtonSize.y * 0.5f }
    )
  )
  {
    // Send a request to connect to a game
    SNetStream oStream;
    byte pBuffer[NET_MAX_PACKET_SIZE];
    memset(pBuffer, 0, NET_MAX_PACKET_SIZE);
    InitNetStream(&oStream, pBuffer, NET_MAX_PACKET_SIZE);

    SPacketHeader oHeader;
    WriteHeader(&oStream, EMsgType::CONNECT_REQUEST);

    m_pNetworkClient->Send(&oStream, EMsgPriority::HIGH);

    m_pfncLoop = &CGameClient::LoopConnecting;
  }
}

// ------------------------

void CGameClient::LoopConnecting()
{
  if (IsWindowReady)
  {
    char sBuffer[16] = { "Connecting" };
    for (uint32 i = 0; i < (uint32)std::roundf(m_fConnectingTextCount); ++i)
    {
      strcat(sBuffer, ".");
    }
    m_fConnectingTextCount += m_fTargetFrameTime;
    m_fConnectingTextCount = std::fmodf(m_fConnectingTextCount, 3.0f + m_fTargetFrameTime);
    const uint32 uFontSize = GetFontDefault().baseSize;
    DrawText(sBuffer, 
      IGame::sm_uWindowWidth / 2u - strlen("Connecting") * uFontSize * 2.5f, 
      IGame::sm_uWindowHeight / 2u - uFontSize * 4, 
      uFontSize * 10, 
      WHITE);
  }
}

// ------------------------

void CGameClient::LoopGame()
{
  // TODO: to avoid checking against booleans every time (like m_bGameStarted),
  //  use different scenes, like WaitingRoom, Game, etc.

  if (m_bWantClose == false && m_bGameStarted == true)
  {
    // -------- INPUT --------
    ProcessInput();
    SendInput();

    // -------- UPDATE --------
    Update(GetFrameTime());

    // -------- DRAW --------
    // Game
    Draw();

    // UI
    if (m_bGameBegun == false)
    {
      char sBuffer[4] = { '\0' };
      sprintf(sBuffer, "%u", (uint32)std::roundf(m_fCountdownTimer));
      DrawText(sBuffer, IGame::sm_uWindowWidth / 2u, IGame::sm_uWindowHeight / 2u, 50, WHITE);
    }
  }
}

// ------------------------

void CGameClient::LoopPause()
{
  // Draw game to keep track of what's happening while on pause
  Draw();

  // UI
  Rectangle oBounds;
  oBounds.width = IGame::sm_uWindowWidth * 0.2f;
  oBounds.height = IGame::sm_uWindowHeight * 0.7f;
  oBounds.x = IGame::sm_uWindowWidth * 0.5f - oBounds.width * 0.5f;
  oBounds.y = IGame::sm_uWindowHeight * 0.5f - oBounds.height * 0.5f;
  DrawRectangleRounded(oBounds, 0.05f, 8, Color{ 180u, 180u, 180u, 255u });

  CVector2D v2ButtonSize = CButton::CalculateSize("RESUME");
  const CVector2D v2ButtonPos{oBounds.x + oBounds.width * 0.5f,
    oBounds.y + oBounds.height * 0.5f};

  if (CButton("RESUME", 
        CVector2D{ v2ButtonPos.x - v2ButtonSize.x * 0.5f, 
                   v2ButtonPos.y - v2ButtonSize.y * 0.5f }
    )
  )
  {
    // Pause end
    SNetStream oStream;
    byte pBuffer[NET_MAX_PACKET_SIZE];
    memset(pBuffer, 0, NET_MAX_PACKET_SIZE);
    InitNetStream(&oStream, pBuffer, NET_MAX_PACKET_SIZE);

    SPacketHeader oHeader;
    WriteHeader(&oStream, EMsgType::PAUSE_END);

    WriteUint8(&oStream, m_uClientId);

    m_pNetworkClient->Send(&oStream, EMsgPriority::HIGH);
  }

  if (CCheckbox("Right handed", &m_bRightHanded,
    CVector2D(v2ButtonPos.x - v2ButtonSize.x * 0.5f, v2ButtonPos.y * 1.1f)))
  {
    
  }

  v2ButtonSize = CButton::CalculateSize("QUIT");
  if (CButton("QUIT",
        CVector2D{  v2ButtonPos.x - v2ButtonSize.x * 0.5f,
                    (oBounds.y + oBounds.height) - v2ButtonSize.y * 2.0f }
    )
  )
  {
    SNetStream oStream;
    byte pBuffer[NET_MAX_PACKET_SIZE];
    memset(pBuffer, 0, NET_MAX_PACKET_SIZE);
    InitNetStream(&oStream, pBuffer, NET_MAX_PACKET_SIZE);

    SPacketHeader oHeader;
    WriteHeader(&oStream, EMsgType::END_REQUEST);

    WriteUint8(&oStream, m_uClientId);

    m_pNetworkClient->Send(&oStream, EMsgPriority::HIGH);

    // Here we don't wait for confirmation from the server
    End();
  }
}

// ------------------------
// ------------------------
// ------------------------