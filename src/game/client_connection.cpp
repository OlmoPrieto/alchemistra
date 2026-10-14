#include "game/client_connection.h"

#include "enet/enet.h"
#include "game/gameserver.h"

// ------------------------

void CClientConnection::Clear()
{
  m_pClient = nullptr;
  m_pGameServer = nullptr;
  m_uGameId = UINT32_MAX;
  m_bGameStarted = false;
}

// ------------------------
// ------------------------
// ------------------------