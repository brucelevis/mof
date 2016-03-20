//
//  launcher.cpp
//  GameSrv
//
//  Created by 丁志坚 on 15/1/19.
//
//

#include <stdio.h>
#include <getopt.h>
#include "server_app.h"

extern ServerApp* getApp();

int main(int argc, char** argv)
{
    ServerApp *app = getApp();
    return app->run(argc, argv);
}