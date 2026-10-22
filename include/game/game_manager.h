#pragma once

#include "raylib/raylib.h"
#include "base/types.h"
#include "base/maths.h"
#include "game/game.h"
#include "game/scene.h"

#include <memory>
#include <string>

class CGameManager
{
public:
  
  static constexpr uint32 sm_uWindowWidth = 384;
  static constexpr uint32 sm_uWindowHeight = 682u;

  static constexpr uint32 sm_uRenderFactor = 2u;

  static constexpr uint32 sm_uVirtualWindowWidth = sm_uWindowWidth * sm_uRenderFactor;
  static constexpr uint32 sm_uVirtualWindowHeight = sm_uWindowHeight * sm_uRenderFactor;

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

private:
  CGameManager();
  ~CGameManager();

  CScene* m_aoScenes[1] = { nullptr };
  RenderTexture2D m_oRenderTarget;
  std::shared_ptr<Texture2D> m_spTriangle;
  std::shared_ptr<Texture2D> m_spSquare;
  std::shared_ptr<Texture2D> m_spPentagon;
  std::shared_ptr<Texture2D> m_spCircle;
  bool m_bWantClose = false;
};
