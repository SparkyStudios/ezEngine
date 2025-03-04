#include <Foundation/FoundationPCH.h>

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/IO/MemoryMappedFile.h>
#  include <Foundation/Logging/Log.h>
#  include <Foundation/Platform/Win/DosDevicePath_Win.h>
#  include <Foundation/Strings/PathUtils.h>
#  include <Foundation/Strings/StringConversion.h>

struct ezMemoryMappedFileImpl
{
  ezMemoryMappedFile::Mode m_Mode = ezMemoryMappedFile::Mode::None;
  void* m_pMappedFilePtr = nullptr;
  ezUInt64 m_uiFileSize = 0;
  HANDLE m_hFile = INVALID_HANDLE_VALUE;
  HANDLE m_hMapping = INVALID_HANDLE_VALUE;

  ~ezMemoryMappedFileImpl()
  {
    if (m_pMappedFilePtr != nullptr)
    {
      UnmapViewOfFile(m_pMappedFilePtr);
      m_pMappedFilePtr = nullptr;
    }

    if (m_hMapping != INVALID_HANDLE_VALUE)
    {
      CloseHandle(m_hMapping);
      m_hMapping = INVALID_HANDLE_VALUE;
    }

    if (m_hFile != INVALID_HANDLE_VALUE)
    {
      CloseHandle(m_hFile);
      m_hFile = INVALID_HANDLE_VALUE;
    }
  }
};

ezMemoryMappedFile::ezMemoryMappedFile()
{
  m_pImpl = EZ_DEFAULT_NEW(ezMemoryMappedFileImpl);
}

ezMemoryMappedFile::~ezMemoryMappedFile()
{
  Close();
}

ezResult ezMemoryMappedFile::Open(ezStringView sAbsolutePath, Mode mode)
{
  EZ_ASSERT_DEV(mode != Mode::None, "Invalid mode to open the memory mapped file");
  EZ_ASSERT_DEV(ezPathUtils::IsAbsolutePath(sAbsolutePath), "ezMemoryMappedFile::Open() can only be used with absolute file paths");

  EZ_LOG_BLOCK("MemoryMapFile", sAbsolutePath);


  Close();

  m_pImpl->m_Mode = mode;

  DWORD access = GENERIC_READ;

  if (mode == Mode::ReadWrite)
  {
    access |= GENERIC_WRITE;
  }

  m_pImpl->m_hFile = CreateFileW(ezDosDevicePath(sAbsolutePath), access, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  DWORD errorCode = GetLastError();

  if (m_pImpl->m_hFile == nullptr || m_pImpl->m_hFile == INVALID_HANDLE_VALUE)
  {
    ezLog::Error("Could not open file for memory mapping - {}", ezArgErrorCode(errorCode));
    Close();
    return EZ_FAILURE;
  }

  if (GetFileSizeEx(m_pImpl->m_hFile, reinterpret_cast<LARGE_INTEGER*>(&m_pImpl->m_uiFileSize)) == FALSE || m_pImpl->m_uiFileSize == 0)
  {
    ezLog::Error("File for memory mapping is empty");
    Close();
    return EZ_FAILURE;
  }

  m_pImpl->m_hMapping = CreateFileMappingW(m_pImpl->m_hFile, nullptr, m_pImpl->m_Mode == Mode::ReadOnly ? PAGE_READONLY : PAGE_READWRITE, 0, 0, nullptr);

  if (m_pImpl->m_hMapping == nullptr || m_pImpl->m_hMapping == INVALID_HANDLE_VALUE)
  {
    errorCode = GetLastError();

    ezLog::Error("Could not create memory mapping of file - {}", ezArgErrorCode(errorCode));
    Close();
    return EZ_FAILURE;
  }

  m_pImpl->m_pMappedFilePtr = MapViewOfFile(m_pImpl->m_hMapping, mode == Mode::ReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);

  if (m_pImpl->m_pMappedFilePtr == nullptr)
  {
    errorCode = GetLastError();

    ezLog::Error("Could not create memory mapping view of file - {}", ezArgErrorCode(errorCode));
    Close();
    return EZ_FAILURE;
  }

  return EZ_SUCCESS;
}

ezResult ezMemoryMappedFile::OpenShared(ezStringView sSharedName, ezUInt64 uiSize, Mode mode)
{
  EZ_ASSERT_DEV(mode != Mode::None, "Invalid mode to open the memory mapped file");
  EZ_ASSERT_DEV(uiSize > 0, "ezMemoryMappedFile::OpenShared() needs a valid file size to map");

  EZ_LOG_BLOCK("MemoryMapFile", sSharedName);

  Close();

  m_pImpl->m_Mode = mode;

  DWORD errorCode = 0;
  DWORD sizeHigh = static_cast<DWORD>((uiSize >> 32) & 0xFFFFFFFFu);
  DWORD sizeLow = static_cast<DWORD>(uiSize & 0xFFFFFFFFu);

  m_pImpl->m_hMapping = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, m_pImpl->m_Mode == Mode::ReadOnly ? PAGE_READONLY : PAGE_READWRITE, sizeHigh,
    sizeLow, ezStringWChar(sSharedName).GetData());

  if (m_pImpl->m_hMapping == nullptr || m_pImpl->m_hMapping == INVALID_HANDLE_VALUE)
  {
    errorCode = GetLastError();

    ezLog::Error("Could not create memory mapping of file - {}", ezArgErrorCode(errorCode));
    Close();
    return EZ_FAILURE;
  }

  m_pImpl->m_pMappedFilePtr = MapViewOfFile(m_pImpl->m_hMapping, mode == Mode::ReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);

  if (m_pImpl->m_pMappedFilePtr == nullptr)
  {
    errorCode = GetLastError();

    ezLog::Error("Could not create memory mapping view of file - {}", ezArgErrorCode(errorCode));
    Close();
    return EZ_FAILURE;
  }

  return EZ_SUCCESS;
}

void ezMemoryMappedFile::Close()
{
  m_pImpl = EZ_DEFAULT_NEW(ezMemoryMappedFileImpl);
}

ezMemoryMappedFile::Mode ezMemoryMappedFile::GetMode() const
{
  return m_pImpl->m_Mode;
}

const void* ezMemoryMappedFile::GetReadPointer(ezUInt64 uiOffset /*= 0*/, OffsetBase base /*= OffsetBase::Start*/) const
{
  EZ_ASSERT_DEBUG(m_pImpl->m_Mode >= Mode::ReadOnly, "File must be opened with read access before accessing it for reading.");
  EZ_ASSERT_DEBUG(uiOffset <= m_pImpl->m_uiFileSize, "Read offset must be smaller than mapped file size");

  if (base == OffsetBase::Start)
  {
    return ezMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, static_cast<std::ptrdiff_t>(uiOffset));
  }
  else
  {
    return ezMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, static_cast<std::ptrdiff_t>(m_pImpl->m_uiFileSize - uiOffset));
  }
}

void* ezMemoryMappedFile::GetWritePointer(ezUInt64 uiOffset /*= 0*/, OffsetBase base /*= OffsetBase::Start*/)
{
  EZ_ASSERT_DEBUG(m_pImpl->m_Mode >= Mode::ReadWrite, "File must be opened with read/write access before accessing it for writing.");
  EZ_ASSERT_DEBUG(uiOffset <= m_pImpl->m_uiFileSize, "Read offset must be smaller than mapped file size");

  if (base == OffsetBase::Start)
  {
    return ezMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, static_cast<std::ptrdiff_t>(uiOffset));
  }
  else
  {
    return ezMemoryUtils::AddByteOffset(m_pImpl->m_pMappedFilePtr, static_cast<std::ptrdiff_t>(m_pImpl->m_uiFileSize - uiOffset));
  }
}

ezUInt64 ezMemoryMappedFile::GetFileSize() const
{
  return m_pImpl->m_uiFileSize;
}

#endif
