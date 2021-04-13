//
// Created by dawid12349 on 2/14/21.
//

#ifndef GRA_CONNECTION_H
#define GRA_CONNECTION_H
#include <signal.h>
int receive_signal(void (*handler)(int signum, siginfo_t * info, void* ucontext), int sig){
    struct sigaction sigusrSignal;
    sigusrSignal.sa_flags = SA_SIGINFO;
    sigusrSignal.sa_sigaction = handler;
    return sigaction(sig, &sigusrSignal, NULL);
}

#endif //GRA_CONNECTION_H
