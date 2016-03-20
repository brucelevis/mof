//
//  curl_helper.h
//  GameSrv
//
//  Created by Huang Kunchao on 13-7-1.
//  Copyright (c) Xinyou Network Technology Corp., ltd. All rights reserved.
//

#ifndef GameSrv_curl_helper_h
#define GameSrv_curl_helper_h

#include <curl/curl.h>
#include "log.h"
#include <string>
#include <vector>
using std::string;


//curl写回调函数，印象中会被调用多次，所以要append
static size_t CurlWriteBuffer( char *buffer, size_t size, size_t nmemb, std::string* stream )
{
	size_t sizes = size * nmemb;
	if(stream == NULL) return sizes;
	stream->append(buffer,sizes);
	return sizes;
}


class CurlAutoRelease
{
public:
    CurlAutoRelease(){
        p = curl_easy_init();
        if(p == NULL)
            log_error("init curl falied");
    }
    ~CurlAutoRelease(){	curl_easy_cleanup(p);}
    CURL * GetPtr()	{	return p;}
protected:
    CURL * p;
};


/************************************************
 函数:    Http通用请求
 输入:    
 url                请求地址
 content            请求内容
 ret                存储返回结果
 timeout            超时时间
 https              是否加密，苹果的需要加密
 headers			HTTP消息头参数赋值
 输出:    CURLcode
 ************************************************/
static CURLcode xyCurlCommonRequestBase(/*IN */const string& url,
										/*IN */const string& content,
										/*OUT*/string& ret,
										/*IN */unsigned timeout = 5,
										/*IN */bool https = false,
										/*IN */curl_slist *headers = NULL)
{
	CurlAutoRelease curlAutoRelease;
	CURL* curl = curlAutoRelease.GetPtr();
    if(curl == NULL)
        return CURLE_FAILED_INIT;
	
	ret.clear();
	
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());						//目标URL
    //curl_easy_setopt(curl,CURLOPT_VERBOSE,1);								//打开调试
    if (content.size() > 0)
    {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.c_str());            //post请求
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, content.size());
    }
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	
    if(https){
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,  0);					//https 加密
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST,  0);
    }
	
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteBuffer);			//写回调
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,&ret);                          //写存储
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,timeout);							//最长执行时间
	
	// http消息头参数
	if (NULL != headers) {
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	}
	
    return curl_easy_perform(curl);
}

/************************************************
 函数:    Http通用请求
 输入:    url                请求地址
         content            请求内容
         ret                存储返回结果
         timeout            超时时间
         https              是否加密，苹果的需要加密
 输出:    CURLcode
 ************************************************/
static CURLcode xyCurlCommonRequest(/*IN */const string& url,
                       /*IN */const string& content,
                       /*OUT*/string& ret,
                       /*IN */unsigned timeout = 5,
                       /*IN */bool https = false)
{
	return xyCurlCommonRequestBase(url, content, ret, timeout, https);
}


// HTTP请求, 带http头消息参数
static CURLcode xyCurlCommonRequestHeaders(/*IN */const string& url,
									/*IN */const string& content,
									/*OUT*/string& ret,
									/*IN */unsigned timeout = 5,
									/*IN */bool https = false,
									/*IN */std::vector<string> *headerMsg = NULL)
{
	// 添加HTTP消息头
	struct curl_slist *headers = NULL;
	if (NULL != headerMsg && !headerMsg->empty()) {
		std::vector<string>::iterator it;
		for (it = headerMsg->begin(); it != headerMsg->end(); it++) {
			headers = curl_slist_append(headers, it->c_str());
		}
	}
	CURLcode code = xyCurlCommonRequestBase(url, content, ret, timeout, https, headers);
	curl_slist_free_all(headers);
	return code;
}

/************************************************
 函数:    Http通用请求
 输入:    url                请求地址
         content            请求内容
         ret                存储返回结果
         timeout            超时时间
         https              是否加密，苹果的需要加密
         retry              重试次数
 输出:    CURLcode
 ************************************************/
static CURLcode xyCurlCommonRequest(/*IN */const string& url,
                       /*IN */const string& content,
                       /*OUT*/string& ret,
                       /*IN */unsigned timeout,
                       /*IN */bool https,
                       /*IN */unsigned short retry
                       )
{
    CURLcode curlcode;
    unsigned short num = 0;
    while(num < retry)
    {
        curlcode = xyCurlCommonRequest(url, content, ret, timeout, https);
        if( curlcode == CURLE_OK)
        {
            break;
        }
        num++;
    }
    return curlcode;
}
#endif
