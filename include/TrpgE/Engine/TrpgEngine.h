#pragma once

#include <TrpgE/Core/TCommon.h>

namespace trpg
{
	class TrpgEngine
	{
	public:
		TrpgEngine() {}
		~TrpgEngine() {}
		virtual void release() = 0;

	};

	TRPG_API TrpgEngine* createTrpgEngine();
}