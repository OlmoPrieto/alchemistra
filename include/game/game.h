#pragma once

#include <chrono>
#include <random>
#include <vector>

#include "base/types.h"
#include "base/maths.h"
#include "raylib/raylib.h"

struct SNetStream;

class CGame
{
public:
  static constexpr uint32 sm_uWindowWidth   = 384;
  static constexpr uint32 sm_uWindowHeight  = 682u;

  static constexpr uint32 sm_uRenderFactor = 2u;

  static constexpr uint32 sm_uVirtualWindowWidth  = sm_uWindowWidth   * sm_uRenderFactor;
  static constexpr uint32 sm_uVirtualWindowHeight = sm_uWindowHeight  * sm_uRenderFactor;

  static constexpr uint32 sm_uMaxHorizontalElements = 10u;
  static constexpr uint32 sm_uMaxVerticalElements   = 8u;
  static constexpr uint32 sm_uBackgroundWidth       = sm_uWindowWidth - 32u * 2u; // Two sprites empty on the sides
  static constexpr uint32 sm_uBackgroundHeight      = sm_uWindowHeight - 32u * 5u - 32u * 3u;
  static constexpr uint32 sm_uBackgroundXPos        = 32u;
  static constexpr uint32 sm_uBackgroundYPos        = 32u * 3u;
  static constexpr uint32 sm_uPendingElementHeight  = sm_uBackgroundYPos - 32u;
  static constexpr uint8  sm_uNumElements           = 16u;

  static CGame& GetInstance()
  {
    static CGame oInstance;
    return oInstance;
  }

  CGame(const CGame&) = delete;
  void operator=(const CGame&) = delete;

  void Init();
  void Update(float _fDt);
  void Draw();
  bool WantClose() const { return m_bWantClose; }
  CVector2D GetMousePos() const { return { (float)(GetMouseX() / sm_uRenderFactor), (float)(GetMouseY() / sm_uRenderFactor)}; }

  bool CheckAndMerge();

private:

  // This will represent where the element on the left of the pending elements is
  enum class EPendingElementsState : uint8
  {
    WEST = 0,
    NORTH,
    EAST,
    SOUTH,

    COUNT = 4
  };
  
  CGame();
  ~CGame();

  Texture2D* GetTextureByElement(uint8 _uType);
  Color GetColorByElement(uint8 _uType);
  std::pair<uint8, uint8> GetRandomPendingElements();

  // TODO: probably will need to add +2 to vertical, in case you have elements atop and try to dump 2 vertical one (need to represent them but the game will be over)
  uint8 m_auPlacedElements[sm_uMaxVerticalElements][sm_uMaxHorizontalElements] = { 0u };
  std::pair<uint8, uint8> m_oPendingElements;
  RenderTexture2D m_oRenderTarget;
  std::shared_ptr<Texture2D> m_spBackground;
  std::shared_ptr<Texture2D> m_spTriangle;
  std::shared_ptr<Texture2D> m_spSquare;
  std::shared_ptr<Texture2D> m_spPentagon;
  std::shared_ptr<Texture2D> m_spCircle;
  std::discrete_distribution<uint32> m_oElementRandomDistribution;
  uint32 m_uPlacedElements = 0u;
  EPendingElementsState m_ePendingElementsState = EPendingElementsState::WEST;
  bool m_bWantClose = false;
};