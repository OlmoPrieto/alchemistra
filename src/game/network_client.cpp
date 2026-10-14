#include "game/network_client.h"

#include <cassert>

#include "enet/enet.h"
#include "game/network_protocol.h"

// ------------------------

CNetworkClient::CNetworkClient(char* _sIp, uint16 _uPort)
  : m_uPort(_uPort)
{
  memcpy(m_sIp, _sIp, 3*4);
}

// ------------------------

CNetworkClient::~CNetworkClient()
{

}

// ------------------------

void CNetworkClient::Init(bool _bLocalGame /*= false*/)
{
  m_oGameClient.Init(_bLocalGame);
}

// ------------------------

void CNetworkClient::Connect()
{
  ENetAddress oAddress;
  oAddress.host = ENET_HOST_ANY;
  oAddress.port = m_uPort;

  m_pHost = enet_host_create(nullptr, 1, 2, 0, 0);
  assert(m_pHost);
  enet_address_set_host(&oAddress, m_sIp);

  m_pServerPeer = enet_host_connect(m_pHost, &oAddress, 2, 0);
  assert(m_pServerPeer);
}

// ------------------------

void CNetworkClient::Update()
{
  // ----------- NETWORK ----------- 
  ENetEvent oEvent;
  while (enet_host_service(m_pHost, &oEvent, 0) > 0)
  {
    switch (oEvent.type)
    {
      case ENET_EVENT_TYPE_CONNECT: // ----------------------------------------------
      {
        printf("Client connected!\n");

        SNetStream oStream;
        byte pBuffer[NET_MAX_PACKET_SIZE];
        memset(pBuffer, 0, NET_MAX_PACKET_SIZE);
        InitNetStream(&oStream, pBuffer, NET_MAX_PACKET_SIZE);
        SPacketHeader oHeader;
        WriteHeader(&oStream, EMsgType::CONNECT_REQUEST);
        ENetPacket* pPacket = enet_packet_create(oStream.m_pData, oStream.m_uOffset, ENET_PACKET_FLAG_RELIABLE);
        if (enet_peer_send(oEvent.peer, 0, pPacket) < 0)
        {
          // Only destroy packets manually if send fails
          enet_packet_destroy(pPacket);
        }

        enet_host_flush(m_pHost);

        break;
      }
      case ENET_EVENT_TYPE_RECEIVE: // ----------------------------------------------
      {
        SNetStream oStream;
        InitNetStream(&oStream, oEvent.packet->data, oEvent.packet->dataLength);
        SPacketHeader oHeader;
        ReadHeader(&oStream, &oHeader);
        switch (oHeader.m_uMsgType)
        {
          case EMsgType::CONNECT_ACCEPT:
          {
            printf("  Connection accepted by the server. Waiting for the server to launch\n");

            //m_oGameClient.Begin(this, uPlayerId);

            break;
          }
          case EMsgType::START_GAME:
          {
            ReadUint8(&oStream, &m_oGameClient.m_uClientId);

            m_oGameClient.Begin(this, m_oGameClient.m_uClientId);
            printf("  Start of the game in client %u\n", m_oGameClient.m_uClientId);

            break;
          }
          case EMsgType::GAME_STATE:
          {
            memcpy(oStream.m_pData, oEvent.packet->data, oEvent.packet->dataLength);
            oStream.m_uSize = oEvent.packet->dataLength;
            oStream.m_uOffset = 0u;
            oStream.m_bError = false;

            m_oGameClient.OnGameStateReceived(&oStream);

            break;
          }
          case EMsgType::START_GAME_COUNTDOWN:
          {
            float fCountdown = 100.0f;
            ReadFloat32(&oStream, &fCountdown);
            m_oGameClient.SetStartGameCurrentTime(fCountdown);
            
            break;
          }
          case EMsgType::PAUSE_GAME:
          {
            m_oGameClient.Pause();

            break;
          }
          case EMsgType::PAUSE_END:
          {
            m_oGameClient.Resume();

            break;
          }
          case EMsgType::GAME_ENDED:
          {
            m_oGameClient.End();
            
            break;
          }
          default:
          {
            //assert(false);
            break;
          }
        }

        break;
      }
      case ENET_EVENT_TYPE_DISCONNECT:  // ----------------------------------------------
      {
        break;
      }
      case ENET_EVENT_TYPE_NONE:        // ----------------------------------------------
      {
        assert(false);
        break;
      }
    }

    enet_packet_destroy(oEvent.packet);
  }

  // ----------- LOGIC ----------- 
  m_oGameClient.Loop();
  /*if (m_oGameClient.WantClose())
  {
    m_oGameClient.End();
  }*/
  m_bWantClose = m_oGameClient.WantClose();
}

// ------------------------

void CNetworkClient::SetIp(char* _sIp)
{
  memcpy(m_sIp, _sIp, 3 * 4);
}

// ------------------------

bool CNetworkClient::Send(SNetStream* _pStream, EMsgPriority _eMsgPriority)
{
  if (m_pServerPeer)
  {
    ENetPacket* pPacket = enet_packet_create(_pStream->m_pData, _pStream->m_uOffset, ENET_PACKET_FLAG_RELIABLE);
    int32 iReturnValue = enet_peer_send(m_pServerPeer, (int32)(_eMsgPriority), pPacket);
    if (iReturnValue < 0)
    {
      // Only destroy packets manually if send fails
      enet_packet_destroy(pPacket);
      return false;
    }

    enet_host_flush(m_pHost);

    return true;
  }

  return false;
}

// ------------------------

float CNetworkClient::GetNetworkTime() const
{
  return m_pServerPeer ? m_pServerPeer->roundTripTime : -1.0f;
}

// ------------------------



// ------------------------