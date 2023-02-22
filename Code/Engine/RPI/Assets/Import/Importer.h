#pragma once

#include <RPI/RPIDLL.h>

/// \brief Base class for all RPI assets importers.
/// \tparam TConfig The configuration type of the asset to be imported.
/// \tparam TAsset The type of the asset to be imported.
template <typename TConfig, class TAsset>
class SP_RPI_DLL spImporter
{
public:
  /// \brief Initializes a new instance of the importer.
  /// \param configuration The configuration of the asset to be imported.
  explicit spImporter(TConfig configuration) : m_Configuration(std::move(configuration)) {}

  virtual ~spImporter() = default;

  /// \brief The current importer configuration.
  EZ_NODISCARD EZ_ALWAYS_INLINE TConfig GetConfiguration() const { return m_Configuration; }

  /// \brief Imports the file at the given path and convert it to an asset.
  virtual ezResult Import(ezStringView sAssetPath, TAsset* out_pAsset) = 0;

protected:
  TConfig m_Configuration;
};
