/***************************************************************

CopyRight(C) liubin(liubinbj@gmail.com) 

This code is published under GPL v2

GPL v2Э鷢.

****************************************************************/

#include "stdafx.h"

#include <SHA1.h>
#include "../include/MSE_BigInt.h"
#include "../include/MSE_BTDHTKey.h"


namespace MSE
{
	/*
	static const BigInt P = BigInt(
			"0xFFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD"
			"129024E088A67CC74020BBEA63B139B22514A08798E3404"
			"DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C"
			"245E485B576625E7EC6F44C42E9A63A36210000000000090563");
	*/
	static const BigInt P = BigInt("FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A63A36210000000000090563",16);
	static const BigInt G = BigInt("02",16);

	void GeneratePublicPrivateKey(BigInt & priv,BigInt & pub)
	{
		priv = BigInt::random();			//private key
		pub = BigInt::powerMod(G,priv,P);	//public key
	}

	BigInt DHSecret(const BigInt & our_priv,const BigInt & peer_pub)
	{
		return BigInt::powerMod(peer_pub,our_priv,P);
	}
	
	//SKEY = Stream Identifier/Shared secret used to drop connections early if we
	//don't have a matching stream. It's additionally used to harden the protocol
	//against MITM attacks and portscanning
	//ENCRYPT() is RC4, that uses one of the following keys to send data:
	//"HASH('keyA', S, SKEY)" if you're A
	//"HASH('keyB', S, SKEY)" if you're B
	//DH secret: S = (Ya^Xb) mod P = (Yb^Xa) mod P
	//S=DH secret
	//Note: For BitTorrent, the SKEY should be the torrent info hash.
	BTDHTKey EncryptionKey(bool a,const BigInt & s, BTDHTKey & skey)
	{
		unsigned char buf[120];

		memcpy(buf,"key",3);
		buf[3] = (unsigned char)(a ? 'A' : 'B');
		s.toBuffer(buf + 4,96);

		unsigned char* skeydata=skey.GetData();

		memcpy(buf + 100,skeydata,20);

		//unsigned char out[20];

		//SHA1Block(buf,120,out);

		HashLib::CSHA1 tmp;
		tmp.Hash((const char*)buf,120);

		//BTDHTKey retkey((char*)out);
		BTDHTKey retkey((char*)tmp.GetHash());

		return retkey;

	}

}
