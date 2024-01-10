#pragma once

#include <RHI/RHIDLL.h>

#include <RHI/Core.h>
#include <RHI/Resource.h>

namespace RHI
{
  /// \brief Describes a single input layout element.
  struct spInputElementDescription : ezHashableStruct<spInputElementDescription>
  {
    /// \brief The input layout element name.
    ezHashedString m_sName;

    /// \brief The input layout element semantic/location.
    ezEnum<spInputElementLocationSemantic> m_eSemantic;

    /// \brief The input layout element format.
    ezEnum<spInputElementFormat> m_eFormat;

    /// \brief The input layout element offset.
    ezUInt32 m_uiOffset{0};

    spInputElementDescription() = default;

    spInputElementDescription(ezStringView szName, const ezEnum<spInputElementLocationSemantic>& eSemantic, const ezEnum<spInputElementFormat>& eFormat, ezUInt32 uiOffset = 0)
      : m_eSemantic(eSemantic)
      , m_eFormat(eFormat)
      , m_uiOffset(uiOffset)
    {
      m_sName.Assign(szName);
    }

    spInputElementDescription(ezStringView szName, ezUInt32 uiLocation, const ezEnum<spInputElementFormat>& eFormat, ezUInt32 uiOffset = 0)
      : m_eSemantic(static_cast<spInputElementLocationSemantic::Enum>(uiLocation))
      , m_eFormat(eFormat)
      , m_uiOffset(uiOffset)
    {
      m_sName.Assign(szName);
    }

    /// \brief Compares this \a spInputElementDescription with the given \a other description for equality.
    EZ_ALWAYS_INLINE bool operator==(const spInputElementDescription& other) const
    {
      return other.m_sName == m_sName && other.m_eSemantic == m_eSemantic && other.m_eFormat == m_eFormat && other.m_uiOffset == m_uiOffset;
    }

    /// \brief Compares this \a spInputElementDescription with the given \a other description for inequality.
    EZ_ALWAYS_INLINE bool operator!=(const spInputElementDescription& other) const { return !(*this == other); }
  };

  struct spInputLayoutDescription : ezHashableStruct<spInputLayoutDescription>
  {
    ezUInt32 m_uiStride{0};
    ezStaticArray<spInputElementDescription, 32> m_Elements{};
    ezUInt32 m_uiInstanceStepRate{0};

    spInputLayoutDescription() = default;

    ~spInputLayoutDescription()
    {
      m_Elements.Clear();
    }

    spInputLayoutDescription(const spInputLayoutDescription& copy)
    {
      m_uiStride = copy.m_uiStride;
      m_uiInstanceStepRate = copy.m_uiInstanceStepRate;

      m_Elements.EnsureCount(copy.m_Elements.GetCount());
      for (ezUInt32 i = 0, l = m_Elements.GetCount(); i < l; ++i)
      {
        const auto& element = copy.m_Elements[i];
        m_Elements[i].m_eFormat = element.m_eFormat;
        m_Elements[i].m_eSemantic = element.m_eSemantic;
        m_Elements[i].m_uiOffset = element.m_uiOffset;
        m_Elements[i].m_sName = element.m_sName;
      }
    }

    spInputLayoutDescription(spInputLayoutDescription&& move) noexcept
    {
      ezMath::Swap(m_uiStride, move.m_uiStride);
      ezMath::Swap(m_uiInstanceStepRate, move.m_uiInstanceStepRate);
      ezMath::Swap(m_Elements, move.m_Elements);
    }

    EZ_ALWAYS_INLINE spInputLayoutDescription& operator=(const spInputLayoutDescription& copy)
    {
      m_uiStride = copy.m_uiStride;
      m_uiInstanceStepRate = copy.m_uiInstanceStepRate;

      m_Elements = copy.m_Elements;

      return *this;
    }

    EZ_ALWAYS_INLINE spInputLayoutDescription& operator=(spInputLayoutDescription&& move) noexcept
    {
      ezMath::Swap(m_uiStride, move.m_uiStride);
      ezMath::Swap(m_uiInstanceStepRate, move.m_uiInstanceStepRate);
      ezMath::Swap(m_Elements, move.m_Elements);

      return *this;
    }

    /// \brief Compares this \a spInputLayoutDescription with the given \a other description for equality.
    EZ_ALWAYS_INLINE bool operator==(const spInputLayoutDescription& other) const
    {
      bool bElementsAreEquals = true;
      for (ezUInt32 i = 0, l = m_Elements.GetCount(); i < l; i++)
      {
        if (m_Elements[i] == other.m_Elements[i])
          continue;

        bElementsAreEquals = false;
        break;
      }

      return bElementsAreEquals && m_uiStride == other.m_uiStride && m_uiInstanceStepRate == other.m_uiInstanceStepRate;
    }

    /// \brief Compares this \a spInputLayoutDescription with the given \a other description for inequality.
    EZ_ALWAYS_INLINE bool operator!=(const spInputLayoutDescription& other) const { return !(*this == other); }
  };

  /// \brief Base class for an input layout.
  class SP_RHI_DLL spInputLayout : public spMappableResource
  {
    EZ_ADD_DYNAMIC_REFLECTION(spInputLayout, spMappableResource);

  public:
    EZ_NODISCARD EZ_ALWAYS_INLINE const spInputLayoutDescription& GetDescription() const { return m_Description; }

  protected:
    explicit spInputLayout(spInputLayoutDescription description);

    spInputLayoutDescription m_Description;
  };
} // namespace RHI

#include <RHI/Implementation/Input_inl.h>
