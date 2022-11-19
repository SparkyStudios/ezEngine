#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Core/Module.h>
#include <SparkLangPlugin/Core/ScriptContext.h>

#include <Core/World/World.h>
#include <Foundation/Time/Time.h>

// ez.Clock.SetSpeed(number)
static SQInteger ezspClockSetSpeed(HSQUIRRELVM vm)
{
  const auto* pContext = ezSparkLangScriptContext::FromVM(vm);

  if (pContext == nullptr)
  {
    sq_throwerror(vm, "The script has no context");
    return 0;
  }

  if (sq_gettop(vm) == 2)
  {
    SQFloat speed = 1.0f;
    if (SQ_SUCCEEDED(sq_getfloat(vm, -1, &speed)))
      pContext->GetWorld()->GetClock().SetSpeed(speed);
  }
  else
  {
    sq_throwerror(vm, _SC("ez.Clock.SetSpeed called without a speed parameter."));
  }

  return 0;
}

// ez.Clock.GetSpeed(): number
static SQInteger ezspClockGetSpeed(HSQUIRRELVM vm)
{
  const auto* pContext = ezSparkLangScriptContext::FromVM(vm);

  if (pContext == nullptr)
  {
    sq_throwerror(vm, "The script has no context");
    return 0;
  }

  if (sq_gettop(vm) == 1)
  {
    Sqrat::PushVar(vm, pContext->GetWorld()->GetClock().GetSpeed());
  }

  return 1;
}

// ez.Clock.GetTimeDiff(): number
static SQInteger ezspClockGetTimeDiff(HSQUIRRELVM vm)
{
  const auto* pContext = ezSparkLangScriptContext::FromVM(vm);

  if (pContext == nullptr)
  {
    sq_throwerror(vm, "The script has no context");
    return 0;
  }

  if (sq_gettop(vm) == 1)
  {
    Sqrat::PushVar(vm, pContext->GetWorld()->GetClock().GetTimeDiff().AsFloatInSeconds());
  }

  return 1;
}

// ez.Clock.GetTimeDiff(): number
static SQInteger ezspClockGetAccumulatedTime(HSQUIRRELVM vm)
{
  const auto* pContext = ezSparkLangScriptContext::FromVM(vm);

  if (pContext == nullptr)
  {
    sq_throwerror(vm, "The script has no context");
    return 0;
  }

  if (sq_gettop(vm) == 1)
  {
    Sqrat::PushVar(vm, pContext->GetWorld()->GetClock().GetAccumulatedTime().AsFloatInSeconds());
  }

  return 1;
}

// ez.Clock.IsPause(): bool
static SQInteger ezspClockIsPaused(HSQUIRRELVM vm)
{
  const auto* pContext = ezSparkLangScriptContext::FromVM(vm);

  if (pContext == nullptr)
  {
    sq_throwerror(vm, "The script has no context");
    return 0;
  }

  if (sq_gettop(vm) == 1)
  {
    Sqrat::PushVar(vm, pContext->GetWorld()->GetClock().GetPaused());
  }

  return 1;
}

// ez.Clock.SetPaused(bool)
static SQInteger ezspClockSetPaused(HSQUIRRELVM vm)
{
  const auto* pContext = ezSparkLangScriptContext::FromVM(vm);

  if (pContext == nullptr)
  {
    sq_throwerror(vm, "The script has no context");
    return 0;
  }

  if (sq_gettop(vm) == 1)
  {
    SQBool paused = false;
    if (SQ_SUCCEEDED(sq_getbool(vm, -1, &paused)))
      pContext->GetWorld()->GetClock().SetPaused(paused);
  }

  return 0;
}

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
  Sqrat::Table Clock(module.GetVM());

  Sqrat::Class<ezTime> TimeClass(module.GetVM(), _SC("ezTime"));
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

  Clock
    .Bind(_SC("Time"), TimeClass)
    .SquirrelFunc(_SC("SetPaused"), ezspClockSetPaused, 2, _SC(".b"))
    .SquirrelFunc(_SC("IsPaused"), ezspClockIsPaused, 1, _SC("."))
    .SquirrelFunc(_SC("GetAccumulatedTime"), ezspClockGetAccumulatedTime, 1, _SC("."))
    .SquirrelFunc(_SC("GetTimeDiff"), ezspClockGetTimeDiff, 1, _SC("."))
    .SquirrelFunc(_SC("GetSpeed"), ezspClockGetSpeed, 1, _SC("."))
    .SquirrelFunc(_SC("SetSpeed"), ezspClockSetSpeed, 2, _SC(".f"));

  module.Bind(_SC("Clock"), Clock);

  return SQ_OK;
}
