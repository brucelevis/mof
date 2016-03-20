// GlobalServer.cpp : 定义控制台应用程序的入口点。
//

//#include "stdafx.h"

#include "Game.h"

int main(int argc, char* argv[])
{
	// 创建CGame对象
	CGame * pGame = new CGame;

	if (!pGame)
	{
		return 0;
	}

	char* config = NULL;
	if (argc > 1) {
		config = argv[1];
	}

	// 初始化游戏
	pGame->Initialize(config);

	// 运行, 等待命令行输入, 同时开始一个逻辑线程
	pGame->Run();

	// 关闭游戏
	pGame->UnInitalize();

	// 删除CGame对象
	delete pGame;

	return 0;
}

