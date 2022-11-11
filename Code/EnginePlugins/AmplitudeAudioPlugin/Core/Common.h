#pragma once

#define AMPLITUDE_ASSETS_DIR_NAME "amplitude_assets"
#define AMPLITUDE_PROJECT_DIR_NAME "amplitude_project"

static constexpr char kProjectRootPath[] = "sounds/" AMPLITUDE_PROJECT_DIR_NAME "/";
static constexpr char kAssetsRootPath[] = "sounds/" AMPLITUDE_ASSETS_DIR_NAME "/";
static constexpr char kDefaultBanksPath[] = "sounds/" AMPLITUDE_ASSETS_DIR_NAME "/soundbanks/";

static constexpr char kEngineConfigFile[] = "audio_config.json";
static constexpr char kBusesConfigFile[] = "buses.json";
static constexpr char kInitBankFile[] = "init.ambank";

static constexpr char kProjectFileExtension[] = ".json";
static constexpr char kSoundBankFileExtension[] = ".ambank";
static constexpr char kAssetBusFileExtension[] = ".ambus";
static constexpr char kAssetCollectionFileExtension[] = ".amcollection";
static constexpr char kAssetSoundFileExtension[] = ".amsound";
static constexpr char kAssetEventFileExtension[] = ".amevent";
static constexpr char kAssetEnvironmentFileExtension[] = ".amenv";
static constexpr char kAssetAttenuationFileExtension[] = ".amattenuation";
static constexpr char kAssetSwitchFileExtension[] = ".amswitch";
static constexpr char kAssetSwitchContainerFileExtension[] = ".amswitchcontainer";
static constexpr char kAssetRtpcFileExtension[] = ".amrtpc";
static constexpr char kAssetMediaFileExtension[] = ".ams";

// Project Folders
static constexpr char kEventsFolder[] = "events";
static constexpr char kRtpcFolder[] = "rtpc";
static constexpr char kSoundBanksFolder[] = "soundbanks";
static constexpr char kSoundsFolder[] = "sounds";
static constexpr char kSwitchesFolder[] = "switches";
static constexpr char kEffectsFolder[] = "effects";
static constexpr char kEnvironmentsFolder[] = "environments";

static constexpr char s_szAmplitudeMiddlewareName[] = "Amplitude";

static constexpr char s_szAmplitudeConfigKeyInitBank[] = "InitBank";
static constexpr char s_szAmplitudeConfigKeyEngineConfigFileName[] = "EngineConfigFileName";
