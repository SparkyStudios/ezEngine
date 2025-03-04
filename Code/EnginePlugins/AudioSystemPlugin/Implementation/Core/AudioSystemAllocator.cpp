#include <AudioSystemPlugin/Core/AudioSystemAllocator.h>

EZ_IMPLEMENT_SINGLETON(ezAudioSystemAllocator);
EZ_IMPLEMENT_SINGLETON(ezAudioMiddlewareAllocator);

ezAudioSystemAllocator::ezAudioSystemAllocator()
  : ezAlignedHeapAllocator("AudioSystemAllocator")
  , m_SingletonRegistrar(this)
{
}

ezAudioMiddlewareAllocator::ezAudioMiddlewareAllocator(ezAudioSystemAllocator* pParentAllocator)
  : ezAlignedHeapAllocator("AudioMiddlewareAllocator", pParentAllocator)
  , m_SingletonRegistrar(this)
{
}

EZ_STATICLINK_FILE(AudioSystemPlugin, AudioSystemPlugin_Implementation_AudioSystemAllocator);
