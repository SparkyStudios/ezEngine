#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Core/Module.h>
#include <SparkLangPlugin/Core/ScriptContext.h>

#include <Core/World/World.h>
#include <Foundation/Time/Time.h>

// ez.Clock.Time <=> ez.Clock.Time
SQInteger ezspClockTimeCompare(HSQUIRRELVM vm)
{
  Sqrat::Var<ezTime> lhs(vm, 1);
  Sqrat::Var<ezTime> rhs(vm, 2);

  if (lhs.value > rhs.value)
    sq_pushinteger(vm, 1);
  else if (lhs.value < rhs.value)
    sq_pushinteger(vm, -1);
  else
    sq_pushinteger(vm, 0);

  return 1;
}

SQRESULT ezSparkLangModule::ezClock(Sqrat::Table& module)
{
  Sqrat::Class<class ezClock> ClockClass(module.GetVM(), _SC("ezClock"));
  Sqrat::Class<ezTime> TimeClass(module.GetVM(), _SC("ezTime"));

  ClockClass
    .Ctor<const char*>()
    .StaticFunc(_SC("GetGlobalClock"), &ezClock::GetGlobalClock)
    .Func(_SC("Reset"), &ezClock::Reset)
    .Func(_SC("Update"), &ezClock::Update)
    .Func(_SC("SetPaused"), &ezClock::SetPaused)
    .Func(_SC("GetPaused"), &ezClock::GetPaused)
    .Prop(_SC("FixedTimeStep"), &ezClock::GetFixedTimeStep, &ezClock::SetFixedTimeStep)
    .Prop(_SC("AccumulatedTime"), &ezClock::GetAccumulatedTime, &ezClock::SetAccumulatedTime)
    .Func(_SC("GetTimeDiff"), &ezClock::GetTimeDiff)
    .Prop(_SC("Speed"), &ezClock::GetSpeed, &ezClock::SetSpeed)
    .Prop(_SC("MinimumTimeStep"), &ezClock::GetMinimumTimeStep, &ezClock::SetMinimumTimeStep)
    .Prop(_SC("MaximumTimeStep"), &ezClock::GetMaximumTimeStep, &ezClock::SetMaximumTimeStep)
    .Func(_SC("ClockName"), &ezClock::SetClockName)
    .Func(_SC("GetClockName"), &ezClock::GetClockName);

  TimeClass
    .Ctor()
    .StaticFunc(_SC("Now"), &ezTime::Now)
    .StaticFunc(_SC("Nanoseconds"), &ezTime::Nanoseconds)
    .StaticFunc(_SC("Microseconds"), &ezTime::Microseconds)
    .StaticFunc(_SC("Milliseconds"), &ezTime::Milliseconds)
    .StaticFunc(_SC("Seconds"), &ezTime::Seconds)
    .StaticFunc(_SC("Minutes"), &ezTime::Minutes)
    .StaticFunc(_SC("Hours"), &ezTime::Hours)
    .StaticFunc(_SC("Zero"), &ezTime::Zero)
    .Func(_SC("SetZero"), &ezTime::SetZero)
    .Prop(_SC("IsZero"), &ezTime::IsZero)
    .Prop(_SC("IsNegative"), &ezTime::IsNegative)
    .Prop(_SC("IsPositive"), &ezTime::IsPositive)
    .Prop(_SC("IsZeroOrNegative"), &ezTime::IsZeroOrNegative)
    .Prop(_SC("IsZeroOrPositive"), &ezTime::IsZeroOrPositive)
    .Prop(_SC("Nanoseconds"), &ezTime::GetNanoseconds)
    .Prop(_SC("Microseconds"), &ezTime::GetMicroseconds)
    .Prop(_SC("Milliseconds"), &ezTime::GetMilliseconds)
    .Prop(_SC("Seconds"), &ezTime::GetSeconds)
    .Prop(_SC("Minutes"), &ezTime::GetMinutes)
    .Prop(_SC("Hours"), &ezTime::GetHours)
    .Func(_SC("_add"), &ezTime::operator+)
    .Func(_SC("_sub"), static_cast<ezTime (ezTime::*)(const ezTime&) const>(&ezTime::operator-))
    .SquirrelFunc(_SC("_cmp"), ezspClockTimeCompare, 2, _SC(".x"));

  module
    .Bind(_SC("Clock"), ClockClass)
    .Bind(_SC("Time"), TimeClass);

  return SQ_OK;
}
