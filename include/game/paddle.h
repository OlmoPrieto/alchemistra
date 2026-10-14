#pragma once

#include <memory>

#include "raylib/raylib.h"
#include "base/types.h"
#include "base/maths.h"

class CBall;

class CPaddle
{
public:
  CPaddle();
  ~CPaddle();

  void Init();
  void Update(float _fDt);
  void Draw();

  void CheckCollisions(CBall* _pBall);

  CVector2D m_v2Pos;
  CVector2D m_v2PrevPos;
  CVector2D m_v2Velocity;
  CVector2D m_v2Size;
  CVector2D m_v2HalfSize;
  float     m_fSpeed = 1.0f;
  float     m_fDirection = 0.0f;

private:
  const CBall* GetClosestBall();

  std::shared_ptr<Texture2D> m_spTexture;
};