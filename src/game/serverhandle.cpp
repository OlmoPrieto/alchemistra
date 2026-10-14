#include "game/serverhandle.h"

#include "enet/enet.h"
#include "base/network_serialization.h"
#include "game/network_protocol.h"

// ------------------------

CServerHandle::CServerHandle(ENetPeer* _pPeer)
  : m_pPeer(_pPeer)
{

}

// ------------------------

void CServerHandle::Setup(ENetPeer* _pPeer)
{
  m_pPeer = _pPeer;
}

// ------------------------

void CServerHandle::Invalidate()
{
  m_pPeer = nullptr;
}

// ------------------------

bool CServerHandle::Receive(SNetStream* _pStream)
{
  if (m_pPeer)
  {
    ENetEvent oEvent;
    bool bReceived = false;
    while (enet_host_service(m_pPeer->host, &oEvent, 0) > 0)
    {
      if (oEvent.type == ENET_EVENT_TYPE_RECEIVE)
      {
        memcpy(_pStream->m_pData, oEvent.packet->data, oEvent.packet->dataLength);
        _pStream->m_uSize = oEvent.packet->dataLength;
        _pStream->m_uOffset = 0u;
        _pStream->m_bError = false;
        bReceived = true;

        enet_packet_destroy(oEvent.packet);
      }
    }
    return bReceived;
  }

  return false;
}

// ------------------------
// ------------------------
// ------------------------
// ------------------------