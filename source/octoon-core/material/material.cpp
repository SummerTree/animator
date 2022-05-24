#include <octoon/material/material.h>
#include <octoon/hal/graphics_texture.h>
#include <functional>

namespace octoon
{
	OctoonImplementSubClass(Material, runtime::RttiInterface, "Material");
	
	Material::Material() noexcept
		: _enableScissorTest(false)
		, _enableSrgb(false)
		, _enableMultisample(false)
		, _enableRasterizerDiscard(false)
		, _enableDepth(true)
		, _enableDepthBounds(false)
		, _enableDepthWrite(true)
		, _enableDepthBias(false)
		, _enableDepthClamp(false)
		, _enableDepthBiasClamp(false)
		, _enableStencil(false)
		, _enableBlend(false)
		, _lineWidth(1.0f)
		, _cullMode(hal::CullMode::Back)
		, _polygonMode(hal::PolygonMode::Solid)
		, _primitiveType(hal::VertexType::TriangleList)
		, _frontFace(hal::FrontFace::CW)
		, _colorWriteMask(hal::ColorWriteMask::RGBABit)
		, _blendOp(hal::BlendOp::Add)
		, _blendAlphaOp(hal::BlendOp::Add)
		, _blendSrc(hal::BlendMode::SrcAlpha)
		, _blendDest(hal::BlendMode::OneMinusSrcAlpha)
		, _blendAlphaSrc(hal::BlendMode::SrcAlpha)
		, _blendAlphaDest(hal::BlendMode::OneMinusSrcAlpha)
		, _depthMin(0.0)
		, _depthMax(1.0)
		, _depthSlopeScaleBias(0)
		, _depthBias(0)
		, _depthFunc(hal::CompareFunction::Lequal)
		, _stencilFrontRef(0)
		, _stencilBackRef(0)
		, _stencilFrontReadMask(0xFFFFFFFF)
		, _stencilFrontWriteMask(0xFFFFFFFF)
		, _stencilBackReadMask(0xFFFFFFFF)
		, _stencilBackWriteMask(0xFFFFFFFF)
		, _stencilFrontFunc(hal::CompareFunction::Always)
		, _stencilBackFunc(hal::CompareFunction::Always)
		, _stencilFrontFail(hal::StencilOp::Keep)
		, _stencilFrontZFail(hal::StencilOp::Keep)
		, _stencilFrontPass(hal::StencilOp::Keep)
		, _stencilBackFail(hal::StencilOp::Keep)
		, _stencilBackZFail(hal::StencilOp::Keep)
		, _stencilBackPass(hal::StencilOp::Keep)
		, dirty_(true)
	{
	}

	Material::Material(std::string_view name) noexcept
		: Material()
	{
		this->setName(name);
	}

	Material::Material(const std::shared_ptr<Shader>& shader) noexcept
		: Material()
	{
		_shader = shader;
	}

	Material::~Material() noexcept
	{
		for (auto& it : _properties)
		{
			if (it.data)
				delete it.data;
		}
	}

	void
	Material::setName(std::string_view name) noexcept
	{
		this->name_ = name;
		this->setDirty(true);
	}

	const std::string&
	Material::getName() const noexcept
	{
		return this->name_;
	}

	void
	Material::setShader(const std::shared_ptr<Shader>& shader) noexcept
	{
		_shader = shader;
		this->setDirty(true);
	}

	std::shared_ptr<Shader>
	Material::getShader() const noexcept
	{
		return _shader;
	}

	bool
	Material::set(std::string_view key, bool value) noexcept
	{
		assert(key.size());

		auto it = _properties.begin();
		auto end = _properties.end();

		for (; it != end; ++it)
		{
			if ((*it).key == key)
			{
				_properties.erase(it);
				break;
			}
		}

		MaterialParam prop;
		prop.key = key;
		prop.length = sizeof(bool);
		prop.type = PropertyTypeInfoBool;
		prop.data = new char[prop.length];

		std::memcpy(prop.data, &value, prop.length);

		_properties.push_back(prop);

		this->setDirty(true);

		return true;
	}

	bool
	Material::set(std::string_view key, int value) noexcept
	{
		assert(key.size());

		auto it = _properties.begin();
		auto end = _properties.end();

		for (; it != end; ++it)
		{
			if ((*it).key == key)
			{
				_properties.erase(it);
				break;
			}
		}

		MaterialParam prop;
		prop.key = key;
		prop.length = sizeof(int);
		prop.type = PropertyTypeInfoInt | PropertyTypeInfoBuffer;
		prop.data = new char[prop.length];

		std::memcpy(prop.data, &value, prop.length);

		_properties.push_back(prop);

		this->setDirty(true);

		return true;
	}

	bool
	Material::set(std::string_view key, float value) noexcept
	{
		assert(key.size());

		auto it = _properties.begin();
		auto end = _properties.end();

		for (; it != end; ++it)
		{
			if ((*it).key == key)
			{
				_properties.erase(it);
				break;
			}
		}

		MaterialParam prop;
		prop.key = key;
		prop.length = sizeof(float);
		prop.type = PropertyTypeInfoFloat;
		prop.data = new char[prop.length];

		std::memcpy(prop.data, &value, prop.length);

		_properties.push_back(prop);

		this->setDirty(true);

		return true;
	}

	bool
	Material::set(std::string_view key, const math::Vector2& value) noexcept
	{
		assert(key.size());

		auto it = _properties.begin();
		auto end = _properties.end();

		for (; it != end; ++it)
		{
			if ((*it).key == key)
			{
				_properties.erase(it);
				break;
			}
		}

		MaterialParam prop;
		prop.key = key;
		prop.length = sizeof(math::float2);
		prop.type = PropertyTypeInfoFloat2;
		prop.data = new char[prop.length];

		std::memcpy(prop.data, &value, prop.length);

		_properties.push_back(prop);

		this->setDirty(true);

		return true;
	}

	bool
	Material::set(std::string_view key, const math::Vector3& value) noexcept
	{
		assert(key.size());

		auto it = _properties.begin();
		auto end = _properties.end();

		for (; it != end; ++it)
		{
			if ((*it).key == key)
			{
				_properties.erase(it);
				break;
			}
		}

		MaterialParam prop;
		prop.key = key;
		prop.length = sizeof(math::float3);
		prop.type = PropertyTypeInfoFloat3;
		prop.data = new char[prop.length];

		std::memcpy(prop.data, &value, prop.length);

		_properties.push_back(prop);

		this->setDirty(true);

		return true;
	}

	bool
	Material::set(std::string_view key, const math::Vector4& value) noexcept
	{
		assert(key.size());

		auto it = _properties.begin();
		auto end = _properties.end();

		for (; it != end; ++it)
		{
			if ((*it).key == key)
			{
				_properties.erase(it);
				break;
			}
		}

		MaterialParam prop;
		prop.key = key;
		prop.length = sizeof(math::float4);
		prop.type = PropertyTypeInfoFloat4;
		prop.data = new char[prop.length];

		std::memcpy(prop.data, &value, prop.length);

		_properties.push_back(prop);

		this->setDirty(true);

		return true;
	}

	bool
	Material::set(std::string_view key, std::string_view value) noexcept
	{
		assert(key.size());

		auto it = _properties.begin();
		auto end = _properties.end();

		for (; it != end; ++it)
		{
			if ((*it).key == key)
			{
				_properties.erase(it);
				break;
			}
		}

		MaterialParam prop;
		prop.key = key;
		prop.length = value.length();
		prop.type = PropertyTypeInfoString;
		prop.data = new char[prop.length];

		std::memcpy(prop.data, value.data(), prop.length);

		_properties.push_back(prop);

		this->setDirty(true);

		return true;
	}

	bool
	Material::set(std::string_view key, const std::shared_ptr<GraphicsTexture>& value) noexcept
	{
		assert(key.size());

		auto it = _properties.begin();
		auto end = _properties.end();

		for (; it != end; ++it)
		{
			if ((*it).key == key)
			{
				_properties.erase(it);
				break;
			}
		}

		MaterialParam prop;
		prop.key = key;
		prop.type = PropertyTypeInfoTexture;
		prop.texture = value;
		prop.data = nullptr;

		_properties.push_back(prop);

		this->setDirty(true);

		return true;
	}

	bool
	Material::set(const MaterialParam& value) noexcept
	{
		_properties.push_back(value);
		this->setDirty(true);
		return true;
	}

	bool
	Material::get(std::string_view key, int& value) const noexcept
	{
		assert(key.size());

		MaterialParam prop;
		if (this->get(key, prop))
		{
			if (prop.type == PropertyTypeInfoInt)
			{
				if (prop.length == sizeof(int))
				{
					std::memcpy(&value, prop.data, prop.length);
					return true;
				}
			}
		}

		return false;
	}

	bool
	Material::get(std::string_view key, float& value) const noexcept
	{
		assert(key.size());

		MaterialParam prop;
		if (this->get(key, prop))
		{
			if (prop.type & PropertyTypeInfoFloat)
			{
				if (prop.length == sizeof(float))
				{
					std::memcpy(&value, prop.data, prop.length);
					return true;
				}
			}
		}

		return false;
	}

	bool
	Material::get(std::string_view key, math::Vector2& value) const noexcept
	{
		assert(key.size());

		MaterialParam prop;
		if (this->get(key, prop))
		{
			if (prop.type & PropertyTypeInfoFloat2)
			{
				if (prop.length == sizeof(math::Vector2))
				{
					std::memcpy(&value, prop.data, prop.length);
					return true;
				}
			}
		}

		return false;
	}

	bool
	Material::get(std::string_view key, math::Vector3& value) const noexcept
	{
		assert(key.size());

		MaterialParam prop;
		if (this->get(key, prop))
		{
			if (prop.type & PropertyTypeInfoFloat3)
			{
				if (prop.length == sizeof(math::Vector3))
				{
					std::memcpy(&value, prop.data, prop.length);
					return true;
				}
			}
		}

		return false;
	}

	bool
	Material::get(std::string_view key, math::Vector4& value) const noexcept
	{
		assert(key.size());

		MaterialParam prop;
		if (this->get(key, prop))
		{
			if (prop.type & PropertyTypeInfoFloat4)
			{
				if (prop.length == sizeof(math::Vector4))
				{
					std::memcpy(&value, prop.data, prop.length);
					return true;
				}
			}
		}

		return false;
	}

	bool
	Material::get(std::string_view key, std::string& value) const noexcept
	{
		assert(key.size());

		MaterialParam prop;
		if (this->get(key, prop))
		{
			if (prop.type == PropertyTypeInfoString)
			{
				value.assign(prop.data, prop.length);
				return true;
			}
		}

		return false;
	}

	bool
	Material::get(std::string_view key, std::shared_ptr<GraphicsTexture>& value) const noexcept
	{
		assert(key.size());

		MaterialParam prop;
		if (this->get(key, prop))
		{
			if (prop.type == PropertyTypeInfoTexture)
			{
				value = prop.texture;
				return true;
			}
		}

		return false;
	}

	bool
	Material::get(std::string_view key, MaterialParam& out) const noexcept
	{
		assert(key.size());

		for (auto& it : _properties)
		{
			if (it.key == key)
			{
				out = it;
				return true;
			}
		}

		return false;
	}
	
	void
	Material::setDirty(bool dirty) noexcept
	{
		this->dirty_ = dirty;
	}

	bool
	Material::isDirty() const noexcept
	{
		return this->dirty_;
	}

	const std::vector<MaterialParam>&
	Material::getMaterialParams() const noexcept
	{
		return this->_properties;
	}

	void
	Material::setBlendEnable(bool enable) noexcept
	{
		_enableBlend = enable;
	}

	void
	Material::setBlendOp(hal::BlendOp blendOp) noexcept
	{
		_blendOp = blendOp;
	}

	void
	Material::setBlendSrc(hal::BlendMode factor) noexcept
	{
		_blendSrc = factor;
	}

	void
	Material::setBlendDest(hal::BlendMode factor) noexcept
	{
		_blendDest = factor;
	}

	void
	Material::setBlendAlphaOp(hal::BlendOp blendOp) noexcept
	{
		_blendAlphaOp = blendOp;
	}

	void
	Material::setBlendAlphaSrc(hal::BlendMode factor) noexcept
	{
		_blendAlphaSrc = factor;
	}

	void
	Material::setBlendAlphaDest(hal::BlendMode factor) noexcept
	{
		_blendAlphaDest = factor;
	}

	void
	Material::setColorWriteMask(hal::ColorWriteMaskFlags mask) noexcept
	{
		_colorWriteMask = mask;
	}

	void
	Material::setCullMode(hal::CullMode mode) noexcept
	{
		_cullMode = mode;
	}

	void
	Material::setPolygonMode(hal::PolygonMode mode) noexcept
	{
		_polygonMode = mode;
	}

	void
	Material::setPrimitiveType(hal::VertexType type) noexcept
	{
		_primitiveType = type;
	}

	void
	Material::setFrontFace(hal::FrontFace face) noexcept
	{
		_frontFace = face;
	}

	void
	Material::setScissorTestEnable(bool enable) noexcept
	{
		_enableScissorTest = enable;
	}

	void
	Material::setLinear2sRGBEnable(bool enable) noexcept
	{
		_enableSrgb = enable;
	}

	void
	Material::setMultisampleEnable(bool enable) noexcept
	{
		_enableMultisample = enable;
	}

	void
	Material::setRasterizerDiscardEnable(bool enable) noexcept
	{
		_enableRasterizerDiscard = enable;
	}

	void
	Material::setLineWidth(float width) noexcept
	{
		_lineWidth = width;
	}

	void
	Material::setDepthEnable(bool enable) noexcept
	{
		_enableDepth = enable;
	}

	void
	Material::setDepthWriteEnable(bool enable) noexcept
	{
		_enableDepthWrite = enable;
	}

	void
	Material::setDepthBoundsEnable(bool enable) noexcept
	{
		_enableDepthBounds = enable;
	}

	void
	Material::setDepthMin(float min) noexcept
	{
		_depthMin = min;
	}

	void
	Material::setDepthMax(float max) noexcept
	{
		_depthMax = max;
	}

	void
	Material::setDepthFunc(hal::CompareFunction func) noexcept
	{
		_depthFunc = func;
	}

	void
	Material::setDepthBiasEnable(bool enable) noexcept
	{
		_enableDepthBias = enable;
	}

	void
	Material::setDepthBias(float bias) noexcept
	{
		_depthBias = bias;
	}

	void
	Material::setDepthSlopeScaleBias(float bias) noexcept
	{
		_depthSlopeScaleBias = bias;
	}

	void
	Material::setDepthBiasClamp(bool bias) noexcept
	{
		_enableDepthBiasClamp = bias;
	}

	void
	Material::setDepthClampEnable(bool enable) noexcept
	{
		_enableDepthClamp = enable;
	}

	void
	Material::setStencilEnable(bool enable) noexcept
	{
		_enableStencil = enable;
	}

	void
	Material::setStencilFrontRef(std::uint32_t ref) noexcept
	{
		_stencilFrontRef = ref;
	}

	void
	Material::setStencilFrontFunc(hal::CompareFunction func) noexcept
	{
		_stencilFrontFunc = func;
	}

	void
	Material::setStencilFrontReadMask(std::uint32_t mask) noexcept
	{
		_stencilFrontReadMask = mask;
	}

	void
	Material::setStencilFrontWriteMask(std::uint32_t mask) noexcept
	{
		_stencilFrontWriteMask = mask;
	}

	void
	Material::setStencilFrontFail(hal::StencilOp stencilOp) noexcept
	{
		_stencilFrontFail = stencilOp;
	}

	void
	Material::setStencilFrontZFail(hal::StencilOp stencilOp) noexcept
	{
		_stencilFrontZFail = stencilOp;
	}

	void
	Material::setStencilFrontPass(hal::StencilOp stencilOp) noexcept
	{
		_stencilFrontPass = stencilOp;
	}

	void
	Material::setStencilBackRef(std::uint32_t ref) noexcept
	{
		_stencilBackRef = ref;
	}

	void
	Material::setStencilBackFunc(hal::CompareFunction func) noexcept
	{
		_stencilBackFunc = func;
	}

	void
	Material::setStencilBackReadMask(std::uint32_t mask) noexcept
	{
		_stencilBackReadMask = mask;
	}

	void
	Material::setStencilBackWriteMask(std::uint32_t mask) noexcept
	{
		_stencilBackWriteMask = mask;
	}

	void
	Material::setStencilBackFail(hal::StencilOp stencilOp) noexcept
	{
		_stencilBackFail = stencilOp;
	}

	void
	Material::setStencilBackZFail(hal::StencilOp stencilOp) noexcept
	{
		_stencilBackZFail = stencilOp;
	}

	void
	Material::setStencilBackPass(hal::StencilOp stencilOp) noexcept
	{
		_stencilBackPass = stencilOp;
	}

	hal::CullMode
	Material::getCullMode() const noexcept
	{
		return _cullMode;
	}

	hal::PolygonMode
	Material::getPolygonMode() const noexcept
	{
		return _polygonMode;
	}

	hal::VertexType
	Material::getPrimitiveType() const noexcept
	{
		return _primitiveType;
	}

	hal::FrontFace
	Material::getFrontFace() const noexcept
	{
		return _frontFace;
	}

	bool
	Material::getScissorTestEnable() const noexcept
	{
		return _enableScissorTest;
	}

	bool
	Material::getLinear2sRGBEnable() const noexcept
	{
		return _enableSrgb;
	}

	bool
	Material::getMultisampleEnable() const noexcept
	{
		return _enableMultisample;
	}

	bool
	Material::getRasterizerDiscardEnable() const noexcept
	{
		return _enableRasterizerDiscard;
	}

	float
	Material::getLineWidth() const noexcept
	{
		return _lineWidth;
	}

	bool
	Material::getBlendEnable() const noexcept
	{
		return _enableBlend;
	}

	hal::BlendOp
	Material::getBlendOp() const noexcept
	{
		return _blendOp;
	}

	hal::BlendMode
	Material::getBlendSrc() const noexcept
	{
		return _blendSrc;
	}

	hal::BlendMode
	Material::getBlendDest() const noexcept
	{
		return _blendDest;
	}

	hal::BlendOp
	Material::getBlendAlphaOp() const noexcept
	{
		return _blendAlphaOp;
	}

	hal::BlendMode
	Material::getBlendAlphaSrc() const noexcept
	{
		return _blendAlphaSrc;
	}

	hal::BlendMode
	Material::getBlendAlphaDest() const noexcept
	{
		return _blendAlphaDest;
	}

	hal::ColorWriteMaskFlags
	Material::getColorWriteMask() const noexcept
	{
		return _colorWriteMask;
	}

	bool
	Material::getDepthEnable() const noexcept
	{
		return _enableDepth;
	}

	bool
	Material::getDepthWriteEnable() const noexcept
	{
		return _enableDepthWrite;
	}

	bool
	Material::getDepthBoundsEnable() const noexcept
	{
		return _enableDepthBounds;
	}

	float
	Material::getDepthMin() const noexcept
	{
		return _depthMin;
	}

	float
	Material::getDepthMax() const noexcept
	{
		return _depthMax;
	}

	hal::CompareFunction
	Material::getDepthFunc() const noexcept
	{
		return _depthFunc;
	}

	bool
	Material::getDepthBiasEnable() const noexcept
	{
		return _enableDepthBias;
	}

	float
	Material::getDepthBias() const noexcept
	{
		return _depthBias;
	}

	float
	Material::getDepthSlopeScaleBias() const noexcept
	{
		return _depthSlopeScaleBias;
	}

	bool
	Material::getDepthBiasClamp() const noexcept
	{
		return _enableDepthBiasClamp;
	}

	bool
	Material::getDepthClampEnable() const noexcept
	{
		return _enableDepthClamp;
	}

	bool
	Material::getStencilEnable() const noexcept
	{
		return _enableStencil;
	}

	std::uint32_t
	Material::getStencilFrontRef() const noexcept
	{
		return _stencilFrontRef;
	}

	hal::CompareFunction
	Material::getStencilFrontFunc() const noexcept
	{
		return _stencilFrontFunc;
	}

	std::uint32_t
	Material::getStencilFrontReadMask() const noexcept
	{
		return _stencilFrontReadMask;
	}

	std::uint32_t
	Material::getStencilFrontWriteMask() const noexcept
	{
		return _stencilFrontWriteMask;
	}

	hal::StencilOp
	Material::getStencilFrontFail() const noexcept
	{
		return _stencilFrontFail;
	}

	hal::StencilOp
	Material::getStencilFrontZFail() const noexcept
	{
		return _stencilFrontZFail;
	}

	hal::StencilOp
	Material::getStencilFrontPass() const noexcept
	{
		return _stencilFrontPass;
	}

	std::uint32_t
	Material::getStencilBackRef() const noexcept
	{
		return _stencilBackRef;
	}

	hal::CompareFunction
	Material::getStencilBackFunc() const noexcept
	{
		return _stencilBackFunc;
	}

	std::uint32_t
	Material::getStencilBackReadMask() const noexcept
	{
		return _stencilBackReadMask;
	}

	std::uint32_t
	Material::getStencilBackWriteMask() const noexcept
	{
		return _stencilBackWriteMask;
	}

	hal::StencilOp
	Material::getStencilBackFail() const noexcept
	{
		return _stencilBackFail;
	}

	hal::StencilOp
	Material::getStencilBackZFail() const noexcept
	{
		return _stencilBackZFail;
	}

	hal::StencilOp
	Material::getStencilBackPass() const noexcept
	{
		return _stencilBackPass;
	}

	std::size_t
	Material::hash() const noexcept
	{
		std::hash<std::string> hash;
		std::string hash_string;

		for (auto& it : _properties)
		{
			hash_string += it.key;

			switch (it.type)
			{
			case PropertyTypeInfoBool:
			{
				bool value;
				std::memcpy(&value, it.data, it.length);
				hash_string += std::to_string(value);
			}
			break;
			case PropertyTypeInfoFloat:
			{
				float value;
				std::memcpy(&value, it.data, it.length);
				hash_string += std::to_string(value);
			}
			break;
			case PropertyTypeInfoFloat2:
			case PropertyTypeInfoFloat3:
			case PropertyTypeInfoFloat4:
			{
				for (std::size_t i = 0; i < it.length; i += 4)
				{
					float value;
					std::memcpy(&value, it.data + i, 4);
					hash_string += std::to_string(value);
				}
			}
			break;
			case PropertyTypeInfoInt:
			{
				int value;
				std::memcpy(&value, it.data, it.length);
				hash_string += std::to_string(value);
			}
			break;
			case PropertyTypeInfoInt | PropertyTypeInfoBuffer:
			{
				for (std::size_t i = 0; i < it.length; i += 4)
				{
					int value;
					std::memcpy(&value, it.data + i, 4);
					hash_string += std::to_string(value);
				}
			}
			break;
			case PropertyTypeInfoString:
			{
				hash_string += std::string_view(it.data, it.length);
			}
			break;
			case PropertyTypeInfoTexture:
			{
				if (it.texture)
					hash_string += it.texture->getTextureDesc().getName();
			}
			break;
			default:
				break;
			}
					
		}

		return hash(hash_string);
	}

	void
	Material::copy(const Material& material) noexcept
	{
		this->setName(material.getName());
		this->setShader(material.getShader());
		this->setBlendEnable(material.getBlendEnable());
		this->setBlendOp(material.getBlendOp());
		this->setBlendSrc(material.getBlendSrc());
		this->setBlendDest(material.getBlendDest());
		this->setBlendAlphaOp(material.getBlendAlphaOp());
		this->setBlendAlphaSrc(material.getBlendAlphaSrc());
		this->setBlendAlphaDest(material.getBlendAlphaDest());
		this->setColorWriteMask(material.getColorWriteMask());
		this->setCullMode(material.getCullMode());
		this->setPolygonMode(material.getPolygonMode());
		this->setPrimitiveType(material.getPrimitiveType());
		this->setFrontFace(material.getFrontFace());
		this->setScissorTestEnable(material.getScissorTestEnable());
		this->setLinear2sRGBEnable(material.getLinear2sRGBEnable());
		this->setMultisampleEnable(material.getMultisampleEnable());
		this->setRasterizerDiscardEnable(material.getRasterizerDiscardEnable());
		this->setLineWidth(material.getLineWidth());
		this->setDepthEnable(material.getDepthEnable());
		this->setDepthWriteEnable(material.getDepthWriteEnable());
		this->setDepthBoundsEnable(material.getDepthBoundsEnable());
		this->setDepthMin(material.getDepthMin());
		this->setDepthMax(material.getDepthMax());
		this->setDepthFunc(material.getDepthFunc());
		this->setDepthBiasEnable(material.getDepthBiasEnable());
		this->setDepthBias(material.getDepthBias());
		this->setDepthSlopeScaleBias(material.getDepthSlopeScaleBias());
		this->setDepthBiasClamp(material.getDepthBiasClamp());
		this->setDepthClampEnable(material.getDepthClampEnable());
		this->setStencilEnable(material.getStencilEnable());
		this->setStencilFrontRef(material.getStencilFrontRef());
		this->setStencilFrontFunc(material.getStencilFrontFunc());
		this->setStencilFrontReadMask(material.getStencilFrontReadMask());
		this->setStencilFrontWriteMask(material.getStencilFrontWriteMask());
		this->setStencilFrontFail(material.getStencilFrontFail());
		this->setStencilFrontZFail(material.getStencilFrontZFail());
		this->setStencilFrontPass(material.getStencilFrontPass());
		this->setStencilBackRef(material.getStencilBackRef());
		this->setStencilBackFunc(material.getStencilBackFunc());
		this->setStencilBackReadMask(material.getStencilBackReadMask());
		this->setStencilBackWriteMask(material.getStencilBackWriteMask());
		this->setStencilBackFail(material.getStencilBackFail());
		this->setStencilBackZFail(material.getStencilBackZFail());
		this->setStencilBackPass(material.getStencilBackPass());
	}

	std::shared_ptr<Material>
	Material::clone() const noexcept
	{
		auto material = std::make_shared<Material>();
		for (auto& prop : this->_properties)
			material->set(prop);
		return material;
	}
}