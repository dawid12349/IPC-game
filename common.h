//
// Created by dawid12349 on 2/1/21.
//

#ifndef GRA_MAIN_H
#define GRA_MAIN_H
#include <stdio.h>
#include <stdlib.h>
#include <curses.h> // ui
#include <locale.h> //???
#include <unistd.h> //sleep
#include <pthread.h> // beast and handlers
#include <semaphore.h> // critical section, sending signals from player to server and vice versa
#include <sys/mman.h> // shared_mem (database)
#include <fcntl.h> //file descriptors
#include <signal.h> // signal hijacking, sending input from player to server
#include <errno.h> // ???
#include <time.h> //seed gen
#include <math.h> // beast
#include <string.h> // exit message
#include "constants.h"



typedef struct {
    pid_t server_pid;
    int heartbeat;
    sem_t hcs;
    int current_players;
} SERVER_HEARTBEAT_T;

typedef struct {
    int x;
    int y;
} POSITION_T;


typedef enum{
    //mutable entities
    E_PLAYER = 0, E_BEAST, E_COIN
} ENTITIES_E;

typedef struct {
    POSITION_T pos;
    char type;
    int value;
    bool exist;
} BONUS_T;

typedef struct {
    BONUS_T arr[MAX_BONUSES];
    int bonuses_count;
} BONUSES_T;

typedef struct {
    POSITION_T position;
    POSITION_T spawn_point;
    POSITION_T dir;
    wchar_t surroundings[FOV][FOV];
    int id;
    int slow_counter;
    int coins_found;
    int coins_brought;
    int deaths;
} PLAYER_T;

typedef struct {
    pid_t pid;
    PLAYER_T player;
    POSITION_T campsite_pos;
    int round;
    bool used;
    sem_t cs;
} PLAYER_PAYLOAD_T;

typedef enum {
    S_IDLE = 0,
    S_SPOTTED,
    S_CHASE,
} STATE_E;

typedef struct {
    POSITION_T pos;
    POSITION_T dir;
    STATE_E state;
    bool exist;
    sem_t cs;
} BEAST_T;


#endif //GRA_MAIN_H
