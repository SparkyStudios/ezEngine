#pragma once

#include "../Common/Platforms.h"
#include "../Common/ConstantBufferMacros.h"

CONSTANT_BUFFER(ezFrameConstants, 3)
{
  FLOAT3(Color);
  FLOAT1(Ratio);
};
