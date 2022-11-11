#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Logging/Log.h>

// <StaticLinkUtil::StartHere>
// all include's before this will be left alone and not replaced by the StaticLinkUtil
// all include's AFTER this will be removed by the StaticLinkUtil and updated by what is actually used throughout the library

#include <EditorPluginAmplitudeAudio/EditorPluginAmplitudeAudioDLL.h>

#include <EditorFramework/EditorApp/EditorApp.moc.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Strings/TranslationLookup.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/PropertyGrid/PropertyGridWidget.moc.h>
#include <GuiFoundation/UIServices/ImageCache.moc.h>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>

#include <QLabel>
#include <QLayout>
