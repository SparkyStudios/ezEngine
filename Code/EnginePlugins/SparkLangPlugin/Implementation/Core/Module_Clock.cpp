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

SQRESULT ezSparkLangModule::ezClock(Sqrat::Table& module)
{
  Sqrat::Table Clock(module.GetVM());

  Clock
    .SquirrelFunc(_SC("SetPaused"), ezspClockSetPaused, 2, _SC(".b"))
    .SquirrelFunc(_SC("IsPaused"), ezspClockIsPaused, 1, _SC("."))
    .SquirrelFunc(_SC("GetAccumulatedTime"), ezspClockGetAccumulatedTime, 1, _SC("."))
    .SquirrelFunc(_SC("GetTimeDiff"), ezspClockGetTimeDiff, 1, _SC("."))
    .SquirrelFunc(_SC("GetSpeed"), ezspClockGetSpeed, 1, _SC("."))
    .SquirrelFunc(_SC("SetSpeed"), ezspClockSetSpeed, 2, _SC(".f"));

  module.Bind(_SC("Clock"), Clock);

  return SQ_OK;
}
