#pragma once

#include <TrpgE/Engine/TrpgEngine.h>



namespace trpg
{
	class CEngine : public trpg::TrpgEngine
	{
	private:
		CEngine();
		~CEngine();
	public:
		virtual void release() override;

	public:
		static CEngine* create();

	private:
		static void destroy();

	private:
		static CEngine* m_instance;
	};
}