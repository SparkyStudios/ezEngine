#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Configuration/Plugin.h>

// BEGIN-DOCS-CODE-SNIPPET: dll-export-defines
// Configure the DLL Import/Export Define
#if EZ_ENABLED(EZ_COMPILE_ENGINE_AS_DLL)
#  ifdef BUILDSYSTEM_BUILDING_EDITORPLUGINAMPLITUDEAUDIO_LIB
#    define EZ_EDITORPLUGINAMPLITUDEAUDIO_DLL EZ_DECL_EXPORT
#  else
#    define EZ_EDITORPLUGINAMPLITUDEAUDIO_DLL EZ_DECL_IMPORT
#  endif
#else
#  define EZ_EDITORPLUGINAMPLITUDEAUDIO_DLL
#endif
// END-DOCS-CODE-SNIPPET
