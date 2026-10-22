#include "game/game_scene.h"

#include "game/game_manager.h"
#include "ui/ui.h"

// ------------------------

CGameScene::CGameScene()
{
}

// ------------------------

CGameScene::~CGameScene()
{
}

// ------------------------

void CGameScene::Init(const std::string& _sName)
{
  CScene::Init(_sName);

  m_oGame.Init();
}

// ------------------------

void CGameScene::Update(float _fDt)
{
  if (m_oGame.GetGameStarted())
  {
    m_oGame.Update(_fDt);
  }
}

// ------------------------

void CGameScene::Draw()
{
  if (m_oGame.GetGameStarted())
  {
    m_oGame.Draw();
  }
  else
  {
    const CVector2D v2PlayButtonSize = CButton::CalculateSize("PLAY");
    if (CButton("PLAY", CVector2D(CGameManager::sm_uWindowWidth * 0.5f - v2PlayButtonSize.x * 0.5f, CGameManager::sm_uWindowHeight * 0.5f - v2PlayButtonSize.y)))
    {
      m_oGame.Start();
    }
  }
}

// ------------------------
// ------------------------