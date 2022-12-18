#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

CONSTANT_BUFFER(ezSharpeningConstants, 3)
{
  FLOAT1(Strength);
};
