#include <AmplitudeAudioPlugin/AmplitudeAudioPluginPCH.h>

#include <AmplitudeAudioPlugin/AmplitudeAudioSingleton.h>
#include <AmplitudeAudioPlugin/Resources/AudioControlCollectionResource.h>

#include <AudioSystemPlugin/Core/AudioSystem.h>

#include <Core/World/World.h>
#include <Foundation/Configuration/Startup.h>
#include <GameEngine/GameApplication/GameApplication.h>

static ezAmplitude* s_pAmplitudeSingleton = nullptr;

EZ_BEGIN_SUBSYSTEM_DECLARATION(SparkyStudios, AmplitudeAudioPlugin)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
      "Core",
      "AudioSystem"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
    s_pAmplitudeSingleton = EZ_AUDIOSYSTEM_NEW(ezAmplitude);

    // Audio Control Collection Resources
    {
      ezResourceManager::RegisterResourceForAssetType("Audio Control Collection", ezGetStaticRTTI<ezAmplitudeAudioControlCollectionResource>());

      ezAmplitudeAudioControlCollectionResourceDescriptor desc;
      const auto hResource = ezResourceManager::CreateResource<ezAmplitudeAudioControlCollectionResource>("AudioControlCollectionMissing", std::move(desc), "Fallback for missing audio control collections.");
      ezResourceManager::SetResourceTypeMissingFallback<ezAmplitudeAudioControlCollectionResource>(hResource);
    }

    ezAudioSystem::GetSingleton()->Startup();
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    ezAudioSystem::GetSingleton()->Shutdown();

    ezAmplitudeAudioControlCollectionResource::CleanupDynamicPluginReferences();

    EZ_AUDIOSYSTEM_DELETE(s_pAmplitudeSingleton);
  }

EZ_END_SUBSYSTEM_DECLARATION;

#if defined(AM_WINDOWS_VERSION)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#endif

EZ_STATICLINK_FILE(AmplitudeAudioPlugin, AmplitudeAudioPlugin_AmplitudeAudioPluginStartup);
