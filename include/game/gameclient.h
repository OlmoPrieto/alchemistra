#pragma once

#include <vector>

#include "game/game.h"
#include "game/serverhandle.h"
#include "base/types.h"
#include "game/ball.h"
#include "game/client_paddle.h"

struct _ENetPeer;
typedef struct _ENetPeer ENetPeer;
class CNetworkClient;

class CClientBall
{
public:
  CClientBall();
  ~CClientBall();

  void Init(bool _bLoadTextures = true);
  void UnloadResources();
  void Draw();

  CVector2D m_v2Pos;
  CVector2D m_v2Velocity;

private:
  std::shared_ptr<Texture2D> m_spTexture;
};

class CGameClient : public IGame
{
public:
  CGameClient(void);
  ~CGameClient();
  CGameClient(const CGameClient& _oGame) = delete;
  CGameClient(CGameClient&& _oGame) = delete;
  CGameClient& operator =(const CGameClient& _oGame) = delete;
  CGameClient& operator =(CGameClient&& _oGame) = delete;

  void Init(bool _bLocalGame = false);  // Initialize state, create window
  void End();   // End game
  void Begin(CNetworkClient* _pNetworkClient, uint8 _uPlayerId); // Can start the match
  void Pause();
  void Resume();
  void Loop();
  void OnGameStateReceived(SNetStream* _pStream);
  void ProcessInput();
  void SendInput();
  void Update(float _fDt);
  void Draw();
  bool WantClose();
  bool GetGameStarted() const { return m_bGameStarted; }
  void SetRightHanded(bool _bState);
  void SetStartGameCurrentTime(float _fTime);

  const std::vector<CClientBall>& GetBalls() const { return m_vctBalls; }

  //static constexpr uint32 sm_uWindowWidth = 1260u;
  //static constexpr uint32 sm_uWindowHeight = 580u;

  uint64 m_uLogicTick = 0u;
  constexpr static float m_fTargetFrameTime = 1.0f / 60.0f;

private:
  void UpdateBalls(float _fDt);
  void UpdateGameState();
  void LoopMainMenu();
  void LoopConnecting();
  void LoopGame();
  void LoopPause();

  friend class CNetworkClient;

  constexpr static uint32 sm_uGameStatesBufferSize = 128u; // NOTE: keep this always a power of 2

  SGameState m_aGameStatesBuffer[sm_uGameStatesBufferSize]; // TODO: should optimize this to not use that much memory
  Texture2D m_oConnectionTexture;
  SGameState m_oGameState;
  SGameState m_oPrevGameState;
  std::vector<CClientBall> m_vctBalls;
  // Ordering looks odd; m_bRightHanded need to be declared before paddles
  uint8 m_uClientId = UINT8_MAX;
  bool m_bGameStarted = false;
  bool m_bRightHanded = true;
  bool m_bGameBegun = false;
  float m_fCountdownTimer = 0.0f;
  CClientPaddle m_oPlayerPaddle;
  CClientPaddle m_oEnemyPaddle;
  CNetworkClient* m_pNetworkClient = nullptr;
  void (CGameClient::*m_pfncLoop)(void) = nullptr;
  float m_fConnectingTextCount = 0.0f;
  float m_fDeltaTimeBetweenGameStates = 0.0f;
  uint32 m_uGameStatesBufferIndex = 0u;
  uint32 m_uLastProcessedGameStateIndex = 0u;
  bool m_bWantClose = false;
  bool m_bLocalGame = false;
};