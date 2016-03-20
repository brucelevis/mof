//
//  ItemChange.h
//  GameSrv
//
//  Created by jin on 13-10-24.
//
//

#ifndef __GameSrv__ItemChange__
#define __GameSrv__ItemChange__

#include <iostream>
using namespace std;

class Role;

class ItemChangeMgr
{
public:

	// 物品转换
	static void itemChange(Role *role);
	static void itemChange(int roleId);

};



#endif /* defined(__GameSrv__ItemChange__) */
