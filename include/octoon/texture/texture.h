#ifndef OCTOON_TEXTURE_H_
#define OCTOON_TEXTURE_H_

#include <octoon/texture/texture_format.h>
#include <octoon/texture/texture_loader.h>
#include <octoon/runtime/rtti_object.h>

namespace octoon
{
	class OCTOON_EXPORT Texture final : public RttiObject
	{
		OctoonDeclareSubClass(Texture, RttiObject)
	public:
		Texture() noexcept;
		Texture(Texture&& move) noexcept;
		Texture(const Texture& move) noexcept;
		Texture(Format format, std::uint32_t width, std::uint32_t height) noexcept(false);
		Texture(Format format, std::uint32_t width, std::uint32_t height, std::uint8_t pixels[]) noexcept(false);
		Texture(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t depth) noexcept(false);
		Texture(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t depth, std::uint32_t mipLevel, std::uint32_t layerLevel, std::uint32_t mipBase = 0, std::uint32_t layerBase = 0) noexcept(false);
		explicit Texture(istream& stream, const char* type = nullptr) noexcept;
		explicit Texture(const char* filepath, const char* type = nullptr) noexcept;
		explicit Texture(const std::string& filepath, const char* type = nullptr) noexcept;
		virtual ~Texture() noexcept;

		bool create(Format format, std::uint32_t width, std::uint32_t height) noexcept(false);
		bool create(Format format, std::uint32_t width, std::uint32_t height, std::uint8_t pixels[]) noexcept(false);
		bool create(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t depth) noexcept(false);
		bool create(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t depth, std::uint32_t mipLevel, std::uint32_t layerLevel, std::uint32_t mipBase = 0, std::uint32_t layerBase = 0) noexcept(false);

		bool empty() const noexcept;

		const Format& format() const noexcept;

		std::size_t size() const noexcept;

		std::uint32_t width() const noexcept;
		std::uint32_t height() const noexcept;
		std::uint32_t depth() const noexcept;

		std::uint32_t mipBase() const noexcept;
		std::uint32_t mipLevel() const noexcept;

		std::uint32_t layerBase() const noexcept;
		std::uint32_t layerLevel() const noexcept;

		const std::uint8_t* data() const noexcept;
		const std::uint8_t* data(std::size_t i) const noexcept;

		Texture scale(std::uint32_t width, std::uint32_t height) noexcept(false);

		Texture convert(Format format) noexcept(false);

	public:
		bool load(istream& stream, const char* type = nullptr) noexcept;
		bool load(const char* filepath, const char* type = nullptr) noexcept;
		bool load(const std::string& filepath, const char* type = nullptr) noexcept;

		bool save(ostream& stream, const char* type = "tga") noexcept;
		bool save(const char* filepath, const char* type = "tga") noexcept;
		bool save(const std::string& filepath, const char* type = "tga") noexcept;
		bool save(const std::string& filepath, const std::string& type = "tga") noexcept;

	private:
		Format format_;

		std::uint32_t width_;
		std::uint32_t height_;
		std::uint32_t depth_;

		std::uint32_t mipBase_;
		std::uint32_t mipLevel_;

		std::uint32_t layerBase_;
		std::uint32_t layerLevel_;

		std::vector<std::uint8_t> data_;
	};
}

#endif