#include "game/paddle.h"

#include <limits>
#include <cmath>
#include <cassert>

#include "game/game.h"
#include "game/ball.h"

// ------------------------

CPaddle::CPaddle()
{
}

// ------------------------

CPaddle::~CPaddle()
{
  UnloadTexture(*m_spTexture.get());
}

// ------------------------

void CPaddle::Init()
{
  m_spTexture = std::make_shared<Texture2D>(LoadTexture("assets/paddle_basic.png"));

  m_v2Size.x = m_spTexture->width;
  m_v2Size.y = m_spTexture->height;
  m_v2HalfSize = m_v2Size * 0.5f;

  m_v2Pos.x = CGame::sm_uWindowWidth * 0.5f - m_v2HalfSize.x;
  m_v2Pos.y = CGame::sm_uWindowHeight * 0.9f;
}

// ------------------------

void CPaddle::Update(float _fDt)
{
  m_v2Pos.x = GetMouseX() / CGame::sm_uRenderFactor;
}

// ------------------------

void CPaddle::Draw()
{
  // if (m_spTexture.get())
  DrawTextureEx(*m_spTexture.get(), 
   Vector2{m_v2Pos.x - m_v2HalfSize.x, 
     m_v2Pos.y - m_v2HalfSize.y},
     0.0f, 1.0f, WHITE);

  /*
  DrawTextureEx(*m_spTexture.get(),
      Vector2{ m_v2Pos.x - m_v2HalfSize.x,
        m_v2Pos.y - m_v2HalfSize.y },
      0.0f, 1.0f, Color({ (byte)(m_aColor[0] * 255.0f), 
        (byte)(m_aColor[1] * 255.0f), (byte)(m_aColor[2] * 255.0f), 255 }));
  */
}

// ------------------------

void CPaddle::CheckCollisions(CBall* _pBall)
{
  const CVector2D v2BallPos = _pBall->m_v2Pos;
  const CVector2D v2BallHalfSize = _pBall->m_v2HalfSize;
  const float fBallRadius = _pBall->m_fRadius;
  if (v2BallPos.x + fBallRadius > m_v2Pos.x - m_v2HalfSize.x &&
    v2BallPos.x - fBallRadius < m_v2Pos.x + m_v2HalfSize.x &&
    v2BallPos.y + fBallRadius > m_v2Pos.y - m_v2HalfSize.y &&
    v2BallPos.y - fBallRadius < m_v2Pos.y + m_v2HalfSize.y
    )
  {
#if 0
    const float fPercentage = Clamp((v2BallPos.x - m_v2Pos.x) / (m_v2Size.x), -0.5f, 0.5f);
    const CVector2D v2BallVelocity = _pBall->m_v2Velocity;
    const float fSpeed = std::sqrtf(v2BallVelocity.x * v2BallVelocity.x + v2BallVelocity.y * v2BallVelocity.y);
    constexpr float fPi = 3.14159265f;
    constexpr float fBaseAngle = fPi * 0.5f;
    constexpr float fMaxAngle = 60.0f * (fPi / 180.0f);
    const float fNewAngle = fBaseAngle - (fPercentage * 2.0f * fMaxAngle);
    _pBall->m_v2Velocity.y *= -1.0f;
    _pBall->m_v2Velocity.x = fSpeed * std::cosf(fNewAngle);
#else
    const float fPercentage = Clamp((v2BallPos.x - m_v2Pos.x) / (m_v2Size.x), -0.5f, 0.5f);
    // Convert to range [0.0f, 1.0f]
    const float fDistanceToCenter = (fPercentage * fPercentage) * 4.0f;
    constexpr float fSpeedMultiplier = 1.25f;
    const float fInverseSpeedMultiplier = fSpeedMultiplier - (fDistanceToCenter * (fSpeedMultiplier - 1.0f));
    _pBall->m_v2Velocity.x = CGame::sm_fBallBaseVelocityY * (fPercentage * 2.0f);
    // To avoid the ball to accelerate in diagonal, we subtract a % on Y of what we add in X
    // If fPercentage is 0.5f (furthest), fDistanceToCenter is 1.0f
    // We subtract a % of vertical velocity to compensate diagonal stretching
    const float fDiagonalCompensation = 1.0f - (fDistanceToCenter * 0.15f); // Reduce up to 15% on Y on the edges
    _pBall->m_v2Velocity.y = -CGame::sm_fBallBaseVelocityY * fInverseSpeedMultiplier * fDiagonalCompensation;

#endif
  }
}

// ------------------------

const CBall* CPaddle::GetClosestBall()
{
  const std::vector<CBall>& vctBalls = CGame::GetInstance().GetBalls();

  float fClosestX = std::numeric_limits<float>::max();
  const CBall* pClosestBall = nullptr;

  for (uint32 i = 0; i < vctBalls.size(); ++i)
  {
    const float fX = vctBalls[i].m_v2Pos.x;
    if (fX < fClosestX)
    {
      fClosestX = fX;
      pClosestBall = &vctBalls[i];
    }
  }

  return pClosestBall;
}

// ------------------------

// ------------------------
// ------------------------
// ------------------------