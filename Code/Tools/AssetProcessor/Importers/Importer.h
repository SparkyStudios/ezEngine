#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Logging/Log.h>

/// \brief Base class for all RAI assets importers.
/// \tparam TConfig The configuration type of the importer.
template <typename TConfig>
class spImporter
{
public:
  /// \brief Initializes a new instance of the importer.
  /// \param configuration The importer configuration to use.
  explicit spImporter(TConfig configuration)
    : m_Configuration(std::move(configuration))
  {
  }

  virtual ~spImporter() = default;

  /// \brief Imports the asset from the specified root node.
  ///
  /// This will typically convert the raw data in each nodes starting
  /// at the root node to asset data. Node data are assumed to be already
  /// properly parsed from mesh files and all transformations (tangent space
  /// calculation, mesh optimization, etc.) are applied.
  ///
  /// \param [in] sFilePath The path to the input file.
  /// \param [in] sOutputPath The path to the output directory.
  virtual ezResult Import(ezStringView sFilePath, ezStringView sOutputPath) = 0;

  /// \brief The current importer configuration.
  EZ_NODISCARD EZ_ALWAYS_INLINE TConfig GetConfiguration() const { return m_Configuration; }

protected:
  TConfig m_Configuration;
};
