//
//  CryptUtils.h
//  testEncypt
//
//  Created by Forest Lin on 13-7-8.
//
//  文件加密和解密的工具
//#if defined(Crypt)
#ifndef testEncypt_CryptUtils_h
#define testEncypt_CryptUtils_h

#include <string>
#include <vector>

class CryptUtils
{
    static std::string sKey;
    static int kCCBlockSizeAES128;
public:
    static std::vector<unsigned char> decryptFile(std::string file);
    static std::vector<unsigned char> decryptBuf(unsigned char* cryptBuffer, unsigned long cryptBufferSize);
    static int getBuffSize(unsigned char *buffer);
};

#endif
//#endif