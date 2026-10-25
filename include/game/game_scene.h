#pragma once

#include "game/scene.h"
#include "game/game.h"

class CGameScene : public CScene
{
public:

  CGameScene();
  ~CGameScene();

  void Init(const std::string& _sName) override;
  void Update(float _fDt) override;
  void Draw() override;
  bool CanBeChanged() const override;

private:
  CGame m_oGame;
};