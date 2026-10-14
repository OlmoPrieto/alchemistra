#pragma once

#include "base/types.h"
#include "base/maths.h"

class CServerBall
{
public:
  CServerBall();
  ~CServerBall();

  void Update(float _fDt);

  CVector2D m_v2Pos;
  CVector2D m_v2PrevPos;
  CVector2D m_v2Velocity;
  CVector2D m_v2Acceleration;
  CVector2D m_v2Size;
  CVector2D m_v2HalfSize;
  float     m_fSpeed = 1.0f;
  float     m_fRadius = 0.0f;
  float     m_fAccTime = 0.0f;
  uint32    m_uId = 99999u;
  bool      m_bCanCollide = true;

private:
  static inline uint32 sm_uBallIdCounter = 0u;
};