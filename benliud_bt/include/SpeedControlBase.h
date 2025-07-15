/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// SpeedControlBase.h: interface for the CSpeedControlBase class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SPEEDCONTROLBASE_H
#define _SPEEDCONTROLBASE_H

class CBTPeer;
class CSpeedControlBase  
{
public:
	CSpeedControlBase();
	virtual ~CSpeedControlBase();
	virtual void RegisteClient(CBTPeer*)=0;
	virtual void UnregisteClient(CBTPeer*)=0;
};

#endif // !defined(AFX_SPEEDCONTROLBASE_H__9C004DF3_9D3A_4324_BDCC_43A7E2E1C792__INCLUDED_)
