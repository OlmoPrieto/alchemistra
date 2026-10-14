#include "game/server_ball.h"

#include "game/gameserver.h"

// ------------------------

CServerBall::CServerBall()
{
  m_v2Size = IGame::sm_v2BallSize;
  m_v2HalfSize = m_v2Size * 0.5f;
  m_fRadius = m_v2HalfSize.x;

  m_v2Pos = { IGame::sm_uWindowWidth / 2u, IGame::sm_uWindowHeight / 2u - 100.0f };

  m_v2Velocity.x = GetRandomValue(700.0f, 500.0f);
  m_v2Velocity.y = GetRandomValue(400.0f, 150.0f);

  //m_fSpeed = 5.0f;
}

// ------------------------

CServerBall::~CServerBall()
{

}

// ------------------------

void CServerBall::Update(float _fDt)
{
  // Apply physics
  m_v2Pos += (m_v2Velocity * m_fSpeed * _fDt) + 0.5f * m_v2Acceleration * (_fDt * _fDt);

  bool bReapplyPhysics = false;
  // Check with screen boundaries
  //  Sides
  if (m_v2Pos.x + m_v2HalfSize.x > CGameServer::sm_uWindowWidth || m_v2Pos.x - m_v2HalfSize.x < 0)
  {
    m_v2Velocity.x *= -1.0f;
    m_v2Acceleration.y *= -1.0f;
    bReapplyPhysics = true;
  }
  //  Vertical
  else if (m_v2Pos.y + m_v2HalfSize.y > CGameServer::sm_uWindowHeight || m_v2Pos.y - m_v2HalfSize.y < 0)
  {
    m_v2Velocity.y *= -1.0f;
    m_v2Acceleration.y *= -1.0f;
    bReapplyPhysics = true;
  }

  /*if (bReapplyPhysics)
  {
    m_v2Pos += (m_v2Velocity * m_fSpeed * _fDt) + 0.5f * m_v2Acceleration * (_fDt * _fDt);
  }*/

  // Reduce acceleration ("air" friction)
  m_v2Acceleration.y *= 0.95f;
  float fPrevAccY = m_v2Acceleration.y;
  if (m_v2Acceleration.y >= -0.001f && m_v2Acceleration.y <= 0.001f)
  {
    m_v2Acceleration.y = 0.0f;
  }

  //printf("VelX: %.3f\n", m_v2Velocity.x);
}

// ------------------------
// ------------------------