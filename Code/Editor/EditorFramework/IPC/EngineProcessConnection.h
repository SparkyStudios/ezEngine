#pragma once

#include <EditorEngineProcessFramework/EngineProcess/EngineProcessMessages.h>
#include <EditorFramework/EditorFrameworkDLL.h>
#include <EditorFramework/IPC/EditorProcessCommunicationChannel.h>
#include <Foundation/Application/Config/PluginConfig.h>
#include <Foundation/Communication/Event.h>
#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Types/UniquePtr.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>

class ezEditorEngineConnection;
class ezDocument;
class ezDocumentObject;
struct ezDocumentObjectPropertyEvent;
struct ezDocumentObjectStructureEvent;
class ezQtEngineDocumentWindow;
class ezAssetDocument;

class EZ_EDITORFRAMEWORK_DLL ezEditorEngineProcessConnection
{
  EZ_DECLARE_SINGLETON(ezEditorEngineProcessConnection);

public:
  ezEditorEngineProcessConnection();
  ~ezEditorEngineProcessConnection();

  /// \brief The given file system configuration will be used by the engine process to setup the runtime data directories.
  ///        This only takes effect if the editor process is restarted.
  void SetFileSystemConfig(const ezApplicationFileSystemConfig& cfg) { m_FileSystemConfig = cfg; }

  /// \brief The given plugin configuration will be used by the engine process to load runtime plugins.
  ///        This only takes effect if the editor process is restarted.
  void SetPluginConfig(const ezApplicationPluginConfig& cfg) { m_PluginConfig = cfg; }

  void Update();
  ezResult RestartProcess();
  void ShutdownProcess();
  bool IsProcessCrashed() const { return m_bProcessCrashed; }

  ezEditorEngineConnection* CreateEngineConnection(ezAssetDocument* pDocument);
  void DestroyEngineConnection(ezAssetDocument* pDocument);

  bool SendMessage(ezProcessMessage* pMessage);

  /// /brief Waits for a message of type pMessageType. If tTimeout is zero, the function will not timeout. If the timeout is valid
  ///        and is it, EZ_FAILURE is returned. If the message type matches and pCallback is valid, the function will be called
  ///        and the return values decides whether the message is to be accepted and the waiting has ended.
  ezResult WaitForMessage(const ezRTTI* pMessageType, ezTime timeout, ezProcessCommunicationChannel ::WaitForMessageCallback* pCallback = nullptr);
  /// /brief Same as WaitForMessage but the message must be to a specific document. Therefore,
  ///        pMessageType must be derived from ezEditorEngineDocumentMsg and the function will only return if the received
  ///        message matches both type, document and is accepted by pCallback.
  ezResult WaitForDocumentMessage(
    const ezUuid& assetGuid, const ezRTTI* pMessageType, ezTime timeout, ezProcessCommunicationChannel::WaitForMessageCallback* pCallback = nullptr);

  bool IsEngineSetup() const { return m_bClientIsConfigured; }

  void ActivateRemoteProcess(const ezAssetDocument* pDocument, ezUInt32 uiViewID);

  ezProcessCommunicationChannel& GetCommunicationChannel() { return m_IPC; }

  struct Event
  {
    enum class Type
    {
      Invalid,
      ProcessStarted,
      ProcessCrashed,
      ProcessShutdown,
      ProcessMessage,
      ProcessRestarted,
    };

    Event()
    {
      m_Type = Type::Invalid;
      m_pMsg = nullptr;
    }

    Type m_Type;
    const ezProcessMessage* m_pMsg;
  };

  static ezEvent<const Event&> s_Events;

private:
  void Initialize(const ezRTTI* pFirstAllowedMessageType);
  void HandleIPCEvent(const ezProcessCommunicationChannel::Event& e);
  void UIServicesTickEventHandler(const ezQtUiServices::TickEvent& e);
  bool ConnectToRemoteProcess();
  void ShutdownRemoteProcess();

  bool m_bProcessShouldBeRunning;
  bool m_bProcessCrashed;
  bool m_bClientIsConfigured;
  ezEventSubscriptionID m_TickEventSubscriptionID = 0;
  ezUInt32 m_uiRedrawCountSent = 0;
  ezUInt32 m_uiRedrawCountReceived = 0;

  ezEditorProcessCommunicationChannel m_IPC;
  ezUniquePtr<ezEditorProcessRemoteCommunicationChannel> m_pRemoteProcess;
  ezApplicationFileSystemConfig m_FileSystemConfig;
  ezApplicationPluginConfig m_PluginConfig;
  ezHashTable<ezUuid, ezAssetDocument*> m_DocumentByGuid;
};

class EZ_EDITORFRAMEWORK_DLL ezEditorEngineConnection
{
public:
  bool SendMessage(ezEditorEngineDocumentMsg* pMessage);
  void SendHighlightObjectMessage(ezViewHighlightMsgToEngine* pMessage);

  ezDocument* GetDocument() const { return m_pDocument; }

private:
  friend class ezEditorEngineProcessConnection;
  ezEditorEngineConnection(ezDocument* pDocument) { m_pDocument = pDocument; }
  ~ezEditorEngineConnection() = default;

  ezDocument* m_pDocument;
};
