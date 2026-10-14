#pragma once

#include "base/types.h"
#include "base/maths.h"

class CServerBall;

class CServerPaddle
{
public:
  CServerPaddle();
  ~CServerPaddle();

  void Init();
  void Update(float _fDt);

  void CheckCollisions(CServerBall* _pBall);

  CVector2D m_v2Pos;
  CVector2D m_v2PrevPos;
  CVector2D m_v2Velocity;
  CVector2D m_v2Size;
  CVector2D m_v2HalfSize;
  CVector2D m_v2BallStoredVelocity;
  float     m_fSpeed = 1.0f;
  float     m_fDirection = 0.0f;
  float     m_fAccVelocityOnContact = 0.0f;
  float     m_fAccTime = 0.0f;
  uint32    m_uMaxContactFrames = 4u;
  uint32    m_uContactFrames = 0u;
  uint32    m_uId = UINT32_MAX; // HACK temporal, remove

private:
};