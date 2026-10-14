#pragma once

#include <cassert>
#include <climits>
#include <cstring>

#include "base/types.h"

// ------------------------

struct SNetStream
{
  byte*   m_pData   = nullptr;
  uint32  m_uSize   = 0u;
  uint32  m_uOffset = 0u;
  bool    m_bError  = false;
};

// ------------------------

inline void InitNetStream(SNetStream* _pStream, byte* _pData, uint32 _uSize)
{
  _pStream->m_pData   = _pData;
  _pStream->m_uSize   = _uSize;
  _pStream->m_uOffset = 0u;
  _pStream->m_bError  = false;
}

// ------------------------

inline bool WriteUint8(SNetStream* _pStream, uint8 _uValue)
{
  assert(_pStream);
  if (_pStream->m_uSize - _pStream->m_uOffset < sizeof(uint8))
  {
    _pStream->m_bError = true;
    return false;
  }

  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)(_uValue & 0xff);
  _pStream->m_uOffset += sizeof(uint8);

  return true;
}

// ------------------------

inline bool WriteUint16(SNetStream* _pStream, uint16 _uValue)
{
  assert(_pStream);
  if (_pStream->m_uSize - _pStream->m_uOffset < sizeof(uint16))
  {
    _pStream->m_bError = true;
    return false;
  }

  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)((_uValue >> 8) & 0xff);
  _pStream->m_uOffset += sizeof(uint8);
  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)(_uValue & 0xff);
  _pStream->m_uOffset += sizeof(uint8);

  return true;
}

// ------------------------

inline bool WriteUint32(SNetStream* _pStream, uint32 _uValue)
{
  assert(_pStream);
  if (_pStream->m_uSize - _pStream->m_uOffset < sizeof(uint32))
  {
    _pStream->m_bError = true;
    return false;
  }

  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)((_uValue >> 24) & 0xff);
  _pStream->m_uOffset += sizeof(uint8);
  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)((_uValue >> 16) & 0xff);
  _pStream->m_uOffset += sizeof(uint8);
  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)((_uValue >> 8) & 0xff);
  _pStream->m_uOffset += sizeof(uint8);
  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)(_uValue & 0xff);
  _pStream->m_uOffset += sizeof(uint8);

  return true;
}

// ------------------------

inline bool WriteFloat32(SNetStream* _pStream, float _fValue)
{
  assert(_pStream);
  assert(sizeof(float) == sizeof(uint32));
  if (_pStream->m_uSize - _pStream->m_uOffset < sizeof(float))
  {
    _pStream->m_bError = true;
    return false;
  }

  uint32 uValue = 0u;
  memcpy(&uValue, &_fValue, sizeof(float));

  WriteUint32(_pStream, uValue);

  return true;
}

// ------------------------

inline bool WriteUint64(SNetStream* _pStream, uint64 _uValue)
{
  assert(_pStream);
  if (_pStream->m_uSize - _pStream->m_uOffset < sizeof(uint64))
  {
    _pStream->m_bError = true;
    return false;
  }

  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)((_uValue >> 56) & 0xff);
  _pStream->m_uOffset += sizeof(uint8);
  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)((_uValue >> 48) & 0xff);
  _pStream->m_uOffset += sizeof(uint8);
  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)((_uValue >> 40) & 0xff);
  _pStream->m_uOffset += sizeof(uint8);
  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)((_uValue >> 32) & 0xff);
  _pStream->m_uOffset += sizeof(uint8);
  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)((_uValue >> 24) & 0xff);
  _pStream->m_uOffset += sizeof(uint8);
  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)((_uValue >> 16) & 0xff);
  _pStream->m_uOffset += sizeof(uint8);
  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)((_uValue >> 8) & 0xff);
  _pStream->m_uOffset += sizeof(uint8);
  *(_pStream->m_pData + _pStream->m_uOffset) = (uint8)(_uValue & 0xff);
  _pStream->m_uOffset += sizeof(uint8);

  return true;
}

// ------------------------
// ------------------------

inline bool ReadUint8(SNetStream* _pStream, uint8* _pValue)
{
  assert(_pStream);
  if (_pStream->m_uSize - _pStream->m_uOffset < sizeof(uint8))
  {
    _pStream->m_bError = true;
    return false;
  }

  *_pValue = *(_pStream->m_pData + _pStream->m_uOffset);
  _pStream->m_uOffset += sizeof(uint8);

  return true;
}

// ------------------------

inline bool ReadUint16(SNetStream* _pStream, uint16* _pValue)
{
  assert(_pStream);
  if (_pStream->m_uSize - _pStream->m_uOffset < sizeof(uint16))
  {
    _pStream->m_bError = true;
    return false;
  }

  *_pValue |= ((uint16)*(_pStream->m_pData + _pStream->m_uOffset)) << 8u;
  _pStream->m_uOffset += sizeof(uint8);
  *_pValue |= (uint16)*(_pStream->m_pData + _pStream->m_uOffset);
  _pStream->m_uOffset += sizeof(uint8);

  return true;
}

// ------------------------

inline bool ReadUint32(SNetStream* _pStream, uint32* _pValue)
{
  assert(_pStream);
  if (_pStream->m_uSize - _pStream->m_uOffset < sizeof(uint32))
  {
    _pStream->m_bError = true;
    return false;
  }

  *_pValue = 0u;

  *_pValue |= ((uint32)*(_pStream->m_pData + _pStream->m_uOffset)) << 24u;
  _pStream->m_uOffset += sizeof(uint8);
  *_pValue |= ((uint32)*(_pStream->m_pData + _pStream->m_uOffset)) << 16u;
  _pStream->m_uOffset += sizeof(uint8);
  *_pValue |= ((uint32)*(_pStream->m_pData + _pStream->m_uOffset)) << 8u;
  _pStream->m_uOffset += sizeof(uint8);
  *_pValue |= (uint32)*(_pStream->m_pData + _pStream->m_uOffset);
  _pStream->m_uOffset += sizeof(uint8);

  return true;
}

// ------------------------

inline bool ReadFloat32(SNetStream* _pStream, float* _pValue)
{
  assert(_pStream);
  assert(sizeof(float) == sizeof(uint32));
  if (_pStream->m_uSize - _pStream->m_uOffset < sizeof(float))
  {
    _pStream->m_bError = true;
    return false;
  }

  uint32 uValue = UINT_MAX;
  ReadUint32(_pStream, &uValue);
  assert(uValue != UINT_MAX);

  memcpy(_pValue, &uValue, sizeof(float));

  return true;
}

// ------------------------

inline bool ReadUint64(SNetStream* _pStream, uint64* _pValue)
{
  assert(_pStream);
  if (_pStream->m_uSize - _pStream->m_uOffset < sizeof(uint64))
  {
    _pStream->m_bError = true;
    return false;
  }

  *_pValue = 0u;

  *_pValue |= ((uint64) * (_pStream->m_pData + _pStream->m_uOffset)) << 56u;
  _pStream->m_uOffset += sizeof(uint8);
  *_pValue |= ((uint64) * (_pStream->m_pData + _pStream->m_uOffset)) << 48u;
  _pStream->m_uOffset += sizeof(uint8);
  *_pValue |= ((uint64) * (_pStream->m_pData + _pStream->m_uOffset)) << 40u;
  _pStream->m_uOffset += sizeof(uint8);
  *_pValue |= ((uint64) * (_pStream->m_pData + _pStream->m_uOffset)) << 32;
  _pStream->m_uOffset += sizeof(uint8);
  *_pValue |= ((uint64) * (_pStream->m_pData + _pStream->m_uOffset)) << 24u;
  _pStream->m_uOffset += sizeof(uint8);
  *_pValue |= ((uint64) * (_pStream->m_pData + _pStream->m_uOffset)) << 16u;
  _pStream->m_uOffset += sizeof(uint8);
  *_pValue |= ((uint64) * (_pStream->m_pData + _pStream->m_uOffset)) << 8u;
  _pStream->m_uOffset += sizeof(uint8);
  *_pValue |= (uint64) * (_pStream->m_pData + _pStream->m_uOffset);
  _pStream->m_uOffset += sizeof(uint8);

  return true;
}

// ------------------------