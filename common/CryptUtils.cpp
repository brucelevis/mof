//
//  CryptUtils.cpp
//  testEncypt
//
//  Created by Forest Lin on 13-7-8.
//
//

#include "CryptUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "aes.h"
#include "log.h"
//#include "ClientLog.h"

using namespace std;
//string CryptUtils::sKey = "abcdefghijklmnopqrstuvwxyz123456";
string CryptUtils::sKey = "anni_are_you_ok_are_you_ok_anni?";
int CryptUtils::kCCBlockSizeAES128 = 16;

//vector<unsigned char> CryptUtils::decryptFile(std::string inFile)
//{
//    // Open input and output files
//    FILE *fin = fopen(inFile.c_str(), "rb");
//
//    if(!fin)
//    {
//        printf("%s: Can't open input file\n", inFile.c_str());
//    }
//
//    //解密
//    CCOperation cryptoOp = kCCDecrypt;
//    // Set up cryptor
//    CCCryptorRef cryptoRef;
//    CCCryptorCreate(cryptoOp, kCCAlgorithmAES128, kCCOptionPKCS7Padding, sKey.c_str(), kCCKeySizeAES256, NULL, &cryptoRef);
//    unsigned char *inBuffer = (unsigned char *) malloc(kCCBlockSizeAES128);
//    unsigned char *outBuffer = (unsigned char *) malloc(kCCBlockSizeAES128);
//    size_t numBytes;
//    vector<unsigned char> buff;
//    // Encrypt/decrypt
//    while (!feof(fin))
//    {
//        numBytes = fread(inBuffer, sizeof(char), kCCBlockSizeAES128, fin);
//
//        if (kCCSuccess != CCCryptorUpdate(cryptoRef, inBuffer, numBytes, outBuffer, kCCBlockSizeAES128, &numBytes))
//        {
//            printf("%s: Crypto error processing file\n", "");
//        }
//
//        for (int i = 0 ; i<numBytes; ++i)
//        {
//            buff.push_back(outBuffer[i]);
//        }
//
//    }
//
//    // Flush cryptor
//    CCCryptorFinal(cryptoRef, outBuffer, kCCBlockSizeAES128, &numBytes);
//    
//    for (int i = 0 ; i<numBytes; ++i)
//    {
//        buff.push_back(outBuffer[i]);
//    }
//
//    // Clean up
//    fclose(fin);
//    free(inBuffer);
//    free(outBuffer);
//    CCCryptorRelease(cryptoRef);
//    
//    return buff;
//}

vector<unsigned char> CryptUtils::decryptFile(std::string inFile)
{
    FILE *fin = fopen(inFile.c_str(), "r");
    if(!fin)
    {
        printf("%s: Can't open input file\n", inFile.c_str());
    }
    
    aes_context aes;
    aes_setkey_dec(&aes, (const unsigned char *)(sKey.c_str()), 256);
    
    unsigned char *inBuffer = (unsigned char *) malloc(kCCBlockSizeAES128);
    unsigned char *outBuffer = (unsigned char *) malloc(kCCBlockSizeAES128);
    
    vector<unsigned char> buff;
    // Encrypt/decrypt
    while (!feof(fin))
    {
        int readBytes = fread(inBuffer, sizeof(char), kCCBlockSizeAES128, fin);
        if(readBytes == 0)
            break;
        if(readBytes < kCCBlockSizeAES128)
        {
            for (size_t i=readBytes; i<kCCBlockSizeAES128; ++i)
            {
                inBuffer[i] = '\0';
            }
        }
        
        if (aes_crypt_ecb(&aes, AES_DECRYPT,inBuffer, outBuffer) != 0)
        {
            printf("%s: Crypto error processing file\n", "");
        }
        int outBufSize =getBuffSize(outBuffer);
        if(outBufSize > kCCBlockSizeAES128)
            outBufSize = kCCBlockSizeAES128;
        
        for (int i = 0 ; i< outBufSize; ++i)
        {
            buff.push_back(outBuffer[i]);
        }
    }
    
    fclose(fin);
    free(inBuffer);
    free(outBuffer);
    //CCLOG("decrypt File:%s---%s", inFile.c_str(), (char*)&buff[0]);
    return buff;
}

vector<unsigned char> CryptUtils::decryptBuf(unsigned char* cryptBuffer, unsigned long cryptBufferSize)
{
    aes_context aes;
    aes_setkey_dec(&aes, (const unsigned char *)(sKey.c_str()), 256);
    
    unsigned char *inBuffer = (unsigned char *) malloc(kCCBlockSizeAES128);
    unsigned char *outBuffer = (unsigned char *) malloc(kCCBlockSizeAES128);
    
    vector<unsigned char> buff;
    // Encrypt/decrypt
    int leftReadedBytes = cryptBufferSize;
    while (leftReadedBytes>0)
    {
        
        if(leftReadedBytes < kCCBlockSizeAES128)
        {
            memcpy(inBuffer, cryptBuffer + (cryptBufferSize - leftReadedBytes), leftReadedBytes);
            leftReadedBytes = 0;
        }
        else
        {
            memcpy(inBuffer, cryptBuffer + (cryptBufferSize - leftReadedBytes), kCCBlockSizeAES128);
            leftReadedBytes = leftReadedBytes - kCCBlockSizeAES128;
        }
        
        if (aes_crypt_ecb(&aes, AES_DECRYPT,inBuffer, outBuffer) != 0)
        {
            printf("%s: Crypto error processing file\n", "");
        }
        
        int outBufSize =getBuffSize(outBuffer);
        if(outBufSize > kCCBlockSizeAES128)
            outBufSize = kCCBlockSizeAES128;
        
        for (int i = 0 ; i< outBufSize; ++i)
        {
            buff.push_back(outBuffer[i]);
        }
    }

    free(inBuffer);
    free(outBuffer);
//    CCLOG("decrypt File:%s", (char*)&buff[0]);
    return buff;
}

int CryptUtils::getBuffSize(unsigned char *buffer)
{
    int size = 0;
    while (buffer[size] != 0)
    {
        size++;
    }
    return  size;
}

