#pragma once

#include <Texture/TextureDLL.h>

// auto_delete_file from scoped.h uses FILE_DISPOSITION_INFO which has a member called DeleteFile
// due to the great preprocessor overloads for win32 functions, this apparently is always defined (usually as DeleteFileA)
// since ez undef's DeleteFile to PREVENT such name issues, this actually now CREATES an issue,
// so in this lib we need to 'fix' this again

#include <Foundation/Platform/Win/Utils/IncludeWindows.h>

#if defined(_UNICODE) || defined(UNICODE)
#  define DeleteFile DeleteFileW
#else
#  define DeleteFile DeleteFileA
#endif
