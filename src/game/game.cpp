#include "game/game.h"

#include <random>
#include <chrono>
#include <cassert>
#include <fstream>
#include <filesystem>

#include "ui/ui.h"
#include "game/game_manager.h"

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
  //static_assert(sm_uNumElements == 16u);
  std::vector<double> vctWeigths(sm_uNumElements);
  for (uint32 i = 0; i < sm_uNumElements; ++i)
  {
    switch (i % 4)  // Number of tiers
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

  SaveBackup();
}

// ------------------------

void CGame::Init()
{
  //Image oWhiteImage = GenImageColor(CGameManager::sm_uWindowWidth - 64u, CGameManager::sm_uWindowHeight * 0.5f, Color{200, 200, 200, 255});
  Image oWhiteImage = GenImageColor(CGame::sm_uBackgroundWidth, CGame::sm_uBackgroundHeight, Color{ 200, 200, 200, 255 });
  m_spBackground = std::make_shared<Texture2D>(LoadTextureFromImage(oWhiteImage));
  UnloadImage(oWhiteImage);

  //m_oPendingElements = { 1, 5};
  m_oPendingElements = GetRandomPendingElements();
}

// ------------------------

void CGame::Update(float _fDt)
{
  if (IsKeyPressed(KEY_R))
  {
    memset(m_auPlacedElements, 0, sizeof(uint8) * sm_uMaxHorizontalElements * sm_uMaxVerticalElements);
  }

  const CVector2D v2MousePos = CGameManager::GetInstance().GetMousePos();
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
      uElement0Column = (uint32)(((int32)oPendingElementsPos.first.x - sm_uBackgroundXPos) / sm_uElementPixelSize);
      uElement1Column = uElement0Column + 1;
      uElement0 = m_oPendingElements.first;
      uElement1 = m_oPendingElements.second;
      break;
    }
    case EPendingElementsState::NORTH:
    {
      uElement0Column = (oPendingElementsPos.first.x - sm_uBackgroundXPos) / sm_uElementPixelSize;
      uElement1Column = uElement0Column;
      uElement0 = m_oPendingElements.second;
      uElement1 = m_oPendingElements.first;
      break;
    }
    case EPendingElementsState::EAST:
    {
      uElement0Column = (uint32)(((int32)oPendingElementsPos.second.x - sm_uBackgroundXPos) / sm_uElementPixelSize);
      uElement1Column = uElement0Column + 1;
      uElement0 = m_oPendingElements.second;
      uElement1 = m_oPendingElements.first;
      break;
    }
    case EPendingElementsState::SOUTH:
    {
      uElement0Column = (oPendingElementsPos.first.x - sm_uBackgroundXPos) / sm_uElementPixelSize;
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
  assert(IsWindowReady());

  // Draw background
  DrawTexture(*m_spBackground.get(), sm_uBackgroundXPos, sm_uBackgroundYPos, WHITE);

  int32 iLineHeight = sm_uBackgroundYPos + (sm_uBackgroundHeight - sm_uElementPixelSize * sm_uMaxVerticalElements);
  DrawLine(sm_uBackgroundXPos, iLineHeight, sm_uBackgroundXPos + sm_uBackgroundWidth, iLineHeight, Color{ 100, 100, 100, 255 });

  // Draw placed elements
  for (uint32 i = 0; i < sm_uMaxVerticalElements; ++i)
  {
    for (uint32 j = 0; j < sm_uMaxHorizontalElements; ++j)
    {
      if (m_auPlacedElements[i][j] != 0u)
      {
        std::shared_ptr<Texture2D> spTargetTexture = CGameManager::GetInstance().GetTextureByElement(m_auPlacedElements[i][j]);
        const CVector2D v2ElementPos(sm_uBackgroundXPos + j * sm_uElementPixelSize, (sm_uBackgroundYPos + sm_uBackgroundHeight - sm_uElementPixelSize) - i * sm_uElementPixelSize);
        DrawTexture(*spTargetTexture.get(), v2ElementPos.x, v2ElementPos.y, CGameManager::GetInstance().GetColorByElement(m_auPlacedElements[i][j]));
      }
    }
  }

  // Draw pending elements
  std::pair<CVector2D, CVector2D> oPendingElementsPos = GetPendingElementsPosition();
  std::shared_ptr<Texture2D> spTargetTexture = CGameManager::GetInstance().GetTextureByElement(m_oPendingElements.first);
  DrawTexture(*spTargetTexture.get(),
    oPendingElementsPos.first.x,
    oPendingElementsPos.first.y,
    CGameManager::GetInstance().GetColorByElement(m_oPendingElements.first));
  spTargetTexture = CGameManager::GetInstance().GetTextureByElement(m_oPendingElements.second);
  DrawTexture(*spTargetTexture.get(),
    oPendingElementsPos.second.x,
    oPendingElementsPos.second.y,
    CGameManager::GetInstance().GetColorByElement(m_oPendingElements.second));

  // -------- UI -------- 

  if (CButton("Prev.", CVector2D()))
  {
    if (m_uCurrentGameState == UINT32_MAX)
    {
      SaveCurrentState();
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
  if (CButton("Next", CVector2D(CGameManager::sm_uWindowWidth - v2NextButtonSize.x, 0.0f)))
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
      if (m_uCurrentGameState != UINT32_MAX)
      {
        printf("Going to the present state\n");
        m_ePendingElementsState   = (EPendingElementsState)m_oCurrentGameState.m_uPendingElementsState;
        m_oPendingElements.first  = m_oCurrentGameState.m_uPendingElement0;
        m_oPendingElements.second = m_oCurrentGameState.m_uPendingElement1;
        memcpy(m_auPlacedElements, m_oCurrentGameState.m_auPlacedElements, sizeof(uint8) * sm_uMaxHorizontalElements * sm_uMaxVerticalElements);

        m_uCurrentGameState = UINT32_MAX;
      }
    }
  }

  static std::string sLoadFileBuffer;
  SUIButtonConfig oSaveButtonConfig = CButton::GetDefaultConfig();
  oSaveButtonConfig.m_v2Size.x = 10.0f;
  if (CButton("SAVE", CVector2D(CGameManager::sm_uWindowWidth * 0.5f - CButton::GetDefaultConfig().m_v2Size.x * 0.25f, 0.0f), oSaveButtonConfig))
  {
    SaveGame(sLoadFileBuffer.empty() ? "saves/latest.txt" : sLoadFileBuffer);
  }

  const CVector2D v2InputSize(250.0f, 36.0f);
  SUIInputTextConfig oLoadFileInputConfig = CInputText::GetDefaultConfig();
  oLoadFileInputConfig.m_v2Size = v2InputSize;
  if (CInputText(&sLoadFileBuffer, CVector2D(CGameManager::sm_uWindowWidth * 0.5f - v2InputSize.x * 0.5f, CGameManager::sm_uWindowHeight - v2InputSize.y * 3 + v2InputSize.y * 1.5f), oLoadFileInputConfig))
  {

  }
  SUIButtonConfig oDeleteButtonConfig = CButton::GetDefaultConfig();
  oDeleteButtonConfig.m_v2Size.x = 7.5f;
  if (CButton("X", CVector2D(CGameManager::sm_uWindowWidth * 0.5f - v2InputSize.x * 0.6f - oDeleteButtonConfig.m_v2Size.x * 2.0f, CGameManager::sm_uWindowHeight - v2InputSize.y * 3 + v2InputSize.y * 1.5f), oDeleteButtonConfig))
  {
    sLoadFileBuffer.clear();
  }
  SUIButtonConfig oLoadButtonConfig = CButton::GetDefaultConfig();
  oLoadButtonConfig.m_v2Size.x = 1.0f;
  oLoadButtonConfig.m_iFontSize = 10;
  if (CButton("LOAD", CVector2D(CGameManager::sm_uWindowWidth * 0.5f + v2InputSize.x * 0.53f, CGameManager::sm_uWindowHeight - v2InputSize.y * 3 + v2InputSize.y * 1.5f), oLoadButtonConfig))
  {
    LoadGame(sLoadFileBuffer);
  }

  const CVector2D v2QuitButtonSize = CButton::CalculateSize("QUIT");
  if (CButton("QUIT", CVector2D(CGameManager::sm_uWindowWidth * 0.5f - v2QuitButtonSize.x * 0.5f, CGameManager::sm_uWindowHeight - v2InputSize.y * 2.75)))
  {
    End();
  }
}

// ------------------------

void CGame::Start()
{
  m_bStarted = true;
}

// ------------------------

void CGame::End()
{
  m_bStarted = false;
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
          ++m_auCombinations[m_auPlacedElements[i][j] - 1];
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
          ++m_auCombinations[m_auPlacedElements[j][i] - 1];
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

std::pair<uint8, uint8> CGame::GetRandomPendingElements()
{
  return { m_oElementRandomDistribution(GetRandomEngine()) + 1u, m_oElementRandomDistribution(GetRandomEngine()) + 1u};
}

// ------------------------

std::pair<CVector2D, CVector2D> CGame::GetPendingElementsPosition()
{
  std::pair<CVector2D, CVector2D> oPositions;
  CVector2D v2SnappedMousePos = CGameManager::GetInstance().GetMousePosSnappedToBackground();
  // Dont allow X leftmost and rightmost pos to exceed the background (which is aligned to sm_uBackgroundXPos)
  switch (m_ePendingElementsState)
  {
    case EPendingElementsState::WEST:
    {
      v2SnappedMousePos.x = Clamp(v2SnappedMousePos.x, sm_uBackgroundXPos + sm_uElementPixelSize, sm_uBackgroundXPos + sm_uBackgroundWidth - sm_uElementPixelSize);
      oPositions.first.x  = v2SnappedMousePos.x - 32.0f;
      oPositions.first.y  = sm_uPendingElementHeight;
      oPositions.second.x = v2SnappedMousePos.x;
      oPositions.second.y = sm_uPendingElementHeight;
      break;
    }
    case EPendingElementsState::NORTH:
    {
      v2SnappedMousePos.x = Clamp(v2SnappedMousePos.x, sm_uBackgroundXPos, sm_uBackgroundXPos + sm_uBackgroundWidth - sm_uElementPixelSize);
      oPositions.first.x  = v2SnappedMousePos.x;
      oPositions.first.y  = sm_uPendingElementHeight - 32.0f;
      oPositions.second.x = v2SnappedMousePos.x;
      oPositions.second.y = sm_uPendingElementHeight;
      break;
    }
    case EPendingElementsState::EAST:
    {
      v2SnappedMousePos.x = Clamp(v2SnappedMousePos.x, sm_uBackgroundXPos, sm_uBackgroundXPos + sm_uBackgroundWidth - sm_uElementPixelSize);
      oPositions.first.x  = Clamp(v2SnappedMousePos.x, sm_uBackgroundXPos + sm_uElementPixelSize, CGameManager::sm_uWindowWidth - sm_uBackgroundXPos);
      oPositions.first.y  = sm_uPendingElementHeight;
      oPositions.second.x = oPositions.first.x - 32.0f;
      oPositions.second.y = sm_uPendingElementHeight;
      break;
    }
    case EPendingElementsState::SOUTH:
    {
      v2SnappedMousePos.x = Clamp(v2SnappedMousePos.x, sm_uBackgroundXPos, sm_uBackgroundXPos + sm_uBackgroundWidth - sm_uElementPixelSize);
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

void CGame::DrawCombinedElements()
{
  std::shared_ptr<Texture2D> spTriangle = CGameManager::GetInstance().GetTextureByElement(0);
  std::shared_ptr<Texture2D> spSquare   = CGameManager::GetInstance().GetTextureByElement(1);
  std::shared_ptr<Texture2D> spPentagon = CGameManager::GetInstance().GetTextureByElement(2);
  std::shared_ptr<Texture2D> spCircle   = CGameManager::GetInstance().GetTextureByElement(3);

  DrawTexture(*spTriangle.get(), sm_uBackgroundXPos, sm_uBackgroundYPos + sm_uBackgroundHeight, CGameManager::GetInstance().GetColorByElement(1));
  DrawTexture(*spSquare  .get(), sm_uBackgroundXPos, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 1.25f, CGameManager::GetInstance().GetColorByElement(2));
  DrawTexture(*spPentagon.get(), sm_uBackgroundXPos, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 2.50f, CGameManager::GetInstance().GetColorByElement(3));
  DrawTexture(*spCircle  .get(), sm_uBackgroundXPos, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 3.75f, CGameManager::GetInstance().GetColorByElement(4));

  DrawTexture(*spTriangle.get(), sm_uBackgroundXPos + sm_uElementPixelSize * 3, sm_uBackgroundYPos + sm_uBackgroundHeight, CGameManager::GetInstance().GetColorByElement(5));
  DrawTexture(*spSquare  .get(), sm_uBackgroundXPos + sm_uElementPixelSize * 3, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 1.25f, CGameManager::GetInstance().GetColorByElement(6));
  DrawTexture(*spPentagon.get(), sm_uBackgroundXPos + sm_uElementPixelSize * 3, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 2.50f, CGameManager::GetInstance().GetColorByElement(7));
  DrawTexture(*spCircle  .get(), sm_uBackgroundXPos + sm_uElementPixelSize * 3, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 3.75f, CGameManager::GetInstance().GetColorByElement(8));

  DrawTexture(*spTriangle.get(), sm_uBackgroundXPos + sm_uElementPixelSize * 6, sm_uBackgroundYPos + sm_uBackgroundHeight, CGameManager::GetInstance().GetColorByElement(9));
  DrawTexture(*spSquare  .get(), sm_uBackgroundXPos + sm_uElementPixelSize * 6, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 1.25f, CGameManager::GetInstance().GetColorByElement(10));
  DrawTexture(*spPentagon.get(), sm_uBackgroundXPos + sm_uElementPixelSize * 6, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 2.50f, CGameManager::GetInstance().GetColorByElement(11));
  DrawTexture(*spCircle  .get(), sm_uBackgroundXPos + sm_uElementPixelSize * 6, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 3.75f, CGameManager::GetInstance().GetColorByElement(12));

  char sScoreBuffer[8] = { '\0' };

  sprintf(sScoreBuffer, "x %u", m_auCombinations[0]);
  DrawText(sScoreBuffer, sm_uBackgroundXPos + sm_uElementPixelSize * 1.1f, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 0.3f, GetFontDefault().baseSize * 2, WHITE);
  sprintf(sScoreBuffer, "x %u", m_auCombinations[1]);
  DrawText(sScoreBuffer, sm_uBackgroundXPos + sm_uElementPixelSize * 1.1f, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 1.55f, GetFontDefault().baseSize * 2, WHITE);
  sprintf(sScoreBuffer, "x %u", m_auCombinations[2]);
  DrawText(sScoreBuffer, sm_uBackgroundXPos + sm_uElementPixelSize * 1.1f, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 2.80f, GetFontDefault().baseSize * 2, WHITE);
  sprintf(sScoreBuffer, "x %u", m_auCombinations[3]);
  DrawText(sScoreBuffer, sm_uBackgroundXPos + sm_uElementPixelSize * 1.1f, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 4.05f, GetFontDefault().baseSize * 2, WHITE);

  sprintf(sScoreBuffer, "x %u", m_auCombinations[4]);
  DrawText(sScoreBuffer, sm_uBackgroundXPos + sm_uElementPixelSize * 4.1f, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 0.3f, GetFontDefault().baseSize * 2, WHITE);
  sprintf(sScoreBuffer, "x %u", m_auCombinations[5]);
  DrawText(sScoreBuffer, sm_uBackgroundXPos + sm_uElementPixelSize * 4.1f, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 1.55f, GetFontDefault().baseSize * 2, WHITE);
  sprintf(sScoreBuffer, "x %u", m_auCombinations[6]);
  DrawText(sScoreBuffer, sm_uBackgroundXPos + sm_uElementPixelSize * 4.1f, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 2.80f, GetFontDefault().baseSize * 2, WHITE);
  sprintf(sScoreBuffer, "x %u", m_auCombinations[7]);
  DrawText(sScoreBuffer, sm_uBackgroundXPos + sm_uElementPixelSize * 4.1f, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 4.05f, GetFontDefault().baseSize * 2, WHITE);

  sprintf(sScoreBuffer, "x %u", m_auCombinations[8]);
  DrawText(sScoreBuffer, sm_uBackgroundXPos + sm_uElementPixelSize * 7.1f, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 0.3f, GetFontDefault().baseSize * 2, WHITE);
  sprintf(sScoreBuffer, "x %u", m_auCombinations[9]);
  DrawText(sScoreBuffer, sm_uBackgroundXPos + sm_uElementPixelSize * 7.1f, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 1.55f, GetFontDefault().baseSize * 2, WHITE);
  sprintf(sScoreBuffer, "x %u", m_auCombinations[10]);
  DrawText(sScoreBuffer, sm_uBackgroundXPos + sm_uElementPixelSize * 7.1f, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 2.80f, GetFontDefault().baseSize * 2, WHITE);
  sprintf(sScoreBuffer, "x %u", m_auCombinations[11]);
  DrawText(sScoreBuffer, sm_uBackgroundXPos + sm_uElementPixelSize * 7.1f, sm_uBackgroundYPos + sm_uBackgroundHeight + sm_uElementPixelSize * 4.05f, GetFontDefault().baseSize * 2, WHITE);
}

// ------------------------

void CGame::SaveGame(const std::string& _sPath)
{
  std::filesystem::path oPath(_sPath);
  std::ofstream oFile(oPath);
  if (oFile.is_open() == false)
  {
    assert(false);
    return;
  }

  oFile << (uint32)sm_uSaveDataVersion << "\n";
  oFile << (uint32)sm_uMaxVerticalElements << "\n";
  oFile << (uint32)sm_uMaxHorizontalElements << "\n";
  oFile << (uint32)m_vctPreviousStates.size() << "\n";
  for (uint32 i = 0; i < m_vctPreviousStates.size(); ++i)
  {
    oFile << (uint32)m_vctPreviousStates[i].m_uPendingElementsState << "\n";
    oFile << (uint32)m_vctPreviousStates[i].m_uPendingElement0 << "\n";
    oFile << (uint32)m_vctPreviousStates[i].m_uPendingElement1 << "\n";
    for (uint32 j = 0; j < CGame::sm_uMaxVerticalElements; ++j)
    {
      for (uint32 k = 0; k < CGame::sm_uMaxHorizontalElements; ++k)
      {
        oFile << (uint32)m_vctPreviousStates[i].m_auPlacedElements[j][k] << " ";
      }
      oFile << "\n";
    }
    oFile << "\n";
  }

  SaveCurrentState();
  oFile << (uint32)m_oCurrentGameState.m_uPendingElementsState << "\n";
  oFile << (uint32)m_oCurrentGameState.m_uPendingElement0 << "\n";
  oFile << (uint32)m_oCurrentGameState.m_uPendingElement1 << "\n";
  for (uint32 j = 0; j < CGame::sm_uMaxVerticalElements; ++j)
  {
    for (uint32 k = 0; k < CGame::sm_uMaxHorizontalElements; ++k)
    {
      oFile << (uint32)m_oCurrentGameState.m_auPlacedElements[j][k] << " ";
    }
    oFile << "\n";
  }

  printf("File saved on [%s]\n", _sPath.c_str());
}

// ------------------------

void CGame::SaveCurrentState()
{
  m_oCurrentGameState.m_uPendingElementsState = (uint8)m_ePendingElementsState;
  m_oCurrentGameState.m_uPendingElement0 = m_oPendingElements.first;
  m_oCurrentGameState.m_uPendingElement1 = m_oPendingElements.second;
  memcpy(m_oCurrentGameState.m_auPlacedElements, m_auPlacedElements, sizeof(uint8) * sm_uMaxHorizontalElements * sm_uMaxVerticalElements);
}

// ------------------------

void CGame::LoadGame(const std::string& _sPath)
{
  std::filesystem::path oPath(_sPath);
  std::ifstream oFile(oPath);
  if (oFile.is_open() == false)
  {
    assert(false);
    return;
  }

  uint32 uSaveDataVersion       = UINT32_MAX;
  uint32 uMaxVerticalElements   = UINT32_MAX;
  uint32 uMaxHorizontalElements = UINT32_MAX;
  uint32 uPreviousStatesSize    = UINT32_MAX;
  oFile >> uSaveDataVersion;        assert(uSaveDataVersion       == sm_uSaveDataVersion);
  oFile >> uMaxVerticalElements;    assert(uMaxVerticalElements   == sm_uMaxVerticalElements);
  oFile >> uMaxHorizontalElements;  assert(uMaxHorizontalElements == sm_uMaxHorizontalElements);
  oFile >> uPreviousStatesSize;
  m_vctPreviousStates.resize(uPreviousStatesSize);
  for (uint32 i = 0; i < m_vctPreviousStates.size(); ++i)
  {
    uint32 uTmp = UINT32_MAX;
    oFile >> uTmp;
    m_vctPreviousStates[i].m_uPendingElementsState = (uint8)uTmp;
    oFile >> uTmp;
    m_vctPreviousStates[i].m_uPendingElement0 = (uint8)uTmp;
    oFile >> uTmp;
    m_vctPreviousStates[i].m_uPendingElement1 = (uint8)uTmp;
    for (uint32 j = 0; j < sm_uMaxVerticalElements; ++j)
    {
      for (uint32 k = 0; k < sm_uMaxHorizontalElements; ++k)
      {
        oFile >> uTmp;
        m_vctPreviousStates[i].m_auPlacedElements[j][k] = (uint8)uTmp;
      }
    }
  }

  uint32 uTmp = UINT32_MAX;
  oFile >> uTmp;
  m_ePendingElementsState = (EPendingElementsState)uTmp;
  oFile >> uTmp;
  m_oPendingElements.first = (uint8)uTmp;
  oFile >> uTmp;
  m_oPendingElements.second = (uint8)uTmp;
  for (uint32 j = 0; j < sm_uMaxVerticalElements; ++j)
  {
    for (uint32 k = 0; k < sm_uMaxHorizontalElements; ++k)
    {
      oFile >> uTmp;
      m_auPlacedElements[j][k] = (uint8)uTmp;
    }
  }
}

// ------------------------

void CGame::SaveBackup()
{
  uint8 uCurrentBackup = 0;
  char sBuffer[128] = { '\0' };
  for (; uCurrentBackup < sm_uMaxBackups; ++uCurrentBackup)
  {
    sprintf(sBuffer, "%sbackup_%u.txt", sm_sBackupsPath, uCurrentBackup);
    if (std::filesystem::exists(sBuffer) == false)
    {
      break;
    }
  }

  if (uCurrentBackup < sm_uMaxBackups)
  {
    SaveGame(sBuffer);
  }
  else
  {
    // Need to overwrite the oldest one
    std::filesystem::file_time_type oCurrentTime = std::filesystem::file_time_type::clock::now();
    std::filesystem::file_time_type oOldestTime = oCurrentTime;
    uCurrentBackup = 0;
    uint8 uTargetBackup = 0;
    for (; uCurrentBackup < sm_uMaxBackups; ++uCurrentBackup)
    {
      sprintf(sBuffer, "%sbackup_%u.txt", sm_sBackupsPath, uCurrentBackup);
      std::filesystem::file_time_type oTime = std::filesystem::last_write_time(sBuffer);
      if (oTime < oOldestTime)
      {
        oOldestTime = oTime;
        uTargetBackup = uCurrentBackup;
      }
    }

    sprintf(sBuffer, "%sbackup_%u.txt", sm_sBackupsPath, uTargetBackup);
    SaveGame(sBuffer);
  }
}

// ------------------------
