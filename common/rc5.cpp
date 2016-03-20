
#include "rc5.h"


bool RC5_Encrypt_Stream::RC5_SetPWD(const char* pwd)
{
	unsigned char pwd_t[8] = { 0 };

	size_t pwd_len = 0;
	if (pwd && ( pwd_len = strlen(pwd) ) > 0 )
	{
		if (pwd_len > 8) pwd_len = 8;
		for (size_t i=0; i < pwd_len; i++)
		{
			pwd_t[i] = (unsigned char)( pwd[i] * ( i + 1 ) );
		}
	}

	m_bInited = (CRYPT_OK == rc5_setup(pwd_t, 8, 12, &m_key));
	return m_bInited;
}

bool  RC5_Encrypt_Stream::RC5_Encrypt(void* pData, size_t len)
{
	if (!pData || !m_bInited) return false;

	unsigned char* block;

	// 以8byte分组
	for (size_t i=8; i <= len; i += 8)
	{
		block = (unsigned char*)pData;
		block += (  i - 8 );

		if (rc5_ecb_encrypt(block, block, &m_key) != CRYPT_OK)
		{
			return false;
		}
	}
	return true;
}

bool RC5_Encrypt_Stream:: RC5_Decrypt(void* pData, size_t len)
{
	if (!pData || !m_bInited) return false;

	unsigned char* block;

	// 以8byte分组
	for (size_t i=8; i <= len; i += 8)
	{
		block = (unsigned char*)pData;
		block += (  i - 8 );

		if (rc5_ecb_decrypt(block, block, &m_key) != CRYPT_OK)
		{
			return false;
		}
	}
	return true;
}

int  RC5_Encrypt_Stream::rc5_setup(const unsigned char *key, int keylen, int num_rounds, Rc5_key *skey)
{
	ulong32 L[64], *S, A, B, i, j, v, s, t, l;

	LTC_ARGCHK(skey != NULL);
	LTC_ARGCHK(key  != NULL);

	/* test parameters */
	if (num_rounds == 0) { 
	   num_rounds = RC5_DEFAULT_ROUNDS;
	}

	if (num_rounds < 12 || num_rounds > 24) { 
	   return CRYPT_INVALID_ROUNDS;
	}

	/* key must be between 64 and 1024 bits */
	if (keylen < 8 || keylen > 128) {
	   return CRYPT_INVALID_KEYSIZE;
	}
    
	skey->rounds = num_rounds;
	S = skey->K;

	/* copy the key into the L array */
	for (A = i = j = 0; i < (ulong32)keylen; ) { 
		A = (A << 8) | ((ulong32)(key[i++] & 255));
		if ((i & 3) == 0) {
		   L[j++] = BSWAP(A);
		   A = 0;
		}
	}

	if ((keylen & 3) != 0) { 
	   A <<= (ulong32)((8 * (4 - (keylen&3)))); 
	   L[j++] = BSWAP(A);
	}

	/* setup the S array */
	t = (ulong32)(2 * (num_rounds + 1));
	XMEMCPY(S, stab, t * sizeof(*S));

	/* mix buffer */
	s = 3 * MAX(t, j);
	l = j;
	for (A = B = i = j = v = 0; v < s; v++) { 
		A = S[i] = ROLc(S[i] + A + B, 3);
		B = L[j] = ROL(L[j] + A + B, (A+B));
		if (++i == t) { i = 0; }
		if (++j == l) { j = 0; }
	}
	return CRYPT_OK;
}

int  RC5_Encrypt_Stream::rc5_ecb_encrypt(const unsigned char *pt, unsigned char *ct, Rc5_key *skey)
{
   ulong32 A, B, *K;
   int r;
   LTC_ARGCHK(skey != NULL);
   LTC_ARGCHK(pt   != NULL);
   LTC_ARGCHK(ct   != NULL);

   LOAD32L(A, &pt[0]);
   LOAD32L(B, &pt[4]);
   A += skey->K[0];
   B += skey->K[1];
   K  = skey->K + 2;
   
   if ((skey->rounds & 1) == 0) {
	  for (r = 0; r < skey->rounds; r += 2) {
		  A = ROL(A ^ B, B) + K[0];
		  B = ROL(B ^ A, A) + K[1];
		  A = ROL(A ^ B, B) + K[2];
		  B = ROL(B ^ A, A) + K[3];
		  K += 4;
	  }
   } else {
	  for (r = 0; r < skey->rounds; r++) {
		  A = ROL(A ^ B, B) + K[0];
		  B = ROL(B ^ A, A) + K[1];
		  K += 2;
	  }
   }

   STORE32L(A, &ct[0]);
   STORE32L(B, &ct[4]);

   return CRYPT_OK;
}

int  RC5_Encrypt_Stream::rc5_ecb_decrypt(const unsigned char *ct, unsigned char *pt, Rc5_key *skey)
{
	ulong32 A, B, *K;
	int r;
	LTC_ARGCHK(skey != NULL);
	LTC_ARGCHK(pt   != NULL);
	LTC_ARGCHK(ct   != NULL);

	LOAD32L(A, &ct[0]);
	LOAD32L(B, &ct[4]);
	K = skey->K + (skey->rounds << 1);

	if ((skey->rounds & 1) == 0) {
	   K -= 2;
	   for (r = skey->rounds - 1; r >= 0; r -= 2) {
		  B = ROR(B - K[3], A) ^ A;
		  A = ROR(A - K[2], B) ^ B;
		  B = ROR(B - K[1], A) ^ A;
		  A = ROR(A - K[0], B) ^ B;
		  K -= 4;
		}
	} else {
	  for (r = skey->rounds - 1; r >= 0; r--) {
		  B = ROR(B - K[1], A) ^ A;
		  A = ROR(A - K[0], B) ^ B;
		  K -= 2;
	  }
	}
	A -= skey->K[0];
	B -= skey->K[1];
	STORE32L(A, &pt[0]);
	STORE32L(B, &pt[4]);

	return CRYPT_OK;
}

