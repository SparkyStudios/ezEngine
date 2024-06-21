#pragma once

#include <RHIMTL/RHIMTLDLL.h>

#include <RHI/ResourceLayout.h>
#include <RHI/Shader.h>

namespace RHI
{
  class spDeviceMTL;
  class spShaderMTL;

  class SP_RHIMTL_DLL spShaderProgramMTL final : public spShaderProgram
  {
    EZ_ADD_DYNAMIC_REFLECTION(spShaderProgramMTL, spShaderProgram);

    // spDeviceResource

  public:
    void ReleaseResource() override;
    bool IsReleased() const override;

    // spShaderProgram

  public:
    void Attach(ezSharedPtr<spShader> pShader) override;
    void Detach(ezSharedPtr<spShader> pShader) override;
    void Detach(const ezEnum<spShaderStage>& eStage) override;
    void DetachAll() override;
    void Use() override;
    [[nodiscard]] ezSharedPtr<spShader> Get(const ezEnum<spShaderStage>& eStage) const override;
    void GetResourceLayoutDescriptions(ezDynamicArray<spResourceLayoutDescription>& out_resourceLayouts) const override;

    // spShaderProgramMTL

  public:
    explicit spShaderProgramMTL(spDeviceMTL* pDevice);
    ~spShaderProgramMTL() override;

    [[nodiscard]] EZ_ALWAYS_INLINE ezSharedPtr<spShaderMTL> GetVertexShader() const { return m_pVertexShader; }
    [[nodiscard]] EZ_ALWAYS_INLINE ezSharedPtr<spShaderMTL> GetGeometryShader() const { return m_pGeometryShader; }
    [[nodiscard]] EZ_ALWAYS_INLINE ezSharedPtr<spShaderMTL> GetHullShader() const { return m_pHullShader; }
    [[nodiscard]] EZ_ALWAYS_INLINE ezSharedPtr<spShaderMTL> GetDomainShader() const { return m_pDomainShader; }
    [[nodiscard]] EZ_ALWAYS_INLINE ezSharedPtr<spShaderMTL> GetPixelShader() const { return m_pPixelShader; }
    [[nodiscard]] EZ_ALWAYS_INLINE ezSharedPtr<spShaderMTL> GetComputeShader() const { return m_pComputeShader; }

  private:
    void GetResourceLayoutElementsForStage(const ezEnum<spShaderStage>& eStage, ezDynamicArray<spResourceLayoutElementDescription>& out_elements) const;

    MTL::Device* m_pMTLDevice{nullptr};

    ezSharedPtr<spShaderMTL> m_pVertexShader{nullptr};
    ezSharedPtr<spShaderMTL> m_pGeometryShader{nullptr};
    ezSharedPtr<spShaderMTL> m_pHullShader{nullptr};
    ezSharedPtr<spShaderMTL> m_pDomainShader{nullptr};
    ezSharedPtr<spShaderMTL> m_pPixelShader{nullptr};
    ezSharedPtr<spShaderMTL> m_pComputeShader{nullptr};
  };

  class SP_RHIMTL_DLL spShaderMTL final : public spShader, public spDeferredDeviceResource
  {
    friend class spDeviceResourceFactoryMTL;

    EZ_ADD_DYNAMIC_REFLECTION(spShaderMTL, spShader);

  public:
    // spDeviceResource

    void ReleaseResource() override;
    bool IsReleased() const override;
    void SetDebugName(ezStringView sDebugName) override;

    // spDeferredDeviceResource

    void CreateResource() override;

    // spShaderMTL

    spShaderMTL(spDeviceMTL* pDevice, const spShaderDescription& description);
    ~spShaderMTL() override;

    [[nodiscard]] EZ_ALWAYS_INLINE MTL::Library* GetMTLShaderLibrary() const { return m_pMTLShaderLibrary; }
    [[nodiscard]] EZ_ALWAYS_INLINE MTL::Function* GetMTLShaderFunction() const { return m_pMTLShaderFunction; }

    [[nodiscard]] EZ_ALWAYS_INLINE bool HasFunctionConstants() const { return m_bHasFunctionConstants; }

    [[nodiscard]] MTL::ArgumentEncoder* GetArgumentEncoder(ezUInt32 uiArgumentIndex);

  private:
    MTL::Device* m_pMTLDevice{nullptr};

    MTL::Library* m_pMTLShaderLibrary{nullptr};
    MTL::Function* m_pMTLShaderFunction{nullptr};

    ezArrayMap<ezUInt32, MTL::ArgumentEncoder*> m_ArgumentEncoders;

    bool m_bHasFunctionConstants{false};
  };
} // namespace RHI
