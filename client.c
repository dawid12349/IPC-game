//
// Created by dawid12349 on 2/3/21.
//
#include "common.h"
#include "connection.h"
#include "display.h"
//ncurses
void init_display();
void display_info();
void display_surroundings(WINDOW* win);
WINDOW* legend_win = NULL;
WINDOW* maze_win = NULL;
WINDOW* info_win = NULL;

//signals
void sigint_handler(int signum, siginfo_t* info, void * c);
void server_signal_handler(int signum, siginfo_t* info, void * c);

//threads
void* check_heartbeat(void* args);
void* displayer_routine(void* args);
pthread_t check_hb;
pthread_t displayer;

//shared mem
void init_channel(int n);
void destroy_channel();
int server_info_fd;
int my_bus;
SERVER_HEARTBEAT_T * server_info = NULL;
PLAYER_PAYLOAD_T * my_payload = NULL;

//other
char exit_message[30] = "You exited the game!";
sem_t* con = NULL;

int main(void){
    con = sem_open(SEM_CON, 0);
    if(con == SEM_FAILED){
        printf("could not communicate with server!\n");
        return 1;
    }
    receive_signal(server_signal_handler, SIGUSR1);
    receive_signal(sigint_handler, SIGINT);
    server_info_fd = shm_open(SHM_SERVERINFO, O_RDWR ,0666);
    if(server_info_fd == -1){
        sem_close(con);
        printf("could not open the bus!\n");
        return 2;
    }
    server_info = (SERVER_HEARTBEAT_T*) mmap(NULL, sizeof(SERVER_HEARTBEAT_T), PROT_WRITE | PROT_READ,  MAP_SHARED , server_info_fd, 0);
    if(server_info == NULL){
        printf("could not connect to database!\n");
        sem_close(con);
        close(server_info_fd);
        return 2;
    }
    pthread_create(&check_hb, NULL, check_heartbeat, NULL);
    init_display();
    sigqueue(server_info->server_pid, SIGUSR1, (union sigval){ .sival_int = SV_CLIENT_CNT});
    sem_wait(con);
    pthread_create(&displayer, NULL , displayer_routine, NULL);
    while(1){
        char c = getch();
        switch(c){
            case 'w':
            case 's':
            case 'a':
            case 'd':
                sigqueue(server_info->server_pid, SIGUSR1, (union sigval){.sival_int = c });
                break;
            case 'q':
                sigqueue(server_info->server_pid, SIGUSR1, (union sigval){.sival_int = SV_CLIENT_DCNT});
                kill(getpid(), SIGINT);
                break;
        }

    }
    return 0;
}

void destroy_channel(){
    munmap(my_payload, sizeof(PLAYER_PAYLOAD_T));
    close(my_bus);
}

void init_channel(int n){
    switch(n){
        case 1:
            my_bus = shm_open(SHM_PLAYERBUS1, O_RDWR, 0666);
            break;
        case 2:
            my_bus = shm_open(SHM_PLAYERBUS2, O_RDWR, 0666);
            break;
        case 3:
            my_bus = shm_open(SHM_PLAYERBUS3, O_RDWR, 0666);
            break;
        case 4:
            my_bus = shm_open(SHM_PLAYERBUS4, O_RDWR, 0666);
            break;
    }
    my_payload = (PLAYER_PAYLOAD_T*) mmap(NULL, sizeof(PLAYER_PAYLOAD_T), PROT_WRITE|PROT_READ, MAP_SHARED, my_bus, 0);
}

void* displayer_routine(void* args){
    while(1){
        sem_wait(&my_payload->cs);
        display_surroundings(maze_win);
        display_info(info_win, my_payload);
        display_legend(legend_win);
        refresh_display(maze_win, legend_win, info_win);
    }
}

void server_signal_handler(int signum, siginfo_t* info, void * c){
    pid_t pid = (pid_t)info->si_pid;
    int sv= info->si_value.sival_int;
    switch(sv){
        case 1:
        case 2:
        case 3:
        case 4:
            init_channel(sv);
            break;
        case SV_SERVER_DECLINE:
            strcpy(exit_message, "Server decline, Game is full!");
            kill(getpid(), SIGINT);
            break;
    }
}

void sigint_handler(int signum, siginfo_t * info, void* c){
    pthread_cancel(displayer);
    pthread_cancel(check_hb);
    destroy_display(maze_win, legend_win, info_win);
    printf("%s", exit_message);
    sem_close(con);
    munmap(server_info, sizeof(SERVER_HEARTBEAT_T));
    close(server_info_fd);
    destroy_channel();
    exit(0);
}

void* check_heartbeat(void* args){
    int prev = 0;
    while(1){
        sem_wait(&server_info->hcs);
        prev = server_info->heartbeat;
        sem_post(&server_info->hcs);
        sleep(1);
        sem_wait(&server_info->hcs);
        if(prev == server_info->heartbeat && (kill(server_info->server_pid, 0) == -1 && errno == ESRCH)){
            if(my_payload) my_payload->pid =0;
            if(my_payload) my_payload->used = false;
            if(my_payload) sem_close(&my_payload->cs);
            strcpy(exit_message, "Server is not running!");
            kill(getpid(), SIGINT);
        }
        sem_post(&server_info->hcs);
    }
    pthread_exit(NULL);
}

void init_display(){
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    curs_set(FALSE);
    timeout(500);
    maze_win =  createWindow(11,25,0,0);
    legend_win = createWindow(11,35,0, 25);
    info_win = createWindow(11, 25, 0, 60);
}
void display_surroundings(WINDOW* win){
    mvwprintw(win, 0,1, "MAZE");
    for(int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            wchar_t c = my_payload->player.surroundings[y][x];
            if ((c > 30 && c < 127) && c != 'q' && c != 'l' && c != 'x') {
                mvwaddch(win, y+3, x+10, c);
            } else
                mvwaddch(win, y+3, x+10, CH_WALL);
        }
    }
}

void display_info(WINDOW* win, PLAYER_PAYLOAD_T *p){
    mvwprintw(win, 0, 1, "GAME INFO");
    mvwprintw(win, 1, 1, "Server pid:    %d", server_info->server_pid);
    mvwprintw(win, 2, 1, "Round number:  %d", p->round);
    mvwprintw(win, 3, 1, "Number:        %d", p->player.id);
    mvwprintw(win, 4, 1, "Type:          HUMAN");
    mvwprintw(win, 5, 1, "Curr X/Y:      %d/%d", p->player.position.x, p->player.position.y);
    mvwprintw(win, 6, 1, "Deaths:        %d", p->player.deaths);
    mvwprintw(win, 7, 1, "Coins found:   %d", p->player.coins_found);
    mvwprintw(win, 8, 1, "Coins brought: %d", p->player.coins_brought);
    mvwprintw(win, 9, 1, "Campsite X/Y:  %d|%d", p->campsite_pos.x, p->campsite_pos.y);
}