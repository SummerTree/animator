#include "unreal_model.h"

namespace unreal
{
	UnrealModule::UnrealModule() noexcept
		: enable_(true)
	{
	}

	UnrealModule::~UnrealModule() noexcept
	{
	}

	void
	UnrealModule::setEnable(bool enable) noexcept
	{
		enable_ = enable;
	}

	bool
	UnrealModule::getEnable() const noexcept
	{
		return enable_;
	}

	void
	UnrealModule::onValidate() noexcept
	{
	}
}