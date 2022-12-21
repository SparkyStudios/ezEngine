#pragma once

#include "../Common/Platforms.h"
#include "../Common/ConstantBufferMacros.h"

CONSTANT_BUFFER(ezTonemapConstants, 3)
{
  UINT1(ToneMappingMode);
};
