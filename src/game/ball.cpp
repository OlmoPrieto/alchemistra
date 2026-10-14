#include "game/ball.h"

#include <cassert>

#include "game/game.h"

// ------------------------

CBall::CBall()
{
  m_uId = sm_uBallIdCounter++;
}

// ------------------------

CBall::~CBall()
{
  
}

// ------------------------

void CBall::Init()
{
  assert(!m_spTexture);

  // Image img = LoadImage("assets/ball_basic.png");
  // ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  // Texture2D ballTex = LoadTextureFromImage(img);
  // m_spTexture = std::make_shared<Texture2D>(ballTex);
  m_spTexture = std::make_shared<Texture2D>(LoadTexture("assets/ball_basic.png"));
  m_v2Size.x = m_spTexture->width;
  m_v2Size.y = m_spTexture->height;
  m_v2HalfSize = m_v2Size * 0.5f;
  m_fRadius = m_v2HalfSize.x;

  m_v2Pos.x = CGame::sm_uWindowWidth * 0.5f;
  m_v2Pos.y = CGame::sm_uWindowHeight * 0.88f;

  m_v2Velocity.x = GetRandom(-200.0f, 200.0f);
  m_v2Velocity.y = -CGame::sm_fBallBaseVelocityY;
}

// ------------------------

void CBall::Update(float _fDt)
{
  // Apply physics
  m_v2Pos += (m_v2Velocity * m_fSpeed * _fDt) + 0.5f * m_v2Acceleration * (_fDt * _fDt);

  bool bReapplyPhysics = false;
  // Check with screen boundaries
  //  Sides
  if (m_v2Pos.x + m_v2HalfSize.x > CGame::sm_uWindowWidth || m_v2Pos.x - m_v2HalfSize.x < 0)
  {
    m_v2Velocity.x *= -1.0f;
    bReapplyPhysics = true;
  }
  //  Vertical
  else if (m_v2Pos.y + m_v2HalfSize.y > CGame::sm_uWindowHeight || m_v2Pos.y - m_v2HalfSize.y < 0)
  {
    m_v2Velocity.y *= -1.0f;
    bReapplyPhysics = true;
  }

  if (bReapplyPhysics)
  {
    m_v2Pos += (m_v2Velocity * m_fSpeed * _fDt) + 0.5f * m_v2Acceleration * (_fDt * _fDt);
  }
}

// ------------------------

void CBall::Draw()
{
  // DrawTextureEx(*m_spTexture.get(), 
  //   Vector2{m_v2Pos.x - m_v2HalfSize.x, 
  //   m_v2Pos.y - m_v2HalfSize.y},
  //   0.0f, 1.0f, WHITE);

  DrawTextureV(*m_spTexture, Vector2{m_v2Pos.x - m_v2HalfSize.x,
    m_v2Pos.y - m_v2HalfSize.y},
    WHITE);

  // const float fAlpha = m_fAccTime / CGame::m_fTargetFrameTime;
  // const CVector2D v2RenderPos = m_v2PrevPos + fAlpha * (m_v2Pos - m_v2PrevPos);
  // DrawTextureEx(*m_spTexture.get(), 
  //   Vector2{v2RenderPos.x - m_v2HalfSize.x, 
  //   v2RenderPos.y - m_v2HalfSize.y},
  //   0.0f, 1.0f, WHITE);
}

// ------------------------

void CBall::UnloadResources()
{
  UnloadTexture(*m_spTexture.get());
}

// ------------------------

int32 CBall::CheckCollisions(const std::vector<CBlock>& _vctBlocks)
{
  int32 iIndex = -1;
  for (uint32 i = 0; i < _vctBlocks.size(); ++i)
  {
    const CVector2D v2BlockPos = _vctBlocks[i].m_v2Pos;
    const CVector2D v2BlockHalfSize = _vctBlocks[i].m_v2HalfSize;
    if (m_v2Pos.x + m_fRadius > v2BlockPos.x - v2BlockHalfSize.x &&
        m_v2Pos.x - m_fRadius < v2BlockPos.x + v2BlockHalfSize.x &&
        m_v2Pos.y + m_fRadius > v2BlockPos.y - v2BlockHalfSize.y &&
        m_v2Pos.y - m_fRadius < v2BlockPos.y + v2BlockHalfSize.y
      )
    {
      iIndex = i;

      m_v2Velocity.y *= -1.0f;
      m_v2Pos += (m_v2Velocity * m_fSpeed * 0.0166666f) + 0.5f * m_v2Acceleration * (0.0166666f * 0.0166666f);

      break;
    }
  }

  return iIndex;
}

// ------------------------

// ------------------------