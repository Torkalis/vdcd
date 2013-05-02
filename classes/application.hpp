//
//  application.hpp
//  p44bridged
//
//  Created by Lukas Zeller on 02.05.13.
//  Copyright (c) 2013 plan44.ch. All rights reserved.
//

#ifndef __p44bridged__application__
#define __p44bridged__application__

#include "p44bridged_common.hpp"

class MainLoop;

class Application
{
	MainLoop *mainLoopP;
public:
	/// constructor
	Application(MainLoop *aMainLoopP);
	/// default constructor
	Application();
	/// destructor
	~Application();
	/// main routine
	virtual int main(int argc, char **argv);
protected:
	/// daemonize
	void daemonize();
	/// start running the app's main loop
	int run();
	/// scheduled to run when mainloop has started
	virtual void initialize();
};


#endif /* defined(__p44bridged__application__) */
