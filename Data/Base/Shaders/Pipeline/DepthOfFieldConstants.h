#pragma once

#include "../Common/Platforms.h"
#include "../Common/ConstantBufferMacros.h"

CONSTANT_BUFFER(ezDepthOfFieldConstants, 3)
{
  FLOAT1(Radius);
};
