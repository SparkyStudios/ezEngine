#pragma once

#include "../Common/Platforms.h"
#include "../Common/ConstantBufferMacros.h"

CONSTANT_BUFFER(ezUpscaleConstants, 3)
{
  UINT4(Const0);
  UINT4(Const1);
  UINT4(Const2);
  UINT4(Const3);
};
