#pragma once

#include <memory>

#include "raylib/raylib.h"
#include "base/types.h"
#include "base/maths.h"

class CBlock
{
public:
  
  CBlock(uint16 _uType);
  ~CBlock();

  void Init();
  void Update(float _fDt);
  void Draw();

  CVector2D m_v2Pos;
  CVector2D m_v2Size;
  CVector2D m_v2HalfSize;

private:
  std::shared_ptr<Texture2D> m_spTexture;
  uint16 m_uType = 1u;
};