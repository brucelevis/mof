//
//  HmacSHA1Encryption.cpp
//  GameSrv
//
//  Created by jin on 13-10-10.
//
//

#include "HmacSHA1Encryption.h"

string HmacSHA1Encryption::hmacSHA1Encrypt(string encryptText, string encryptKey)
{
	CHMAC_SHA1 sha;
	unsigned char out[20];
	sha.HMAC_SHA1((unsigned char*)encryptText.c_str(),
				  encryptText.length(),
				  (unsigned char*)encryptKey.c_str(),
				  encryptKey.length(),
				  (unsigned char*)out);
	
	char out_str[41] = {0};
	out_str[40] = 0;
	
	for(int i = 0; i < 20; i++ )
	{
		sprintf(out_str + 2 * i, "%02x", out[i]);
	}
	return out_str;
}

string HmacSHA1Encryption::sha1(const string &encryptText)
{
	CSHA1 sha1;
	sha1.Update((unsigned char*)encryptText.c_str(), encryptText.length());
	sha1.Final();
	
	unsigned char out[20];
	sha1.GetHash(out);
	
	char out_str[41] = {0};
	out_str[40] = 0;
	
	for(int i = 0; i < 20; i++ )
	{
		sprintf(out_str + 2 * i, "%02x", out[i]);
	}
	return out_str;
}