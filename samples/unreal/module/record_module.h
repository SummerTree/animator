#ifndef UNREAL_RECORD_MODULE_H_
#define UNREAL_RECORD_MODULE_H_

#include <vector>
#include <unreal_model.h>
#include <octoon/math/vector3.h>

namespace unreal
{
	class RecordModule final : public UnrealModule
	{
	public:
		RecordModule() noexcept;
		virtual ~RecordModule() noexcept;

		virtual void reset() noexcept override;
		virtual void resize(std::uint32_t width, std::uint32_t height) noexcept;

		virtual void load(octoon::runtime::json& reader, std::string_view path) noexcept override;
		virtual void save(octoon::runtime::json& writer, std::string_view path) noexcept override;

		virtual void disconnect() noexcept;

	private:
		RecordModule(const RecordModule&) = delete;
		RecordModule& operator=(const RecordModule&) = delete;

	public:
		MutableLiveData<bool> hdr;
		MutableLiveData<bool> srgb;
		MutableLiveData<bool> active;
		MutableLiveData<bool> denoise;

		MutableLiveData<std::uint32_t> width;
		MutableLiveData<std::uint32_t> height;
	};
}

#endif