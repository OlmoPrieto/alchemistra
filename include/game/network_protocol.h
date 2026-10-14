#pragma once

#include "base/types.h"
#include "base/network_serialization.h"

// ------------------------

#define NET_MAX_PACKET_SIZE 1024

enum class EMsgType : uint8
{
  CONNECT_REQUEST = 0,  // TODO: have the payload size in the enum (would have to change underlaying type)
  CONNECT_ACCEPT,
  DISCONNECT,
  START_GAME,
  PAUSE_REQUEST,
  PAUSE_GAME,
  PAUSE_END,
  END_REQUEST,
  GAME_ENDED,
  START_GAME_COUNTDOWN,
  GAME_STATE,
  PLAYER_INPUT,
  //
  COUNT
};

enum class EMsgPriority : uint8
{
  HIGH = 0,
  LOW,
  //
  COUNT
};

inline const uint32 GetMsgPayload(EMsgType _eMsgType)
{
  switch (_eMsgType)
  {
    case EMsgType::CONNECT_REQUEST:
    {
      return 0u;
    }
    case EMsgType::CONNECT_ACCEPT:
    {
      return 0u;
    }
    case EMsgType::DISCONNECT:
    {
      return 0u;
    }
    case EMsgType::COUNT:
    {
      assert(false);
    }
  }

  return UINT_MAX;
}

constexpr static uint32 s_uPongDxProtocol = 0x50445850;
constexpr static uint8 s_uVersion = 1u;

struct SPacketHeader
{
  uint32 m_uProtocolId = s_uPongDxProtocol; // magic number: ASCII PDXP (Pong DX Protocol)
  uint8 m_uVersion = s_uVersion;
  EMsgType m_uMsgType = EMsgType::COUNT;
  uint16 m_uPacketSize = 0u;
  uint8 m_uPacketId = 0xff;
  uint8 m_uFlags = 0u;
}; // 10 bytes + 2 padding

static constexpr inline const uint32 _GetPacketHeaderSize()
{
  constexpr uint32 uStructSize = 10u;
  static_assert(uStructSize == 10u);
  return uStructSize;
}

static constexpr inline const uint32 _GetPacketHeaderPadding()
{
  constexpr uint32 uPadding = 2u;
  static_assert(uPadding == 2u);
  return uPadding;
}

inline const uint32 GetPacketHeaderSize()
{
  assert(sizeof(SPacketHeader) == _GetPacketHeaderSize() + _GetPacketHeaderPadding());
  return _GetPacketHeaderSize();
}

// ------------------------

inline bool ReadHeader(SNetStream* _pStream, SPacketHeader* _pHeader)
{
  // Validate header?
  
  ReadUint32(_pStream, &_pHeader->m_uProtocolId);
  assert(_pHeader->m_uProtocolId == s_uPongDxProtocol);

  ReadUint8(_pStream,   &_pHeader->m_uVersion);
  ReadUint8(_pStream,   (uint8*)(&_pHeader->m_uMsgType));
  ReadUint16(_pStream,  &_pHeader->m_uPacketSize);
  ReadUint8(_pStream,   &_pHeader->m_uPacketId);
  ReadUint8(_pStream,   &_pHeader->m_uFlags);

  return true;
}

// ------------------------

inline bool WriteHeader(SNetStream* _pStream, EMsgType _eMsgType)
{
  WriteUint32(_pStream, s_uPongDxProtocol);
  WriteUint8(_pStream, s_uVersion);
  WriteUint8(_pStream, (uint8)_eMsgType);
  WriteUint16(_pStream, GetMsgPayload(_eMsgType));
  WriteUint8(_pStream, 0u);
  WriteUint8(_pStream, 0u);

  return true;
}

// ------------------------