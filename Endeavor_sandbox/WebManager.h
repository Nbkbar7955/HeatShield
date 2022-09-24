// WebManager.h

#ifndef _WebManager_h
#define _WebManager_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class WebManager
{
 protected:


 public:
	void init();
};

extern WebManager ;

#endif

