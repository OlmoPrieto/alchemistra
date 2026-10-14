#include "game/network_server.h"

#include "enet/enet.h"
#include "game/client_connection.h"

// ------------------------

CNetworkServer::CNetworkServer()
{
  
}

// ------------------------

CNetworkServer::~CNetworkServer()
{
  for (uint32 i = 0; i < m_vctClients.size(); ++i)
  {
    if (m_vctClients[i])
    {
      delete m_vctClients[i];
    }
  }
}

// ------------------------

void CNetworkServer::Init()
{
  ENetAddress oAddress;
  oAddress.host = ENET_HOST_ANY;
  oAddress.port = 1234;

  m_pHost = enet_host_create(&oAddress, 4, 2, 0, 0);
  assert(m_pHost != nullptr);
}

// ------------------------

void CNetworkServer::Update()
{
  ENetEvent oEvent;
  while (enet_host_service(m_pHost, &oEvent, 0) > 0)
  {
    switch (oEvent.type)
    {
      case ENET_EVENT_TYPE_CONNECT: // ----------------------------------------------
      {
        printf("Server received a connect request!\n");

        CClientConnection* pClientConnection = new CClientConnection();
        pClientConnection->m_pClient = oEvent.peer;
        //pClientConnection->m_uPlayerId = m_vctClients.size() % 2;
        oEvent.peer->data = pClientConnection;

        m_vctClients.push_back(pClientConnection);

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
          case EMsgType::CONNECT_REQUEST: // ------------------------
          {
            printf("  Server accepted a connect request\n");

            SNetStream oSendStream;
            byte pBuffer[NET_MAX_PACKET_SIZE];
            memset(pBuffer, 0, NET_MAX_PACKET_SIZE);
            InitNetStream(&oSendStream, pBuffer, NET_MAX_PACKET_SIZE);
            WriteHeader(&oSendStream, EMsgType::CONNECT_ACCEPT);

            // The client here will be waiting. Maybe send information about the connection or something
            //WriteUint8(&oSendStream, sm_uClientsId++);

            ENetPacket* pPacket = enet_packet_create(oSendStream.m_pData, oSendStream.m_uOffset, ENET_PACKET_FLAG_RELIABLE);
            if (enet_peer_send(oEvent.peer, 0, pPacket) < 0)
            {
              // Only destroy packets manually if send fails
              enet_packet_destroy(pPacket);
            }

            //enet_host_flush(m_pHost);

            m_vctClientsQueue.push_back(static_cast<CClientConnection*>(oEvent.peer->data));

            if (m_vctClientsQueue.size() >= 2)
            {
              printf("2 players connected, starting game\n");

              // Start game
              // TODO: sm_uClients could run out of numbers. Implement a stack of indices in reverse order,
              //  so every new game takes one number and if it's destroyed, returns it to the stack. Fixed size of supported concurrent games.
              m_vctGameServers.emplace_back(sm_uClientsId++);
              CGameServer& oGameServer = m_vctGameServers.back();
              

              // TODO: implement a matching system (?).
              CClientConnection* pClient0 = m_vctClientsQueue.front();
              pClient0->m_pGameServer = &oGameServer;
              m_vctClientsQueue.pop_front();
              CClientConnection* pClient1 = m_vctClientsQueue.front();
              pClient1->m_pGameServer = &oGameServer;
              m_vctClientsQueue.pop_front();
              assert(pClient0->m_bGameStarted == false && pClient1->m_bGameStarted == false);
            
              oGameServer.Begin(this, pClient0, pClient1);

              // Send the signal to start the game
              memset(pBuffer, 0, NET_MAX_PACKET_SIZE);
              InitNetStream(&oSendStream, pBuffer, NET_MAX_PACKET_SIZE);
              WriteHeader(&oSendStream, EMsgType::START_GAME);

              // Send it to all clients
              // Client0
              // Client id
              WriteUint8(&oSendStream, 0u);
              ENetPacket* pPacket = enet_packet_create(oSendStream.m_pData, oSendStream.m_uOffset, ENET_PACKET_FLAG_RELIABLE);
              if (enet_peer_send(pClient0->m_pClient, 0, pPacket) < 0)
              {
                // Only destroy packets manually if send fails
                enet_packet_destroy(pPacket);
              }
              pClient0->m_uGameId = oGameServer.GetGameId();
              // --------
              // Client1
              memset(pBuffer, 0, NET_MAX_PACKET_SIZE);
              InitNetStream(&oSendStream, pBuffer, NET_MAX_PACKET_SIZE);
              WriteHeader(&oSendStream, EMsgType::START_GAME);

              // Client id
              WriteUint8(&oSendStream, 1u);
              pPacket = enet_packet_create(oSendStream.m_pData, oSendStream.m_uOffset, ENET_PACKET_FLAG_RELIABLE);
              if (enet_peer_send(pClient1->m_pClient, 0, pPacket) < 0)
              {
                // Only destroy packets manually if send fails
                enet_packet_destroy(pPacket);
              }
              pClient1->m_uGameId = oGameServer.GetGameId();


              printf("  Starting game %u\n", oGameServer.GetGameId());
            }

            enet_host_flush(m_pHost);

            break;
          }
          case EMsgType::PLAYER_INPUT: // ------------------------
          {
            assert(oStream.m_uOffset == GetPacketHeaderSize());
            
            uint8 uClientId = UINT8_MAX;
            ReadUint8(&oStream, &uClientId);
            assert(uClientId != UINT8_MAX);
            
            // Look for the client connection that sent the message
            assert(oEvent.peer->data != nullptr);
            CClientConnection* pClientConnection = static_cast<CClientConnection*>(oEvent.peer->data);

            CVector2D v2PlayerPos;
            ReadFloat32(&oStream, ((&v2PlayerPos.x) + 0));
            ReadFloat32(&oStream, ((&v2PlayerPos.x) + 1));

            //CGameServer* pGameServer = m_vctClients[uClientConnectionIndex]->m_pGameServer;
            CGameServer* pGameServer = pClientConnection->m_pGameServer;
            // TODO: Maybe this should be encapsulated in a function in CGameServer?
            // pGameServer->ProcessPlayerInput(uClientId, v2PlayerPos);
            if (uClientId == 0)
            {
              pGameServer->m_oPaddle0.m_v2Pos = v2PlayerPos;
            }
            else
            {
              pGameServer->m_oPaddle1.m_v2Pos = v2PlayerPos;
            }
            // --

            break;
          }
          case EMsgType::PAUSE_REQUEST:
          {
            uint8 uClientId = UINT8_MAX;
            ReadUint8(&oStream, &uClientId);
            assert(uClientId != UINT8_MAX);
            // TODO: handle pause requests limit per player

            // Look for the client connection that sent the message
            assert(oEvent.peer->data != nullptr);
            CClientConnection* pClientConnection = static_cast<CClientConnection*>(oEvent.peer->data);


            //CGameServer* pGameServer = m_vctClients[uClientConnectionIndex]->m_pGameServer;
            CGameServer* pGameServer = pClientConnection->m_pGameServer;
            if (pGameServer->SetPause(true, uClientId))
            {
              // Create message
              SNetStream oSendStream;
              byte pBuffer[NET_MAX_PACKET_SIZE];
              memset(pBuffer, 0, NET_MAX_PACKET_SIZE);
              InitNetStream(&oSendStream, pBuffer, NET_MAX_PACKET_SIZE);
              WriteHeader(&oSendStream, EMsgType::PAUSE_GAME);
              ENetPacket* pPacket = enet_packet_create(oSendStream.m_pData, oSendStream.m_uOffset, ENET_PACKET_FLAG_RELIABLE);
              // Client0
              if (enet_peer_send(pGameServer->m_pClient0->m_pClient, 0, pPacket) < 0)
              {
                // Only destroy packets manually if send fails
                enet_packet_destroy(pPacket);
              }
              // Client1
              pPacket = enet_packet_create(oSendStream.m_pData, oSendStream.m_uOffset, ENET_PACKET_FLAG_RELIABLE);
              if (enet_peer_send(pGameServer->m_pClient1->m_pClient, 0, pPacket) < 0)
              {
                // Only destroy packets manually if send fails
                enet_packet_destroy(pPacket);
              }
            }

            break;
          }
          case EMsgType::PAUSE_END:
          {
            uint8 uClientId = UINT8_MAX;
            ReadUint8(&oStream, &uClientId);
            assert(uClientId != UINT8_MAX);

            // Look for the client connection that sent the message
            assert(oEvent.peer->data != nullptr);
            CClientConnection* pClientConnection = static_cast<CClientConnection*>(oEvent.peer->data);

            CGameServer* pGameServer = pClientConnection->m_pGameServer;
            if (pGameServer->SetPause(false, uClientId))
            {
              // Create message
              SNetStream oSendStream;
              byte pBuffer[NET_MAX_PACKET_SIZE];
              memset(pBuffer, 0, NET_MAX_PACKET_SIZE);
              InitNetStream(&oSendStream, pBuffer, NET_MAX_PACKET_SIZE);
              WriteHeader(&oSendStream, EMsgType::PAUSE_END);
              ENetPacket* pPacket = enet_packet_create(oSendStream.m_pData, oSendStream.m_uOffset, ENET_PACKET_FLAG_RELIABLE);
              // Client0
              if (enet_peer_send(pGameServer->m_pClient0->m_pClient, 0, pPacket) < 0)
              {
                // Only destroy packets manually if send fails
                enet_packet_destroy(pPacket);
              }
              // Client1
              pPacket = enet_packet_create(oSendStream.m_pData, oSendStream.m_uOffset, ENET_PACKET_FLAG_RELIABLE);
              if (enet_peer_send(pGameServer->m_pClient1->m_pClient, 0, pPacket) < 0)
              {
                // Only destroy packets manually if send fails
                enet_packet_destroy(pPacket);
              }
            }

            break;
          }
          case EMsgType::END_REQUEST:
          {
            uint8 uClientId = UINT8_MAX;
            ReadUint8(&oStream, &uClientId);
            assert(uClientId != UINT8_MAX);

            assert(oEvent.peer->data != nullptr);
            CClientConnection* pClientConnection = static_cast<CClientConnection*>(oEvent.peer->data);

            CGameServer* pGameServer = pClientConnection->m_pGameServer;

            printf("Ending game %u\n", pGameServer->GetGameId());

            SNetStream oSendStream;
            byte pBuffer[NET_MAX_PACKET_SIZE];
            memset(pBuffer, 0, NET_MAX_PACKET_SIZE);
            InitNetStream(&oSendStream, pBuffer, NET_MAX_PACKET_SIZE);
            WriteHeader(&oSendStream, EMsgType::GAME_ENDED);
            if (uClientId == 0u)
            {
              ENetPacket* pPacket = enet_packet_create(oSendStream.m_pData, oSendStream.m_uOffset, ENET_PACKET_FLAG_RELIABLE);
              // Client0
              if (enet_peer_send(pGameServer->m_pClient1->m_pClient, 0, pPacket) < 0)
              {
                // Only destroy packets manually if send fails
                enet_packet_destroy(pPacket);
              }
            }
            else
            {
              ENetPacket* pPacket = enet_packet_create(oSendStream.m_pData, oSendStream.m_uOffset, ENET_PACKET_FLAG_RELIABLE);
              // Client0
              if (enet_peer_send(pGameServer->m_pClient0->m_pClient, 0, pPacket) < 0)
              {
                // Only destroy packets manually if send fails
                enet_packet_destroy(pPacket);
              }
            }

            // Destroy game server
            for (uint32 i = 0; i < m_vctGameServers.size(); ++i)
            {
              if (pGameServer == &m_vctGameServers[i])
              {
                pGameServer->m_pClient0->m_pGameServer = nullptr;
                pGameServer->m_pClient0->m_uGameId = UINT32_MAX;
                pGameServer->m_pClient0->m_bGameStarted = false;
                pGameServer->m_pClient1->m_pGameServer = nullptr;
                pGameServer->m_pClient1->m_uGameId = UINT32_MAX;
                pGameServer->m_pClient1->m_bGameStarted = false;

                m_vctGameServers.erase(m_vctGameServers.begin() + i);

                break;
              }
            }

            break;
          }
          default:
          {
            assert(false);
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

  for (uint32 i = 0; i < m_vctGameServers.size(); ++i)
  {
    m_vctGameServers[i].Loop();
  }
}

// ------------------------

bool CNetworkServer::SendToClient(SNetStream* _pStream, CClientConnection* _pClient, EMsgPriority _eMsgPriority)
{
  if (_pClient->m_pClient)
  {
    ENetPacket* pPacket = enet_packet_create(_pStream->m_pData, _pStream->m_uOffset, ENET_PACKET_FLAG_RELIABLE);
    int32 iReturnValue = enet_peer_send(_pClient->m_pClient, (int32)(_eMsgPriority), pPacket);
    if (iReturnValue < 0)
    {
      // Only destroy packets manually if send fails
      enet_packet_destroy(pPacket);
      return false;
    }

    enet_host_flush(m_pHost);
    //printf("Server sent data\n");

    return true;
  }

  return false;
}

// ------------------------

bool CNetworkServer::Receive(SNetStream* _pStream)
{
  if (m_pHost)
  {
    ENetEvent oEvent;
    bool bReceived = false;
    while (enet_host_service(m_pHost, &oEvent, 0) > 0)
    {
      if (oEvent.type == ENET_EVENT_TYPE_RECEIVE)
      {
        memcpy(_pStream->m_pData, oEvent.packet->data, oEvent.packet->dataLength);
        _pStream->m_uSize = oEvent.packet->dataLength;
        _pStream->m_uOffset = 0u;
        _pStream->m_bError = false;
        bReceived = true;

        enet_packet_destroy(oEvent.packet);

        // TODO: there could be more than one packet
      }
    }
    return bReceived;
  }

  return false;
}

// ------------------------