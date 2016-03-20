//
//  signal_handler.cpp
//  GameSrv
//
//  Created by prcv on 14-9-24.
//
//

#include "signal_handler.h"
#include "application.h"
#include "log.h"
#include <signal.h>

struct signal
{
    int signo;
    const char* signame;
    int flags;
    void (*handler)(int signo);
};

void signalHandler(int signo);

struct signal signals[] =
{
    {SIGUSR1, "SIGUSR1", 0, signalHandler},
    {SIGUSR2, "SIGUSR2", 0, signalHandler},
    
    {SIGQUIT, "SIGQUIT", 0, signalHandler},
    {SIGTERM, "SIGTERM", 0, signalHandler},
    
    {SIGHUP, "SIGHUP", 0, SIG_IGN},
    {SIGPIPE, "SIGPIPE", 0, SIG_IGN},
    {0, NULL, 0, NULL}
};

int signalInit()
{
    struct  signal* sig;
    for (sig = signals; sig->signo != 0; sig++) {
        struct sigaction action = {0};
        action.sa_handler = sig->handler;
        sigaction(sig->signo, &action, NULL);
    }
    
    return 0;
}
    
void signalHandler(int signo)
{
    struct  signal* sig;
    for (sig = signals; sig->signo != 0; sig++) {
        if (sig->signo == signo) {
            break;
        }
    }
    
    switch (signo) {
        case SIGUSR1:
            log_level_up();
            break;
        case SIGUSR2:
            log_level_down();
            break;
        case SIGQUIT:
        case SIGTERM:
            Application::terminate();
            break;
        default:
            break;
    }
    
}

int signalDeinit()
{
    return 0;
}