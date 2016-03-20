//
//  RSA.cpp
//  GameSrv
//
//  Created by jin on 13-12-10.
//
//

#include "RSA.h"
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include "base64.h"

using namespace std;

// base64位编码(带换行符)
void base64(const unsigned char *input, int inlen, vector<unsigned char> &out)
{
	EVP_ENCODE_CTX	ctx;
	int base64Len = (((inlen + 2) / 3) * 4) + 1; // Base64 text length
	int pemLen = base64Len + base64Len / 64; // PEM adds a newline every 64 bytes
	out.resize(pemLen);
	int result;
	EVP_EncodeInit(&ctx);
	EVP_EncodeUpdate(&ctx, out.data(), &result, (const unsigned char *)input, inlen);
	EVP_EncodeFinal(&ctx, &out[result], &result);
}

// base64位解码(带换行符)
void unbase64(const char *input, int inlen, vector<unsigned char> &out, int &outlen)
{
	EVP_ENCODE_CTX ctx;
	int orgLen = (((inlen + 2) / 4) * 3) + 1;
	out.resize(orgLen);
	int ret = 0;
	int tmpLen = 0;
	EVP_DecodeInit(&ctx);
	EVP_DecodeUpdate(&ctx, out.data(), &ret, (const unsigned char *)input, inlen);
	EVP_DecodeFinal(&ctx, &out[ret], &tmpLen);
	outlen = ret + tmpLen;
}

string rsa_set_public_key(const char *pub_key_str)
{
	string str = pub_key_str;
	string tmp;
	tmp.append("-----BEGIN PUBLIC KEY-----\n");
	int n = 1;
	for (string::iterator it = str.begin(); it != str.end(); it++)
	{
		tmp.append(it, it + 1);
		if (n++ % 64 == 0)
		{
			tmp.append("\n");
		}
	}
	tmp.append("\n");
	tmp.append("-----END PUBLIC KEY-----");
	return tmp;
}

string rsa_set_private_key(const char *pri_key_str)
{
	string str = pri_key_str;
	string tmp;
	tmp.append("-----BEGIN RSA PRIVATE KEY-----\n");
	int n = 1;
	for (string::iterator it = str.begin(); it != str.end(); it++)
	{
		tmp.append(it, it + 1);
		if (n++ % 64 == 0)
		{
			tmp.append("\n");
		}
	}
	tmp.append("\n");
	tmp.append("-----END RSA PRIVATE KEY-----");
	return tmp;
}

string toHex(unsigned char* bin, int len)
{
	static char hexChar[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'A', 'B', 'C', 'D', 'E', 'F'};
	
	string ret;
	for (int i = 0; i < len; i++)
	{
		
		ret.push_back(hexChar[bin[i] >> 4]);
		ret.push_back(hexChar[bin[i] & 4]);
	}
	
	return ret;
}
						 
int rsa_private_pkcs1_encrypt(const char *in, int len, RSA *rsa, vector<unsigned char> &out)
{
	int rsa_len = RSA_size(rsa);
	// 比较源文长度 (用了RSA_PKCS1_PADDING, rsa长度需减11)
	if (len > rsa_len - 11) {
		return -1;
	}
	out.resize(rsa_len);
	return RSA_private_encrypt(len, (unsigned char*)in, out.data(), rsa, RSA_PKCS1_PADDING);
}

void rsa_private_encrypt(const char *in, int len, const char *pri_key, vector<unsigned char> &out)
{
	FILE *file = tmpfile();
	fputs(rsa_set_private_key(pri_key).c_str(), file);
	rewind(file);
	
	RSA *rsa = PEM_read_RSAPrivateKey(file, NULL, NULL, NULL);
	fclose(file);
	if (NULL == rsa)
	{
		return;
	}
	int result = rsa_private_pkcs1_encrypt(in, len, rsa, out);
	
	RSA_free(rsa);
	CRYPTO_cleanup_all_ex_data();
}


int rsa_public_pkcs1_decrypt(const unsigned char *in, int len, RSA *rsa, vector<unsigned char> &out)
{
	int rsa_len = RSA_size(rsa);
	out.resize(rsa_len);
	return RSA_public_decrypt(rsa_len, in, out.data(), rsa, RSA_PKCS1_PADDING);
}

void rsa_public_decrypt(const unsigned char *in, int len, const char *pub_key, vector<unsigned char> &out)
{
	FILE *file = tmpfile();
	fputs(rsa_set_public_key(pub_key).c_str(), file);
	rewind(file);
	
	RSA *rsa = PEM_read_RSA_PUBKEY(file, NULL, NULL, NULL);
	fclose(file);
	if (NULL == rsa)
	{
		return;
	}
	int result = rsa_public_pkcs1_decrypt(in, len, rsa, out);
	
	RSA_free(rsa);
	CRYPTO_cleanup_all_ex_data();
}

// rsa 加密函数(源串, 私钥)
string rsa_private_base64_encrypt(const string &in, const char *pri_key)
{
	vector<unsigned char> ret;
	rsa_private_encrypt(in.c_str(), in.length(), pri_key, ret);
	
	return base64_encode(ret.data(), ret.size());
}


// rsa 解密函数(base64位密串, 公钥)
string rsa_public_base64_decrypt(const string &in, const char *pub_key)
{
	if (in.empty())
	{
		return "";
	} 
	vector<unsigned char> de64;
	base64_decode(in, de64);
	
	vector<unsigned char> out;
	rsa_public_decrypt(de64.data(), de64.size(), pub_key, out);
	out.push_back('\0');
	return (char *)out.data();
}
