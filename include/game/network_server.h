#pragma once

#include <vector>
#include <deque>

#include "game/gameserver.h"

struct _ENetHost;
typedef struct _ENetHost ENetHost;
class CClientConnection;

class CNetworkServer
{
public:
  CNetworkServer();
  ~CNetworkServer();

  void Init();  // Creates net host
  void Update();
  bool SendToClient(SNetStream* _pStream, CClientConnection* _pClient, EMsgPriority _eMsgPriority);
  bool Receive(SNetStream* _pStream);
  void SetLocalServer(bool _bStatus) { m_bLocalServer = _bStatus; }

private:
  static inline uint32 sm_uClientsId = 0u;

  std::vector<CGameServer> m_vctGameServers;
  std::vector<CClientConnection*> m_vctClients;
  std::deque<CClientConnection*> m_vctClientsQueue;
  ENetHost* m_pHost = nullptr;
  bool m_bLocalServer = false;
};