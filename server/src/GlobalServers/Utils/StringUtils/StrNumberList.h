/*
 * StrNumberList.h
 *
 *  Created on: Dec 11, 2012
 *      Author: root
 *
 *      一个存放数字列表的字符串，数字间用“,“分割。
 */

#ifndef STRNUMBERLIST_H_
#define STRNUMBERLIST_H_
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "StringUtil.h"

template <class Number>
class StrNumberList
{
	typedef bool(*Str2Num)( std::string , Number* num ) ;
	typedef bool(*Num2Str)( Number , std::string& ) ;
private :
	int	readPos ;
	std::string	str ;
	Str2Num	str2Num ;		// 字符串到数字的转换函数
	Num2Str num2Str ;		// 数字到字符串的转换函数

public :
	StrNumberList( Str2Num str2Num , Num2Str num2Str )
	{
		this->str2Num = str2Num ;
		this->num2Str = num2Str ;
		this->str = "" ;
		this->readPos = 0 ;
	}
	~StrNumberList( )
	{

	}
	bool read( Number *num )
	{
		int commaPos ;
		while( 1 )
		{
			commaPos = this->str.find( ',' , readPos ) ;
			if( commaPos == readPos )
			{
				++readPos ;
				++commaPos ;
			}
			else
			{
				break ;
			}
		}
		if( commaPos < 0 )
		{	// 没有找到假设逗号在最末尾
			commaPos = str.length( ) ;
			if( readPos >= commaPos )
			{
				return false ;
			}
		}
		std::string numStr = this->str.substr( readPos , commaPos - readPos ) ;
		this->readPos = commaPos ;
		Trim2SideSpace( numStr ) ;
		return str2Num( numStr , num ) ;
	}
	void write( Number num )
	{
		std::string numStr ;
		num2Str( num , numStr ) ;
		if( this->str.length( ) == 0 )
			this->str += numStr ;
		else
			this->str += ',' + numStr ;
	}
	std::string getValue()
	{
		return this->str ;
	}
	void setValue( std::string value )
	{
		this->str = value ;
		this->readPos = 0 ;
	}
	int toList( Number list[] )
	{
		int count ;
		for( count = 0 ; read( list + count ) ; ++count ) ;
		return count ;
	}
	void clear()
	{
		this->str = "" ;
		this->readPos = 0 ;
	}
};

class StrIntList
: public StrNumberList<int>
{
public :
	StrIntList() : StrNumberList<int>( str2Int , int2Str )
	{ } ;
	~StrIntList() { } ;

	static bool int2Str( int num , std::string& str )
	{
		char buf[16] ;
		sprintf( buf , "%d" , num ) ;
		str = std::string( buf ) ;
		return true ;
	}
	static bool str2Int( std::string str , int* num )
	{
		*num = atoi( str.c_str() ) ;
		return true ;
	}
} ;


#endif /* STRNUMBERLIST_H_ */
