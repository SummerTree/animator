#ifndef OCTOON_RTTI_OBJECT_H_
#define OCTOON_RTTI_OBJECT_H_

#include <string>
#include <memory>
#include <vector>

#include <octoon/runtime/json.h>
#include <octoon/runtime/rtti.h>
#include <octoon/runtime/rtti_macros.h>

namespace octoon
{
	typedef std::shared_ptr<class RttiObject> RttiObjectPtr;

	class OCTOON_EXPORT RttiObject : public std::enable_shared_from_this<RttiObject>
	{
		OctoonDeclareClass(RttiObject)
	public:
		RttiObject() = default;
		virtual ~RttiObject() = default;

		bool isInstanceOf(const Rtti* rtti) const noexcept;
		bool isInstanceOf(const Rtti& rtti) const noexcept;
		bool isInstanceOf(std::string_view className) const noexcept;

		template<typename T>
		bool isInstanceOf() const noexcept
		{
			return this->isInstanceOf(T::getRtti());
		}

		bool isA(const Rtti* rtti) const noexcept;
		bool isA(const Rtti& rtti) const noexcept;
		bool isA(std::string_view rttiName) const noexcept;

		template<typename T>
		bool isA() const noexcept
		{
			return this->isA(T::getRtti());
		}

		template<typename T>
		T* upcast() noexcept
		{
			assert(this->isA<T>());
			return dynamic_cast<T*>(this);
		}

		template<typename T>
		T* downcast() noexcept
		{
			assert(this->isA<T>());
			return dynamic_cast<T*>(this);
		}

		template<typename T>
		T* cast() noexcept
		{
			return reinterpret_cast<T*>(this);
		}

		template<typename T>
		const T* upcast() const noexcept
		{
			assert(this->isA<T>());
			return dynamic_cast<const T*>(this);
		}

		template<typename T>
		const T* downcast() const noexcept
		{
			assert(this->isA<T>());
			return dynamic_cast<const T*>(this);
		}

		template<typename T>
		const T* cast() const noexcept
		{
			return reinterpret_cast<const T*>(this);
		}

		template<typename T>
		std::shared_ptr<T> upcast_pointer() noexcept
		{
			assert(this->isA<T>());
			return std::dynamic_pointer_cast<T>(this->shared_from_this());
		}

		template<typename T>
		std::shared_ptr<T> downcast_pointer() noexcept
		{
			assert(this->isA<T>());
			return std::dynamic_pointer_cast<T>(this->shared_from_this());
		}

		template<typename T>
		std::shared_ptr<T> cast_pointer() noexcept
		{
			assert(this->isA<T>());
			return std::dynamic_pointer_cast<T>(this->shared_from_this());
		}

		template<typename T>
		std::shared_ptr<const T> upcast_pointer() const noexcept
		{
			assert(this->isA<T>());
			return std::dynamic_pointer_cast<const T>(this->shared_from_this());
		}

		template<typename T>
		std::shared_ptr<const T> downcast_pointer() const noexcept
		{
			assert(this->isA<T>());
			return std::dynamic_pointer_cast<const T>(this->shared_from_this());
		}

		template<typename T>
		std::shared_ptr<const T> cast_pointer() const noexcept
		{
			assert(this->isA<T>());
			return std::dynamic_pointer_cast<const T>(this->shared_from_this());
		}

		virtual void load(const nlohmann::json& json) noexcept(false)
		{
		}

		virtual void save(nlohmann::json& json) noexcept(false)
		{
			json["_type"] = this->type_name();
		}
	};
}

#endif