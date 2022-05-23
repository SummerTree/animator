#include <octoon/video/scriptable_render_context.h>
#include <octoon/video/scriptable_render_buffer.h>
#include <octoon/video/scriptable_render_material.h>
#include <octoon/video/rendering_data.h>
#include <octoon/mesh/plane_mesh.h>
#include <octoon/camera/perspective_camera.h>
#include <octoon/light/ambient_light.h>
#include <octoon/light/directional_light.h>
#include <octoon/light/point_light.h>
#include <octoon/light/spot_light.h>
#include <octoon/light/disk_light.h>
#include <octoon/light/rectangle_light.h>
#include <octoon/light/environment_light.h>
#include <octoon/light/tube_light.h>

#include <octoon/hal/graphics_device.h>
#include <octoon/hal/graphics_framebuffer.h>
#include <octoon/hal/graphics_data.h>
#include <octoon/hal/graphics_context.h>

namespace octoon
{
	ScriptableRenderContext::ScriptableRenderContext()
	{
	}

	ScriptableRenderContext::ScriptableRenderContext(const hal::GraphicsContextPtr& context)
		: context_(context)
	{
	}

	ScriptableRenderContext::~ScriptableRenderContext()
	{
	}

	void
	ScriptableRenderContext::setGraphicsContext(const hal::GraphicsContextPtr& context) noexcept(false)
	{
		this->context_ = context;
	}

	const hal::GraphicsContextPtr&
	ScriptableRenderContext::getGraphicsContext() const noexcept(false)
	{
		return this->context_;
	}

	hal::GraphicsInputLayoutPtr
	ScriptableRenderContext::createInputLayout(const hal::GraphicsInputLayoutDesc& desc) noexcept
	{
		return this->context_->getDevice()->createInputLayout(desc);
	}

	hal::GraphicsDataPtr
	ScriptableRenderContext::createGraphicsData(const hal::GraphicsDataDesc& desc) noexcept
	{
		return this->context_->getDevice()->createGraphicsData(desc);
	}

	hal::GraphicsTexturePtr
	ScriptableRenderContext::createTexture(const hal::GraphicsTextureDesc& desc) noexcept
	{
		return this->context_->getDevice()->createTexture(desc);
	}

	hal::GraphicsSamplerPtr
	ScriptableRenderContext::createSampler(const hal::GraphicsSamplerDesc& desc) noexcept
	{
		return this->context_->getDevice()->createSampler(desc);
	}

	hal::GraphicsFramebufferPtr
	ScriptableRenderContext::createFramebuffer(const hal::GraphicsFramebufferDesc& desc) noexcept
	{
		return this->context_->getDevice()->createFramebuffer(desc);
	}

	hal::GraphicsFramebufferLayoutPtr
	ScriptableRenderContext::createFramebufferLayout(const hal::GraphicsFramebufferLayoutDesc& desc) noexcept
	{
		return this->context_->getDevice()->createFramebufferLayout(desc);
	}

	hal::GraphicsShaderPtr
	ScriptableRenderContext::createShader(const hal::GraphicsShaderDesc& desc) noexcept
	{
		return this->context_->getDevice()->createShader(desc);
	}

	hal::GraphicsProgramPtr
	ScriptableRenderContext::createProgram(const hal::GraphicsProgramDesc& desc) noexcept
	{
		return this->context_->getDevice()->createProgram(desc);
	}

	hal::GraphicsStatePtr
	ScriptableRenderContext::createRenderState(const hal::GraphicsStateDesc& desc) noexcept
	{
		return this->context_->getDevice()->createRenderState(desc);
	}

	hal::GraphicsPipelinePtr
	ScriptableRenderContext::createRenderPipeline(const hal::GraphicsPipelineDesc& desc) noexcept
	{
		return this->context_->getDevice()->createRenderPipeline(desc);
	}

	hal::GraphicsDescriptorSetPtr
	ScriptableRenderContext::createDescriptorSet(const hal::GraphicsDescriptorSetDesc& desc) noexcept
	{
		return this->context_->getDevice()->createDescriptorSet(desc);
	}

	hal::GraphicsDescriptorSetLayoutPtr
	ScriptableRenderContext::createDescriptorSetLayout(const hal::GraphicsDescriptorSetLayoutDesc& desc) noexcept
	{
		return this->context_->getDevice()->createDescriptorSetLayout(desc);
	}

	hal::GraphicsDescriptorPoolPtr
	ScriptableRenderContext::createDescriptorPool(const hal::GraphicsDescriptorPoolDesc& desc) noexcept
	{
		return this->context_->getDevice()->createDescriptorPool(desc);
	}

	void
	ScriptableRenderContext::setViewport(std::uint32_t i, const math::float4& viewport) noexcept
	{
		this->context_->setViewport(i, viewport);
	}

	const math::float4&
	ScriptableRenderContext::getViewport(std::uint32_t i) const noexcept
	{
		return this->context_->getViewport(i);
	}

	void
	ScriptableRenderContext::setScissor(std::uint32_t i, const math::uint4& scissor) noexcept
	{
		this->context_->setScissor(i, scissor);
	}

	const math::uint4&
	ScriptableRenderContext::getScissor(std::uint32_t i) const noexcept
	{
		return this->context_->getScissor(i);
	}

	void
	ScriptableRenderContext::setStencilCompareMask(hal::StencilFaceFlags face, std::uint32_t mask) noexcept
	{
		this->context_->setStencilCompareMask(face, mask);
	}
	std::uint32_t
	ScriptableRenderContext::getStencilCompareMask(hal::StencilFaceFlags face) noexcept
	{
		return this->context_->getStencilCompareMask(face);
	}

	void
	ScriptableRenderContext::setStencilReference(hal::StencilFaceFlags face, std::uint32_t reference) noexcept
	{
		this->context_->setStencilReference(face, reference);
	}

	std::uint32_t
	ScriptableRenderContext::getStencilReference(hal::StencilFaceFlags face) noexcept
	{
		return this->context_->getStencilReference(face);
	}

	void
	ScriptableRenderContext::setStencilWriteMask(hal::StencilFaceFlags face, std::uint32_t mask) noexcept
	{
		this->context_->setStencilWriteMask(face, mask);
	}

	std::uint32_t
	ScriptableRenderContext::getStencilWriteMask(hal::StencilFaceFlags face) noexcept
	{
		return this->context_->getStencilWriteMask(face);
	}

	void
	ScriptableRenderContext::setRenderPipeline(const hal::GraphicsPipelinePtr& pipeline) noexcept
	{
		this->context_->setRenderPipeline(pipeline);
	}

	hal::GraphicsPipelinePtr
	ScriptableRenderContext::getRenderPipeline() const noexcept
	{
		return this->context_->getRenderPipeline();
	}

	void
	ScriptableRenderContext::setDescriptorSet(const hal::GraphicsDescriptorSetPtr& descriptorSet) noexcept
	{
		this->context_->setDescriptorSet(descriptorSet);
	}

	hal::GraphicsDescriptorSetPtr
	ScriptableRenderContext::getDescriptorSet() const noexcept
	{
		return this->context_->getDescriptorSet();
	}

	void
	ScriptableRenderContext::setVertexBufferData(std::uint32_t i, const hal::GraphicsDataPtr& data, std::intptr_t offset) noexcept
	{
		this->context_->setVertexBufferData(i, data, offset);
	}

	hal::GraphicsDataPtr
	ScriptableRenderContext::getVertexBufferData(std::uint32_t i) const noexcept
	{
		return this->context_->getVertexBufferData(i);
	}

	void
	ScriptableRenderContext::setIndexBufferData(const hal::GraphicsDataPtr& data, std::intptr_t offset, hal::IndexFormat indexType) noexcept
	{
		this->context_->setIndexBufferData(data, offset, indexType);
	}

	hal::GraphicsDataPtr
	ScriptableRenderContext::getIndexBufferData() const noexcept
	{
		return this->context_->getIndexBufferData();
	}

	void
	ScriptableRenderContext::configureTarget(const hal::GraphicsFramebufferPtr& target) noexcept
	{
		this->context_->setFramebuffer(target);
	}

	void
	ScriptableRenderContext::configureClear(hal::ClearFlags flags, const math::float4& color, float depth, std::int32_t stencil) noexcept
	{
		this->context_->clearFramebuffer(0, flags, color, depth, stencil);
	}

	void
	ScriptableRenderContext::discardFramebuffer(const hal::GraphicsFramebufferPtr& src, hal::ClearFlags flags) noexcept
	{
		this->context_->discardFramebuffer(src, flags);
	}

	void
	ScriptableRenderContext::blitFramebuffer(const hal::GraphicsFramebufferPtr& src, const math::float4& v1, const hal::GraphicsFramebufferPtr& dest, const math::float4& v2) noexcept
	{
		this->context_->blitFramebuffer(src, v1, dest, v2);
	}

	void
	ScriptableRenderContext::readFramebuffer(std::uint32_t i, const hal::GraphicsTexturePtr& texture, std::uint32_t miplevel, std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height) noexcept
	{
		this->context_->readFramebuffer(i, texture, miplevel, x, y, width, height);
	}

	void
	ScriptableRenderContext::readFramebufferToCube(std::uint32_t i, std::uint32_t face, const hal::GraphicsTexturePtr& texture, std::uint32_t miplevel, std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height) noexcept
	{
		this->context_->readFramebufferToCube(i, face, texture, miplevel, x, y, width, height);
	}

	hal::GraphicsFramebufferPtr
	ScriptableRenderContext::getFramebuffer() const noexcept
	{
		return this->context_->getFramebuffer();
	}

	void
	ScriptableRenderContext::draw(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t startVertice, std::uint32_t startInstances) noexcept
	{
		this->context_->draw(numVertices, numInstances, startVertice, startInstances);
	}

	void
	ScriptableRenderContext::drawIndexed(std::uint32_t numIndices, std::uint32_t numInstances, std::uint32_t startIndice, std::uint32_t startVertice, std::uint32_t startInstances) noexcept
	{
		this->context_->drawIndexed(numIndices, numInstances, startIndice, startVertice, startInstances);
	}

	void
	ScriptableRenderContext::drawIndirect(const hal::GraphicsDataPtr& data, std::size_t offset, std::uint32_t drawCount, std::uint32_t stride) noexcept
	{
		this->context_->drawIndirect(data, offset, drawCount, stride);
	}

	void
	ScriptableRenderContext::drawIndexedIndirect(const hal::GraphicsDataPtr& data, std::size_t offset, std::uint32_t drawCount, std::uint32_t stride) noexcept
	{
		this->context_->drawIndexedIndirect(data, offset, drawCount, stride);
	}

	void
	ScriptableRenderContext::compileMaterial(const std::shared_ptr<Material>& material, const RenderingData& renderingData)
	{
		auto it = this->materials_.find(material.get());
		if (it == this->materials_.end())
			this->materials_[material.get()] = std::make_shared<ScriptableRenderMaterial>(this->context_, material, renderingData);
	}

	void
	ScriptableRenderContext::setMaterial(const std::shared_ptr<Material>& material, const RenderingData& renderingData, const Camera& camera, const Geometry& geometry)
	{
		assert(material);

		auto& pipeline = this->materials_.at(material.get());
		pipeline->update(renderingData, camera, geometry);

		this->setRenderPipeline(pipeline->getPipeline());
		this->setDescriptorSet(pipeline->getDescriptorSet());
	}

	void
	ScriptableRenderContext::generateMipmap(const hal::GraphicsTexturePtr& texture) noexcept
	{
		this->context_->generateMipmap(texture);
	}

	void
	ScriptableRenderContext::drawMesh(const std::shared_ptr<Mesh>& mesh, std::size_t subset)
	{
		auto& buffer = buffers_.at(mesh.get());
		this->setVertexBufferData(0, buffer->getVertexBuffer(), 0);
		this->setIndexBufferData(buffer->getIndexBuffer(), 0, hal::IndexFormat::UInt32);

		if (buffer->getIndexBuffer())
			this->drawIndexed((std::uint32_t)buffer->getNumIndices(subset), 1, (std::uint32_t)buffer->getStartIndices(subset), 0, 0);
		else
			this->draw((std::uint32_t)buffer->getNumVertices(), 1, 0, 0);
	}

	void
	ScriptableRenderContext::drawRenderers(const Geometry& geometry, const Camera& camera, const RenderingData& renderingData, const std::shared_ptr<Material>& overrideMaterial) noexcept
	{
		if (camera.getLayer() != geometry.getLayer())
			return;

		if (geometry.getVisible())
		{
			auto numMaterials = geometry.getMaterials().size();

			for (std::size_t i = 0; i < numMaterials; i++)
			{
				auto mesh = geometry.getMesh();
				auto material = geometry.getMaterial(i);
				if (material && overrideMaterial)
				{
					if (material->getPrimitiveType() == overrideMaterial->getPrimitiveType())
						material = overrideMaterial;
				}

				if (material && mesh && i < mesh->getNumSubsets())
				{
					this->setMaterial(overrideMaterial ? overrideMaterial : material, renderingData, camera, geometry);
					this->drawMesh(mesh, i);
				}
			}
		}
	}

	void
	ScriptableRenderContext::drawRenderers(const std::vector<Geometry*>& geometries, const Camera& camera, const RenderingData& renderingData, const std::shared_ptr<Material>& overrideMaterial) noexcept
	{
		for (auto& geometry : geometries)
			this->drawRenderers(*geometry, camera, renderingData, overrideMaterial);
	}
}
