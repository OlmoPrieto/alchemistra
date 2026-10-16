#include "game/game.h"

#include <random>
#include <chrono>
#include <cassert>>

#include "ui/ui.h"

// ------------------------------------------------
// ------------------------------------------------

// ------------------------

CGame::CGame()
{
  memset(m_auPlacedElements, 0, sizeof(uint8) * sm_uMaxHorizontalElements * sm_uMaxVerticalElements);
  InitializeRandom();

  // Init element distribution by weight
  //  The formula for the distribution is:
  //  TotalWeigth = Tier0NumElements * Tier0Weight + Tier1NumElements * Tier1Weight + Tier2NumElements * Tier2Weight + Tier3NumElements * Tier3Weight
  //  And then: Tier0Weigth / TotalWeight = % per element
  //  Eg. 70.0 -> 69.65%
  //      22.0 -> 21.89%
  //       7.0 ->  7.46%
  //       0.5 ->  1.00%
  static_assert(sm_uNumElements == 16u);
  std::vector<double> vctWeigths(sm_uNumElements);
  for (uint32 i = 0; i < sm_uNumElements; ++i)
  {
    switch (i % 4)
    {
      case 0: vctWeigths[i] = 70.0; break; // Basic tier probability
      case 1: vctWeigths[i] = 20.0; break;
      case 2: vctWeigths[i] =  5.0; break;
      case 3: vctWeigths[i] =  0.2; break;
    }
  }

  m_oElementRandomDistribution = std::discrete_distribution<uint32>(vctWeigths.begin(), vctWeigths.end());
}

// ------------------------

CGame::~CGame()
{
  if (m_spBackground && m_spBackground.use_count() == 0)
  {
    UnloadTexture(*m_spBackground.get());
  }
  if (m_spTriangle && m_spTriangle.use_count() == 0)
  {
    UnloadTexture(*m_spTriangle.get());
  }
  if (m_spSquare && m_spSquare.use_count() == 0)
  {
    UnloadTexture(*m_spSquare.get());
  }
  if (m_spPentagon && m_spPentagon.use_count() == 0)
  {
    UnloadTexture(*m_spPentagon.get());
  }
  if (m_spCircle && m_spCircle.use_count() == 0)
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

  //m_oPendingElements = { 1, 5};
  m_oPendingElements = GetRandomPendingElements();
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
  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && 
    v2MousePos.x >= sm_uBackgroundXPos && v2MousePos.x <= sm_uBackgroundXPos + sm_uBackgroundWidth &&
    v2MousePos.y >= sm_uBackgroundYPos && v2MousePos.y <= sm_uBackgroundYPos + sm_uBackgroundHeight
  )
  {
    // Save state first
    printf("Saving game state %u\n", (uint32)m_vctPreviousStates.size());
    if (m_uPreviousGameState >= m_vctPreviousStates.size())
    {
      m_vctPreviousStates.resize(m_uPreviousGameState);
    }
    m_uPreviousGameState = m_vctPreviousStates.size();
    m_uCurrentGameState = UINT32_MAX;
    m_vctPreviousStates.emplace_back();
    m_vctPreviousStates.back().m_uPendingElementsState = (uint8)m_ePendingElementsState;
    m_vctPreviousStates.back().m_uPendingElement0 = m_oPendingElements.first;
    m_vctPreviousStates.back().m_uPendingElement1 = m_oPendingElements.second;
    memcpy(m_vctPreviousStates.back().m_auPlacedElements, m_auPlacedElements, sizeof(uint8) * sm_uMaxHorizontalElements * sm_uMaxVerticalElements);
    PrintGrid(m_vctPreviousStates.back().m_auPlacedElements);

    // Drop elements
    std::pair<CVector2D, CVector2D> oPendingElementsPos = GetPendingElementsPosition();
    uint32 uElement0Column = 9999u;
    uint32 uElement1Column = 9999u;
    uint8 uElement0 = 0xff;
    uint8 uElement1 = 0xff;
    switch (m_ePendingElementsState)
    {
      case EPendingElementsState::WEST:
      {
        uElement0Column = (uint32)(((int32)oPendingElementsPos.first.x - 32) / 32);
        uElement1Column = uElement0Column + 1;
        uElement0 = m_oPendingElements.first;
        uElement1 = m_oPendingElements.second;
        break;
      }
      case EPendingElementsState::NORTH:
      {
        uElement0Column = (oPendingElementsPos.first.x - 32) / 32;
        uElement1Column = uElement0Column;
        uElement0 = m_oPendingElements.second;
        uElement1 = m_oPendingElements.first;
        break;
      }
      case EPendingElementsState::EAST:
      {
        uElement0Column = (uint32)(((int32)oPendingElementsPos.second.x - 32) / 32);
        uElement1Column = uElement0Column + 1;
        uElement0 = m_oPendingElements.second;
        uElement1 = m_oPendingElements.first;
        break;
      }
      case EPendingElementsState::SOUTH:
      {
        uElement0Column = (oPendingElementsPos.first.x - 32) / 32;
        uElement1Column = uElement0Column;
        uElement0 = m_oPendingElements.first;
        uElement1 = m_oPendingElements.second;
        break;
      }
    }

    printf("Dropping on: ");
    if (uElement0Column != 9999u && uElement1Column != 9999u)
    {
      for (uint32 i = 0; i < sm_uMaxVerticalElements; ++i)
      {
        if (m_auPlacedElements[i][uElement0Column] == 0)
        {
          // Found a place to put the element
          m_auPlacedElements[i][uElement0Column] = uElement0;
          printf("[%u|%u] ", i, uElement0Column);
          ++m_uPlacedElements;
          break;
        }
      }
    
      for (uint32 i = 0; i < sm_uMaxVerticalElements; ++i)
      {
        if (m_auPlacedElements[i][uElement1Column] == 0)
        {
          // Found a place to put the element
          m_auPlacedElements[i][uElement1Column] = uElement1;
          printf("and [%u|%u]\n", i, uElement1Column);
          ++m_uPlacedElements;
          break;
        }
      }
    }

    // Check for merges
    while (CheckAndMerge())
    {
      printf("Merged!\n");
    }

    // Regenerate pending elements
    m_oPendingElements = GetRandomPendingElements();
  }

  if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
  {
    // Rotate pending elements clockwise
    m_ePendingElementsState = (EPendingElementsState)(((uint8)m_ePendingElementsState + 1) & ((uint8)EPendingElementsState::COUNT - 1));
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
        DrawTexture(*pTargetTexture, v2ElementPos.x, v2ElementPos.y, GetColorByElement(m_auPlacedElements[i][j]));
      }
    }
  }
#endif

  // Draw pending elements
  std::pair<CVector2D, CVector2D> oPendingElementsPos = GetPendingElementsPosition();
  Texture2D* pTargetTexture = GetTextureByElement(m_oPendingElements.first);
  /*DrawTexture(*pTargetTexture, 
    Clamp(oPendingElementsPos.first.x, sm_uBackgroundXPos, sm_uBackgroundXPos + sm_uBackgroundWidth), 
    oPendingElementsPos.first.y, 
    GetColorByElement(m_oPendingElements.first));*/
  DrawTexture(*pTargetTexture,
    oPendingElementsPos.first.x,
    oPendingElementsPos.first.y,
    GetColorByElement(m_oPendingElements.first));
  pTargetTexture = GetTextureByElement(m_oPendingElements.second);
  /*DrawTexture(*pTargetTexture, 
    Clamp(oPendingElementsPos.second.x, sm_uBackgroundXPos, sm_uBackgroundXPos + sm_uBackgroundWidth),
    oPendingElementsPos.second.y, 
    GetColorByElement(m_oPendingElements.second));*/
  DrawTexture(*pTargetTexture,
    oPendingElementsPos.second.x,
    oPendingElementsPos.second.y,
    GetColorByElement(m_oPendingElements.second));

  // UI
  if (CButton("Prev.", CVector2D()))
  {
    if (m_uCurrentGameState == UINT32_MAX)
    {
      m_oCurrentGameState.m_uPendingElementsState = (uint8)m_ePendingElementsState;
      m_oCurrentGameState.m_uPendingElement0 = m_oPendingElements.first;
      m_oCurrentGameState.m_uPendingElement1 = m_oPendingElements.second;
      memcpy(m_oCurrentGameState.m_auPlacedElements, m_auPlacedElements, sizeof(uint8) * sm_uMaxHorizontalElements * sm_uMaxVerticalElements);
      m_uCurrentGameState = m_vctPreviousStates.size();
    }

    printf("Going to previous state: %u\n", m_uPreviousGameState);
    PrintGrid(m_vctPreviousStates[m_uPreviousGameState].m_auPlacedElements);

    m_ePendingElementsState = (EPendingElementsState)(m_vctPreviousStates[m_uPreviousGameState].m_uPendingElementsState);
    m_oPendingElements = { m_vctPreviousStates[m_uPreviousGameState].m_uPendingElement0, m_vctPreviousStates[m_uPreviousGameState].m_uPendingElement1 };
    memcpy(m_auPlacedElements, m_vctPreviousStates[m_uPreviousGameState].m_auPlacedElements, sizeof(uint8) * sm_uMaxHorizontalElements * sm_uMaxVerticalElements);

    m_uPreviousGameState = Clamp((int32)m_uPreviousGameState - 1, 0, m_vctPreviousStates.size());
  }
  const CVector2D v2NextButtonSize = CButton::CalculateSize("Next");
  if (CButton("Next", CVector2D(sm_uWindowWidth - v2NextButtonSize.x, 0.0f)))
  {
    if (m_uPreviousGameState < m_vctPreviousStates.size() - 1)
    {
      m_uPreviousGameState = Clamp((int32)m_uPreviousGameState + 1, 0, m_vctPreviousStates.size() - 1);
      printf("Going to next state: %u\n", m_uPreviousGameState);
      PrintGrid(m_vctPreviousStates[m_uPreviousGameState].m_auPlacedElements);

      m_ePendingElementsState = (EPendingElementsState)(m_vctPreviousStates[m_uPreviousGameState].m_uPendingElementsState);
      m_oPendingElements = { m_vctPreviousStates[m_uPreviousGameState].m_uPendingElement0, m_vctPreviousStates[m_uPreviousGameState].m_uPendingElement1 };
      memcpy(m_auPlacedElements, m_vctPreviousStates[m_uPreviousGameState].m_auPlacedElements, sizeof(uint8) * sm_uMaxHorizontalElements * sm_uMaxVerticalElements);
    }
    else
    {
      printf("Going to the present state\n");
      m_ePendingElementsState   = (EPendingElementsState)m_oCurrentGameState.m_uPendingElementsState;
      m_oPendingElements.first  = m_oCurrentGameState.m_uPendingElement0;
      m_oPendingElements.second = m_oCurrentGameState.m_uPendingElement1;
      memcpy(m_auPlacedElements, m_oCurrentGameState.m_auPlacedElements, sizeof(uint8) * sm_uMaxHorizontalElements * sm_uMaxVerticalElements);
    }
  }

  // --------------
  EndTextureMode();

  // -------- END    REGULAR  DRAWING     --------
  // -------- BEGIN RENDER TARGET DRAWING --------

  BeginDrawing();
  ClearBackground(BLACK);
  // --------------

  // Flip texture on Y axis (raylib draws render textures inverted by default)
  Rectangle oSourceRect = { 0.0f, 0.0f, (float)m_oRenderTarget.texture.width, -(float)m_oRenderTarget.texture.height };

  // DestRect size will be the total size of the current physical window
  Rectangle oDestRect = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };

  // Draw virtual canvas stretching perfectly in the big window
  DrawTexturePro(m_oRenderTarget.texture, oSourceRect, oDestRect, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);

  // --------------
  EndDrawing();
}

// ------------------------

bool CGame::CheckAndMerge()
{
  bool bMerged = false;

  // TODO: make the merged cell to be the one where you placed the element???

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

  if (bMerged)
    ApplyPhysics();

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

  if (bMerged)
    ApplyPhysics();

  return bMerged;
}

// ------------------------

void CGame::ApplyPhysics()
{
  // Start checking by columns
  for (uint32 i = 0; i < sm_uMaxHorizontalElements; ++i)
  {
    // For each column, start from the bottom
    for (uint32 j = 0; j < sm_uMaxVerticalElements; ++j)
    {
      // If an element is empty 
      if (m_auPlacedElements[j][i] == 0u)
      {
        // Start looking for elements above
        uint32 k = j;
        bool bFoundElement = false;
        for (; k < sm_uMaxVerticalElements; ++k)
        {
          if (m_auPlacedElements[k][i] != 0u)
          {
            bFoundElement = true;
            break;
          }
        }

        if (bFoundElement)
        {
          for (uint32 l = j; l < sm_uMaxVerticalElements && k + (l - j) < sm_uMaxVerticalElements; ++l)
          {
            std::swap(m_auPlacedElements[l][i], m_auPlacedElements[k + (l - j)][i]);
          }

          j = k;
        }
      }
    }
  }
}

// ------------------------

void CGame::PrintGrid(const uint8 _auGrid[sm_uMaxVerticalElements][sm_uMaxHorizontalElements])
{
  printf("------------------------------------\n");
  for (int32 i = sm_uMaxVerticalElements - 1; i >= 0 ; --i)
  {
    for (uint32 j = 0; j < sm_uMaxHorizontalElements; ++j)
    {
      printf("%u ", _auGrid[(uint32)i][(uint32)j]);
    }
    printf("\n");
  }
  printf("------------------------------------\n");
}

// ------------------------

Texture2D* CGame::GetTextureByElement(uint8 _uType)
{
  switch (_uType)
  {
    case 1:
    case 5:
    case 9:
    case 13:
    {
      return m_spTriangle.get();
    }
    case 2:
    case 6:
    case 10:
    case 14:
    {
      return m_spSquare.get();
    }
    case 3:
    case 7:
    case 11:
    case 15:
    {
      return m_spPentagon.get();
    }
    case 4:
    case 8:
    case 12:
    case 16:
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

Color CGame::GetColorByElement(uint8 _uType)
{
  switch (_uType)
  {
    case 1:
    case 2:
    case 3:
    case 4:
    {
      return RED;
    }
    case 5:
    case 6:
    case 7:
    case 8:
    {
      return GREEN;
    }
    case 9 :
    case 10:
    case 11:
    case 12:
    {
      return BLUE;
    }
    case 13:
    case 14:
    case 15:
    case 16:
    {
      return YELLOW;
    }
    default:
    {

    }
  }

  return PURPLE;
}

// ------------------------

std::pair<uint8, uint8> CGame::GetRandomPendingElements()
{
  return { m_oElementRandomDistribution(GetRandomEngine()) + 1u, m_oElementRandomDistribution(GetRandomEngine()) + 1u};
}

// ------------------------

std::pair<CVector2D, CVector2D> CGame::GetPendingElementsPosition()
{
  std::pair<CVector2D, CVector2D> oPositions;
  CVector2D v2SnappedMousePos = GetMousePosSnappedToBackground();
  // Dont allow X leftmost and rightmost pos to exceed the background (which is aligned to sm_uBackgroundXPos)
  switch (m_ePendingElementsState)
  {
    case EPendingElementsState::WEST:
    {
      v2SnappedMousePos.x = Clamp(v2SnappedMousePos.x, sm_uBackgroundXPos * 2, sm_uWindowWidth - sm_uBackgroundXPos * 2);
      oPositions.first.x  = v2SnappedMousePos.x - 32.0f;
      oPositions.first.y  = sm_uPendingElementHeight;
      oPositions.second.x = v2SnappedMousePos.x;
      oPositions.second.y = sm_uPendingElementHeight;
      break;
    }
    case EPendingElementsState::NORTH:
    {
      v2SnappedMousePos.x = Clamp(v2SnappedMousePos.x, sm_uBackgroundXPos, sm_uWindowWidth - sm_uBackgroundXPos * 2);
      oPositions.first.x  = v2SnappedMousePos.x;
      oPositions.first.y  = sm_uPendingElementHeight - 32.0f;
      oPositions.second.x = v2SnappedMousePos.x;
      oPositions.second.y = sm_uPendingElementHeight;
      break;
    }
    case EPendingElementsState::EAST:
    {
      v2SnappedMousePos.x = Clamp(v2SnappedMousePos.x, sm_uBackgroundXPos, sm_uWindowWidth - sm_uBackgroundXPos * 2);
      oPositions.first.x  = Clamp(v2SnappedMousePos.x, sm_uBackgroundXPos * 2, sm_uWindowWidth - sm_uBackgroundXPos);
      oPositions.first.y  = sm_uPendingElementHeight;
      oPositions.second.x = oPositions.first.x - 32.0f;
      oPositions.second.y = sm_uPendingElementHeight;
      break;
    }
    case EPendingElementsState::SOUTH:
    {
      v2SnappedMousePos.x = Clamp(v2SnappedMousePos.x, sm_uBackgroundXPos, sm_uWindowWidth - sm_uBackgroundXPos * 2);
      oPositions.first.x  = v2SnappedMousePos.x;
      oPositions.first.y  = sm_uPendingElementHeight;
      oPositions.second.x = v2SnappedMousePos.x;
      oPositions.second.y = sm_uPendingElementHeight - 32.0f;
      break;
    }
    case EPendingElementsState::COUNT: break;
  }

  return oPositions;
}

// ------------------------
