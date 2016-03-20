// GlobalServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

//#include "stdafx.h"

#include "Game.h"

int main(int argc, char* argv[])
{
	// ����CGame����
	CGame * pGame = new CGame;

	if (!pGame)
	{
		return 0;
	}

	char* config = NULL;
	if (argc > 1) {
		config = argv[1];
	}

	// ��ʼ����Ϸ
	pGame->Initialize(config);

	// ����, �ȴ�����������, ͬʱ��ʼһ���߼��߳�
	pGame->Run();

	// �ر���Ϸ
	pGame->UnInitalize();

	// ɾ��CGame����
	delete pGame;

	return 0;
}

