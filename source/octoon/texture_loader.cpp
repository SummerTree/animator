#include <octoon/texture_loader.h>
#include <octoon/runtime/except.h>
#include <octoon/hal/graphics_texture.h>
#include <octoon/video/renderer.h>

#include <map>

namespace octoon
{
	/*std::shared_ptr<GraphicsTexture>
	TextureLoader::load(const Texture& image, std::string_view filepath, bool generateMipmap) noexcept(false)
	{
		GraphicsFormat format = GraphicsFormat::Undefined;
		switch (image.format())
		{
		case Format::BC1RGBUNormBlock: format = GraphicsFormat::BC1RGBUNormBlock; break;
		case Format::BC1RGBAUNormBlock: format = GraphicsFormat::BC1RGBAUNormBlock; break;
		case Format::BC1RGBSRGBBlock: format = GraphicsFormat::BC1RGBSRGBBlock; break;
		case Format::BC1RGBASRGBBlock: format = GraphicsFormat::BC1RGBASRGBBlock; break;
		case Format::BC3UNormBlock: format = GraphicsFormat::BC3UNormBlock; break;
		case Format::BC3SRGBBlock: format = GraphicsFormat::BC3SRGBBlock; break;
		case Format::BC4UNormBlock: format = GraphicsFormat::BC4UNormBlock; break;
		case Format::BC4SNormBlock: format = GraphicsFormat::BC4SNormBlock; break;
		case Format::BC5UNormBlock: format = GraphicsFormat::BC5UNormBlock; break;
		case Format::BC5SNormBlock: format = GraphicsFormat::BC5SNormBlock; break;
		case Format::BC6HUFloatBlock: format = GraphicsFormat::BC6HUFloatBlock; break;
		case Format::BC6HSFloatBlock: format = GraphicsFormat::BC6HSFloatBlock; break;
		case Format::BC7UNormBlock: format = GraphicsFormat::BC7UNormBlock; break;
		case Format::BC7SRGBBlock: format = GraphicsFormat::BC7SRGBBlock; break;
		case Format::R8G8B8UNorm: format = GraphicsFormat::R8G8B8UNorm; break;
		case Format::R8G8B8SRGB: format = GraphicsFormat::R8G8B8UNorm; break;
		case Format::R8G8B8A8UNorm: format = GraphicsFormat::R8G8B8A8UNorm; break;
		case Format::R8G8B8A8SRGB: format = GraphicsFormat::R8G8B8A8UNorm; break;
		case Format::B8G8R8UNorm: format = GraphicsFormat::B8G8R8UNorm; break;
		case Format::B8G8R8SRGB: format = GraphicsFormat::B8G8R8UNorm; break;
		case Format::B8G8R8A8UNorm: format = GraphicsFormat::B8G8R8A8UNorm; break;
		case Format::B8G8R8A8SRGB: format = GraphicsFormat::B8G8R8A8UNorm; break;
		case Format::R8UNorm: format = GraphicsFormat::R8UNorm; break;
		case Format::R8SRGB: format = GraphicsFormat::R8UNorm; break;
		case Format::R8G8UNorm: format = GraphicsFormat::R8G8UNorm; break;
		case Format::R8G8SRGB: format = GraphicsFormat::R8G8UNorm; break;
		case Format::R16SFloat: format = GraphicsFormat::R16SFloat; break;
		case Format::R16G16SFloat: format = GraphicsFormat::R16G16SFloat; break;
		case Format::R16G16B16SFloat: format = GraphicsFormat::R16G16B16SFloat; break;
		case Format::R16G16B16A16SFloat: format = GraphicsFormat::R16G16B16A16SFloat; break;
		case Format::R32SFloat: format = GraphicsFormat::R32SFloat; break;
		case Format::R32G32SFloat: format = GraphicsFormat::R32G32SFloat; break;
		case Format::R32G32B32SFloat: format = GraphicsFormat::R32G32B32SFloat; break;
		case Format::R32G32B32A32SFloat: format = GraphicsFormat::R32G32B32A32SFloat; break;
		default:
			throw runtime_error::create("This image type is not supported by this function:");
		}

		GraphicsTextureDesc textureDesc;
		textureDesc.setName(filepath);
		textureDesc.setSize(image.width(), image.height(), image.depth());
		textureDesc.setTexDim(TextureDimension::Texture2D);
		textureDesc.setTexFormat(format);
		textureDesc.setStream(image.data());
		textureDesc.setStreamSize(image.size());
		textureDesc.setLayerBase(image.layerBase());
		textureDesc.setLayerNums(image.layerLevel());

		if (generateMipmap)
		{
			textureDesc.setMipBase(0);
			textureDesc.setMipNums(8);
		}
		else
		{
			textureDesc.setMipBase(image.mipBase());
			textureDesc.setMipNums(image.mipLevel());
		}

		auto texture = Renderer::instance()->getGraphicsDevice()->createTexture(textureDesc);
		if (!texture)
			return nullptr;

		if (generateMipmap)
			Renderer::instance()->getScriptableRenderContext()->generateMipmap(texture);

		return texture;
	}

	std::shared_ptr<GraphicsTexture>
	TextureLoader::load(std::string_view filepath, bool generateMipmap) noexcept(false)
	{
		assert(!filepath.empty());

		std::string path = std::string(filepath);

		Texture texture;
		if (texture.load(path))
			return load(texture, path, generateMipmap);
		else
			throw std::runtime_error("Failed to open file :" + path);
	}*/

	bool
	TextureLoader::save(std::string_view filepath, const std::shared_ptr<Texture>& texture) noexcept(false)
	{
		auto outputPath = std::string(filepath);
		auto extension = outputPath.substr(outputPath.find_last_of(".") + 1);
		texture->save(outputPath, extension.c_str());

		return true;
	}
}