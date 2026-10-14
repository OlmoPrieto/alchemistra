#include "game/game.h"

#include <random>
#include <chrono>

// ------------------------------------------------
// ------------------------------------------------

// ------------------------

CGame::CGame()
{
  memset(m_auPlacedElements, 0, sizeof(uint8) * sm_uMaxHorizontalElements * sm_uMaxVerticalElements);
}

// ------------------------

CGame::~CGame()
{
  if (m_spBackground)
  {
    UnloadTexture(*m_spBackground.get());
  }
  if (m_spTriangle)
  {
    UnloadTexture(*m_spTriangle.get());
  }
  if (m_spSquare)
  {
    UnloadTexture(*m_spSquare.get());
  }
  if (m_spPentagon)
  {
    UnloadTexture(*m_spPentagon.get());
  }
  if (m_spCircle)
  {
    UnloadTexture(*m_spCircle.get());
  }

  if (m_bWantClose)
  {
    CloseWindow();
  }
}

// ------------------------

void CGame::Init()
{
  InitWindow(sm_uVirtualWindowWidth, sm_uVirtualWindowHeight, "alchemistra");
  SetTargetFPS(60);

  m_oRenderTarget = LoadRenderTexture(sm_uWindowWidth, sm_uWindowHeight);
  SetTextureFilter(m_oRenderTarget.texture, TEXTURE_FILTER_POINT);

  //Image oWhiteImage = GenImageColor(sm_uWindowWidth - 64u, sm_uWindowHeight * 0.5f, Color{200, 200, 200, 255});
  Image oWhiteImage = GenImageColor(sm_uBackgroundWidth, sm_uBackgroundHeight, Color{200, 200, 200, 255});
  m_spBackground = std::make_shared<Texture2D>(LoadTextureFromImage(oWhiteImage));
  UnloadImage(oWhiteImage);

  m_spTriangle = std::make_shared<Texture2D>(LoadTexture("assets/triangle.png"));
  m_spSquare   = std::make_shared<Texture2D>(LoadTexture("assets/square.png"));
  m_spPentagon = std::make_shared<Texture2D>(LoadTexture("assets/pentagon.png"));
  m_spCircle   = std::make_shared<Texture2D>(LoadTexture("assets/circle.png"));

  m_oPendingElements.first  = 1;
  m_oPendingElements.second = 1;
}

// ------------------------

void CGame::Update(float _fDt)
{
  m_bWantClose = IsKeyPressed(KEY_ESCAPE);

  if (IsKeyPressed(KEY_R))
  {
    memset(m_auPlacedElements, 0, sizeof(uint8) * sm_uMaxHorizontalElements * sm_uMaxVerticalElements);
  }

  const CVector2D v2MousePos = GetMousePos();
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && v2MousePos.x >= 32u && v2MousePos.x <= sm_uWindowWidth - 32u)
  {
    // Drop elements
    //  Translate mouse coordinates into row/column indices
    //  First find out the column
    const CVector2D v2SnappedMousePos(((int32)v2MousePos.x / 32) * 32, ((int32)v2MousePos.y / 32) * 32);
    printf("Snapped pos: [%.2f, %.2f]\n", v2SnappedMousePos.x, v2SnappedMousePos.y);
    const uint32 uColumn = Clamp(((int32)v2SnappedMousePos.x - 32 * 2) / 32, 0u, sm_uMaxHorizontalElements - 1u); // 32u*2u because you need to subtract 2 elements to the left
    //  Then find the height of that row (depending on the stacked elements)
    printf("Dropping on: ");
    if (uColumn + 1 < sm_uMaxHorizontalElements)
    {
      for (uint32 i = 0; i < sm_uMaxVerticalElements; ++i)
      {
        if (m_auPlacedElements[i][uColumn] == 0)
        {
          // Found a place to put the element
          m_auPlacedElements[i][uColumn] = m_oPendingElements.first;
          printf("[%u|%u] ", i, uColumn);
          ++m_uPlacedElements;
          break;
        }
      }
    
      for (uint32 i = 0; i < sm_uMaxVerticalElements; ++i)
      {
        if (m_auPlacedElements[i][uColumn + 1] == 0)
        {
          // Found a place to put the element
          m_auPlacedElements[i][uColumn + 1] = m_oPendingElements.second;
          printf("and [%u|%u]\n", i, uColumn + 1);
          ++m_uPlacedElements;
          break;
        }
      }
    }

    while (CheckAndMerge())
    {
      printf("Merged!\n");
    }
  }
}

// ------------------------

void CGame::Draw()
{
  BeginTextureMode(m_oRenderTarget);
  ClearBackground(BLACK);
  // --------------
  
  // Draw background
  DrawTexture(*m_spBackground.get(), sm_uBackgroundXPos, sm_uBackgroundYPos, WHITE);
  
  // Draw placed elements
#if 0
  for (uint32 i = 0; i < m_vctPlacedElements.size(); ++i)
  {
    Texture2D* pTargetTexture = GetTextureByElement(m_vctPlacedElements[i].second);
    DrawTexture(*pTargetTexture, m_vctPlacedElements[i].first.x, m_vctPlacedElements[i].first.y, WHITE);
  }
#else
  for (uint32 i = 0; i < sm_uMaxVerticalElements; ++i)
  {
    for (uint32 j = 0; j < sm_uMaxHorizontalElements; ++j)
    {
      if (m_auPlacedElements[i][j] != 0u)
      {
        Texture2D* pTargetTexture = GetTextureByElement(m_auPlacedElements[i][j]);
        const CVector2D v2ElementPos(32u + j * 32u, (sm_uBackgroundYPos + sm_uBackgroundHeight - 32u) - i * 32u);
        DrawTexture(*pTargetTexture, v2ElementPos.x, v2ElementPos.y, WHITE);
      }
    }
  }
#endif

  // Draw pending elements
  const CVector2D v2MousePos = GetMousePos();
  CVector2D v2SnappedMousePos(((int32)v2MousePos.x / 32) * 32, ((int32)v2MousePos.y / 32) * 32);
  v2SnappedMousePos.x = Clamp(v2SnappedMousePos.x, 64.0f, sm_uWindowWidth - 64.0f);
  Texture2D* pTargetTexture = GetTextureByElement(m_oPendingElements.first);
  DrawTexture(*pTargetTexture, v2SnappedMousePos.x - 32, sm_uPendingElementHeight, WHITE);
  pTargetTexture = GetTextureByElement(m_oPendingElements.second);
  DrawTexture(*pTargetTexture, v2SnappedMousePos.x, sm_uPendingElementHeight, WHITE);

  // --------------
  EndTextureMode();

  // -----------------------------

  BeginDrawing();
  ClearBackground(BLACK);
  // --------------

  // Flip texture on Y axis (raylib draws render textures inverted by default)
  Rectangle oSourceRect = { 0.0f, 0.0f, (float)m_oRenderTarget.texture.width, -(float)m_oRenderTarget.texture.height };

  // DestRect size will be the total size of the current physical window
  Rectangle oDestRect = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };

  // Draw virtual canvas stretching perfectly in the big window
  DrawTexturePro(m_oRenderTarget.texture, oSourceRect, oDestRect, Vector2{ 0, 0 }, 0.0f, WHITE);

  // --------------
  EndDrawing();
}

// ------------------------

bool CGame::CheckAndMerge()
{
  bool bMerged = false;

  // Horizontal
  for (uint32 i = 0; i < sm_uMaxVerticalElements; ++i)
  {
    for (uint32 j = 0; j < sm_uMaxHorizontalElements; ++j)
    {
      uint8 uOccurrence = 1u;
      const uint8 uCurrentType = m_auPlacedElements[i][j];
      if (uCurrentType != 0u)
      {
        uint32 k = j + 1;
        for (; k < sm_uMaxHorizontalElements; ++k)
        {
          if (m_auPlacedElements[i][k] != uCurrentType)
          {
            break;
          }
          else
          {
            ++uOccurrence;
          }
        }

        if (uOccurrence >= 3u)
        {
          // Merge
          m_auPlacedElements[i][j] += 1u;
          for (uint32 l = j + 1; l < k; ++l)
          {
            m_auPlacedElements[i][l] = 0u;
          }

          bMerged = true;
        }
      }
    }
  }

  // Vertical
  for (uint32 i = 0; i < sm_uMaxHorizontalElements; ++i)
  {
    for (uint32 j = 0; j < sm_uMaxVerticalElements; ++j)
    {
      uint8 uOccurrence = 1u;
      const uint8 uCurrentType = m_auPlacedElements[j][i];
      if (uCurrentType != 0u)
      {
        uint32 k = j + 1;
        for (; k < sm_uMaxVerticalElements; ++k)
        {
          if (m_auPlacedElements[k][i] != uCurrentType)
          {
            break;
          }
          else
          {
            ++uOccurrence;
          }
        }

        if (uOccurrence >= 3u)
        {
          // Merge
          m_auPlacedElements[j][i] += 1u;
          for (uint32 l = j + 1; l < k; ++l)
          {
            m_auPlacedElements[l][i] = 0u;
          }

          bMerged = true;
        }
      }
    }
  }

  return bMerged;
}

// ------------------------

Texture2D* CGame::GetTextureByElement(uint8 _uType)
{
  switch (_uType)
  {
    case 1:
    {
      return m_spTriangle.get();
    }
    case 2:
    {
      return m_spSquare.get();
    }
    case 3:
    {
      return m_spPentagon.get();
    }
    case 4:
    {
      return m_spCircle.get();
    }
    default:
    {

    }
  }

  return nullptr;
}

// ------------------------