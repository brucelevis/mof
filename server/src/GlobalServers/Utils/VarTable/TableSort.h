/*--------------------------------------------------------------------
	created:	2010/11/24
	filename: 	e:\Project_SVN\Server\Utils\VarTable\TableSort.h
	author:		
	
	purpose:	
--------------------------------------------------------------------*/

#pragma once

#include "VarTable.h"

typedef bool (*LPTABLESORT)(CVarRow* row1, CVarRow* row2);
extern LPTABLESORT GetSortFunc(int col, bool asc);
