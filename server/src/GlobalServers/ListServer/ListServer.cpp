// CrossDomain.cpp : 定义控制台应用程序的入口点。
//


#define SERVER_PORT 8181

#include "NanoWeb.h"


void OnHttpRequest(PersistID id, string& cmd)
{
    /*
    NanoWeb::SetTableTitle(id, "请求回应", "参数名", "值");
    NanoWeb::AddTableItem(id, "请求ip", NanoWeb::GetClientIP(id));
    NanoWeb::AddTableItem(id, "命令行", cmd.c_str());

    NanoWeb::AddTableItem(id, "name", "yahoo");
    NanoWeb::AddTableItem(id, "sex", "1");
    NanoWeb::AddTableItem(id, "age", "26");
    NanoWeb::AddTableItem(id, "job", "leader");
    NanoWeb::AddTableItem(id, "blood", "O");*/

    NanoWeb::Write(id, "{}");

    NanoWeb::Flush(id);
    NanoWeb::Disconnect(id, 3);
}

int main(int argc, char* argv[])
{
    NanoWeb::Open( SERVER_PORT );
    NanoWeb::SetCallBack( OnHttpRequest );

    char cmd[1024];
    while(1)
    {
        scanf("%s", cmd);

        if (stricmp(cmd, "q") == 0) break;

        usleep( 100000 );
    }

    NanoWeb::Close();

	return 0;
}
