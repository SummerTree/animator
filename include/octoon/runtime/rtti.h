#ifndef OCTOON_RTTI_H_
#define OCTOON_RTTI_H_

#include <string>
#include <memory>
#include <vector>

#include <octoon/runtime/rtti_macros.h>

namespace octoon
{
	class RttiObject;

	class OCTOON_EXPORT Rtti
	{
	public:
		typedef RttiObject*(*RttiConstruct)();
	public:
		Rtti(std::string_view name, RttiConstruct creator, const Rtti* parent) noexcept;
		~Rtti() = default;

		std::shared_ptr<class RttiObject> create() const noexcept(false); // throw(std::bad_alloc)

		const Rtti* getParent() const noexcept;

		const std::string& type_name() const noexcept;

		bool isDerivedFrom(const Rtti* other) const noexcept;
		bool isDerivedFrom(const Rtti& other) const noexcept;
		bool isDerivedFrom(std::string_view name) const noexcept;

	private:
		std::string name_;
		const Rtti* parent_;
		RttiConstruct construct_;
	};
}

#endif