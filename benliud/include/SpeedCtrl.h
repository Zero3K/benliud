// GlobalSpeedCtrl.h: interface for the CGlobalSpeedCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GLOBALSPEEDCTRL_H
#define _GLOBALSPEEDCTRL_H



#include <wx/thread.h>
#include <list>
#include "../include/datatype_def.h"

class CSpeedCtrl  
{
public:
	unsigned int GetTicks();
	void SetSpeedLimit( int kbyte=0);
	CSpeedCtrl();
	virtual ~CSpeedCtrl();
	int DataIn( int len );

private:

    struct _sl
    {
        llong bytes;  //data bytes
        unsigned int   ticks;   //time
    };

	wxMutex m_Mutex;
	int		m_Limit; //the speed limit in KB/s, where 0 is unlimit

	std::list<_sl> m_Speed;
};

#endif
