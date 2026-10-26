#pragma once

#include "game/scene.h"
#include "base/types.h"

class CPotionsScene : public CScene
{
public:

  CPotionsScene();
  ~CPotionsScene();

  void Init(const std::string& _sName) override;
  void Update(float _fDt) override;
  void Draw() override;
  bool CanBeChanged() const override;

private:
  uint8 m_uDraggedElement = 0;
  bool m_bIsDragging = false;
};