#pragma once

#include "../Common/ConstantBufferMacros.h"
#include "../Common/Platforms.h"

CONSTANT_BUFFER(ezFilmGrainConstants, 3)
{
  FLOAT1(Intensity);
  FLOAT1(Speed);
  FLOAT1(Mean);
  FLOAT1(Variance);
};
