#include <octoon/hal/render_state.h>

namespace octoon
{
	OctoonImplementSubInterface(RenderState, GraphicsResource, "RenderState")

	RenderStateDesc::RenderStateDesc() noexcept
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
	{
	}

	RenderStateDesc::~RenderStateDesc() noexcept
	{
	}

	void
	RenderStateDesc::setCullMode(hal::CullMode mode) noexcept
	{
		_cullMode = mode;
	}

	void
	RenderStateDesc::setPolygonMode(hal::PolygonMode mode) noexcept
	{
		_polygonMode = mode;
	}

	void
	RenderStateDesc::setPrimitiveType(hal::VertexType type) noexcept
	{
		_primitiveType = type;
	}

	void
	RenderStateDesc::setFrontFace(hal::FrontFace face) noexcept
	{
		_frontFace = face;
	}

	void
	RenderStateDesc::setScissorTestEnable(bool enable) noexcept
	{
		_enableScissorTest = enable;
	}

	void
	RenderStateDesc::setLinear2sRGBEnable(bool enable) noexcept
	{
		_enableSrgb = enable;
	}

	void
	RenderStateDesc::setMultisampleEnable(bool enable) noexcept
	{
		_enableMultisample = enable;
	}

	void
	RenderStateDesc::setRasterizerDiscardEnable(bool enable) noexcept
	{
		_enableRasterizerDiscard = enable;
	}

	void
	RenderStateDesc::setLineWidth(float width) noexcept
	{
		_lineWidth = width;
	}

	void
	RenderStateDesc::setBlendEnable(bool enable) noexcept
	{
		_enableBlend = enable;
	}

	void
	RenderStateDesc::setBlendOp(hal::BlendOp blendOp) noexcept
	{
		_blendOp = blendOp;
	}

	void
	RenderStateDesc::setBlendSrc(hal::BlendMode factor) noexcept
	{
		_blendSrc = factor;
	}

	void
	RenderStateDesc::setBlendDest(hal::BlendMode factor) noexcept
	{
		_blendDest = factor;
	}

	void
	RenderStateDesc::setBlendAlphaOp(hal::BlendOp blendOp) noexcept
	{
		_blendAlphaOp = blendOp;
	}

	void
	RenderStateDesc::setBlendAlphaSrc(hal::BlendMode factor) noexcept
	{
		_blendAlphaSrc = factor;
	}

	void
	RenderStateDesc::setBlendAlphaDest(hal::BlendMode factor) noexcept
	{
		_blendAlphaDest = factor;
	}

	void
	RenderStateDesc::setColorWriteMask(hal::ColorWriteMaskFlags mask) noexcept
	{
		_colorWriteMask = mask;
	}

	void
	RenderStateDesc::setDepthEnable(bool enable) noexcept
	{
		_enableDepth = enable;
	}

	void
	RenderStateDesc::setDepthWriteEnable(bool enable) noexcept
	{
		_enableDepthWrite = enable;
	}

	void
	RenderStateDesc::setDepthBoundsEnable(bool enable) noexcept
	{
		_enableDepthBounds = enable;
	}

	void
	RenderStateDesc::setDepthMin(float min) noexcept
	{
		_depthMin = min;
	}

	void
	RenderStateDesc::setDepthMax(float max) noexcept
	{
		_depthMax = max;
	}

	void
	RenderStateDesc::setDepthFunc(hal::CompareFunction func) noexcept
	{
		_depthFunc = func;
	}

	void
	RenderStateDesc::setDepthBiasEnable(bool enable) noexcept
	{
		_enableDepthBias = enable;
	}

	void
	RenderStateDesc::setDepthBias(float bias) noexcept
	{
		_depthBias = bias;
	}

	void
	RenderStateDesc::setDepthSlopeScaleBias(float bias) noexcept
	{
		_depthSlopeScaleBias = bias;
	}

	void
	RenderStateDesc::setDepthBiasClamp(bool bias) noexcept
	{
		_enableDepthBiasClamp = bias;
	}

	void
	RenderStateDesc::setDepthClampEnable(bool enable) noexcept
	{
		_enableDepthClamp = enable;
	}

	void
	RenderStateDesc::setStencilEnable(bool enable) noexcept
	{
		_enableStencil = enable;
	}

	void
	RenderStateDesc::setStencilFrontRef(std::uint32_t ref) noexcept
	{
		_stencilFrontRef = ref;
	}

	void
	RenderStateDesc::setStencilFrontFunc(hal::CompareFunction func) noexcept
	{
		_stencilFrontFunc = func;
	}

	void
	RenderStateDesc::setStencilFrontReadMask(std::uint32_t mask) noexcept
	{
		_stencilFrontReadMask = mask;
	}

	void
	RenderStateDesc::setStencilFrontWriteMask(std::uint32_t mask) noexcept
	{
		_stencilFrontWriteMask = mask;
	}

	void
	RenderStateDesc::setStencilFrontFail(hal::StencilOp stencilOp) noexcept
	{
		_stencilFrontFail = stencilOp;
	}

	void
	RenderStateDesc::setStencilFrontZFail(hal::StencilOp stencilOp) noexcept
	{
		_stencilFrontZFail = stencilOp;
	}

	void
	RenderStateDesc::setStencilFrontPass(hal::StencilOp stencilOp) noexcept
	{
		_stencilFrontPass = stencilOp;
	}

	void
	RenderStateDesc::setStencilBackRef(std::uint32_t ref) noexcept
	{
		_stencilBackRef = ref;
	}

	void
	RenderStateDesc::setStencilBackFunc(hal::CompareFunction func) noexcept
	{
		_stencilBackFunc = func;
	}

	void
	RenderStateDesc::setStencilBackReadMask(std::uint32_t mask) noexcept
	{
		_stencilBackReadMask = mask;
	}

	void
	RenderStateDesc::setStencilBackWriteMask(std::uint32_t mask) noexcept
	{
		_stencilBackWriteMask = mask;
	}

	void
	RenderStateDesc::setStencilBackFail(hal::StencilOp stencilOp) noexcept
	{
		_stencilBackFail = stencilOp;
	}

	void
	RenderStateDesc::setStencilBackZFail(hal::StencilOp stencilOp) noexcept
	{
		_stencilBackZFail = stencilOp;
	}

	void
	RenderStateDesc::setStencilBackPass(hal::StencilOp stencilOp) noexcept
	{
		_stencilBackPass = stencilOp;
	}

	hal::CullMode
	RenderStateDesc::getCullMode() const noexcept
	{
		return _cullMode;
	}

	hal::PolygonMode
	RenderStateDesc::getPolygonMode() const noexcept
	{
		return _polygonMode;
	}

	hal::VertexType
	RenderStateDesc::getPrimitiveType() const noexcept
	{
		return _primitiveType;
	}

	hal::FrontFace
	RenderStateDesc::getFrontFace() const noexcept
	{
		return _frontFace;
	}

	bool
	RenderStateDesc::getScissorTestEnable() const noexcept
	{
		return _enableScissorTest;
	}

	bool
	RenderStateDesc::getLinear2sRGBEnable() const noexcept
	{
		return _enableSrgb;
	}

	bool
	RenderStateDesc::getMultisampleEnable() const noexcept
	{
		return _enableMultisample;
	}

	bool
	RenderStateDesc::getRasterizerDiscardEnable() const noexcept
	{
		return _enableRasterizerDiscard;
	}

	float
	RenderStateDesc::getLineWidth() const noexcept
	{
		return _lineWidth;
	}

	bool
	RenderStateDesc::getBlendEnable() const noexcept
	{
		return _enableBlend;
	}

	hal::BlendOp
	RenderStateDesc::getBlendOp() const noexcept
	{
		return _blendOp;
	}

	hal::BlendMode
	RenderStateDesc::getBlendSrc() const noexcept
	{
		return _blendSrc;
	}

	hal::BlendMode
	RenderStateDesc::getBlendDest() const noexcept
	{
		return _blendDest;
	}

	hal::BlendOp
	RenderStateDesc::getBlendAlphaOp() const noexcept
	{
		return _blendAlphaOp;
	}

	hal::BlendMode
	RenderStateDesc::getBlendAlphaSrc() const noexcept
	{
		return _blendAlphaSrc;
	}

	hal::BlendMode
	RenderStateDesc::getBlendAlphaDest() const noexcept
	{
		return _blendAlphaDest;
	}

	hal::ColorWriteMaskFlags
	RenderStateDesc::getColorWriteMask() const noexcept
	{
		return _colorWriteMask;
	}

	bool
	RenderStateDesc::getDepthEnable() const noexcept
	{
		return _enableDepth;
	}

	bool
	RenderStateDesc::getDepthWriteEnable() const noexcept
	{
		return _enableDepthWrite;
	}

	bool
	RenderStateDesc::getDepthBoundsEnable() const noexcept
	{
		return _enableDepthBounds;
	}

	float
	RenderStateDesc::getDepthMin() const noexcept
	{
		return _depthMin;
	}

	float
	RenderStateDesc::getDepthMax() const noexcept
	{
		return _depthMax;
	}

	hal::CompareFunction
	RenderStateDesc::getDepthFunc() const noexcept
	{
		return _depthFunc;
	}

	bool
	RenderStateDesc::getDepthBiasEnable() const noexcept
	{
		return _enableDepthBias;
	}

	float
	RenderStateDesc::getDepthBias() const noexcept
	{
		return _depthBias;
	}

	float
	RenderStateDesc::getDepthSlopeScaleBias() const noexcept
	{
		return _depthSlopeScaleBias;
	}

	bool
	RenderStateDesc::getDepthBiasClamp() const noexcept
	{
		return _enableDepthBiasClamp;
	}

	bool
	RenderStateDesc::getDepthClampEnable() const noexcept
	{
		return _enableDepthClamp;
	}

	bool
	RenderStateDesc::getStencilEnable() const noexcept
	{
		return _enableStencil;
	}

	std::uint32_t
	RenderStateDesc::getStencilFrontRef() const noexcept
	{
		return _stencilFrontRef;
	}

	hal::CompareFunction
	RenderStateDesc::getStencilFrontFunc() const noexcept
	{
		return _stencilFrontFunc;
	}

	std::uint32_t
	RenderStateDesc::getStencilFrontReadMask() const noexcept
	{
		return _stencilFrontReadMask;
	}

	std::uint32_t
	RenderStateDesc::getStencilFrontWriteMask() const noexcept
	{
		return _stencilFrontWriteMask;
	}

	hal::StencilOp
	RenderStateDesc::getStencilFrontFail() const noexcept
	{
		return _stencilFrontFail;
	}

	hal::StencilOp
	RenderStateDesc::getStencilFrontZFail() const noexcept
	{
		return _stencilFrontZFail;
	}

	hal::StencilOp
	RenderStateDesc::getStencilFrontPass() const noexcept
	{
		return _stencilFrontPass;
	}

	std::uint32_t
	RenderStateDesc::getStencilBackRef() const noexcept
	{
		return _stencilBackRef;
	}

	hal::CompareFunction
	RenderStateDesc::getStencilBackFunc() const noexcept
	{
		return _stencilBackFunc;
	}

	std::uint32_t
	RenderStateDesc::getStencilBackReadMask() const noexcept
	{
		return _stencilBackReadMask;
	}

	std::uint32_t
	RenderStateDesc::getStencilBackWriteMask() const noexcept
	{
		return _stencilBackWriteMask;
	}

	hal::StencilOp
	RenderStateDesc::getStencilBackFail() const noexcept
	{
		return _stencilBackFail;
	}

	hal::StencilOp
	RenderStateDesc::getStencilBackZFail() const noexcept
	{
		return _stencilBackZFail;
	}

	hal::StencilOp
	RenderStateDesc::getStencilBackPass() const noexcept
	{
		return _stencilBackPass;
	}
}