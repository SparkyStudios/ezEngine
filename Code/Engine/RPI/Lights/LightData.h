#pragma once

#include <RPI/RPIDLL.h>

struct alignas(16) spLightData
{
  ezUInt32 m_uiAmbientColor;
  ezUInt32 m_uiDiffuseColor;
  ezUInt32 m_uiSpecularColor;
  float m_fIntensity;

  ezVec3 m_vPosition;
  ezUInt32 m_uiDirection;

  ezVec3 m_vViewSpacePosition;
  ezUInt32 m_uiViewSpaceDirection;

  ezUInt32 m_uiType;
  ezUInt32 m_uiRange;
  float m_fRadius;
  bool m_bEnabled;
};

EZ_CHECK_AT_COMPILETIME(sizeof(spLightData) == 64);
