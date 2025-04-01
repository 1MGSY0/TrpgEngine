#include "CEngine.h"
#include <TrpgE/Core/TCommon.h>
using namespace trpg;

CEngine* CEngine::m_instance = nullptr;

TrpgEngine* trpg::createTrpgEngine()
{
	return CEngine::create();
}

trpg::CEngine::CEngine()
{
}

trpg::CEngine::~CEngine()
{
}

void trpg::CEngine::release()
{
	CEngine::destroy();
}

CEngine* trpg::CEngine::create()
{
	TASSERT(!m_instance);
	m_instance = new CEngine();
	return nullptr;
}

void trpg::CEngine::destroy()
{
	TASSERT(m_instance);
	delete m_instance;
}
