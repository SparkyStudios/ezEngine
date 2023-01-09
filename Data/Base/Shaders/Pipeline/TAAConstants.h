#pragma once

#include "../Common/Platforms.h"
#include "../Common/ConstantBufferMacros.h"

CONSTANT_BUFFER(ezTAAConstants, 3)
{
  BOOL1(UpsampleEnabled);
};
