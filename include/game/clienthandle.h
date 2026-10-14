#pragma once

#include "game/network_protocol.h"

struct SNetStream;
struct _ENetPeer;
typedef struct _ENetPeer ENetPeer;

class CClientHandle
{
public:
  CClientHandle(ENetPeer* _pPeer = nullptr);

  void Setup(ENetPeer* _pPeer);
  void Invalidate();
  bool Send(SNetStream* _pStream, EMsgPriority _eMsgPriority);

private:
  ENetPeer* m_pPeer = nullptr;
};