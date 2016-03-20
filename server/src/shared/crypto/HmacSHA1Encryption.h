//
//  HmacSHA1Encryption.h
//  GameSrv
//
//  Created by jin on 13-10-10.
//
//

#ifndef __shared__HmacSHA1Encryption__
#define __shared__HmacSHA1Encryption__

#include <iostream>
#include <stdio.h>
#include <string>
#include "HMAC_SHA1.h"
using namespace std;

class HmacSHA1Encryption
{
public:
	static string hmacSHA1Encrypt(string encryptText, string encryptKey);
	static string sha1(const string &encryptText);
};

#endif /* defined(__GameSrv__HmacSHA1Encryption__) */
