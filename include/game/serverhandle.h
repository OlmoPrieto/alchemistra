#pragma once

#include "base/types.h"

struct SNetStream;
struct _ENetPeer;
typedef struct _ENetPeer ENetPeer;

class CServerHandle
{
public:
  CServerHandle(ENetPeer* _pPeer = nullptr);

  void Setup(ENetPeer* _pPeer);
  void Invalidate();
  bool Receive(SNetStream* _pStream);

private:
  ENetPeer* m_pPeer = nullptr;
};