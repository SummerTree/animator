#ifndef UNREAL_OFFLINE_MODULE_H_
#define UNREAL_OFFLINE_MODULE_H_

#include <unreal_model.h>

namespace unreal
{
	class OfflineModule final : public UnrealModule
	{
	public:
		OfflineModule() noexcept;
		virtual ~OfflineModule() noexcept;

		virtual void reset() noexcept override;

		virtual void load(octoon::runtime::json& reader) noexcept override;
		virtual void save(octoon::runtime::json& reader) noexcept override;

	private:
		OfflineModule(const OfflineModule&) = delete;
		OfflineModule& operator=(const OfflineModule&) = delete;

	public:
		std::uint32_t bounces;
	};
}

#endif