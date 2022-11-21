#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Core/Module.h>
#include <SparkLangPlugin/Implementation/Core/Module_Component.h>

#include <Foundation/Math/Random.h>

SQRESULT ezSparkLangModule::ezRandom(Sqrat::Table& module)
{
  Sqrat::Class<class ezRandom> RandomClass(module.GetVM(), _SC("ezRandom"));
  RandomClass
    .Ctor()
    .Func(_SC("Initialize"), &ezRandom::Initialize)
    .Func(_SC("InitializeFromCurrentTime"), &ezRandom::InitializeFromCurrentTime)
    .Func(_SC("UInt"), &ezRandom::UInt)
    .Func(_SC("UIntInRange"), &ezRandom::UIntInRange)
    .Func(_SC("IntInRange"), &ezRandom::IntInRange)
    .Func(_SC("IntMinMax"), &ezRandom::IntMinMax)
    .Func(_SC("Bool"), &ezRandom::Bool)
    .Func(_SC("DoubleZeroToOneExclusive"), &ezRandom::DoubleZeroToOneExclusive)
    .Func(_SC("DoubleZeroToOneInclusive"), &ezRandom::DoubleZeroToOneInclusive)
    .Func(_SC("DoubleInRange"), &ezRandom::DoubleInRange)
    .Func(_SC("DoubleMinMax"), &ezRandom::DoubleMinMax)
    .Func(_SC("DoubleVariance"), &ezRandom::DoubleVariance)
    .Func(_SC("DoubleVarianceAroundZero"), &ezRandom::DoubleVarianceAroundZero)
    .Func(_SC("FloatZeroToOneExclusive"), &ezRandom::FloatZeroToOneExclusive)
    .Func(_SC("FloatZeroToOneInclusive"), &ezRandom::FloatZeroToOneInclusive)
    .Func(_SC("FloatInRange"), &ezRandom::FloatInRange)
    .Func(_SC("FloatMinMax"), &ezRandom::FloatMinMax)
    .Func(_SC("FloatVariance"), &ezRandom::FloatVariance)
    .Func(_SC("FloatVarianceAroundZero"), &ezRandom::FloatVarianceAroundZero);

  module.Bind(_SC("Random"), RandomClass);

  return SQ_OK;
}
