#include "RHI/Resource.h"
#include <RHI/RHIPCH.h>

#include <RHI/Buffer.h>
#include <RHI/Device.h>

#pragma region spBufferRange

spBufferRange::spBufferRange(spBufferRangeDescription description)
  : spShaderResource()
  , m_Description(std::move(description))
{
}

#pragma endregion

#pragma region spBuffer

spBuffer::spBuffer(spBufferDescription description)
  : spMappableResource()
  , m_Description(std::move(description))
  , m_BufferRanges()
{
}

void spBuffer::InitRanges()
{
  m_uiBufferCount = m_Description.m_eUsage.IsSet(spBufferUsage::TripleBuffered)   ? 3
                    : m_Description.m_eUsage.IsSet(spBufferUsage::DoubleBuffered) ? 2
                                                                                  : 1;

  m_BufferRanges.SetCount(m_uiBufferCount);

  ezUInt32 uiBufferAlignmentMinOffset = 1;

  if (m_Description.m_eUsage.IsSet(spBufferUsage::ConstantBuffer))
    uiBufferAlignmentMinOffset = m_pDevice->GetConstantBufferMinOffsetAlignment();
  else if (m_Description.m_eUsage.IsAnySet(spBufferUsage::StructuredBufferReadOnly | spBufferUsage::StructuredBufferReadWrite))
    uiBufferAlignmentMinOffset = m_pDevice->GetStructuredBufferMinOffsetAlignment();

  m_uiBufferAlignedSize = static_cast<ezUInt32>(ezMath::Ceil(static_cast<double>(m_Description.m_uiSize) / uiBufferAlignmentMinOffset)) * uiBufferAlignmentMinOffset;

  for (ezUInt32 i = 0; i < m_uiBufferCount; i++)
  {
    spBufferRangeDescription desc(
      GetHandle(),
      i * m_uiBufferAlignedSize,
      m_Description.m_uiSize,
      spFenceDescription(false));

    m_BufferRanges[i] = m_pDevice->GetResourceFactory()->CreateBufferRange(desc);
  }
}

#pragma endregion

EZ_STATICLINK_FILE(RHI, RHI_Implementation_Buffer);
