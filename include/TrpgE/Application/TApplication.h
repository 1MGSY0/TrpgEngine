#pragma once
#include <TrpgE/Core/TCommon.h>
#include <TrpgE/Engine/TrpgEngine.h>

namespace trpg
{
	/**
	* @class TApplication
	*/
	class TRPG_API TApplication
	{
	public:
		/**
		* @brief add virtual as user will extend the function
		*/ 
		TApplication();
		virtual ~TApplication();

		virtual void onInit() {}
		virtual void onUpdate() {}
		virtual void onExit() {}

		void run();
		void exit();

	public:
		static TApplication* get();

	private:
		static TApplication* m_instance;

	private:
		bool m_isRunning = true;
		TrpgEngine* m_engine = nullptr;
	};
}