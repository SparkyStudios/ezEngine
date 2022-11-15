#include <SparkLangPlugin/SparkLangPluginPCH.h>

#include <SparkLangPlugin/Core/Module.h>

#include <Foundation/Logging/Log.h>

// ez.Log.Log(level, text)
static SQInteger ezspLogLog(HSQUIRRELVM vm)
{
  if (sq_gettop(vm) == 3)
  {
    SQInteger level;
    sq_getinteger(vm, -2, &level);

    const SQChar* szText;
    sq_getstring(vm, -1, &szText);

    switch (level)
    {
      case ezLogMsgType::ErrorMsg:
        ezLog::Error(szText);
        break;
      case ezLogMsgType::SeriousWarningMsg:
        ezLog::SeriousWarning(szText);
        break;
      case ezLogMsgType::WarningMsg:
        ezLog::Warning(szText);
        break;
      case ezLogMsgType::SuccessMsg:
        ezLog::Success(szText);
        break;
      case ezLogMsgType::InfoMsg:
        ezLog::Info(szText);
        break;
      case ezLogMsgType::DevMsg:
        ezLog::Dev(szText);
        break;
      case ezLogMsgType::DebugMsg:
        ezLog::Debug(szText);
        break;
    }
  }
  else
  {
    if (const SQPRINTFUNCTION func = sq_geterrorfunc(vm))
      func(vm, _SC("function 'Log' requires exactly 2 arguments."));
  }

  return 0;
}

// ez.Log.Debug(text)
static SQInteger ezspLogDebug(HSQUIRRELVM vm)
{
  if (sq_gettop(vm) == 2)
  {
    const SQChar* szText;
    sq_getstring(vm, -1, &szText);

    ezLog::Debug(szText);
  }

  return 0;
}

// ez.Log.Error(text)
static SQInteger ezspLogError(HSQUIRRELVM vm)
{
  if (sq_gettop(vm) == 2)
  {
    const SQChar* szText;
    sq_getstring(vm, -1, &szText);

    ezLog::Error(szText);
  }

  return 0;
}

// ez.Log.Info(text)
static SQInteger ezspLogInfo(HSQUIRRELVM vm)
{
  if (sq_gettop(vm) == 2)
  {
    const SQChar* szText;
    sq_getstring(vm, -1, &szText);

    ezLog::Info(szText);
  }

  return 0;
}

// ez.Log.SeriousWarning(text)
static SQInteger ezspLogSeriousWarning(HSQUIRRELVM vm)
{
  if (sq_gettop(vm) == 2)
  {
    const SQChar* szText;
    sq_getstring(vm, -1, &szText);

    ezLog::SeriousWarning(szText);
  }

  return 0;
}

// ez.Log.Success(text)
static SQInteger ezspLogSuccess(HSQUIRRELVM vm)
{
  if (sq_gettop(vm) == 2)
  {
    const SQChar* szText;
    sq_getstring(vm, -1, &szText);

    ezLog::Success(szText);
  }

  return 0;
}

// ez.Log.Warning(text)
static SQInteger ezspLogWarning(HSQUIRRELVM vm)
{
  if (sq_gettop(vm) == 2)
  {
    const SQChar* szText;
    sq_getstring(vm, -1, &szText);

    ezLog::Warning(szText);
  }

  return 0;
}

SQRESULT ezSparkLangModule::ezLog(Sqrat::Table& module)
{
  Sqrat::Table Log(module.GetVM());
  Sqrat::ConstTable LogLevel(module.GetVM());

  LogLevel
    .Const(_SC("Debug"), ezLogMsgType::DebugMsg)
    .Const(_SC("Error"), ezLogMsgType::ErrorMsg)
    .Const(_SC("Info"), ezLogMsgType::InfoMsg)
    .Const(_SC("SeriousWarning"), ezLogMsgType::SeriousWarningMsg)
    .Const(_SC("Success"), ezLogMsgType::SuccessMsg)
    .Const(_SC("Warning"), ezLogMsgType::WarningMsg);

  Log
    .Bind(_SC("LogLevel"), LogLevel)
    .SquirrelFunc(_SC("Log"), ezspLogLog, 3, _SC(".is"))
    .SquirrelFunc(_SC("Debug"), ezspLogDebug, 2, _SC(".s"))
    .SquirrelFunc(_SC("Error"), ezspLogError, 2, _SC(".s"))
    .SquirrelFunc(_SC("Info"), ezspLogInfo, 2, _SC(".s"))
    .SquirrelFunc(_SC("SeriousWarning"), ezspLogSeriousWarning, 2, _SC(".s"))
    .SquirrelFunc(_SC("Success"), ezspLogSuccess, 2, _SC(".s"))
    .SquirrelFunc(_SC("Warning"), ezspLogWarning, 2, _SC(".s"));

  module.Bind(_SC("Log"), Log);

  return SQ_OK;
}
