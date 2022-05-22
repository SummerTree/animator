#ifndef UNREAL_FILE_MODULE_H_
#define UNREAL_FILE_MODULE_H_

#include <unreal_model.h>
#include <vector>

namespace unreal
{
	class FileModule final : public UnrealModule
	{
	public:
		FileModule() noexcept;
		virtual ~FileModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(octoon::runtime::json& reader) noexcept override;
		virtual void save(octoon::runtime::json& reader) noexcept override;

	private:
		FileModule(const FileModule&) = delete;
		FileModule& operator=(const FileModule&) = delete;

	public:
		std::uint32_t PATHLIMIT;

		std::vector<const char*> projectExtensions;
		std::vector<const char*> modelExtensions;
		std::vector<const char*> imageExtensions;
		std::vector<const char*> hdriExtensions;
		std::vector<const char*> videoExtensions;
	};
}

#endif