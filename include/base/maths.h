#pragma once

#include <random>

// ------------------------------------------------------------------------------------------

static std::mt19937 s_oGen;
static std::uniform_real_distribution<float> s_oRandomGenerator;

// ------------------------------------------------------------------------------------------

struct CVector2D
{
  constexpr CVector2D()
  {

  }

  constexpr CVector2D(const float _x, const float _y)
    : x(_x)
    , y(_y)
  {
    
  }

  CVector2D& operator=(const CVector2D& _v2Other)
  {
    x = _v2Other.x;
    y = _v2Other.y;
    
    return *this;
  }

  CVector2D operator+(const CVector2D& _v2Other) const
  {
    CVector2D v = *this;
    v.x += _v2Other.x;
    v.y += _v2Other.y;
    
    return v;
  }

  CVector2D operator-(const CVector2D& _v2Other) const
  {
    CVector2D v = *this;
    v.x -= _v2Other.x;
    v.y -= _v2Other.y;
    
    return v;
  }

  CVector2D& operator+=(const CVector2D& _v2Other)
  {
    x += _v2Other.x;
    y += _v2Other.y;
    
    return *this;
  }


  CVector2D operator*(const CVector2D& _v2Other) const
  {
    CVector2D v = *this;
    v.x *= _v2Other.x;
    v.y *= _v2Other.y;
    
    return v;
  }

  CVector2D& operator*=(const CVector2D& _v2Other)
  {
    x *= _v2Other.x;
    y *= _v2Other.y;
    
    return *this;
  }

  CVector2D operator+(const float _fValue) const
  {
    CVector2D v = *this;
    v.x += _fValue;
    v.y += _fValue;
    
    return v;
  }

  CVector2D operator*(const float _fValue) const
  {
    CVector2D v = *this;
    v.x *= _fValue;
    v.y *= _fValue;

    return v;
  }


  // ---------------

 static const CVector2D v2ZERO;

  float x = 0.0f;
  float y = 0.0f;
};

inline const CVector2D CVector2D::v2ZERO = CVector2D(0.0f, 0.0f);

inline CVector2D operator*(const float _fValue, const CVector2D& _v2Vector)
{
  return _v2Vector * _fValue;
}

// ------------------------------------------------------------------------------------------

inline float Clamp(const float _fValue, const float _fInferior, const float _fSuperior)
{
  return _fValue < _fInferior ? (_fInferior) : (_fValue > _fSuperior ? (_fSuperior) : (_fValue));
}

// ------------------------------------------------------------------------------------------

inline float Remap(const float _fValue, const float _fMinOriginal, const float _fMaxOriginal, 
  const float _fMinNew, const float _fMaxNew)
{
  return _fMinNew + ((_fValue - _fMinOriginal) * (_fMaxNew - _fMinNew)) / (_fMaxOriginal - _fMinOriginal);
}

// ------------------------------------------------------------------------------------------

inline void InitializeRandom(uint32_t _uSeed = 0u)
{
  std::random_device rd;
  s_oGen.seed(_uSeed != 0u ? _uSeed : rd());
}

// ------------------------------------------------------------------------------------------

inline std::mt19937& GetRandomEngine()
{
  return s_oGen;
}

// ------------------------------------------------------------------------------------------

inline float GetRandom(float _fMin, float _fMax)
{
  if (_fMin == _fMax)
    return _fMin;
  if (_fMin > _fMax)
  {
    std::swap(_fMin, _fMax);
  }

  s_oRandomGenerator.param(std::uniform_real_distribution<float>::param_type(_fMin, _fMax));
  return s_oRandomGenerator(s_oGen);
}

// ------------------------------------------------------------------------------------------

inline float Lerp(float _fA, float _fB, float _fD)
{
  return _fA + _fD * (_fB - _fA);
}

// ------------------------------------------------------------------------------------------

inline CVector2D Lerp(CVector2D _v2A, CVector2D _v2B, float _fD)
{
  return _v2A + _fD * (_v2B - _v2A);
}

// ------------------------------------------------------------------------------------------

inline CVector2D HermiteInterpolation(const CVector2D& _v2PosA, const CVector2D& _v2VelA,
  const CVector2D& _v2PosB, const CVector2D& _v2VelB, float _fAlpha, float _fDt)
{
  const float fT2 = _fAlpha * _fAlpha;
  const float fT3 = fT2 * _fAlpha;

  const float fH00 = 2.0f * fT3 - 3.0f * fT2 + 1.0f;
  const float fH10 = fT3 - 2.0f * fT2 + _fAlpha;
  const float fH01 = -2.0f * fT3 + 3.0f * fT2;
  const float fH11 = fT3 - fT2;

  return  fH00 * _v2PosA +
          fH10 * _v2VelA * _fDt +
          fH01 * _v2PosB +
          fH11 * _v2VelB * _fDt;
}

// ------------------------------------------------------------------------------------------