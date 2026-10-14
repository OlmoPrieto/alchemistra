#pragma once

#include <memory>

#include "raylib/raylib.h"
#include "base/types.h"
#include "base/maths.h"

class IGame;
class CClientBall;

class CClientPaddle
{
public:
  CClientPaddle(IGame* _pOwnerGame, const CVector2D& _v2Pos, bool _bPlayerControlled);
  ~CClientPaddle();

  void Init(bool _bLoadTextures = true);
  void Update(float _fDt);
  void Draw();
  const CClientBall* GetClosestBall() const;
  void SetPlayerControlled(bool _bPlayerControlled);
  bool GetPlayerControlled() const;
  
  // ------------------------
  float     m_aColor[3] = { 1.0f, 1.0f, 1.0f };
  CVector2D m_v2Pos;
  CVector2D m_v2Size;
  CVector2D m_v2HalfSize;
  uint32    m_uContactFrames = 0u;
  float     m_fDirection = 0.0f;

private:
  void UpdatePlayer(float _fDt);
  void UpdateAI(float _fDt);

  std::shared_ptr<Texture2D> m_spTexture;
  void (CClientPaddle::* m_pfncUpdate)(float);
  IGame* m_pOwnerGame = nullptr; // TODO: maybe can just be CGameClient
};