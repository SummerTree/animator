#ifndef UNREAL_CONTEXT_H_
#define UNREAL_CONTEXT_H_

namespace unreal
{
	class UnrealContext
	{
	public:
		class UnrealProfile* profile = nullptr;
		class UnrealBehaviour* behaviour = nullptr;
	};
}

#endif