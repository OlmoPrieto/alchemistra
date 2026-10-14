#include "ui/ui.h"

#include <algorithm>
#include <cmath>

namespace
{
  SUIButtonConfig s_oDefaultButtonConfig;
  SUICheckboxConfig s_oDefaultCheckboxConfig;
  SUIInputTextConfig s_oDefaultInputTextConfig;
  std::string* s_pFocusedInputText = nullptr;
  size_t s_iInputTextCursor = 0;
  float s_fInputTextScroll = 0.0f;
  double s_dInputTextBlinkStart = 0.0;

  const SUIStateColors& GetStateColors(const SUIStateColors& _oNormal, const SUIStateColors& _oHovered,
    const SUIStateColors& _oPressed, const SUIStateColors& _oDisabled, EUIElementState _eState)
  {
    switch (_eState)
    {
      case EUIElementState::HOVERED: return _oHovered;
      case EUIElementState::PRESSED: return _oPressed;
      case EUIElementState::DISABLED: return _oDisabled;
      default: return _oNormal;
    }
  }

  Rectangle ToRectangle(const CVector2D& _v2Position, const CVector2D& _v2Size)
  {
    return { _v2Position.x, _v2Position.y, _v2Size.x, _v2Size.y };
  }

  EUIElementState GetElementState(const Rectangle& _oBounds, bool _bEnabled)
  {
    if (!_bEnabled)
      return EUIElementState::DISABLED;

    if (!CheckCollisionPointRec(GetMousePosition(), _oBounds))
      return EUIElementState::NORMAL;

    return IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? EUIElementState::PRESSED : EUIElementState::HOVERED;
  }

  void DrawFrame(const Rectangle& _oBounds, const SUIStateColors& _oColors, float _fBorderWidth, float _fCornerRadius)
  {
    const float fRadius = Clamp(_fCornerRadius, 0.0f, 1.0f);
    DrawRectangleRounded(_oBounds, fRadius, 8, _oColors.m_cFill);
    if (_fBorderWidth > 0.0f)
      DrawRectangleRoundedLinesEx(_oBounds, fRadius, 8, _fBorderWidth, _oColors.m_cBorder);
  }

  bool IsUtf8ContinuationByte(char _cCharacter)
  {
    return ((uint8)_cCharacter & 0xc0) == 0x80;
  }

  size_t GetPreviousCharacterPosition(const std::string& _sText, size_t _iPosition)
  {
    if (_iPosition == 0)
      return 0;

    --_iPosition;
    while (_iPosition > 0 && IsUtf8ContinuationByte(_sText[_iPosition]))
      --_iPosition;
    return _iPosition;
  }

  size_t GetNextCharacterPosition(const std::string& _sText, size_t _iPosition)
  {
    if (_iPosition >= _sText.size())
      return _sText.size();

    ++_iPosition;
    while (_iPosition < _sText.size() && IsUtf8ContinuationByte(_sText[_iPosition]))
      ++_iPosition;
    return _iPosition;
  }

  void AppendCodepoint(std::string* _pText, size_t _iPosition, int32 _iCodepoint)
  {
    if (_iCodepoint <= 0x7f)
      _pText->insert(_iPosition, 1, (char)_iCodepoint);
    else if (_iCodepoint <= 0x7ff)
    {
      _pText->insert(_iPosition, 1, (char)(0xc0 | (_iCodepoint >> 6)));
      _pText->insert(_iPosition + 1, 1, (char)(0x80 | (_iCodepoint & 0x3f)));
    }
    else if (_iCodepoint <= 0xffff)
    {
      _pText->insert(_iPosition, 1, (char)(0xe0 | (_iCodepoint >> 12)));
      _pText->insert(_iPosition + 1, 1, (char)(0x80 | ((_iCodepoint >> 6) & 0x3f)));
      _pText->insert(_iPosition + 2, 1, (char)(0x80 | (_iCodepoint & 0x3f)));
    }
    else
    {
      _pText->insert(_iPosition, 1, (char)(0xf0 | (_iCodepoint >> 18)));
      _pText->insert(_iPosition + 1, 1, (char)(0x80 | ((_iCodepoint >> 12) & 0x3f)));
      _pText->insert(_iPosition + 2, 1, (char)(0x80 | ((_iCodepoint >> 6) & 0x3f)));
      _pText->insert(_iPosition + 3, 1, (char)(0x80 | (_iCodepoint & 0x3f)));
    }
  }

  float GetTextWidthUntil(const std::string& _sText, size_t _iPosition, int32 _iFontSize)
  {
    return (float)MeasureText(_sText.substr(0, _iPosition).c_str(), _iFontSize);
  }

  size_t GetCursorPositionAtX(const std::string& _sText, float _fX, int32 _iFontSize)
  {
    size_t iPosition = 0;
    while (iPosition < _sText.size())
    {
      const size_t iNextPosition = GetNextCharacterPosition(_sText, iPosition);
      const float fCharacterEnd = GetTextWidthUntil(_sText, iNextPosition, _iFontSize);
      if (_fX < fCharacterEnd)
      {
        const float fCharacterStart = GetTextWidthUntil(_sText, iPosition, _iFontSize);
        return _fX - fCharacterStart < fCharacterEnd - _fX ? iPosition : iNextPosition;
      }
      iPosition = iNextPosition;
    }
    return _sText.size();
  }
}

// ------------------------------------------------------------------------------------------

CButton::CButton(const char* _pText, const CVector2D& _v2Position)
{
  Draw(_pText, _v2Position, s_oDefaultButtonConfig);
}

CButton::CButton(const char* _pText, const CVector2D& _v2Position, const SUIButtonConfig& _oConfig)
{
  Draw(_pText, _v2Position, _oConfig);
}

CVector2D CButton::CalculateSize(const char* _pText)
{
  return CalculateSize(_pText, s_oDefaultButtonConfig);
}

CVector2D CButton::CalculateSize(const char* _pText, const SUIButtonConfig& _oConfig)
{
  const char* pText = _pText ? _pText : "";
  const float fTextWidth = (float)MeasureText(pText, _oConfig.m_iFontSize);
  const float fTextHeight = (float)_oConfig.m_iFontSize;
  const float fWidth = fTextWidth + _oConfig.m_v2TextPadding.x * 2.0f;
  const float fHeight = fTextHeight + _oConfig.m_v2TextPadding.y * 2.0f;

  return CVector2D(fWidth > _oConfig.m_v2Size.x ? fWidth : _oConfig.m_v2Size.x,
    fHeight > _oConfig.m_v2Size.y ? fHeight : _oConfig.m_v2Size.y);
}

void CButton::SetDefaultConfig(const SUIButtonConfig& _oConfig)
{
  s_oDefaultButtonConfig = _oConfig;
}

const SUIButtonConfig& CButton::GetDefaultConfig()
{
  return s_oDefaultButtonConfig;
}

void CButton::Draw(const char* _pText, const CVector2D& _v2Position, const SUIButtonConfig& _oConfig)
{
  const char* pText = _pText ? _pText : "";
  const CVector2D v2Size = CalculateSize(pText, _oConfig);
  const Rectangle oBounds = ToRectangle(_v2Position, v2Size);
  m_eState = GetElementState(oBounds, _oConfig.m_bEnabled);
  m_bPressed = _oConfig.m_bEnabled && CheckCollisionPointRec(GetMousePosition(), oBounds) &&
    IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

  const SUIStateColors& oColors = GetStateColors(_oConfig.m_oNormal, _oConfig.m_oHovered,
    _oConfig.m_oPressed, _oConfig.m_oDisabled, m_eState);
  DrawFrame(oBounds, oColors, _oConfig.m_fBorderWidth, _oConfig.m_fCornerRadius);

  const int32 iTextWidth = MeasureText(pText, _oConfig.m_iFontSize);
  const int32 iTextX = (int32)(oBounds.x + (oBounds.width - iTextWidth) * 0.5f);
  const int32 iTextY = (int32)(oBounds.y + (oBounds.height - _oConfig.m_iFontSize) * 0.5f);
  DrawText(pText, iTextX, iTextY, _oConfig.m_iFontSize, oColors.m_cText);
}

// ------------------------------------------------------------------------------------------

CCheckbox::CCheckbox(const char* _pText, bool* _pbValue, const CVector2D& _v2Position, bool _bOnlyBox)
{
  Draw(_pText, _pbValue, _v2Position, s_oDefaultCheckboxConfig, _bOnlyBox);
}

CCheckbox::CCheckbox(const char* _pText, bool* _pbValue, const CVector2D& _v2Position, const SUICheckboxConfig& _oConfig,
  bool _bOnlyBox)
{
  Draw(_pText, _pbValue, _v2Position, _oConfig, _bOnlyBox);
}

void CCheckbox::SetDefaultConfig(const SUICheckboxConfig& _oConfig)
{
  s_oDefaultCheckboxConfig = _oConfig;
}

const SUICheckboxConfig& CCheckbox::GetDefaultConfig()
{
  return s_oDefaultCheckboxConfig;
}

void CCheckbox::Draw(const char* _pText, bool* _pbValue, const CVector2D& _v2Position, const SUICheckboxConfig& _oConfig,
  bool _bOnlyBox)
{
  const char* pText = _pText ? _pText : "";
  const int32 iTextWidth = MeasureText(pText, _oConfig.m_iFontSize);
  const float fTextHeight = (float)_oConfig.m_iFontSize;
  const CVector2D v2ControlSize(_oConfig.m_v2BoxSize.x + (pText[0] ? _oConfig.m_fTextSpacing + iTextWidth : 0.0f),
    _oConfig.m_v2BoxSize.y > fTextHeight ? _oConfig.m_v2BoxSize.y : fTextHeight);
  const Rectangle oHitBounds = ToRectangle(_v2Position, v2ControlSize);
  const Rectangle oBoxBounds = ToRectangle(_v2Position, _oConfig.m_v2BoxSize);
  const Rectangle& oInteractionBounds = _bOnlyBox ? oBoxBounds : oHitBounds;
  const bool bEnabled = _oConfig.m_bEnabled && _pbValue;
  m_eState = GetElementState(oInteractionBounds, bEnabled);
  m_bChanged = bEnabled && CheckCollisionPointRec(GetMousePosition(), oInteractionBounds) &&
    IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
  if (m_bChanged)
    *_pbValue = !*_pbValue;

  const SUIStateColors& oColors = GetStateColors(_oConfig.m_oNormal, _oConfig.m_oHovered,
    _oConfig.m_oPressed, _oConfig.m_oDisabled, m_eState);
  DrawFrame(oBoxBounds, oColors, _oConfig.m_fBorderWidth, _oConfig.m_fCornerRadius);

  if (_pbValue && *_pbValue)
  {
    const float fPadding = _oConfig.m_v2BoxSize.x * 0.22f;
    const Vector2 av2Tick[3] =
    {
      { oBoxBounds.x + fPadding, oBoxBounds.y + oBoxBounds.height * 0.53f },
      { oBoxBounds.x + oBoxBounds.width * 0.43f, oBoxBounds.y + oBoxBounds.height - fPadding },
      { oBoxBounds.x + oBoxBounds.width - fPadding, oBoxBounds.y + fPadding }
    };
    DrawLineEx(av2Tick[0], av2Tick[1], 3.0f, _oConfig.m_cMark);
    DrawLineEx(av2Tick[1], av2Tick[2], 3.0f, _oConfig.m_cMark);
  }

  if (pText[0])
  {
    const int32 iTextX = (int32)(_v2Position.x + _oConfig.m_v2BoxSize.x + _oConfig.m_fTextSpacing);
    const int32 iTextY = (int32)(_v2Position.y + (v2ControlSize.y - _oConfig.m_iFontSize) * 0.5f +
      _oConfig.m_fTextVerticalOffset);
    DrawText(pText, iTextX, iTextY, _oConfig.m_iFontSize, oColors.m_cText);
  }
}

// ------------------------------------------------------------------------------------------

CInputText::CInputText(std::string* _pText, const CVector2D& _v2Position)
{
  Draw(_pText, _v2Position, s_oDefaultInputTextConfig);
}

CInputText::CInputText(std::string* _pText, const CVector2D& _v2Position, const SUIInputTextConfig& _oConfig)
{
  Draw(_pText, _v2Position, _oConfig);
}

void CInputText::SetDefaultConfig(const SUIInputTextConfig& _oConfig)
{
  s_oDefaultInputTextConfig = _oConfig;
}

const SUIInputTextConfig& CInputText::GetDefaultConfig()
{
  return s_oDefaultInputTextConfig;
}

void CInputText::Draw(std::string* _pText, const CVector2D& _v2Position, const SUIInputTextConfig& _oConfig)
{
  const Rectangle oBounds = ToRectangle(_v2Position, _oConfig.m_v2Size);
  const bool bEnabled = _oConfig.m_bEnabled && _pText;
  m_eState = GetElementState(oBounds, bEnabled);
  const bool bClicked = bEnabled && CheckCollisionPointRec(GetMousePosition(), oBounds) &&
    IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

  if (bClicked)
  {
    if (s_pFocusedInputText != _pText)
      s_fInputTextScroll = 0.0f;
    s_pFocusedInputText = _pText;
    const float fTextLeft = oBounds.x + _oConfig.m_v2TextPadding.x;
    s_iInputTextCursor = GetCursorPositionAtX(*_pText, GetMousePosition().x - fTextLeft + s_fInputTextScroll,
      _oConfig.m_iFontSize);
    s_dInputTextBlinkStart = GetTime();
  }
  else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && s_pFocusedInputText == _pText)
  {
    s_pFocusedInputText = nullptr;
  }

  m_bHasFocus = bEnabled && s_pFocusedInputText == _pText;
  if (m_bHasFocus)
  {
    if (s_iInputTextCursor > _pText->size())
      s_iInputTextCursor = _pText->size();

    if (IsKeyPressed(KEY_LEFT))
    {
      s_iInputTextCursor = GetPreviousCharacterPosition(*_pText, s_iInputTextCursor);
      s_dInputTextBlinkStart = GetTime();
    }
    if (IsKeyPressed(KEY_RIGHT))
    {
      s_iInputTextCursor = GetNextCharacterPosition(*_pText, s_iInputTextCursor);
      s_dInputTextBlinkStart = GetTime();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && s_iInputTextCursor > 0)
    {
      const size_t iPreviousPosition = GetPreviousCharacterPosition(*_pText, s_iInputTextCursor);
      _pText->erase(iPreviousPosition, s_iInputTextCursor - iPreviousPosition);
      s_iInputTextCursor = iPreviousPosition;
      m_bChanged = true;
      s_dInputTextBlinkStart = GetTime();
    }
    if (IsKeyPressed(KEY_DELETE) && s_iInputTextCursor < _pText->size())
    {
      _pText->erase(s_iInputTextCursor, GetNextCharacterPosition(*_pText, s_iInputTextCursor) - s_iInputTextCursor);
      m_bChanged = true;
      s_dInputTextBlinkStart = GetTime();
    }

    const bool bControlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    if (bControlDown && IsKeyPressed(KEY_V))
    {
      const char* pClipboardText = GetClipboardText();
      if (pClipboardText)
      {
        std::string sClipboardText(pClipboardText);
        sClipboardText.erase(std::remove(sClipboardText.begin(), sClipboardText.end(), '\r'), sClipboardText.end());
        sClipboardText.erase(std::remove(sClipboardText.begin(), sClipboardText.end(), '\n'), sClipboardText.end());
        if (!sClipboardText.empty())
        {
          _pText->insert(s_iInputTextCursor, sClipboardText);
          s_iInputTextCursor += sClipboardText.size();
          m_bChanged = true;
          s_dInputTextBlinkStart = GetTime();
        }
      }
    }
    else if (!bControlDown)
    {
      int32 iCodepoint = GetCharPressed();
      while (iCodepoint > 0)
      {
        if (iCodepoint >= 32 && iCodepoint != 127)
        {
          AppendCodepoint(_pText, s_iInputTextCursor, iCodepoint);
          s_iInputTextCursor = GetNextCharacterPosition(*_pText, s_iInputTextCursor);
          m_bChanged = true;
          s_dInputTextBlinkStart = GetTime();
        }
        iCodepoint = GetCharPressed();
      }
    }
  }

  const SUIStateColors& oColors = GetStateColors(_oConfig.m_oNormal, _oConfig.m_oHovered,
    _oConfig.m_oPressed, _oConfig.m_oDisabled, m_eState);
  DrawFrame(oBounds, oColors, _oConfig.m_fBorderWidth, _oConfig.m_fCornerRadius);

  const float fTextLeft = oBounds.x + _oConfig.m_v2TextPadding.x;
  const float fVisibleWidth = oBounds.width - _oConfig.m_v2TextPadding.x * 2.0f;
  const float fTextWidth = (float)MeasureText(_pText ? _pText->c_str() : "", _oConfig.m_iFontSize);
  if (m_bHasFocus && fVisibleWidth > 0.0f)
  {
    const float fCursorOffset = GetTextWidthUntil(*_pText, s_iInputTextCursor, _oConfig.m_iFontSize);
    if (fCursorOffset < s_fInputTextScroll)
      s_fInputTextScroll = fCursorOffset;
    else if (fCursorOffset - s_fInputTextScroll > fVisibleWidth - 2.0f)
      s_fInputTextScroll = fCursorOffset - fVisibleWidth + 2.0f;

    const float fMaximumScroll = fTextWidth > fVisibleWidth ? fTextWidth - fVisibleWidth : 0.0f;
    s_fInputTextScroll = Clamp(s_fInputTextScroll, 0.0f, fMaximumScroll);
  }
  else
  {
    s_fInputTextScroll = 0.0f;
  }

  const int32 iScissorX = (int32)fTextLeft;
  const int32 iScissorY = (int32)(oBounds.y + _oConfig.m_v2TextPadding.y);
  const int32 iScissorWidth = (int32)fVisibleWidth;
  const int32 iScissorHeight = (int32)(oBounds.height - _oConfig.m_v2TextPadding.y * 2.0f);
  if (iScissorWidth > 0 && iScissorHeight > 0)
  {
    BeginScissorMode(iScissorX, iScissorY, iScissorWidth, iScissorHeight);
    const int32 iTextY = (int32)(oBounds.y + (oBounds.height - _oConfig.m_iFontSize) * 0.5f);
    DrawText(_pText ? _pText->c_str() : "", (int32)(fTextLeft - s_fInputTextScroll), iTextY,
      _oConfig.m_iFontSize, oColors.m_cText);

    if (m_bHasFocus && std::fmod(GetTime() - s_dInputTextBlinkStart, 1.0) < 0.5)
    {
      const float fCursorX = fTextLeft + GetTextWidthUntil(*_pText, s_iInputTextCursor, _oConfig.m_iFontSize) -
        s_fInputTextScroll;
      DrawRectangle((int32)fCursorX, iTextY, 2, _oConfig.m_iFontSize, oColors.m_cText);
    }
    EndScissorMode();
  }
}
