#pragma once

#include "../Common/Platforms.h"
#include "../Common/ConstantBufferMacros.h"

CONSTANT_BUFFER(ezChromaticAberrationConstants, 3)
{
  FLOAT1(Strength);
};
