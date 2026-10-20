#pragma once

#include <chrono>
#include <random>
#include <vector>

#include "base/types.h"
#include "base/maths.h"
#include "raylib/raylib.h"

class CGameManager;

class CGame
{
public:
  static constexpr uint32 sm_uElementPixelSize      = 32u;
  static constexpr uint32 sm_uMaxHorizontalElements = 8u;
  static constexpr uint32 sm_uMaxVerticalElements   = 6u;
  static constexpr uint32 sm_uBackgroundWidth       = sm_uElementPixelSize * sm_uMaxHorizontalElements;
  static constexpr uint32 sm_uBackgroundHeight      = sm_uElementPixelSize * sm_uMaxVerticalElements + sm_uElementPixelSize * 5u;
  static constexpr uint32 sm_uBackgroundXPos        = (384u - sm_uBackgroundWidth) / 2;
  static constexpr uint32 sm_uBackgroundYPos        = sm_uElementPixelSize * 3u;
  static constexpr uint32 sm_uPendingElementHeight  = sm_uBackgroundYPos - sm_uElementPixelSize;
  static constexpr uint8  sm_uNumElements           = 12u;

  CGame(const CGame&) = delete;
  void operator=(const CGame&) = delete;

  void Init();
  void Update(float _fDt);
  void Draw();

  bool CheckAndMerge();
  void ApplyPhysics(); // Tries to make floating elements occupy empty ones

  void PrintGrid(const uint8 _auGrid[sm_uMaxVerticalElements][sm_uMaxHorizontalElements]);

private:
  
  friend class CGameManager;

  // This will represent where the element on the left of the pending elements is
  enum class EPendingElementsState : uint8
  {
    WEST = 0,
    NORTH,
    EAST,
    SOUTH,

    COUNT = 4
  };

  struct SGameState
  {
    uint8 m_uPendingElementsState = 0;
    uint8 m_uPendingElement0 = 0;
    uint8 m_uPendingElement1 = 0;
    uint8 m_auPlacedElements[sm_uMaxVerticalElements][sm_uMaxHorizontalElements] = { 0 };
  };
  
  CGame();
  ~CGame();

  Color GetColorByElement(uint8 _uType);
  std::pair<uint8, uint8> GetRandomPendingElements();
  std::pair<CVector2D, CVector2D> GetPendingElementsPosition();
  void DrawCombinedElements();

  SGameState m_oCurrentGameState;
  std::vector<SGameState> m_vctPreviousStates;
  std::shared_ptr<Texture2D> m_spBackground;
  uint32 m_uCurrentGameState = UINT32_MAX;
  uint32 m_uPreviousGameState = 0u;
  // TODO: probably will need to add +2 to vertical, in case you have elements atop and try to dump 2 vertical one (need to represent them but the game will be over)
  uint8 m_auPlacedElements[sm_uMaxVerticalElements][sm_uMaxHorizontalElements] = { 0u };
  uint32 m_auCombinations[sm_uNumElements] = { 0u };  // Will store how many combinations of each element the player has made throughout a game
  std::pair<uint8, uint8> m_oPendingElements;
  std::discrete_distribution<uint32> m_oElementRandomDistribution;
  uint32 m_uPlacedElements = 0u;
  EPendingElementsState m_ePendingElementsState = EPendingElementsState::WEST;
};
