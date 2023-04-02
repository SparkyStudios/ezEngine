#pragma once

/// \brief Base class for a file processor.
///
/// A file processor can generate more than one asset as output. The processor
/// behavior is configured through the TConfig type parameter which is defined
/// for each implementation.
///
/// \tparam TConfig The configuration type.
template <typename TConfig>
class spProcessor
{
public:
  virtual ~spProcessor() = default;

  /// \brief Processes the given file.
  /// \param [in] sFilename The path to the file to process.
  /// \param [in] sOutputPath The path to the output directory.
  virtual ezResult Process(ezStringView sFilename, ezStringView sOutputPath) = 0;

protected:
  spProcessor(TConfig configuration)
    : m_Configuration(std::move(configuration))
  {
  }

  TConfig m_Configuration;
};
