#include <AudioSystemPlugin/AudioSystemPluginPCH.h>

#include <AudioSystemPlugin/Core/AudioSystem.h>
#include <AudioSystemPlugin/Core/AudioSystemAllocator.h>

#include <Foundation/Configuration/Startup.h>

static ezAudioSystemAllocator* s_pAudioSystemAllocator = nullptr;
static ezAudioMiddlewareAllocator* s_pAudioMiddlewareAllocator = nullptr;
static ezAudioSystem* s_pAudioSystemSingleton = nullptr;

EZ_BEGIN_SUBSYSTEM_DECLARATION(AudioSystem, AudioSystemPlugin)

  // clang-format off
  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core"
  END_SUBSYSTEM_DEPENDENCIES
  // clang-format on

  ON_BASESYSTEMS_STARTUP
  {
    s_pAudioSystemAllocator = EZ_DEFAULT_NEW(ezAudioSystemAllocator);
    s_pAudioMiddlewareAllocator = EZ_DEFAULT_NEW(ezAudioMiddlewareAllocator, s_pAudioSystemAllocator);
    s_pAudioSystemSingleton = EZ_DEFAULT_NEW(ezAudioSystem);
  }

  ON_CORESYSTEMS_STARTUP
  {
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    EZ_DEFAULT_DELETE(s_pAudioSystemSingleton);
    EZ_DEFAULT_DELETE(s_pAudioMiddlewareAllocator);
    EZ_DEFAULT_DELETE(s_pAudioSystemAllocator);
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
    ezGameApplicationBase::GetGameApplicationBaseInstance()->m_ExecutionEvents.AddEventHandler(&ezAudioSystem::GameApplicationEventHandler);
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    ezGameApplicationBase::GetGameApplicationBaseInstance()->m_ExecutionEvents.RemoveEventHandler(&ezAudioSystem::GameApplicationEventHandler);
  }

EZ_END_SUBSYSTEM_DECLARATION;

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_AudioSystemStartup);
