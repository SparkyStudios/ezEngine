#pragma once

#include <RHI/RHIDLL.h>

#include <Foundation/Algorithm/HashableStruct.h>
#include <Foundation/Strings/HashedString.h>

#include <RHI/Core.h>
#include <RHI/Resource.h>

/// \brief Describes a single input layout element.
struct SP_RHI_DLL spInputElementDescription : ezHashableStruct<spInputElementDescription>
{
  /// \brief The input layout element name.
  ezHashedString m_sName;

  /// \brief The input layout element semantic/location.
  ezUInt32 m_uiLocation;

  /// \brief The input layout element format.
  ezEnum<spInputElementFormat> m_eFormat;

  /// \brief The input layout element offset.
  ezUInt32 m_uiOffset;

  spInputElementDescription(const char* szName, ezEnum<spInputElementLocationSemantic> eSemantic, ezEnum<spInputElementFormat> eFormat, ezUInt32 uiOffset = 0)
    : m_uiLocation(static_cast<ezUInt32>(eSemantic.GetValue()))
    , m_eFormat(eFormat)
    , m_uiOffset(uiOffset)
  {
    m_sName.Assign(szName);
  }

  spInputElementDescription(const char* szName, ezUInt32 uiLocation, ezEnum<spInputElementFormat> eFormat, ezUInt32 uiOffset = 0)
    : m_uiLocation(uiLocation)
    , m_eFormat(eFormat)
    , m_uiOffset(uiOffset)
  {
    m_sName.Assign(szName);
  }

  /// \brief Compares this \see InputElementDescription with the given \a other description for equality.
  EZ_ALWAYS_INLINE bool operator==(const spInputElementDescription& other) const
  {
    return other.m_sName == m_sName && other.m_uiLocation == m_uiLocation && other.m_eFormat == m_eFormat && other.m_uiOffset == m_uiOffset;
  }

  /// \brief Compares this \see InputElementDescription with the given \a other description for inequality.
  EZ_ALWAYS_INLINE bool operator!=(const spInputElementDescription& other) const { return !(*this == other); }
};

struct SP_RHI_DLL spInputLayoutDescription : public ezHashableStruct<spInputLayoutDescription>
{
  ezUInt32 m_uiStride;
  ezDynamicArray<spInputElementDescription> m_Elements;
  ezUInt32 m_uiInstanceStepRate;

  /// \brief Compares this \see InputLayoutDescription with the given \a other description for equality.
  EZ_ALWAYS_INLINE bool operator==(const spInputLayoutDescription& other) const
  {
    bool elementsAreEquals = true;
    for (int i = 0, l = m_Elements.GetCount(); i < l; i++)
    {
      if (m_Elements[i] == other.m_Elements[i])
        continue;

      elementsAreEquals = false;
      break;
    }

    return elementsAreEquals && m_uiStride == other.m_uiStride && m_uiInstanceStepRate == other.m_uiInstanceStepRate;
  }

  /// \brief Compares this \see InputLayoutDescription with the given \a other description for inequality.
  EZ_ALWAYS_INLINE bool operator!=(const spInputLayoutDescription& other) const { return !(*this == other); }
};

/// \brief Base class for an input layout.
class SP_RHI_DLL spInputLayout : public spMappableResource
{
};

EZ_DECLARE_REFLECTABLE_TYPE(SP_RHI_DLL, spInputLayout);
