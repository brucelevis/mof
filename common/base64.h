//
//  base64.h
//  GameSrv
//
//  Created by prcv on 13-10-22.
//
//

#ifndef __GameSrv__base64__
#define __GameSrv__base64__

#include <string>
#include <vector>

std::string base64_encode(const unsigned char* , unsigned int len);
std::string base64_decode(const std::string& s);
void base64_decode(const std::string& encoded_string , std::vector<unsigned char>& out);

#endif /* defined(__GameSrv__base64__) */
