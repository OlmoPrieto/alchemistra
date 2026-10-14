#include "game/server_paddle.h"

#include <limits>
#include <cmath>
#include <cassert>

#include "game/gameserver.h"
#include "game/server_ball.h"

// ------------------------

CServerPaddle::CServerPaddle()
{
  m_v2Size = IGame::sm_v2PaddleSize;
  m_v2HalfSize = m_v2Size * 0.5f;
}

// ------------------------

CServerPaddle::~CServerPaddle()
{

}

// ------------------------

void CServerPaddle::Init()
{
  
}

// ------------------------

void CServerPaddle::Update(float _fDt)
{
  
}

// ------------------------

void CServerPaddle::CheckCollisions(CServerBall* _pBall)
{
  uint32 uPrevContactFrames = m_uContactFrames;

  float fPosDiff = m_v2Pos.y - m_v2PrevPos.y;
  bool bAccelerating = fabsf(fPosDiff) > m_v2Size.y * 0.1f;

  bool bDetattched = false;

  const CVector2D v2BallPos = _pBall->m_v2Pos;
  const CVector2D v2BallHalfSize = _pBall->m_v2HalfSize;
  const float fBallRadius = _pBall->m_fRadius;
  if (v2BallPos.x + fBallRadius > m_v2Pos.x - m_v2HalfSize.x &&
    v2BallPos.x - fBallRadius < m_v2Pos.x + m_v2HalfSize.x &&
    v2BallPos.y + fBallRadius > m_v2Pos.y - m_v2HalfSize.y &&
    v2BallPos.y - fBallRadius < m_v2Pos.y + m_v2HalfSize.y
    )
  {
    // If contacted
    if (m_uContactFrames == 0u)
    {
      m_v2BallStoredVelocity = _pBall->m_v2Velocity;
      m_v2PrevPos = m_v2Pos;
    }

    ++m_uContactFrames;

    m_fAccVelocityOnContact = fPosDiff;

    _pBall->m_v2Velocity.x = 0.0f;
    _pBall->m_v2Velocity.y = 0.0f;

    // -----
    if (m_uContactFrames >= m_uMaxContactFrames)
    {
      bDetattched = true;
    }
  }
  else
  {
    // If not contacted
    if (uPrevContactFrames > 0u)
    {
      bDetattched = true;
    }
  }

  if (bDetattched)
  {
    // Horizontal Speed: reverse trajectory of the ball and
    //  add a bit of velocity depending on the friction with the paddle
    const float fBallXVelocity = m_v2BallStoredVelocity.x;
    float fSpeedX = _pBall->m_v2Pos.x > CGameServer::sm_uWindowWidth * 0.5f ? -fBallXVelocity : fBallXVelocity;
    //fSpeedX *= 1.025f; // Always increase speed a little on each hit || Original
    fSpeedX *= 1.0025f;
    const float fMaxDisplacement = m_v2Size.x * 3.0f;
    const float fClampedPaddlePosDiff = Clamp(fPosDiff, -fMaxDisplacement, fMaxDisplacement);

    // HACK
#define HACK_0 0
#if HACK_0
    if (m_uId == 1)
    {
      assert(m_v2Pos.x == IGame::sm_v2LeftPlayerPos.x);
      printf("fSpeedX: %.3f\n", fSpeedX);
      printf("Remap: %.3f\n", Remap(fClampedPaddlePosDiff, -fMaxDisplacement, fMaxDisplacement,
        1.005f, 1.5f) * std::copysign(1.0f, m_v2BallStoredVelocity.x));
      printf("speed*remap: %.3f\n", fSpeedX * Remap(fClampedPaddlePosDiff, -fMaxDisplacement, fMaxDisplacement,
        1.005f, 1.5f) * std::copysign(1.0f, m_v2BallStoredVelocity.x));
    }
#endif
    // HACK

    _pBall->m_v2Velocity.x = fSpeedX * Remap(fClampedPaddlePosDiff, -fMaxDisplacement, fMaxDisplacement,
      1.0f, 1.5f) * std::copysign(1.0f, m_v2BallStoredVelocity.x);

    // Dettatch the ball from the paddle to not collide anymore
    if (_pBall->m_v2Pos.x > CGameServer::sm_uWindowWidth * 0.5f)
    {
      _pBall->m_v2Pos.x = m_v2Pos.x - m_v2HalfSize.x - _pBall->m_v2HalfSize.x - 1.0f;
    }
    else
    {
      _pBall->m_v2Pos.x = m_v2Pos.x + m_v2HalfSize.x + _pBall->m_v2HalfSize.x + 1.0f;
    }


    // Vertical Speed
    // Positive fPosDiff is downwards | Negative fPosDiff is upwards
    _pBall->m_v2Acceleration.y = fPosDiff * (float)m_uContactFrames * 500.0f;

    m_uContactFrames = 0u;
    m_v2BallStoredVelocity.x = 0.0f;
    m_v2BallStoredVelocity.y = 0.0f;

    m_v2BallStoredVelocity = CVector2D::v2ZERO;
  }
}

// ------------------------

// ------------------------
// ------------------------
// ------------------------