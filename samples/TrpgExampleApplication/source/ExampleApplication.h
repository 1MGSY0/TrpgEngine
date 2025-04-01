#pragma once

#include "TrpgE/Application/TApplication.h"

class ExampleApplication: public trpg::TApplication
{
	public:
		ExampleApplication();
		virtual ~ExampleApplication();

		virtual void onInit() override;
		virtual void onExit() override;
		virtual void onUpdate() override;


};


