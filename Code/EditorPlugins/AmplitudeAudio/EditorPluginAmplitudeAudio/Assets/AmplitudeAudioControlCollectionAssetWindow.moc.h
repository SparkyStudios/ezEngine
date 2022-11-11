#pragma once

#include <Foundation/Basics.h>
#include <GuiFoundation/DocumentWindow/DocumentWindow.moc.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

class ezQtAmplitudeAudioControlCollectionAssetDocumentWindow : public ezQtDocumentWindow
{
  Q_OBJECT

public:
  ezQtAmplitudeAudioControlCollectionAssetDocumentWindow(ezDocument* pDocument);
  ~ezQtAmplitudeAudioControlCollectionAssetDocumentWindow();

  const char* GetWindowLayoutGroupName() const override { return "AudioControlCollectionAsset"; }
};
