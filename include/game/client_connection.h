#pragma once

#include <climits>

#include "base/types.h"

struct _ENetPeer;
typedef struct _ENetPeer ENetPeer;
class CGameServer;

class CClientConnection
{
public:
  ENetPeer* m_pClient = nullptr; // CNetworkClient
  CGameServer* m_pGameServer = nullptr;
  uint32 m_uGameId = UINT_MAX;
  bool m_bGameStarted = false;

  void Clear();

private:
};