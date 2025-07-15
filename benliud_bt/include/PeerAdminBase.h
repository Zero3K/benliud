/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


// PeerAdminBase.h: interface for the CPeerAdminBase class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _PEERADMINBASE_H
#define _PEERADMINBASE_H


class CSpeedControlBase;
class CBTSession;
class CBTPeer;

#include "MSE_BigInt.h"
#include <string>
#include <assert.h>
#include <Dealer.h>
class CPeerAdminBase 
{
public:
	CPeerAdminBase() {};
	virtual ~CPeerAdminBase() {};

	virtual SockLib::CDealer* GetDealer()=0;

	virtual CSpeedControlBase* GetSpeedControl()=0;
	virtual CBTSession*	GetSession()=0;

	virtual bool GotHash(std::string hash, CBTPeer* client)=0;
	virtual bool GotEncryptHash(std::string hashxor, MSE::BigInt dhsecrect, CBTPeer* client)=0;
};

#endif // !defined(AFX_PEERADMINBASE_H__B5A800D8_56E6_4352_92F1_C1230D84FADA__INCLUDED_)
