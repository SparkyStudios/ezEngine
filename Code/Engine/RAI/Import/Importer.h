#pragma once

#include <RAI/RAIDLL.h>

/// \brief Base class for all RAI assets importers.
/// \tparam TConfig The configuration type of the asset to be imported.
/// \tparam TAsset The type of the asset to be imported.
template <typename TConfig, class TAsset>
class SP_RAI_DLL spImporter
{
public:
  /// \brief Initializes a new instance of the importer.
  /// \param configuration The configuration of the asset to be imported.
  explicit spImporter(TConfig configuration)
    : m_Configuration(std::move(configuration))
  {
  }

  virtual ~spImporter() = default;

  /// \brief The current importer configuration.
  EZ_NODISCARD EZ_ALWAYS_INLINE TConfig GetConfiguration() const { return m_Configuration; }

  /// \brief Imports the file at the given path and convert it to an asset.
  /// \param sAssetPath The path to the raw asset file to import.
  /// \param [out] out_pAsset The output asset. Can be a pointer to an array if the importer should generate many assets.
  /// \param uiCount The number of assets to import. This should match the number of elements in \a out_pAsset.
  /// \return EZ_SUCCESS if the import was successful, EZ_FAILURE otherwise.
  virtual ezResult Import(ezStringView sAssetPath, TAsset* out_pAsset, ezUInt32 uiCount) = 0;

protected:
  TConfig m_Configuration;
};
