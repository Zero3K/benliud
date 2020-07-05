
/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/

// TrackerBase.h: interface for the CTrackerBase class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _TRACKERBASE_H
#define _TRACKERBASE_H

#include "TTrackerDefines.h"
class CTrackerBase
{

public:
    CTrackerBase();
    virtual ~CTrackerBase();
    virtual bool Start() = 0;
	virtual void Stop() = 0;
	virtual void SetEvent(TTrackerEvent event) =0;
};

#endif

