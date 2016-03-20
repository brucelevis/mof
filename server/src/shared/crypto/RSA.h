//
//  RSA.h
//  GameSrv
//
//  Created by jin on 13-12-10.
//
//

#ifndef __shared__RSA__
#define __shared__RSA__

#include <iostream>
#include <string>
#include <vector>
using namespace std;


// 此对base64加解编码，处理每65个字符为换行符\n
void base64(const unsigned char *input, int inlen, vector<unsigned char> &out);
void unbase64(const char *input, int inlen, vector<unsigned char> &out, int &outlen);

void rsa_public_decrypt(const unsigned char *in, int len, const char *pub_key, vector<unsigned char> &out);

// rsa pkcs1 base64位编码加解密算法
string rsa_private_base64_encrypt(const string &in, const char *pri_key);
string rsa_public_base64_decrypt(const string &in, const char *pub_key);

#endif /* defined(__GameSrv__RSA__) */
