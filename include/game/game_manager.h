#pragma once

#include "raylib/raylib.h"
#include "base/types.h"
#include "base/maths.h"
#include "game/game.h"
#include "game/scene.h"

#include <memory>
#include <string>
#include <vector>

class CGameManager
{
public:
  
  static constexpr uint32 sm_uWindowWidth = 384;
  static constexpr uint32 sm_uWindowHeight = 682u;

  static constexpr uint32 sm_uRenderFactor = 2u;

  static constexpr uint32 sm_uVirtualWindowWidth = sm_uWindowWidth * sm_uRenderFactor;
  static constexpr uint32 sm_uVirtualWindowHeight = sm_uWindowHeight * sm_uRenderFactor;

  enum class EScenes : uint8
  {
    GAME = 0,
    INVENTORY,
    POTIONS,

    COUNT,
  };

  enum class EPotion : uint8
  {
    POTION_0 = 0,
    POTION_1,

    COUNT
  };

  static CGameManager& GetInstance()
  {
    static CGameManager oInstance;
    return oInstance;
  }

  bool WantClose() const { return m_bWantClose; }
  void Init();
  void Destroy();
  void Loop();
  void Draw();
  CVector2D GetMousePos() const { return { (float)(GetMouseX() / sm_uRenderFactor), (float)(GetMouseY() / sm_uRenderFactor) }; }
  CVector2D GetMousePosSnappedToBackground(int32 _iSnappedTo = 32u)
  {
    // Ints to avoid overflows
    CVector2D v2SnappedPos(GetMousePos());
    //v2SnappedPos.x = (((int32)v2SnappedPos.x + _iSnappedTo / 2) / _iSnappedTo) * _iSnappedTo;
    //v2SnappedPos.y = (((int32)v2SnappedPos.y + _iSnappedTo / 2) / _iSnappedTo) * _iSnappedTo;
    v2SnappedPos.x = ((int32)v2SnappedPos.x / _iSnappedTo) * _iSnappedTo;
    v2SnappedPos.y = ((int32)v2SnappedPos.y / _iSnappedTo) * _iSnappedTo;
    v2SnappedPos.x = Clamp(v2SnappedPos.x, (float)CGame::sm_uBackgroundXPos, (float)CGame::sm_uBackgroundXPos + (float)CGame::sm_uBackgroundWidth);
    return v2SnappedPos;
  }

  std::shared_ptr<Texture2D> GetTextureByElement(uint8 _uType);
  Color GetColorByElement(uint8 _uType);
  const uint32* GetInventory() const { return m_auInventory; }
  const std::vector<uint8>* GetPotionsRecipes() const { return m_vctPotionsRecipes; }
  std::pair<EPotion, uint8>* GetPotions() { return m_aoPotions; }
  std::shared_ptr<Texture2D> GetPotionTexture() const { return m_spPotion; }
  Color GetPotionColor(uint8 _uType);

private:
  CGameManager();
  ~CGameManager();

  CScene* m_aoScenes[(uint8)EScenes::COUNT] = {nullptr};
  std::vector<uint8> m_vctPotionsRecipes[(uint8)EPotion::COUNT];
  std::pair<EPotion, uint8> m_aoPotions[(uint8)EPotion::COUNT];
  uint32 m_auInventory[CGame::sm_uNumGroupsElements * CGame::sm_uNumElementsPerTier] = { 0u };
  RenderTexture2D m_oRenderTarget;
  std::shared_ptr<Texture2D> m_spTriangle;
  std::shared_ptr<Texture2D> m_spSquare;
  std::shared_ptr<Texture2D> m_spPentagon;
  std::shared_ptr<Texture2D> m_spCircle;
  std::shared_ptr<Texture2D> m_spPotion;
  uint8 m_uCurrentScene = 0u;
  bool m_bWantClose = false;
};
