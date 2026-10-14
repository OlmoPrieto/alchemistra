#pragma once

#include "game/gameclient.h"
#include "game/network_protocol.h"

struct _ENetHost;
typedef struct _ENetHost ENetHost;

class CNetworkClient
{
public:
  CNetworkClient(char* _sIp, uint16 _uPort);
  ~CNetworkClient();

  void Init(bool _bLocalGame = false);
  void Connect();
  void Update();
  void SetIp(char* _sIp);
  bool WantClose() const { return m_bWantClose; }
  bool Send(SNetStream* _pStream, EMsgPriority _eMsgPriority);
  float GetNetworkTime() const;

private:
  CGameClient m_oGameClient;
  ENetHost* m_pHost = nullptr;
  ENetPeer* m_pServerPeer = nullptr;
  char m_sIp[3 * 4];
  uint16 m_uPort = UINT16_MAX;
  bool m_bWantClose = false;
};