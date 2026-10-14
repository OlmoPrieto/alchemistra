#include "game/gameserver.h"

#include <cassert>
#include <chrono>
#include <thread>

#include "base/network_serialization.h"
#include "enet/enet.h"
#include "game/gameclient.h"
#include "game/network_protocol.h"
#include "game/network_server.h"

// ------------------------

CGameServer::CGameServer(uint32 _uGameId)
  : m_uGameId(_uGameId)
{
  // Paddle0 (first player) is always located on the right (most of the players are right handed)
  m_oPaddle0.m_v2Pos = sm_v2RightPlayerPos;
  m_oPaddle0.m_uId = 0u; // HACK
  // Paddle1 (second player) is always located on the left
  m_oPaddle1.m_v2Pos = sm_v2LeftPlayerPos;
  m_oPaddle1.m_uId = 1u; // HACK

  m_vctBalls.push_back(CServerBall());

  m_oGameState.m_uNumBalls = 1u;
  m_oGameState.m_vctBallsPos.push_back(CVector2D::v2ZERO);
  m_oGameState.m_vctBallsVel.push_back(CVector2D::v2ZERO);
  m_oGameState.m_av2PlayersPos[0] = m_oPaddle0.m_v2Pos;
  m_oGameState.m_av2PlayersPos[1] = m_oPaddle1.m_v2Pos;
}

// ------------------------

CGameServer::~CGameServer()
{

}

// ------------------------

CGameServer::CGameServer(CGameServer&& _oGame) noexcept
{
  m_oGameState = std::move(_oGame.m_oGameState);
  m_vctBalls = std::move(_oGame.m_vctBalls);
  m_pNetworkServer = std::move(_oGame.m_pNetworkServer);
  m_bGameStarted = std::move(_oGame.m_bGameStarted);
  m_bGameEnded = std::move(_oGame.m_bGameEnded);
}

// ------------------------

CGameServer& CGameServer::operator=(CGameServer&& _oGame) noexcept
{
  m_oGameState = std::move(_oGame.m_oGameState);
  m_vctBalls = std::move(_oGame.m_vctBalls);
  m_pNetworkServer = std::move(_oGame.m_pNetworkServer);
  m_bGameStarted = std::move(_oGame.m_bGameStarted);
  m_bGameEnded = std::move(_oGame.m_bGameEnded);

  return *this;
}

// ------------------------

void CGameServer::Init()
{
  
}

// ------------------------

void CGameServer::Begin(CNetworkServer* _pNetworkServer, CClientConnection* _pClient0, CClientConnection* _pClient1)
{
  m_pNetworkServer = _pNetworkServer;
  m_pClient0 = _pClient0;
  m_pClient1 = _pClient1;
  m_bGameStarted = true;
}

// ------------------------

void CGameServer::Loop()
{
  constexpr std::chrono::duration<float, std::milli> oTargetFrameTime(16.6666666f);

  if (m_bGameStarted && !m_bGameEnded)
  {
    std::chrono::time_point<std::chrono::high_resolution_clock> oFrameStart = std::chrono::high_resolution_clock::now();
    
    if (m_bPause == false)
    {
      //printf("Server tick: %llu\n", m_uLogicTick);
      //printf("  Ball x pos: %.3f\n", m_vctBalls[0].m_v2Pos.x);

      // Receive from clients
      // Update
      Update(oTargetFrameTime.count() * 0.001f);
  
      // Prepare game state
      PrepareGameState();

      // Send to clients
      SendGameState();

      ++m_uLogicTick;
    }

    const std::chrono::time_point<std::chrono::high_resolution_clock> oFrameEnd = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<float, std::milli> oProcessTime = oFrameEnd - oFrameStart;
    
    if (m_bGameBegun == false)
    {
      m_fCurrentCountdownTimer -= m_fTargetFrameTime;

#define SEND_TIMER 1
#if SEND_TIMER
      SNetStream oStream;
      byte pBuffer[NET_MAX_PACKET_SIZE];
      memset(pBuffer, 0, NET_MAX_PACKET_SIZE);
      InitNetStream(&oStream, pBuffer, NET_MAX_PACKET_SIZE);

      SPacketHeader oHeader;
      WriteHeader(&oStream, EMsgType::START_GAME_COUNTDOWN);
      WriteFloat32(&oStream, m_fCurrentCountdownTimer);

      m_pNetworkServer->SendToClient(&oStream, m_pClient0, EMsgPriority::LOW);
      m_pNetworkServer->SendToClient(&oStream, m_pClient1, EMsgPriority::LOW);
#endif

      if (m_fCurrentCountdownTimer <= 0.0f)
      {
        m_bGameBegun = true;
        m_fCurrentCountdownTimer = 0.0f;
      }
    }

    // Sleep for remaining time
    if (oProcessTime < oTargetFrameTime)
    {
      std::this_thread::sleep_for(oTargetFrameTime - oProcessTime);
    }
  }
}

// ------------------------

void CGameServer::Update(float _fDt)
{
  if (m_bGameBegun)
  {
    for (uint32 i = 0; i < m_vctBalls.size(); ++i)
    {
      m_vctBalls[i].Update(_fDt);
      m_oPaddle0.CheckCollisions(&m_vctBalls[i]);
      m_oPaddle1.CheckCollisions(&m_vctBalls[i]);
    }
  }
}

// ------------------------

void CGameServer::SendGameState()
{
  SNetStream oStream;
  byte pBuffer[NET_MAX_PACKET_SIZE];
  memset(pBuffer, 0, NET_MAX_PACKET_SIZE);
  InitNetStream(&oStream, pBuffer, NET_MAX_PACKET_SIZE);
  m_oGameState.Write(&oStream);

  if (m_pNetworkServer->SendToClient(&oStream, m_pClient0, EMsgPriority::LOW))
  {
    //printf("%llu: Server sent data to client0\n", m_uLogicTick);
  }
  if (m_pNetworkServer->SendToClient(&oStream, m_pClient1, EMsgPriority::LOW))
  {
    //printf("%llu: Server sent data to client1\n", m_uLogicTick);
  }
}

// ------------------------

bool CGameServer::GetGameEnded() const
{
  return m_bGameEnded;
}

// ------------------------

bool CGameServer::SetPause(bool _bState, uint32 _uClientId)
{
  if (m_uClientThatPaused == UINT32_MAX)
  {
    m_uClientThatPaused = _uClientId;
    assert(_bState == true);
    m_bPause = _bState;

    return true;
  }
  else
  {
    if (_bState != m_bPause && m_uClientThatPaused == _uClientId)
    {
      // Check that the game can be resumed because the right player has requested the resume
      m_uClientThatPaused = UINT32_MAX;
      assert(_bState == false);
      m_bPause = _bState;
      return true;
    }
  }

  return false;
}

// ------------------------

void CGameServer::PrepareGameState()
{
  m_oGameState.m_uNumBalls = m_vctBalls.size();
  for (uint32 i = 0; i < m_vctBalls.size(); ++i)
  {
    m_oGameState.m_vctBallsPos[i] = m_vctBalls[i].m_v2Pos;
  }
  for (uint32 i = 0; i < m_vctBalls.size(); ++i)
  {
    m_oGameState.m_vctBallsVel[i] = m_vctBalls[i].m_v2Velocity;
  }

  m_oGameState.m_uFrame = m_uLogicTick;

  m_oGameState.m_av2PlayersPos[0] = m_oPaddle0.m_v2Pos;
  m_oGameState.m_av2PlayersPos[1] = m_oPaddle1.m_v2Pos;
}

// ------------------------
// ------------------------
// ------------------------