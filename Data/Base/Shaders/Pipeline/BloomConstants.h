#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

CONSTANT_BUFFER(ezBloomConstants, 3)
{
  FLOAT2(PixelSize);
  FLOAT1(BloomIntensity);
  UINT1(MipCount);

  UINT1(WorkGroupCount);
};
