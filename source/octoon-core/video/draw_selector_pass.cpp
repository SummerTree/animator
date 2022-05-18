#include <octoon/video/draw_selector_pass.h>
#include <octoon/runtime/except.h>
#include <octoon/hal/graphics_device.h>
#include <octoon/hal/graphics_texture.h>
#include <octoon/hal/graphics_framebuffer.h>

namespace octoon
{
	DrawSelectorPass::DrawSelectorPass() noexcept
		: width_(0)
		, height_(0)
		, edgeMaterial_(EdgeMaterial::create())
		, copyMaterial_(MeshCopyMaterial::create())
	{
		edgeMaterial_->setColor(math::float3(1.0f, 0.0f, 1.0f));
		edgeMaterial_->setDepthFunc(hal::CompareFunction::Always);

		octoon::hal::GraphicsColorBlend blend;
		blend.setBlendEnable(true);
		blend.setBlendSrc(octoon::hal::BlendMode::SrcAlpha);
		blend.setBlendDest(octoon::hal::BlendMode::OneMinusSrcAlpha);

		std::vector<octoon::hal::GraphicsColorBlend> blends;
		blends.push_back(blend);

		edgeMaterial_->setColorBlends(std::move(blends));
	}

	void
	DrawSelectorPass::setupFramebuffers(ScriptableRenderContext& context, std::uint32_t w, std::uint32_t h) except
	{
		if (width_ == w && height_ == h)
			return;

		this->width_ = w;
		this->height_ = h;

		hal::GraphicsFramebufferLayoutDesc framebufferLayoutDesc;
		framebufferLayoutDesc.addComponent(hal::GraphicsAttachmentLayout(0, hal::GraphicsImageLayout::ColorAttachmentOptimal, hal::GraphicsFormat::R8G8B8A8UNorm));
		framebufferLayoutDesc.addComponent(hal::GraphicsAttachmentLayout(1, hal::GraphicsImageLayout::DepthStencilAttachmentOptimal, hal::GraphicsFormat::D32_SFLOAT));

		hal::GraphicsTextureDesc depthTextureDesc;
		depthTextureDesc.setWidth(w);
		depthTextureDesc.setHeight(h);
		depthTextureDesc.setTexDim(hal::TextureDimension::Texture2D);
		depthTextureDesc.setTexFormat(hal::GraphicsFormat::D32_SFLOAT);
		depthTexture_ = context.createTexture(depthTextureDesc);
		if (!depthTexture_)
			throw runtime::runtime_error::create("createTexture() failed");

		hal::GraphicsTextureDesc colorTextureDesc;
		colorTextureDesc.setWidth(w);
		colorTextureDesc.setHeight(h);
		colorTextureDesc.setTexDim(hal::TextureDimension::Texture2D);
		colorTextureDesc.setTexFormat(hal::GraphicsFormat::R8G8B8A8UNorm);
		colorTexture_ = context.createTexture(colorTextureDesc);
		if (!colorTexture_)
			throw runtime::runtime_error::create("createTexture() failed");

		hal::GraphicsFramebufferDesc framebufferDesc;
		framebufferDesc.setWidth(w);
		framebufferDesc.setHeight(h);
		framebufferDesc.setFramebufferLayout(context.createFramebufferLayout(framebufferLayoutDesc));
		framebufferDesc.setDepthStencilAttachment(hal::GraphicsAttachmentBinding(depthTexture_, 0, 0));
		framebufferDesc.addColorAttachment(hal::GraphicsAttachmentBinding(colorTexture_, 0, 0));

		colorFramebuffer_ = context.createFramebuffer(framebufferDesc);
		if (!colorFramebuffer_)
			throw runtime::runtime_error::create("createFramebuffer() failed");

		edgeMaterial_->setColorMap(colorTexture_);
	}

	void
	DrawSelectorPass::Execute(ScriptableRenderContext& context, const RenderingData& renderingData) noexcept(false)
	{
		auto& camera = renderingData.camera;
		auto& vp = camera->getPixelViewport();

		auto targetFramebuffer = context.getFramebuffer();

		this->setupFramebuffers(context, vp.width, vp.height);

		context.compileMaterial(edgeMaterial_, renderingData);
		context.compileMaterial(copyMaterial_, renderingData);

		context.configureTarget(colorFramebuffer_);
		context.configureClear(hal::ClearFlagBits::AllBit, math::float4::Zero, 1.0f, 0);
		context.setViewport(0, math::float4((float)vp.x, (float)vp.y, (float)vp.width, (float)vp.height));

		for (auto& geometry : renderingData.geometries)
		{
			if (geometry->getRenderOrder() == 1)
				context.drawRenderers(*geometry, *camera);
		}

		context.configureTarget(targetFramebuffer);
		context.setViewport(0, math::float4((float)vp.x, (float)vp.y, (float)vp.width, (float)vp.height));

		context.setMaterial(edgeMaterial_, *camera, *renderingData.screenQuad);
		context.drawMesh(renderingData.screenQuad->getMesh(), 0);
	}
}