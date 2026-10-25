#pragma once

#include "game/scene.h"

class CInventoryScene : public CScene
{
public:
  
  CInventoryScene();
  ~CInventoryScene();

  void Init(const std::string& _sName) override;
  void Update(float _fDt) override;
  void Draw() override;
  bool CanBeChanged() const override;

private:
  
};