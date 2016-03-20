//
//  HmacMd5.h
//  GameSrv
//
//  Created by nothing on 14-1-8.
//
//

#ifndef __shared__HmacMd5__
#define __shared__HmacMd5__

#include <string>
using std::string;

string HmacMd5(const char *data, const char *key);
string Sha256(const char *data, const char *key);





#endif /* defined(__GameSrv__HmacMd5__) */
