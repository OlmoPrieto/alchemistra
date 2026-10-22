#pragma once

#include <string>

class CScene
{
public:
  
  CScene();
  virtual ~CScene();

  void virtual Init(const std::string& _sName);
  void virtual Update(float _fDt) = 0;
  void virtual Draw() = 0;

private:
  std::string m_sName;
};