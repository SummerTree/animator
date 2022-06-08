#include <octoon/runtime/rtti_object.h>

namespace octoon
{
	OctoonImplementClass(RttiObject, "Object")

	bool
	RttiObject::isInstanceOf(const Rtti* rtti) const noexcept
	{
		return this->rtti() == rtti;
	}

	bool
	RttiObject::isInstanceOf(const Rtti& rtti) const noexcept
	{
		return this->rtti() == &rtti;
	}

	bool
	RttiObject::isInstanceOf(std::string_view className) const noexcept
	{
		return this->rtti()->type_name() == className;
	}

	bool
	RttiObject::isA(const Rtti* rtti) const noexcept
	{
		return this->rtti()->isDerivedFrom(rtti);
	}

	bool
	RttiObject::isA(const Rtti& rtti) const noexcept
	{
		return this->rtti()->isDerivedFrom(rtti);
	}

	bool
	RttiObject::isA(std::string_view rttiName) const noexcept
	{
		return this->rtti()->isDerivedFrom(rttiName);
	}
}