#include <iostream>
#include <cassert>

#include "game/game.h"

int main(int argc, char** argv)
{
  CGame& oGame = CGame::GetInstance();
  oGame.Init();

  while (oGame.WantClose() == false)
  {
    oGame.Update(0.0166666f);
    oGame.Draw();
  }

  return 0;
}