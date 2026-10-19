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

  static constexpr uint32 sm_uElementPixelSize      = 32u;
  static constexpr uint32 sm_uMaxHorizontalElements = 8u;
  static constexpr uint32 sm_uMaxVerticalElements   = 6u;
  static constexpr uint32 sm_uBackgroundWidth       = sm_uElementPixelSize * sm_uMaxHorizontalElements;
  static constexpr uint32 sm_uBackgroundHeight      = sm_uElementPixelSize * sm_uMaxVerticalElements + sm_uElementPixelSize * 5u;
  static constexpr uint32 sm_uBackgroundXPos        = (sm_uWindowWidth - sm_uBackgroundWidth) / 2;
  static constexpr uint32 sm_uBackgroundYPos        = sm_uElementPixelSize * 3u;
  static constexpr uint32 sm_uPendingElementHeight  = sm_uBackgroundYPos - sm_uElementPixelSize;
  static constexpr uint8  sm_uNumElements           = 12u;
  static constexpr uint8  sm_uSaveDataVersion       = 0u;

  static constexpr const char* sm_sBackupsPath      = "saves/backups/";
  static constexpr uint8 sm_uMaxBackups             = 4u;

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
  CVector2D GetMousePosSnappedToBackground(int32 _iSnappedTo = 32u) 
  { 
    // Ints to avoid overflows
    CVector2D v2SnappedPos(GetMousePos());
    //v2SnappedPos.x = (((int32)v2SnappedPos.x + _iSnappedTo / 2) / _iSnappedTo) * _iSnappedTo;
    //v2SnappedPos.y = (((int32)v2SnappedPos.y + _iSnappedTo / 2) / _iSnappedTo) * _iSnappedTo;
    v2SnappedPos.x = ((int32)v2SnappedPos.x / _iSnappedTo) * _iSnappedTo;
    v2SnappedPos.y = ((int32)v2SnappedPos.y / _iSnappedTo) * _iSnappedTo;
    v2SnappedPos.x = Clamp(v2SnappedPos.x, (float)sm_uBackgroundXPos, (float)sm_uBackgroundXPos + (float)sm_uBackgroundWidth);
    return v2SnappedPos;
  }

  bool CheckAndMerge();
  void ApplyPhysics(); // Tries to make floating elements occupy empty ones

  void PrintGrid(const uint8 _auGrid[sm_uMaxVerticalElements][sm_uMaxHorizontalElements]);
  void SaveGame(const std::string& _sPath);
  void SaveCurrentState();
  void LoadGame(const std::string& _sPath);

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

  struct SGameState
  {
    uint8 m_uPendingElementsState = 0;
    uint8 m_uPendingElement0 = 0;
    uint8 m_uPendingElement1 = 0;
    uint8 m_auPlacedElements[sm_uMaxVerticalElements][sm_uMaxHorizontalElements] = { 0 };
  };
  
  CGame();
  ~CGame();

  Texture2D* GetTextureByElement(uint8 _uType);
  Color GetColorByElement(uint8 _uType);
  std::pair<uint8, uint8> GetRandomPendingElements();
  std::pair<CVector2D, CVector2D> GetPendingElementsPosition();
  void SaveBackup();
  void DrawCombinedElements();

  //std::vector<uint8[3 + sm_uMaxVerticalElements * sm_uMaxHorizontalElements]> m_vctPreviousStates;
  std::vector<SGameState> m_vctPreviousStates;
  SGameState m_oCurrentGameState;
  uint32 m_uCurrentGameState = UINT32_MAX;
  uint32 m_uPreviousGameState = 0u;

  // TODO: probably will need to add +2 to vertical, in case you have elements atop and try to dump 2 vertical one (need to represent them but the game will be over)
  uint8 m_auPlacedElements[sm_uMaxVerticalElements][sm_uMaxHorizontalElements] = { 0u };
  uint32 m_auCombinations[sm_uNumElements] = { 0u };  // Will store how many combinations of each element the player has made throughout a game
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