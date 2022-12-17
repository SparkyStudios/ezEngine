#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

struct EZ_SHADER_STRUCT ezAtomicCounterBuffer
{
  UINT1(Value);
};

#if EZ_DISABLED(PLATFORM_SHADER)
EZ_DEFINE_AS_POD_TYPE(ezAtomicCounterBuffer);
#endif

CONSTANT_BUFFER(ezBloomConstants, 3)
{
  FLOAT2(PixelSize);
  FLOAT1(BloomIntensity);
  UINT1(MipCount);

  UINT1(WorkGroupCount);
};
