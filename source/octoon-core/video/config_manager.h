#ifndef OCTOON_VIDEO_RTX_MANAGER_H_
#define OCTOON_VIDEO_RTX_MANAGER_H_

#include <vector>
#include <memory>

#include <GL/glew.h>

#include "clw_output.h"
#include "clw_scene_controller.h"
#include "clw_render_factory.h"

#include <octoon/camera/camera.h>
#include <octoon/hal/graphics.h>
#include <octoon/video/render_scene.h>
#include <octoon/video/scriptable_render_context.h>

namespace octoon
{
	enum DeviceType
	{
		kPrimary,
		kSecondary
	};

	enum Mode
	{
		kUseAll,
		kUseGpus,
		kUseSingleGpu,
		kUseSingleCpu,
		kUseCpus
	};

	class ConfigManager
	{
		struct Config
		{
			DeviceType type;
			std::unique_ptr<octoon::Pipeline> pipeline;
			std::unique_ptr<octoon::SceneController> controller;
			std::unique_ptr<octoon::RenderFactory> factory;
			CLWContext context;
			bool caninterop;
		};

	public:
		ConfigManager() noexcept(false);

		void setOutput(OutputType type, Output* output);
		Output* getOutput(OutputType type) const;

		void setMaxBounces(std::uint32_t num_bounces);
		std::uint32_t getMaxBounces() const;

		void setFramebufferSize(std::uint32_t w, std::uint32_t h) noexcept;
		void getFramebufferSize(std::uint32_t& w, std::uint32_t& h) const noexcept;

		void readColorBuffer(math::float3 data[]);
		void readAlbedoBuffer(math::float3 data[]);
		void readNormalBuffer(math::float3 data[]);

		const hal::GraphicsFramebufferPtr& getFramebuffer() const;

		void render(const std::shared_ptr<ScriptableRenderContext>& context, const std::shared_ptr<RenderScene>& scene);

	private:
		void prepareScene(const std::shared_ptr<ScriptableRenderContext>& context, const std::shared_ptr<RenderScene>& scene) noexcept;
		void generateWorkspace(Config& config, const std::shared_ptr<ScriptableRenderContext>& context, std::uint32_t width, std::uint32_t height);

	private:
		bool dirty_;

		std::uint32_t width_;
		std::uint32_t height_;

		std::uint32_t framebufferWidth_;
		std::uint32_t framebufferHeight_;

		void* colorFramebuffer_;
		void* normalFramebuffer_;
		void* albedoFramebuffer_;

		std::unique_ptr<Output> colorImage_;
		std::unique_ptr<Output> normalImage_;
		std::unique_ptr<Output> albedoImage_;

		hal::GraphicsTexturePtr edgeTexture_;
		hal::GraphicsTexturePtr normalTexture_;
		hal::GraphicsTexturePtr albedoTexture_;

		hal::GraphicsFramebufferPtr framebuffer_;

		std::vector<Config> configs_;
		std::array<Output*, static_cast<std::size_t>(OutputType::kMax)> outputs_;
	};
}

#endif
