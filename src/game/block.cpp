#include "game/block.h"

#include <cassert>

// ------------------------

CBlock::CBlock(uint16 _uType)
  : m_uType(_uType)
{

}

CBlock::~CBlock()
{
  if (m_spTexture && m_spTexture.use_count() == 0)
  {
    UnloadTexture(*m_spTexture.get());
  }
}

// ------------------------

void CBlock::Init()
{
  switch (m_uType)
  {
    case 1:
    {
      m_spTexture = std::make_shared<Texture2D>(LoadTexture("assets/block_basic.png"));
      break;
    }
    default:
    {
      
    }
  }

  assert(m_spTexture.get());

  m_v2Size.x = m_spTexture->width;
  m_v2Size.y = m_spTexture->height;
  m_v2HalfSize = m_v2Size * 0.5f;

  m_v2Pos.x = m_v2HalfSize.x;
  m_v2Pos.y = m_v2HalfSize.y * 10.0f;
}

// ------------------------

void CBlock::Update(float _fDt)
{

}

// ------------------------

void CBlock::Draw()
{
  DrawTextureEx(*m_spTexture.get(),
    Vector2{ m_v2Pos.x - m_v2HalfSize.x,
      m_v2Pos.y - m_v2HalfSize.y },
    0.0f, 1.0f, WHITE);
}
// ------------------------