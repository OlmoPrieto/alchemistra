#pragma once

#include "game/scene.h"
#include "base/types.h"
#include "base/maths.h"
#include "raylib/raylib.h"

class CPotionsScene : public CScene
{
public:

  static constexpr uint32 sm_uMaxIngredients = 32u;

  CPotionsScene();
  ~CPotionsScene();

  void Init(const std::string& _sName) override;
  void Update(float _fDt) override;
  void Draw() override;
  bool CanBeChanged() const override;
  bool CheckIngredientsForPotion();

private:
  uint8 m_auIngredients[sm_uMaxIngredients] = { 0u };
  Rectangle m_oCauldronRect = { 0.0f, 0.0f, 0.0f, 0.0f };
  CVector2D m_v2CraftButtonPos;
  uint32 m_uCurrentNumIngredients = 0u;
  float m_fCellSize         = 0.0f;
  float m_fGridWidth        = 0.0f;
  float m_fGridHeight       = 0.0f;
  float m_fGridX            = 0.0f;
  float m_fGridY            = 0.0f;
  float m_fPadding          = 0.0f;
  uint8 m_uDraggedElement   = 0u;
  bool m_bIsDragging        = false;
};
