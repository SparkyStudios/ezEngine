#pragma once

#include <SparkLangPlugin/SparkLangPluginDLL.h>

#include <SparkLangPlugin/Core/ScriptContext.h>
#include <SparkLangPlugin/Core/ScriptProperty.h>

#include <Core/Messages/EventMessage.h>
#include <Core/World/EventMessageHandlerComponent.h>
#include <Core/World/World.h>

typedef ezComponentManagerSimple<class ezSparkLangScriptComponent, ezComponentUpdateType::WhenSimulating, ezBlockStorageType::FreeList> ezSparkLangScriptComponentManager;

struct EZ_SPARKLANGPLUGIN_DLL ezSparkLangScriptMessageProxy : public ezMessage
{
  EZ_DECLARE_MESSAGE_TYPE(ezSparkLangScriptMessageProxy, ezMessage);

  ezUInt64 m_sMessageTypeNameHash = 0;
  ezMessage* m_pMessage = nullptr;
};

struct ezSparkLangScriptEventMessageProxy : public ezEventMessage
{
  EZ_DECLARE_MESSAGE_TYPE(ezSparkLangScriptEventMessageProxy, ezEventMessage);

  ezUInt64 m_sMessageTypeNameHash = 0;
  ezEventMessage* m_pEventMessage = nullptr;
};

class EZ_SPARKLANGPLUGIN_DLL ezSparkLangScriptComponent : public ezEventMessageHandlerComponent
{
  EZ_DECLARE_COMPONENT_TYPE(ezSparkLangScriptComponent, ezEventMessageHandlerComponent, ezSparkLangScriptComponentManager);

  // ezComponent

public:
  void Initialize() override;
  void Deinitialize() override;
  void OnActivated() override;
  void OnDeactivated() override;
  void OnSimulationStarted() override;
  void SerializeComponent(ezWorldWriter& stream) const override;
  void DeserializeComponent(ezWorldReader& stream) override;

  bool OnUnhandledMessage(ezMessage& msg, bool bWasPostedMsg) override;
  bool OnUnhandledMessage(ezMessage& msg, bool bWasPostedMsg) const override;

  // ezEventMessageHandlerComponent

protected:
  bool HandlesEventMessage(const ezEventMessage& msg) const override;

  // ezSparkLangScriptComponent

public:
  ezSparkLangScriptComponent();
  ~ezSparkLangScriptComponent() override;

  void BroadcastEventMessage(ezEventMessage& msg);

  void SetUpdateInterval(float fIntervalMS);
  void SetUpdateInterval(ezTime interval);
  EZ_NODISCARD ezTime GetUpdateInterval() const;

  enum ScriptFlag
  {
    Loaded = 0,
    Compiled = 1,
    Failed = 2
  };

protected:
  void Update();
  bool HandleUnhandledMessage(ezMessage& msg, bool bWasPostedMsg);

  void OnMsgSparkLangScriptMessageProxy(ezSparkLangScriptMessageProxy& msg);

private:
  struct EventSender
  {
    const ezRTTI* m_pMsgType = nullptr;
    ezEventMessageSender<ezEventMessage> m_Sender;
  };

  ezHybridArray<EventSender, 2> m_EventSenders;
  ezDynamicArray<ezSparkLangScriptProperty*> m_Properties;

  ezSparkLangScriptContext* m_pScriptContext{nullptr};

  Sqrat::Table m_ComponentScope;
  Sqrat::Table m_ComponentInstance;

  Sqrat::Function m_InitializeFunc;
  Sqrat::Function m_DeinitializeFunc;
  Sqrat::Function m_OnActivatedFunc;
  Sqrat::Function m_OnDeactivatedFunc;
  Sqrat::Function m_OnSimulationStarted;
  Sqrat::Function m_UpdateFunc;

  ezMap<ezUInt64, Sqrat::Function> m_MessageHandlers;

  ezTime m_UpdateInterval;
  ezTime m_LastUpdateTime;
};
