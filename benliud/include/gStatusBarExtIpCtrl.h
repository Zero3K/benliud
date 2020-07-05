// gStatusBarExtIpCtrl.h: interface for the gStatusBarExtIpCtrl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _GSTATUSBAREXTIPCTRL_H
#define _GSTATUSBAREXTIPCTRL_H

#include "gStatusBarIconTextCtrl.h"

class gStatusBarExtIpCtrl : public gStatusBarIconTextCtrl 
{
public:
	void OnGetReady(int val);
	gStatusBarExtIpCtrl(wxWindow* parent);
	virtual ~gStatusBarExtIpCtrl();

};

#endif // !defined(AFX_GSTATUSBAREXTIPCTRL_H__3D28D753_CE9F_4297_8BBA_A1D31C7C89D7__INCLUDED_)
