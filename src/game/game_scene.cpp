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
  CScene::Update(_fDt);

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

  // UI
  //if (IsKeyPressed(KEY_P))
  //{
  //  Rectangle oBounds;
  //  oBounds.width   = CGameManager::sm_uWindowWidth   * 0.2f;
  //  oBounds.height  = CGameManager::sm_uWindowHeight  * 0.7f;
  //  oBounds.x       = CGameManager::sm_uWindowWidth   * 0.5f - oBounds.width  * 0.5f;
  //  oBounds.y       = CGameManager::sm_uWindowHeight  * 0.5f - oBounds.height * 0.5f;
  //  DrawRectangleRounded(oBounds, 0.05f, 8, Color{ 180u, 180u, 180u, 255u });

  //  CVector2D v2ButtonSize = CButton::CalculateSize("RESUME");
  //  const CVector2D v2ButtonPos{ oBounds.x + oBounds.width * 0.5f,
  //    oBounds.y + oBounds.height * 0.5f };

  //  if (CButton("RESUME",
  //                CVector2D{ v2ButtonPos.x - v2ButtonSize.x * 0.5f,
  //                v2ButtonPos.y - v2ButtonSize.y * 0.5f }
  //             )
  //  )
  //  {
  //    // Pause end
  //  
  //  }

  //  v2ButtonSize = CButton::CalculateSize("QUIT");
  //  if (CButton("QUIT",
  //                CVector2D{ v2ButtonPos.x - v2ButtonSize.x * 0.5f,
  //                (oBounds.y + oBounds.height) - v2ButtonSize.y * 2.0f }
  //             )
  //  )
  //  {

  //  }
  //}
}

// ------------------------

bool CGameScene::CanBeChanged() const
{
  return m_oGame.GetGameStarted() == false;
}

// ------------------------
// ------------------------