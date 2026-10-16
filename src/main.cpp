#include <iostream>
#include <cassert>
#include <filesystem>

#include "game/game.h"

int main(int argc, char** argv)
{
  // Fix working directory
  std::filesystem::path oCurrentPath = std::filesystem::current_path();
  bool bAnotherFolder = true;
  while (bAnotherFolder)
  {
    std::string sCurrentFolderName = oCurrentPath.filename().string();
    std::transform(sCurrentFolderName.begin(), sCurrentFolderName.end(), sCurrentFolderName.begin(), ::tolower);
    if (sCurrentFolderName == "debug" || sCurrentFolderName == "release" ||
        sCurrentFolderName == "build" || sCurrentFolderName == "win" ||
        sCurrentFolderName == "bin"   || sCurrentFolderName == "x64" ||
        sCurrentFolderName == "x86")
    {
      oCurrentPath = oCurrentPath.parent_path();
    }
    else
    {
      bAnotherFolder = false;
    }
  }
  std::filesystem::current_path(oCurrentPath);

  CGame& oGame = CGame::GetInstance();
  oGame.Init();

  while (oGame.WantClose() == false)
  {
    oGame.Update(0.0166666f);
    oGame.Draw();
  }

  return 0;
}