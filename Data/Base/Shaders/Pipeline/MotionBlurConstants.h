#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

CONSTANT_BUFFER(ezMotionBlurConstants, 3)
{
  FLOAT1(MotionBlurStrength);
};
