/*
	created:	2008/09/08
	created:	8:9:2008   14:19
	filename: 	d:\Ð¡¹¤¾ß\AStarPathFind\AStar.h
	file path:	d:\Ð¡¹¤¾ß\AStarPathFind
	file base:	AStar
	file ext:	h
	author:		deeple

	purpose:
*/

#ifndef ASTAR_H_
#define ASTAR_H_

#include <vector>

using namespace std;

union Pos32Union
{
	struct Pos
	{
		short x;
		short y;
	} Pos;
	int Pos32;
};

struct AStarNode
{
	Pos32Union position;
	AStarNode* pParent;
	int depth;
	int f;
};

struct StartDest
{
	Pos32Union start;
	Pos32Union dest;
};

class DualStack
{
public:
	DualStack();
	~DualStack();
public:
	void push(AStarNode* pNode);
	AStarNode* pop();
	size_t size();
	void clear();

public:
	vector<AStarNode*> m_Data;
	size_t m_nodeCount;
};

class BitMapping
{
public:
	BitMapping();
	~BitMapping();

public:
	bool SetMapSize(short left, short top, short width, short height);
	bool SetBit(short x, short y, bool bTrue);
	bool GetBit(short x, short y, bool & bRet);
	void Clear();

public:
	vector<vector<char> > m_Data;
	short _left;
	short _top;
	short _width;
	short _height;
};

class AStarPathFind
{
public:
	AStarPathFind();
	virtual ~AStarPathFind();

public:
	bool SetMapSize(short left, short top, short width, short height);
	bool FindPath(short startX, short startY, short destX, short destY);
	const vector<AStarNode*>& GetPath();
	size_t nodeUsed();

protected:
	virtual bool WalkAble(short x, short y);

private:
	void Expand(AStarNode* pNode, StartDest& pos);
	void Reset();
	void AddNode(short nodeX, short nodeY, AStarNode* pParent, StartDest& pos);
	int GetCost(AStarNode* pParent, AStarNode* pThis, StartDest& pos);

public:
	short _left;
	short _top;
	short _width;
	short _height;

	DualStack sort;
	BitMapping find;
	vector<AStarNode*> alloc;
	size_t allocUsed;
	vector<AStarNode*> path;
};

// inlines

// 0000 0001  0x1
// 0000 0010  0x2
// 0000 0100  0x4
// 0000 1000  0x8
// 0001 0000  0x10
// 0010 0000  0x20
// 0100 0000  0x40
// 1000 0000  0x80

// 1111 1110  0xFE
// 1111 1101  0xFD
// 1111 1011  0xFB
// 1111 0111  0xF7
// 1110 1111  0xEF
// 1101 1111  0xDF
// 1011 1111  0xBF
// 0111 1111  0x7F

//static BYTE _T1[8] = {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};
//static BYTE _T0[8] = {0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F};
/*
if (bTrue){
	c = c | _T1[nBit];
}
else{
	c = c & _T1[nBit];
}*/

inline void SetBit(char& c, int nBit, bool bTrue)
{
	if (bTrue){
		c = c | (1 << nBit);
	}
	else{
		c = c & (~(1 << nBit));
	}
}

inline void GetBit(char& c, int nBit, bool& bTrue)
{
	if(((c >> nBit) & 1) == 1){
		bTrue = true;
	}
	else{
		bTrue = false;
	}
}

inline int GetDistance(short x, short y, short dx, short dy)
{
	x = (x - dx);
	y = (y - dy);

	return (int)x*(int)x + (int)y*(int)y;
}

#endif


