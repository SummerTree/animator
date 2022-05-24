#include <octoon/hal/graphics_texture.h>

namespace octoon
{
	OctoonImplementSubInterface(GraphicsTexture, GraphicsResource, "GraphicsTexture")

	GraphicsTextureDesc::GraphicsTextureDesc() noexcept
		: _size(0, 0, 1)
		, _layerBase(0)
		, _layer(1)
		, _mipBase(0)
		, _mipLevel(1)
		, _format(hal::GraphicsFormat::Undefined)
		, _dim(hal::TextureDimension::Texture2D)
		, _textureUsage(hal::GraphicsViewUsageFlagBits::SampledBit)
		, _tiling(hal::GraphicsImageTiling::Optimal)
		, _usageFlags(hal::GraphicsUsageFlagBits::ReadBit)
		, _data(nullptr)
		, _dataSize(0)
		, _samples(0)
	{
	}

	GraphicsTextureDesc::~GraphicsTextureDesc() noexcept
	{
	}

	void
	GraphicsTextureDesc::setName(std::string_view name) noexcept
	{
		_name = name;
	}

	const std::string&
	GraphicsTextureDesc::getName() const noexcept
	{
		return _name;
	}

	void
	GraphicsTextureDesc::setWidth(std::uint32_t w) noexcept
	{
		_size.x = w;
	}

	void
	GraphicsTextureDesc::setHeight(std::uint32_t h) noexcept
	{
		_size.y = h;
	}

	void
	GraphicsTextureDesc::setDepth(std::uint32_t d) noexcept
	{
		_size.z = d;
	}

	void
	GraphicsTextureDesc::setSize(std::uint32_t w, std::uint32_t h, std::uint32_t depth) noexcept
	{
		_size.x = w;
		_size.y = h;
		_size.z = depth;
	}

	std::uint32_t
	GraphicsTextureDesc::getWidth() const noexcept
	{
		return _size.x;
	}

	std::uint32_t
	GraphicsTextureDesc::getHeight() const noexcept
	{
		return _size.y;
	}

	std::uint32_t
	GraphicsTextureDesc::getDepth() const noexcept
	{
		return _size.z;
	}

	void
	GraphicsTextureDesc::setTexFormat(hal::GraphicsFormat format) noexcept
	{
		_format = format;
	}

	void
	GraphicsTextureDesc::setTexDim(hal::TextureDimension dim) noexcept
	{
		_dim = dim;
	}

	void
	GraphicsTextureDesc::setTexTiling(hal::GraphicsImageTiling tiling) noexcept
	{
		_tiling = tiling;
	}

	void
	GraphicsTextureDesc::setTexUsage(hal::GraphicsViewUsageFlags flags) noexcept
	{
		_textureUsage = flags;
	}

	void
	GraphicsTextureDesc::setTexMultisample(std::uint32_t samples) noexcept
	{
		_samples = samples;
	}

	void
	GraphicsTextureDesc::setUsageFlagBits(hal::GraphicsUsageFlags flags) noexcept
	{
		_usageFlags = flags;
	}

	hal::GraphicsFormat
	GraphicsTextureDesc::getTexFormat() const noexcept
	{
		return _format;
	}

	hal::TextureDimension
	GraphicsTextureDesc::getTexDim() const noexcept
	{
		return _dim;
	}

	hal::GraphicsViewUsageFlags
	GraphicsTextureDesc::getTexUsage() const noexcept
	{
		return _textureUsage;
	}

	hal::GraphicsImageTiling
	GraphicsTextureDesc::getTexTiling() const noexcept
	{
		return _tiling;
	}

	std::uint32_t
	GraphicsTextureDesc::getTexMultisample() const noexcept
	{
		return _samples;
	}

	hal::GraphicsUsageFlags
	GraphicsTextureDesc::getUsageFlagBits() const noexcept
	{
		return _usageFlags;
	}

	void
	GraphicsTextureDesc::setLayerNums(std::uint32_t layer) noexcept
	{
		_layer = layer;
	}

	std::uint32_t
	GraphicsTextureDesc::getLayerNums() const noexcept
	{
		return _layer;
	}

	void
	GraphicsTextureDesc::setLayerBase(std::uint32_t minLayer) noexcept
	{
		_layerBase = minLayer;
	}

	std::uint32_t
	GraphicsTextureDesc::getLayerBase() const noexcept
	{
		return _layerBase;
	}

	void
	GraphicsTextureDesc::setMipNums(std::uint32_t nums) noexcept
	{
		_mipLevel = nums;
	}

	void
	GraphicsTextureDesc::setMipBase(std::uint32_t level) noexcept
	{
		_mipBase = level;
	}

	std::uint32_t
	GraphicsTextureDesc::getMipNums() const noexcept
	{
		return _mipLevel;
	}

	std::uint32_t
	GraphicsTextureDesc::getMipBase() const noexcept
	{
		return _mipBase;
	}

	void
	GraphicsTextureDesc::setStream(const void* data) noexcept
	{
		_data = data;
	}

	void
	GraphicsTextureDesc::setStreamSize(std::size_t size) noexcept
	{
		_dataSize = size;
	}

	const void*
	GraphicsTextureDesc::getStream() const noexcept
	{
		return _data;
	}

	std::size_t
	GraphicsTextureDesc::getStreamSize() const noexcept
	{
		return _dataSize;
	}
}