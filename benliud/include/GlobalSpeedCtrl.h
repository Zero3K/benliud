// GlobalSpeedCtrl.h: interface for the CGlobalSpeedCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GLOBALSPEEDCTRL_H
#define _GLOBALSPEEDCTRL_H


#include <wx/wx.h>
#include <wx/thread.h>
#include <list>
#include "../include/datatype_def.h"

class SpeedCtrl  
{
public:
	unsigned int GetTicks();
	void SetSpeedLimit( int kbyte=0);
	CGlobalSpeedCtrl();
	virtual ~CGlobalSpeedCtrl();
	int DataIn( int len );

protected:

    struct _ss
    {
        llong bytes;  //data bytes
        unsigned int   ticks;   //time
    };

	wxMutex m_Mutex;
	int		m_Limit; //the speed limit in KB/s, where 0 is unlimit

	std::list<_ss> m_Speed;
};

#endif 