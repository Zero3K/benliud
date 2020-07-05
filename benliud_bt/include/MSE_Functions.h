/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

本代码采用GPL v2协议发布.

****************************************************************/


#ifndef _MSE_FUNCTIONS_H
#define _MSE_FUNCTIONS_H

#include <string>

namespace MSE
{
	class BigInt;
	class SHA1Hash;
	class BTDHTKey;
	
	void GeneratePublicPrivateKey(BigInt & pub,BigInt & priv);
	BigInt DHSecret(const BigInt & our_priv,const BigInt & peer_pub);
	BTDHTKey EncryptionKey(bool a,const BigInt & s, BTDHTKey & skey);
	
	void DumpBigInt(const std::string & name,const BigInt & bi);
}

#endif
