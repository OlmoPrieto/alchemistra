#include "game/clienthandle.h"

#include <iostream>

#include "enet/enet.h"
#include "base/network_serialization.h"

// ------------------------

CClientHandle::CClientHandle(ENetPeer* _pPeer)
  : m_pPeer(_pPeer)
{

}
 
// ------------------------

void CClientHandle::Setup(ENetPeer* _pPeer)
{
  m_pPeer = _pPeer;
}

// ------------------------

void CClientHandle::Invalidate()
{
  m_pPeer = nullptr;
}

// ------------------------

bool CClientHandle::Send(SNetStream* _pStream, EMsgPriority _eMsgPriority)
{
  if (m_pPeer)
  {
    ENetPacket* pPacket = enet_packet_create(_pStream->m_pData, _pStream->m_uOffset, ENET_PACKET_FLAG_RELIABLE);
    int32 iReturnValue = enet_peer_send(m_pPeer, (int32)(_eMsgPriority), pPacket);
    if (iReturnValue < 0)
    {
      // Only destroy packets manually if send fails
      enet_packet_destroy(pPacket);
      return false;
    }

    enet_host_flush(m_pPeer->host);
    //printf("Server sent data\n");

    return true;
  }

  return false;
}

// ------------------------
// ------------------------
// ------------------------