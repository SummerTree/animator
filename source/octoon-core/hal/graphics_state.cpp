#include <octoon/hal/graphics_state.h>

namespace octoon
{
	namespace hal
	{
		OctoonImplementSubInterface(GraphicsState, GraphicsChild, "GraphicsState")

		GraphicsColorBlend::GraphicsColorBlend() noexcept
			: _enable(false)
			, _blendOp(BlendOp::Add)
			, _blendAlphaOp(BlendOp::Add)
			, _blendSrc(BlendMode::SrcAlpha)
			, _blendDest(BlendMode::OneMinusSrcAlpha)
			, _blendAlphaSrc(BlendMode::SrcAlpha)
			, _blendAlphaDest(BlendMode::OneMinusSrcAlpha)
			, _colorWriteMask(ColorWriteMask::RGBABit)
		{
		}

		GraphicsColorBlend::~GraphicsColorBlend() noexcept
		{
		}

		void
		GraphicsColorBlend::setBlendEnable(bool enable) noexcept
		{
			_enable = enable;
		}

		void
		GraphicsColorBlend::setBlendOp(BlendOp blendOp) noexcept
		{
			_blendOp = blendOp;
		}

		void
		GraphicsColorBlend::setBlendSrc(BlendMode factor) noexcept
		{
			_blendSrc = factor;
		}

		void
		GraphicsColorBlend::setBlendDest(BlendMode factor) noexcept
		{
			_blendDest = factor;
		}

		void
		GraphicsColorBlend::setBlendAlphaOp(BlendOp blendOp) noexcept
		{
			_blendAlphaOp = blendOp;
		}

		void
		GraphicsColorBlend::setBlendAlphaSrc(BlendMode factor) noexcept
		{
			_blendAlphaSrc = factor;
		}

		void
		GraphicsColorBlend::setBlendAlphaDest(BlendMode factor) noexcept
		{
			_blendAlphaDest = factor;
		}

		void
		GraphicsColorBlend::setColorWriteMask(ColorWriteMaskFlags mask) noexcept
		{
			_colorWriteMask = mask;
		}

		bool
		GraphicsColorBlend::getBlendEnable() const noexcept
		{
			return _enable;
		}

		BlendOp
		GraphicsColorBlend::getBlendOp() const noexcept
		{
			return _blendOp;
		}

		BlendMode
		GraphicsColorBlend::getBlendSrc() const noexcept
		{
			return _blendSrc;
		}

		BlendMode
		GraphicsColorBlend::getBlendDest() const noexcept
		{
			return _blendDest;
		}

		BlendOp
		GraphicsColorBlend::getBlendAlphaOp() const noexcept
		{
			return _blendAlphaOp;
		}

		BlendMode
		GraphicsColorBlend::getBlendAlphaSrc() const noexcept
		{
			return _blendAlphaSrc;
		}

		BlendMode
		GraphicsColorBlend::getBlendAlphaDest() const noexcept
		{
			return _blendAlphaDest;
		}

		ColorWriteMaskFlags
		GraphicsColorBlend::getColorWriteMask() const noexcept
		{
			return _colorWriteMask;
		}

		GraphicsStateDesc::GraphicsStateDesc() noexcept
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
			, _lineWidth(1.0f)
			, _cullMode(CullMode::Back)
			, _polygonMode(PolygonMode::Solid)
			, _primitiveType(VertexType::TriangleList)
			, _frontFace(FrontFace::CW)
			, _depthMin(0.0)
			, _depthMax(1.0)
			, _depthSlopeScaleBias(0)
			, _depthBias(0)
			, _depthFunc(CompareFunction::Lequal)
			, _stencilFrontRef(0)
			, _stencilBackRef(0)
			, _stencilFrontReadMask(0xFFFFFFFF)
			, _stencilFrontWriteMask(0xFFFFFFFF)
			, _stencilBackReadMask(0xFFFFFFFF)
			, _stencilBackWriteMask(0xFFFFFFFF)
			, _stencilFrontFunc(CompareFunction::Always)
			, _stencilBackFunc(CompareFunction::Always)
			, _stencilFrontFail(StencilOp::Keep)
			, _stencilFrontZFail(StencilOp::Keep)
			, _stencilFrontPass(StencilOp::Keep)
			, _stencilBackFail(StencilOp::Keep)
			, _stencilBackZFail(StencilOp::Keep)
			, _stencilBackPass(StencilOp::Keep)
		{
		}

		GraphicsStateDesc::~GraphicsStateDesc() noexcept
		{
		}

		void
		GraphicsStateDesc::setColorBlends(GraphicsColorBlends&& blends) noexcept
		{
			_blends = std::move(blends);
		}

		void
		GraphicsStateDesc::setColorBlends(const GraphicsColorBlends& blends) noexcept
		{
			_blends = blends;
		}

		GraphicsColorBlends&
		GraphicsStateDesc::getColorBlends() noexcept
		{
			return _blends;
		}

		const GraphicsColorBlends&
		GraphicsStateDesc::getColorBlends() const noexcept
		{
			return _blends;
		}

		void
		GraphicsStateDesc::setCullMode(CullMode mode) noexcept
		{
			_cullMode = mode;
		}

		void
		GraphicsStateDesc::setPolygonMode(PolygonMode mode) noexcept
		{
			_polygonMode = mode;
		}

		void
		GraphicsStateDesc::setPrimitiveType(VertexType type) noexcept
		{
			_primitiveType = type;
		}

		void
		GraphicsStateDesc::setFrontFace(FrontFace face) noexcept
		{
			_frontFace = face;
		}

		void
		GraphicsStateDesc::setScissorTestEnable(bool enable) noexcept
		{
			_enableScissorTest = enable;
		}

		void
		GraphicsStateDesc::setLinear2sRGBEnable(bool enable) noexcept
		{
			_enableSrgb = enable;
		}

		void
		GraphicsStateDesc::setMultisampleEnable(bool enable) noexcept
		{
			_enableMultisample = enable;
		}

		void
		GraphicsStateDesc::setRasterizerDiscardEnable(bool enable) noexcept
		{
			_enableRasterizerDiscard = enable;
		}

		void
		GraphicsStateDesc::setLineWidth(float width) noexcept
		{
			_lineWidth = width;
		}

		void
		GraphicsStateDesc::setDepthEnable(bool enable) noexcept
		{
			_enableDepth = enable;
		}

		void
		GraphicsStateDesc::setDepthWriteEnable(bool enable) noexcept
		{
			_enableDepthWrite = enable;
		}

		void
		GraphicsStateDesc::setDepthBoundsEnable(bool enable) noexcept
		{
			_enableDepthBounds = enable;
		}

		void
		GraphicsStateDesc::setDepthMin(float min) noexcept
		{
			_depthMin = min;
		}

		void
		GraphicsStateDesc::setDepthMax(float max) noexcept
		{
			_depthMax = max;
		}

		void
		GraphicsStateDesc::setDepthFunc(CompareFunction func) noexcept
		{
			_depthFunc = func;
		}

		void
		GraphicsStateDesc::setDepthBiasEnable(bool enable) noexcept
		{
			_enableDepthBias = enable;
		}

		void
		GraphicsStateDesc::setDepthBias(float bias) noexcept
		{
			_depthBias = bias;
		}

		void
		GraphicsStateDesc::setDepthSlopeScaleBias(float bias) noexcept
		{
			_depthSlopeScaleBias = bias;
		}

		void
		GraphicsStateDesc::setDepthBiasClamp(bool bias) noexcept
		{
			_enableDepthBiasClamp = bias;
		}

		void
		GraphicsStateDesc::setDepthClampEnable(bool enable) noexcept
		{
			_enableDepthClamp = enable;
		}

		void
		GraphicsStateDesc::setStencilEnable(bool enable) noexcept
		{
			_enableStencil = enable;
		}

		void
		GraphicsStateDesc::setStencilFrontRef(std::uint32_t ref) noexcept
		{
			_stencilFrontRef = ref;
		}

		void
		GraphicsStateDesc::setStencilFrontFunc(CompareFunction func) noexcept
		{
			_stencilFrontFunc = func;
		}

		void
		GraphicsStateDesc::setStencilFrontReadMask(std::uint32_t mask) noexcept
		{
			_stencilFrontReadMask = mask;
		}

		void
		GraphicsStateDesc::setStencilFrontWriteMask(std::uint32_t mask) noexcept
		{
			_stencilFrontWriteMask = mask;
		}

		void
		GraphicsStateDesc::setStencilFrontFail(StencilOp stencilOp) noexcept
		{
			_stencilFrontFail = stencilOp;
		}

		void
		GraphicsStateDesc::setStencilFrontZFail(StencilOp stencilOp) noexcept
		{
			_stencilFrontZFail = stencilOp;
		}

		void
		GraphicsStateDesc::setStencilFrontPass(StencilOp stencilOp) noexcept
		{
			_stencilFrontPass = stencilOp;
		}

		void
		GraphicsStateDesc::setStencilBackRef(std::uint32_t ref) noexcept
		{
			_stencilBackRef = ref;
		}

		void
		GraphicsStateDesc::setStencilBackFunc(CompareFunction func) noexcept
		{
			_stencilBackFunc = func;
		}

		void
		GraphicsStateDesc::setStencilBackReadMask(std::uint32_t mask) noexcept
		{
			_stencilBackReadMask = mask;
		}

		void
		GraphicsStateDesc::setStencilBackWriteMask(std::uint32_t mask) noexcept
		{
			_stencilBackWriteMask = mask;
		}

		void
		GraphicsStateDesc::setStencilBackFail(StencilOp stencilOp) noexcept
		{
			_stencilBackFail = stencilOp;
		}

		void
		GraphicsStateDesc::setStencilBackZFail(StencilOp stencilOp) noexcept
		{
			_stencilBackZFail = stencilOp;
		}

		void
		GraphicsStateDesc::setStencilBackPass(StencilOp stencilOp) noexcept
		{
			_stencilBackPass = stencilOp;
		}

		CullMode
		GraphicsStateDesc::getCullMode() const noexcept
		{
			return _cullMode;
		}

		PolygonMode
		GraphicsStateDesc::getPolygonMode() const noexcept
		{
			return _polygonMode;
		}

		VertexType
		GraphicsStateDesc::getPrimitiveType() const noexcept
		{
			return _primitiveType;
		}

		FrontFace
		GraphicsStateDesc::getFrontFace() const noexcept
		{
			return _frontFace;
		}

		bool
		GraphicsStateDesc::getScissorTestEnable() const noexcept
		{
			return _enableScissorTest;
		}

		bool
		GraphicsStateDesc::getLinear2sRGBEnable() const noexcept
		{
			return _enableSrgb;
		}

		bool
		GraphicsStateDesc::getMultisampleEnable() const noexcept
		{
			return _enableMultisample;
		}

		bool
		GraphicsStateDesc::getRasterizerDiscardEnable() const noexcept
		{
			return _enableRasterizerDiscard;
		}

		float
		GraphicsStateDesc::getLineWidth() const noexcept
		{
			return _lineWidth;
		}

		bool
		GraphicsStateDesc::getDepthEnable() const noexcept
		{
			return _enableDepth;
		}

		bool
		GraphicsStateDesc::getDepthWriteEnable() const noexcept
		{
			return _enableDepthWrite;
		}

		bool
		GraphicsStateDesc::getDepthBoundsEnable() const noexcept
		{
			return _enableDepthBounds;
		}

		float
		GraphicsStateDesc::getDepthMin() const noexcept
		{
			return _depthMin;
		}

		float
		GraphicsStateDesc::getDepthMax() const noexcept
		{
			return _depthMax;
		}

		CompareFunction
		GraphicsStateDesc::getDepthFunc() const noexcept
		{
			return _depthFunc;
		}

		bool
		GraphicsStateDesc::getDepthBiasEnable() const noexcept
		{
			return _enableDepthBias;
		}

		float
		GraphicsStateDesc::getDepthBias() const noexcept
		{
			return _depthBias;
		}

		float
		GraphicsStateDesc::getDepthSlopeScaleBias() const noexcept
		{
			return _depthSlopeScaleBias;
		}

		bool
		GraphicsStateDesc::getDepthBiasClamp() const noexcept
		{
			return _enableDepthBiasClamp;
		}

		bool
		GraphicsStateDesc::getDepthClampEnable() const noexcept
		{
			return _enableDepthClamp;
		}

		bool
		GraphicsStateDesc::getStencilEnable() const noexcept
		{
			return _enableStencil;
		}

		std::uint32_t
		GraphicsStateDesc::getStencilFrontRef() const noexcept
		{
			return _stencilFrontRef;
		}

		CompareFunction
		GraphicsStateDesc::getStencilFrontFunc() const noexcept
		{
			return _stencilFrontFunc;
		}

		std::uint32_t
		GraphicsStateDesc::getStencilFrontReadMask() const noexcept
		{
			return _stencilFrontReadMask;
		}

		std::uint32_t
		GraphicsStateDesc::getStencilFrontWriteMask() const noexcept
		{
			return _stencilFrontWriteMask;
		}

		StencilOp
		GraphicsStateDesc::getStencilFrontFail() const noexcept
		{
			return _stencilFrontFail;
		}

		StencilOp
		GraphicsStateDesc::getStencilFrontZFail() const noexcept
		{
			return _stencilFrontZFail;
		}

		StencilOp
		GraphicsStateDesc::getStencilFrontPass() const noexcept
		{
			return _stencilFrontPass;
		}

		std::uint32_t
		GraphicsStateDesc::getStencilBackRef() const noexcept
		{
			return _stencilBackRef;
		}

		CompareFunction
		GraphicsStateDesc::getStencilBackFunc() const noexcept
		{
			return _stencilBackFunc;
		}

		std::uint32_t
		GraphicsStateDesc::getStencilBackReadMask() const noexcept
		{
			return _stencilBackReadMask;
		}

		std::uint32_t
		GraphicsStateDesc::getStencilBackWriteMask() const noexcept
		{
			return _stencilBackWriteMask;
		}

		StencilOp
		GraphicsStateDesc::getStencilBackFail() const noexcept
		{
			return _stencilBackFail;
		}

		StencilOp
		GraphicsStateDesc::getStencilBackZFail() const noexcept
		{
			return _stencilBackZFail;
		}

		StencilOp
		GraphicsStateDesc::getStencilBackPass() const noexcept
		{
			return _stencilBackPass;
		}
	}
}