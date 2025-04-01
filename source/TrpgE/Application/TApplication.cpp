#include "TrpgE/Application/TApplication.h"
#include <stdexcept>
using namespace trpg;

/**
* @breif Initialize the application
* */
TApplication* TApplication::m_instance = nullptr;

trpg::TApplication::TApplication()
{
	TASSERT(!m_instance);
	m_instance = this;
	m_engine = createTrpgEngine();
}

trpg::TApplication::~TApplication()
{
	m_engine->release();
}

void trpg::TApplication::run()
{
	onInit();

	while (m_isRunning)
	{
		onUpdate();
	}

	onExit();

}

void trpg::TApplication::exit()
{
		m_isRunning = false;
}

TApplication* trpg::TApplication::get()
{
	TASSERT(m_instance);
	return m_instance;
	m_instance = nullptr;
}
