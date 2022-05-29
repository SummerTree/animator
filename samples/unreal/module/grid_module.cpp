#include "grid_module.h"

namespace unreal
{
	GridModule::GridModule() noexcept
	{
		this->reset();
	}

	GridModule::~GridModule() noexcept
	{
	}

	void
	GridModule::reset() noexcept
	{
	}

	void 
	GridModule::load(octoon::runtime::json& reader) noexcept
	{
		UnrealModule::load(reader);
	}

	void 
	GridModule::save(octoon::runtime::json& writer) noexcept
	{
		UnrealModule::save(writer);
	}
}