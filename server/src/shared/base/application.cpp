//
//  application.cpp
//  GameSrv
//
//  Created by prcv on 13-6-9.
//
//

#include "application.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <cstring>
#include <cstdlib>


bool Application::sTermiated = false;

void Application::terminate()
{
    sTermiated = true;
    return;
    //raise(SIGQUIT);
}

void Application::waitForTerminationRequest()
{
    while (!sTermiated) {
        usleep(100000);
    }
    return;
    
	sigset_t sset;
	sigaddset(&sset, SIGQUIT);
	sigprocmask(SIG_BLOCK, &sset, NULL);
	int sig = 0;
    for (;;){
        int rc = sigwait(&sset, &sig);
        if (rc == 0){
#ifndef __APPLE__
            break;
#else
            continue;
#endif
        }
    }
    
}

int Application::run(int argc, char** argv)
{
	if (init(argc, argv) != 0)
    {
        return -1;
    }
    
	main(argc, argv);
    
    deinit();
    
    return 0;
}

void Application::beDaemon()
{
    int fd;
    pid_t pid = fork();
    if (pid != 0)
    {
        exit(0);
    }
    
    setsid(); /* create a new session */
    umask(0);
    
    pid = fork();
    if (pid != 0)
    {
        exit(0);
    }
    chdir("/");
    
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO) close(fd);
    }
}