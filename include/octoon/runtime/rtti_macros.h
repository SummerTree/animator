#ifndef OCTOON_RTTI_MACROS_H_
#define OCTOON_RTTI_MACROS_H_

#include <octoon/runtime/platform.h>

namespace octoon
{
#define OctoonDeclareInterface(Base)\
public:\
	static OCTOON_PROJECT Rtti RTTI;\
	static OCTOON_PROJECT Rtti* getRtti() noexcept;\
    virtual OCTOON_PROJECT Rtti* rtti() const noexcept;\
	virtual const char* type_name() noexcept;\
private:

#define OctoonImplementInterface(Base, Name) \
    OCTOON_PROJECT Rtti Base::RTTI = OCTOON_PROJECT Rtti(Name, nullptr, nullptr);\
	OCTOON_PROJECT Rtti* Base::rtti() const noexcept { return &RTTI; }\
	static OCTOON_PROJECT Rtti* Base::getRtti() noexcept { return &RTTI; }\
	const char* Base::type_name() noexcept { return Name; };

#define OctoonDeclareSubInterface(Derived, Base)\
public:\
	static OCTOON_PROJECT Rtti RTTI;\
	static OCTOON_PROJECT Rtti* getRtti() noexcept;\
    virtual OCTOON_PROJECT Rtti* rtti() const noexcept override;\
	virtual const char* type_name() noexcept override;\
private:

#define OctoonImplementSubInterface(Derived, Base, Name) \
    OCTOON_PROJECT Rtti Derived::RTTI = OCTOON_PROJECT Rtti(Name, nullptr, Base::getRtti());\
	OCTOON_PROJECT Rtti* Derived::rtti() const noexcept { return &RTTI; }\
	OCTOON_PROJECT Rtti* Derived::getRtti() noexcept { return &RTTI; }\
	const char* Derived::type_name() noexcept { return Name; };

#define OctoonDeclareClass(Base) \
public:\
	static OCTOON_PROJECT Rtti RTTI;\
	static OCTOON_PROJECT Rtti* getRtti() noexcept;\
    virtual OCTOON_PROJECT Rtti* rtti() const noexcept;\
	virtual const char* type_name() noexcept;\
protected:\
	static OCTOON_PROJECT RttiObject* FactoryCreate(); \
private:

#define OctoonImplementClass(Base, Name) \
    OCTOON_PROJECT Rtti Base::RTTI = OCTOON_PROJECT Rtti(Name, Base::FactoryCreate, nullptr);\
	OCTOON_PROJECT Rtti* Base::rtti() const noexcept { return &RTTI; }\
	OCTOON_PROJECT Rtti* Base::getRtti() noexcept { return &RTTI; }\
	OCTOON_PROJECT RttiObject* Base::FactoryCreate() { return new Base; }\
	const char* Base::type_name() noexcept { return Name; };

#define OctoonDeclareSubClass(Derived, Base) \
public:\
    static OCTOON_PROJECT Rtti RTTI;\
	static OCTOON_PROJECT Rtti* getRtti() noexcept;\
    virtual OCTOON_PROJECT Rtti* rtti() const noexcept override;\
	virtual const char* type_name() noexcept override;\
protected:\
	static OCTOON_PROJECT RttiObject* FactoryCreate(); \
private:

#define OctoonImplementSubClass(Derived, Base, Name) \
    OCTOON_PROJECT Rtti Derived::RTTI = OCTOON_PROJECT Rtti(Name, Derived::FactoryCreate, Base::getRtti());\
	OCTOON_PROJECT Rtti* Derived::rtti() const noexcept { return &RTTI; }\
	OCTOON_PROJECT RttiObject* Derived::FactoryCreate() { return new Derived; }\
	OCTOON_PROJECT Rtti* Derived::getRtti() noexcept { return &RTTI; }\
	const char* Derived::type_name() noexcept { return Name; };
}

#endif