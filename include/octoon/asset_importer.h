#ifndef OCTOON_ASSET_IMPORTER_H_
#define OCTOON_ASSET_IMPORTER_H_

#include <octoon/asset_importer_context.h>
#include <octoon/runtime/singleton.h>
#include <octoon/runtime/json.h>
#include <filesystem>
#include <map>

namespace octoon
{
	class OCTOON_EXPORT AssetImporter : public Object
	{
		OctoonDeclareSubInterface(AssetImporter, Object)
	public:
		AssetImporter() noexcept;
		virtual ~AssetImporter() noexcept;

		virtual std::shared_ptr<Object> onImportAsset(AssetImporterContext& context) noexcept(false) = 0;

	private:
		AssetImporter(const AssetImporter&) = delete;
		AssetImporter& operator=(const AssetImporter&) = delete;
	};
}

#endif