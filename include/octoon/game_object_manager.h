#ifndef OCTOON_GAME_OBJECT_MANAGER_H_
#define OCTOON_GAME_OBJECT_MANAGER_H_

#include <stack>
#include <mutex>
#include <octoon/game_object.h>
#include <octoon/runtime/singleton.h>

namespace octoon
{
	class OCTOON_EXPORT GameObjectManager final
	{
		OctoonDeclareSingleton(GameObjectManager)
	public:
		GameObjectManager() noexcept;
		~GameObjectManager() noexcept;

		GameObjectPtr find(const char* name) noexcept;
		GameObjectPtr find(const std::string& name) noexcept;

		const GameObjectRaws& instances() const noexcept;

		void onFixedUpdate() except;
		void onUpdate() except;
		void onLateUpdate() except;

		void onGui() except;

		void sendMessage(const std::string& event, const runtime::any& data = nullptr) noexcept;
		void addMessageListener(const std::string& event, std::function<void(const runtime::any&)> listener) noexcept;
		void removeMessageListener(const std::string& event, std::function<void(const runtime::any&)> listener) noexcept;

	private:
		friend GameObject;

		void _instanceObject(GameObject* entity, std::size_t& instanceID) noexcept;
		void _unsetObject(GameObject* entity) noexcept;
		void _activeObject(GameObject* entity, bool active) noexcept;

	private:
		bool hasEmptyActors_;

		GameObjectRaws instanceLists_;
		GameObjectRaws activeActors_;

		std::mutex lock_;
		std::stack<std::size_t> emptyLists_;

		std::vector<GameComponentRaws> dispatchComponents_;
		std::map<std::string, runtime::signal<void(const runtime::any&)>> dispatchEvents_;
	};
}

#endif
