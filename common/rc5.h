

#pragma once

#include <stdio.h>
#include <string.h>
#include <stdint.h>

enum RC5_CODE{
   CRYPT_OK=0,             /* Result OK */
   CRYPT_ERROR,            /* Generic Error */
   CRYPT_NOP,              /* Not a failure but no operation was performed */

   CRYPT_INVALID_KEYSIZE,  /* Invalid key size given */
   CRYPT_INVALID_ROUNDS,   /* Invalid number of rounds */
};

static int crypt_argchk(char *v, char *s, int d)
{
 fprintf(stderr, "LTC_ARGCHK '%s' failure on line %d of file %s\n",
         v, d, s);

 return CRYPT_NOP;
}
#define LTC_ARGCHK(x) if (!(x)) { return crypt_argchk(#x, __FILE__, __LINE__); }

#define RC5_DEFAULT_ROUNDS 12

/*
#if defined(__x86_64__) || (defined(__sparc__) && defined(__arch64__))
   typedef unsigned ulong32;
#else
   typedef unsigned long ulong32;
#endif
*/

typedef uint32_t ulong32;

// 旋转移动位置
#ifdef _MSC_VER
    #pragma intrinsic           (_lrotr, _lrotl)
#else /* GCC or CC */
    #define _lrotr(x, n)        ((((ulong32)(x)) >> ((int) ((n) & 31))) | (((ulong32)(x)) << ((int) ((-(n)) & 31))))
    #define _lrotl(x, n)        ((((ulong32)(x)) << ((int) ((n) & 31))) | (((ulong32)(x)) >> ((int) ((-(n)) & 31))))
#endif

/* instrinsic rotate */
#define ROR(x,n) _lrotr(x,n)
#define ROL(x,n) _lrotl(x,n)
#define RORc(x,n) _lrotr(x,n)
#define ROLc(x,n) _lrotl(x,n)

#ifndef MAX
   #define MAX(x, y) ( ((x)>(y))?(x):(y) )
#endif

#ifndef MIN
   #define MIN(x, y) ( ((x)<(y))?(x):(y) )
#endif

static const ulong32 stab[50] = {
0xb7e15163UL, 0x5618cb1cUL, 0xf45044d5UL, 0x9287be8eUL, 0x30bf3847UL, 0xcef6b200UL, 0x6d2e2bb9UL, 0x0b65a572UL,
0xa99d1f2bUL, 0x47d498e4UL, 0xe60c129dUL, 0x84438c56UL, 0x227b060fUL, 0xc0b27fc8UL, 0x5ee9f981UL, 0xfd21733aUL,
0x9b58ecf3UL, 0x399066acUL, 0xd7c7e065UL, 0x75ff5a1eUL, 0x1436d3d7UL, 0xb26e4d90UL, 0x50a5c749UL, 0xeedd4102UL,
0x8d14babbUL, 0x2b4c3474UL, 0xc983ae2dUL, 0x67bb27e6UL, 0x05f2a19fUL, 0xa42a1b58UL, 0x42619511UL, 0xe0990ecaUL,
0x7ed08883UL, 0x1d08023cUL, 0xbb3f7bf5UL, 0x5976f5aeUL, 0xf7ae6f67UL, 0x95e5e920UL, 0x341d62d9UL, 0xd254dc92UL,
0x708c564bUL, 0x0ec3d004UL, 0xacfb49bdUL, 0x4b32c376UL, 0xe96a3d2fUL, 0x87a1b6e8UL, 0x25d930a1UL, 0xc410aa5aUL,
0x62482413UL, 0x007f9dccUL
};

#define STORE32L(x, y)        \
     { ulong32  __t = (x); XMEMCPY(y, &__t, 4); }

#define LOAD32L(x, y)         \
     XMEMCPY(&(x), y, 4);

#define BSWAP(x)  ( ((x>>24)&0x000000FFUL) | ((x<<24)&0xFF000000UL)  | \
                    ((x>>8)&0x0000FF00UL)  | ((x<<8)&0x00FF0000UL) )

#define XMEMCPY  memcpy

struct Rc5_key
{
   int rounds;
   ulong32 K[50];
};

class RC5_Encrypt_Stream
{
public:
	RC5_Encrypt_Stream(){}
	RC5_Encrypt_Stream(const char* pwd){ RC5_SetPWD(pwd); }
	~RC5_Encrypt_Stream(){}

public:
	bool RC5_SetPWD(const char* pwd);
	bool RC5_Encrypt(void* pData, size_t len);
	bool RC5_Decrypt(void* pData, size_t len);

private:
	int rc5_setup(const unsigned char *key, int keylen, int num_rounds, Rc5_key *skey);
	int rc5_ecb_encrypt(const unsigned char *pt, unsigned char *ct, Rc5_key *skey);
	int rc5_ecb_decrypt(const unsigned char *ct, unsigned char *pt, Rc5_key *skey);

private:
	bool m_bInited;
	Rc5_key m_key;
};

