#ifndef OCTOON_MESH_ANIMATION_COMPONENT_H_
#define OCTOON_MESH_ANIMATION_COMPONENT_H_

#include <octoon/mesh/mesh.h>
#include <octoon/material/material.h>
#include <octoon/animation_component.h>

namespace octoon
{
	class AnimationData;
	class OCTOON_EXPORT MeshAnimationComponent final : public AnimationComponent
	{
		OctoonDeclareSubClass(MeshAnimationComponent, AnimationComponent)
	public:
		MeshAnimationComponent() noexcept;
		explicit MeshAnimationComponent(std::string_view path) noexcept(false);
		virtual ~MeshAnimationComponent() noexcept;

		bool setFilePath(std::string_view path) noexcept(false);
		const std::string& getFilePath() const noexcept;

		bool play(std::string_view status = "default") noexcept override;
		void pause() noexcept override;
		void reset() noexcept override;

		void setTime(float time) noexcept override;
		float getTime() const noexcept override;

		void sample(float delta = 0.0f) noexcept;
		void evaluate(float delta = 0.0f) noexcept;

		void setMaterials(const std::unordered_map<std::string, MaterialPtr>& materials);
		const std::unordered_map<std::string, MaterialPtr>& getMaterials() const;

		const AnimatorStateInfo<float>& getCurrentAnimatorStateInfo() const noexcept override;

		void load(const nlohmann::json& json) noexcept(false) override;
		void save(nlohmann::json& json) noexcept(false) override;

		GameComponentPtr clone() const noexcept override;

	private:
		void onActivate() noexcept override;
		void onDeactivate() noexcept override;

		void onFixedUpdate() noexcept override;

		void onAnimationUpdate() noexcept;

		void onMoveBefore() noexcept(false) override;
		void onMoveAfter() noexcept(false) override;

	private:
		void createAnimationData(MeshAnimationComponent& parent, const AnimationData& animationData) noexcept(false);

	private:
		MeshAnimationComponent(const MeshAnimationComponent&) = delete;
		MeshAnimationComponent& operator=(const MeshAnimationComponent&) = delete;

	private:
		std::string path_;

		float minTime_;
		float maxTime_;

		bool enableAnimation_;

		AnimatorStateInfo<float> animationState_;

		GameObjectPtr gameObject_;
		std::unordered_map<std::string, MaterialPtr> materials_;
		std::shared_ptr<AnimationData> animationData_;
	};
}

#endif