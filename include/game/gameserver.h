#pragma once

#include <vector>

#include "game/game.h"
#include "game/clienthandle.h"
#include "base/types.h"
#include "game/server_ball.h"
#include "game/server_paddle.h"

class CGameClient;
struct _ENetPeer;
typedef struct _ENetPeer ENetPeer;
class CNetworkServer;
class CClientConnection;

class CGameServer : public IGame
{
public:
  CGameServer(uint32 _uGameId);
  ~CGameServer();
  CGameServer(const CGameServer& _oGame) = delete;
  CGameServer(CGameServer&& _oGame) noexcept;
  CGameServer& operator =(const CGameServer& _oGame) = delete;
  CGameServer& operator =(CGameServer&& _oGame) noexcept;

  void Init();
  void Begin(CNetworkServer* _pNetworkServer, CClientConnection* _pClient0, CClientConnection* _pClient1);
  void Loop();
  void Update(float _fDt);
  void SendGameState();
  bool GetGameEnded() const;
  bool GetGameStarted() const { return m_bGameStarted; }
  bool SetPause(bool _bState, uint32 _uClientId);
  uint32 GetGameId() const { return m_uGameId; }

  //static constexpr uint32 sm_uWindowWidth = 1260u;
  //static constexpr uint32 sm_uWindowHeight = 580u;

  uint64 m_uLogicTick = 0u;
  constexpr static float m_fTargetFrameTime = 1.0f / 60.0f;
  constexpr static float m_fCountdownTimer = 1.0f;

private:
  void PrepareGameState();

  friend class CNetworkServer;

  SGameState m_oGameState;
  std::vector<CServerBall> m_vctBalls;
  CServerPaddle m_oPaddle0;
  CServerPaddle m_oPaddle1;
  CNetworkServer* m_pNetworkServer = nullptr;
  CClientConnection* m_pClient0 = nullptr;
  CClientConnection* m_pClient1 = nullptr;
  uint32 m_uGameId = UINT32_MAX;
  uint32 m_uClientThatPaused = UINT32_MAX;
  float m_fCurrentCountdownTimer = m_fCountdownTimer;
  bool m_bGameStarted = false;
  bool m_bGameBegun = false;
  bool m_bGameEnded = false;
  bool m_bPause = false;
};