#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/CompressedStreamBrotliG.h>

#ifdef BUILDSYSTEM_ENABLE_BROTLIG_SUPPORT

#  include <Foundation/System/SystemInformation.h>
#  include <brotlig/inc/BrotliG.h>

ezCompressedStreamReaderBrotliG::ezCompressedStreamReaderBrotliG() = default;

ezCompressedStreamReaderBrotliG::ezCompressedStreamReaderBrotliG(ezStreamReader* pInputStream)
{
  SetInputStream(pInputStream);
}

ezCompressedStreamReaderBrotliG::~ezCompressedStreamReaderBrotliG()
{
}

void ezCompressedStreamReaderBrotliG::SetInputStream(ezStreamReader* pInputStream)
{
  m_bReachedEnd = false;
  m_pInputStream = pInputStream;

  if (pInputStream != nullptr)
  {
    ezUInt32 uiCompressedSize = 0;

    pInputStream->ReadDWordValue(&uiCompressedSize).AssertSuccess();
    m_CompressedCache.SetCountUninitialized(uiCompressedSize);

    pInputStream->ReadBytes(m_CompressedCache.GetData(), uiCompressedSize);

    ezUInt16 uiTerminator = 0;
    pInputStream->ReadBytes(&uiTerminator, sizeof(ezUInt16));
    EZ_ASSERT_DEV(uiTerminator == 0, "Invalid Brotli stream");

    ezUInt32 uiDecompressedSize = BrotliG::DecompressedSize(m_CompressedCache.GetData());
    m_DecompressedData.SetCountUninitialized(uiDecompressedSize);

    const auto res = BrotliG::DecodeCPU(uiCompressedSize, m_CompressedCache.GetData(), &uiDecompressedSize, m_DecompressedData.GetData(), nullptr);
    EZ_ASSERT_DEV(res == BROTLIG_OK, "Decompression failed");

    m_DecompressedData.SetCount(uiDecompressedSize);
    m_CompressedCache.Clear();
  }
}

ezUInt64 ezCompressedStreamReaderBrotliG::ReadBytes(void* pReadBuffer, ezUInt64 uiBytesToRead)
{
  EZ_ASSERT_DEV(m_pInputStream != nullptr, "No input stream has been specified");

  if (uiBytesToRead == 0 || m_bReachedEnd)
    return 0;

  const ezUInt64 uiReadSize = ezMath::Min(uiBytesToRead, m_DecompressedData.GetCount() - m_uiCursor);

  if (uiReadSize == 0)
    return 0;

  // Implement the 'skip n bytes' feature
  if (pReadBuffer == nullptr)
  {
    m_uiCursor += uiReadSize;
    return uiReadSize;
  }

  ezUInt64 uiBytesRead = 0;
  while (uiBytesRead < uiReadSize)
  {
    static_cast<ezUInt8*>(pReadBuffer)[uiBytesRead] = m_DecompressedData[static_cast<ezUInt32>(m_uiCursor + uiBytesRead)];
    uiBytesRead++;
  }

  m_uiCursor += uiReadSize;

  if (m_uiCursor == m_DecompressedData.GetCount())
    m_bReachedEnd = true;

  return uiReadSize;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ezCompressedStreamWriterBrotliG::ezCompressedStreamWriterBrotliG() = default;

ezCompressedStreamWriterBrotliG::ezCompressedStreamWriterBrotliG(ezStreamWriter* pOutputStream)
{
  SetOutputStream(pOutputStream);
}

ezCompressedStreamWriterBrotliG::~ezCompressedStreamWriterBrotliG()
{
  if (m_pOutputStream != nullptr)
  {
    // NOTE: FinishCompressedStream() WILL write a couple of bytes, even if the user did not write anything.
    // If ezCompressedStreamWriterBrotliG was not supposed to be used, this may end up in a corrupted output file.
    // EZ_ASSERT_DEV(m_uiWrittenBytes > 0, "Output stream was set, but not a single byte was written to the compressed stream before destruction.
    // Incorrect usage?");

    FinishCompressedStream().IgnoreResult();
  }
}

void ezCompressedStreamWriterBrotliG::SetOutputStream(ezStreamWriter* pOutputStream, ezUInt32 uiPageSizeKB)
{
  if (m_pOutputStream == pOutputStream)
    return;

  // limit the cache to 63KB, because at 64KB we run into an endless loop due to a 16 bit overflow
  uiPageSizeKB = ezMath::Max(uiPageSizeKB, 1u);

  // finish anything done on a previous output stream
  FinishCompressedStream().IgnoreResult();

  m_UncompressedCache.Clear();
  m_CompressedCache.Clear();

  m_uiUncompressedSize = 0;
  m_uiCompressedSize = 0;
  m_uiWrittenBytes = 0;
  m_uiPageSize = static_cast<ezUInt64>(uiPageSizeKB) * 1024;

  if (pOutputStream != nullptr)
  {
    m_pOutputStream = pOutputStream;
  }
}

ezResult ezCompressedStreamWriterBrotliG::FinishCompressedStream()
{
  if (m_pOutputStream == nullptr)
    return EZ_SUCCESS;

  if (Flush().Failed())
    return EZ_FAILURE;

  // write a zero-terminator
  constexpr ezUInt16 uiTerminator = 0;
  EZ_SUCCEED_OR_RETURN(m_pOutputStream->WriteBytes(&uiTerminator, sizeof(ezUInt16)));

  m_uiWrittenBytes += sizeof(ezUInt16);
  m_pOutputStream = nullptr;

  return EZ_SUCCESS;
}

ezResult ezCompressedStreamWriterBrotliG::Flush()
{
  if (m_pOutputStream == nullptr)
    return EZ_SUCCESS;

  const ezUInt32 uiMaxCompressedSize = BrotliG::MaxCompressedSize(static_cast<ezUInt32>(m_uiUncompressedSize));
  m_CompressedCache.SetCountUninitialized(ezMath::Max(1U, uiMaxCompressedSize));

  ezUInt32 uiCompressedSize = 0;
  if (BrotliG::Encode(static_cast<ezUInt32>(m_uiUncompressedSize), m_UncompressedCache.GetData(), &uiCompressedSize, m_CompressedCache.GetData(), nullptr) != BROTLIG_OK)
    return EZ_FAILURE;

  EZ_SUCCEED_OR_RETURN(m_pOutputStream->WriteDWordValue(&uiCompressedSize));
  EZ_SUCCEED_OR_RETURN(m_pOutputStream->WriteBytes(m_CompressedCache.GetData(), uiCompressedSize));

  m_UncompressedCache.Clear();
  m_CompressedCache.Clear();

  m_uiCompressedSize = uiCompressedSize;

  return EZ_SUCCESS;
}

ezResult ezCompressedStreamWriterBrotliG::WriteBytes(const void* pWriteBuffer, ezUInt64 uiBytesToWrite)
{
  m_uiUncompressedSize += uiBytesToWrite;
  m_UncompressedCache.Reserve(static_cast<ezUInt32>(m_uiUncompressedSize));

  const auto* pData = static_cast<const ezUInt8*>(pWriteBuffer);
  m_UncompressedCache.PushBackRange(ezMakeArrayPtr(pData, static_cast<ezUInt32>(uiBytesToWrite)));

  return EZ_SUCCESS;
}

#endif

EZ_STATICLINK_FILE(Foundation, Foundation_IO_Implementation_CompressedStreamBrotliG);
