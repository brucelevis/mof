//
//  HmacMd5.cpp
//  GameSrv
//
//  Created by nothing on 14-1-8.
//
//

#include "HmacMd5.h"

#include <openssl/hmac.h>
#include <string>
#include <cstring>

using namespace std;

string HmacMd5(const char *data, const char *key)
{
    const EVP_MD *engine = EVP_md5();
    unsigned char output[EVP_MAX_MD_SIZE];
    unsigned int outputlength = 0;
    
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key, strlen(key), engine, NULL);
    HMAC_Update(&ctx, (unsigned char*)data, strlen(data));
    HMAC_Final(&ctx, output, &outputlength);
    HMAC_CTX_cleanup(&ctx);
    char ch[10];
    
    string str;
    for (int i = 0; i < outputlength; ++i)
    {
        sprintf(ch, "%02x", output[i]);
        str.append(ch);
    }

    return str;
}


string Sha256(const char *data, const char *key)
{
    const EVP_MD *engine = EVP_sha256();
    unsigned char output[EVP_MAX_MD_SIZE];
    unsigned int outputlength = 0;
    
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, key, strlen(key), engine, NULL);
    HMAC_Update(&ctx, (unsigned char*)data, strlen(data));
    HMAC_Final(&ctx, output, &outputlength);
    HMAC_CTX_cleanup(&ctx);
    char ch[10];
    
    string str;
    for (int i = 0; i < outputlength; ++i)
    {
        sprintf(ch, "%02x", output[i]);
        str.append(ch);
    }
    
    return str;
}





