
#include "common.h"
#include "display.h"
#include "connection.h"
//display
void init_display();
void display_maze(WINDOW* win);
void display_info(WINDOW* win);
void display_players(PLAYER_PAYLOAD_T* p[MAX_PLAYERS]);
void display_campsite(POSITION_T* p);
void display_bushes(POSITION_T* p);
void display_bonuses(BONUSES_T* b);
void display_beasts(BEAST_T* b);
WINDOW* maze_win = NULL;
WINDOW* legend_win = NULL;
WINDOW* info_win = NULL;

//signals
void sigint_handler(int signum);
void connection_handler(int signum, siginfo_t* info, void* c);

//threads
void* seed_generator_routine(void* args);
void init_heartbeat(SERVER_HEARTBEAT_T* h);
void* heartbeat_routine(void* args);
void* displayer_routine(void*args);
void* logic_routine(void* args);
pthread_t tid_seedgen;
pthread_t tid_heartbeat;
pthread_t tid_displayer;
pthread_t tid_logic;
sem_t displayer_request;
sem_t displayer_respond;
int server_fd;
SERVER_HEARTBEAT_T * sc;

//Position API
bool set_position(POSITION_T* p, POSITION_T delta, ENTITIES_E e); //set valid position for entity; true = position had been set, false = position not set
bool pos_is_collided(POSITION_T p, POSITION_T other); //two positions collide with each other true otherwise false
bool check_char(wchar_t c, ENTITIES_E e); // check position for entity, if the entity can exist in that place true otherwise false
POSITION_T generateRandomPosition(int maxy, int maxx, ENTITIES_E e); //generates random valid position for entity
POSITION_T pos_convert(int key); //converts wsad to direction vector
POSITION_T get_pointing_direction(POSITION_T a, POSITION_T b); //gets a vector of pointing direction from one object to another

//objects
void init_bonuses(BONUSES_T *b, int n);
void create_bonus(BONUSES_T *b, int value, POSITION_T pos, char type);
char maze[][42] = MAZE_INITIALIZATOR;
POSITION_T campsite = CAMPSITE_INITIALIZATOR;
POSITION_T bushes[MAX_BUSHES] = BUSHES_INITIALIZATOR;
BONUSES_T bonuses;

//BEAST
void init_beast(BEAST_T*b);
void* beast_routine(void* args);
pthread_t tid_beasts[MAX_BEASTS];
BEAST_T beasts[MAX_BEASTS];
int beasts_counter = 0;

//PAYLOAD and PLAYER_T
void open_channels();
void close_channels();
void init_player_payload(PLAYER_PAYLOAD_T* p, int maxy_pos, int maxx_pos, pid_t pid, int round);
void send_payload(PLAYER_PAYLOAD_T* p);
PLAYER_PAYLOAD_T *searchPayload(pid_t key, bool searchActiveSlot);
void handle_player_collisions();
void turn_handler(PLAYER_PAYLOAD_T* p[MAX_PLAYERS]);
PLAYER_PAYLOAD_T* players[MAX_PLAYERS];
int players_buses[MAX_PLAYERS];

//variables
int rounds = 0;
sem_t *con;
int connected_players = 0;

int main() {
    con = sem_open(SEM_CON, O_CREAT, 0666, 0);
    signal(SIGINT, sigint_handler);
    receive_signal(connection_handler, SIGUSR1);
    receive_signal(connection_handler, SIGUSR2);
    open_channels();
    server_fd = shm_open(SHM_SERVERINFO, O_CREAT | O_RDWR ,0666);
    ftruncate(server_fd, sizeof(SERVER_HEARTBEAT_T));
    sc = (SERVER_HEARTBEAT_T*) mmap(NULL, sizeof(SERVER_HEARTBEAT_T), PROT_WRITE | PROT_READ,  MAP_SHARED , server_fd, 0);
    if(sc == NULL){
        perror("COULDN't connect to database...");
        close(server_fd);
        shm_unlink(SHM_SERVERINFO);
        return -1;
    }

    sem_init(&displayer_request, 1, 0);
    sem_init(&displayer_respond, 1, 0);
    for(int i =0 ; i < MAX_BEASTS; i++) sem_init(&beasts[i].cs,1,1);
    init_heartbeat(sc);
    init_display();
    init_bonuses(&bonuses, 3);
    pthread_create(&tid_seedgen, NULL, seed_generator_routine, NULL);
    pthread_create(&tid_heartbeat, NULL, heartbeat_routine ,NULL);
    pthread_create(&tid_displayer, NULL, displayer_routine, NULL);
    pthread_create(&tid_logic, NULL, logic_routine, NULL);
    while(1){
        switch (getch()) {
            case 'B':
            case 'b':
                if(beasts_counter < 3) {
                    init_beast(&beasts[beasts_counter]);
                    pthread_create(&tid_beasts[beasts_counter], NULL, beast_routine, &beasts[beasts_counter]);
                    beasts_counter++;
                }
                break;
            case 'T':
                create_bonus(&bonuses, 50, generateRandomPosition(29,39, E_COIN), CH_LTREASURE);
                break;
            case 't':
                create_bonus(&bonuses, 10, generateRandomPosition(29,39, E_COIN), CH_TREASURE);
                break;
            case 'c':
                create_bonus(&bonuses, 1, generateRandomPosition(29,39, E_COIN), CH_COIN);
                break;
            case 'q':
                sigint_handler(1);
                break;
        }
    }
    kill(getpid(), SIGINT);
    return 0;
}

POSITION_T get_pointing_direction(POSITION_T a, POSITION_T b){
    int x = 0;
    int y = 0;
    if(a.x - b.x == 2 || a.x - b.x == 1)
        x = 1;
    if(a.x-b.x == -2 || a.x - b.x ==  -1 )
        x= -1;
    if(a.y - b.y == 2 || a.y - b.y == 1)
        y = 1;
    if(a.y - b.y == -2 || a.y - b.y == -1)
        y = -1;
    return (POSITION_T){x, y};
}

void open_channels(){
    for(int i = 0; i<MAX_PLAYERS; i++){
        switch (i) {
            case 0:
                players_buses[i] = shm_open(SHM_PLAYERBUS1, O_CREAT|O_RDWR, 0666);
                break;
            case 1:
                players_buses[i] = shm_open(SHM_PLAYERBUS2, O_CREAT|O_RDWR, 0666);
                break;
            case 2:
                players_buses[i] = shm_open(SHM_PLAYERBUS3, O_CREAT|O_RDWR, 0666);
                break;
            case 3:
                players_buses[i] = shm_open(SHM_PLAYERBUS4, O_CREAT|O_RDWR, 0666);
                break;
        }
        ftruncate(players_buses[i], sizeof(PLAYER_PAYLOAD_T));
        players[i] = (PLAYER_PAYLOAD_T*) mmap(NULL, sizeof(PLAYER_PAYLOAD_T), PROT_WRITE | PROT_READ, MAP_SHARED, players_buses[i], 0);
    }
}

void close_channels(){
    for(int i = 0; i<MAX_PLAYERS; i++){
        munmap(players[i], sizeof(PLAYER_PAYLOAD_T));
        close(players_buses[i]);
        switch (i) {
            case 0:
                shm_unlink(SHM_PLAYERBUS1);
                break;
            case 1:
                shm_unlink(SHM_PLAYERBUS2);
                break;
            case 2:
                shm_unlink(SHM_PLAYERBUS3);
                break;
            case 3:
                shm_unlink(SHM_PLAYERBUS4);
                break;
        }
    }
}

void init_beast(BEAST_T*b){
    if(b!=NULL){
        b->state = S_IDLE;
        b->dir = (POSITION_T){0,0};
        b->pos = generateRandomPosition(29,39,E_BEAST);
        b->exist = true;
    }
}


void* beast_routine(void* args){
    BEAST_T* b = (BEAST_T*)args;
    while(1){
        sem_wait(&b->cs);
        if(b->state == S_IDLE) {
            POSITION_T dir = (POSITION_T) {0, 0};
            float min = 3;
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (players[i]->used) {
                    float dist = sqrt(pow(players[i]->player.position.x - b->pos.x, 2) +
                                      pow(players[i]->player.position.y - b->pos.y, 2));
                    if ((dist == 1.0 || dist == 2.0) && dist <= min) {
                        min = dist;
                        dir = get_pointing_direction(players[i]->player.position, b->pos);
                        b->state = S_SPOTTED;
                    }
                }
            }
            b->dir = dir;
        }
        sem_post(&b->cs);
    }
    return NULL;
}

void* displayer_routine(void*args){
    while(1) {
        sem_wait(&displayer_request);
        display_maze(maze_win);
        display_campsite(&campsite);
        display_bushes(bushes);
        display_bonuses(&bonuses);
        display_players(players);
        display_beasts(beasts);
        display_legend(legend_win);
        display_info(info_win);
        refresh_display(maze_win, info_win, legend_win);
        sem_post(&displayer_respond);
    }
    return NULL;
}

void* logic_routine(void* args){
    while(1){
        sleep(1);
        turn_handler(players);
        handle_player_collisions();
        rounds++;
        sem_post(&displayer_request);
        sem_wait(&displayer_respond);
        for(int i = 0; i< MAX_PLAYERS; i++) send_payload(players[i]);
    }
   return NULL;
}

void init_heartbeat(SERVER_HEARTBEAT_T* h){
    if(h != NULL){
        h->heartbeat = 0;
        h->server_pid = getpid();
        sem_init(&h->hcs, 1, 1);
    }
}

void* heartbeat_routine(void* args){
    while(1){
        sem_wait(&sc->hcs);
        sc->heartbeat++;
        sem_post(&sc->hcs);
        sleep(1);
    }
    pthread_exit(NULL);
}

void* seed_generator_routine(void* args){
    while(1){
        srand(time(NULL));
        sleep(1);
    }
}

void handle_player_collisions(){
    for(int i = 0 ; i< MAX_PLAYERS ; i++) {
        PLAYER_PAYLOAD_T *p = players[i];
        if(p->used) {
            for (int j = 0; j < bonuses.bonuses_count; j++) {
                if (pos_is_collided(p->player.position, bonuses.arr[j].pos) && bonuses.arr[j].exist) {
                    p->player.coins_found += bonuses.arr[j].value;
                    if (bonuses.arr[j].type != CH_DTREASURE) {
                        bonuses.arr[j].pos = generateRandomPosition(29, 39, E_COIN);
                    } else bonuses.arr[j].exist = false;
                    break;
                }
            }

            for (int j = 0; j < MAX_PLAYERS; j++) {
                if (i != j) {
                    if (players[j]->used && pos_is_collided(p->player.position, players[j]->player.position)) {
                        POSITION_T prev = p->player.position;
                        p->player.position = p->player.spawn_point;
                        players[j]->player.position = players[j]->player.spawn_point;
                        create_bonus(&bonuses, p->player.coins_found + players[j]->player.coins_found, prev,
                                     CH_DTREASURE);
                        p->player.coins_found = 0;
                        players[j]->player.coins_found = 0;
                        break;
                    }
                }
            }

            for(int j =0 ; j< MAX_BUSHES; j++){
                if(pos_is_collided(p->player.position, bushes[j]) && p->player.slow_counter == 0){
                    p->player.slow_counter = 2;
                    break;
                }
            }

            for(int j = 0; j< beasts_counter; j++){
                sem_wait(&beasts[j].cs);
                if(beasts[j].exist && pos_is_collided(p->player.position, beasts[j].pos)){
                    POSITION_T prevpos = p->player.position;
                    p->player.deaths++;
                    p->player.position = p->player.spawn_point;
                    create_bonus(&bonuses, p->player.coins_found, prevpos, CH_DTREASURE);
                    p->player.coins_found = 0;
                }
                sem_post(&beasts[j].cs);
            }

            if (pos_is_collided(p->player.position, campsite)) {
                p->player.coins_brought += p->player.coins_found;
                p->player.coins_found = 0;
            }
        }
    }

}

void create_bonus(BONUSES_T* b, int value, POSITION_T pos, char type){
    if(b!=NULL && value != 0) {
        int i = b->bonuses_count % MAX_BONUSES;
        b->arr[i].value = value;
        b->arr[i].type = type;
        b->arr[i].pos = pos;
        b->arr[i].exist = true;
        b->bonuses_count = b->bonuses_count + 1;
    }
}

void init_bonuses(BONUSES_T* b, int n){ //n - how many on starts
    if(n < MAX_BONUSES){
        n %= 10;
        for(int i = 0 ; i < n ; i++){
            if(i==0) create_bonus(b, B_COIN, (POSITION_T){12,4},CH_COIN);
            if(i==1) create_bonus(b, B_TREASURE, (POSITION_T){13,24}, CH_TREASURE);
            if(i==2) create_bonus(b, B_LTREASURE, (POSITION_T){36,28}, CH_LTREASURE);
        }
    }
}

void send_payload(PLAYER_PAYLOAD_T* p) {
    if (p->used) {
        for (int y = 0; y < FOV; y++) {
            for (int x = 0; x < FOV; x++) {
                p->player.surroundings[y][x] = mvwinch(maze_win, p->player.position.y + y - 2,
                                                       p->player.position.x + x - 2);
            }
        }
        p->round = rounds;
        p->campsite_pos = campsite;
        sem_post(&p->cs);
    }
}

bool set_position(POSITION_T* p, POSITION_T delta, ENTITIES_E e){
    wchar_t c = mvwinch(maze_win, p->y + delta.y, p->x + delta.x );
    if(p!=NULL && check_char(c, e)) {
        p->y += delta.y;
        p->x += delta.x;
        return true;
    }
    else return false;
}
bool pos_is_collided(POSITION_T p, POSITION_T other){
    return p.x == other.x && p.y == other.y;
}
bool check_char(wchar_t c, ENTITIES_E e){
    switch (e) {
        case E_PLAYER:
            if(c != CH_WALL) return true;
            else return false;
        case E_BEAST:
            if(c == CH_WALL || c == CH_CAMPSITE) return false;
            else return true;
        case E_COIN:
            if(c == CH_PLAYER(1) || c==CH_CORRIDOR || c == CH_PLAYER(2) || c == CH_PLAYER(3) || c == CH_PLAYER(4)) return true;
            else return false;
        default:
            return false;
    }
}

POSITION_T generateRandomPosition(int maxy, int maxx, ENTITIES_E e){
    int x,y;
    do{
        x = rand() % maxx + 2;
        y = rand() % maxy + 2;
    }while(!check_char(mvwinch(maze_win, y,x), e));
    return (POSITION_T){x,y};
}

POSITION_T pos_convert(int key){
    switch(key){
        case 'w':
            return (POSITION_T){ 0, -1};
        case 's':
            return (POSITION_T){ 0, 1};
        case 'a':
            return (POSITION_T){-1, 0};
        case 'd':
            return (POSITION_T){ 1, 0};
        default:
            return (POSITION_T){ 0, 0};
    } 
}

void init_player_payload(PLAYER_PAYLOAD_T* p, int maxy_pos, int maxx_pos, pid_t pid, int round){
    p->pid = pid;
    p->used = true;
    p->player.position =  generateRandomPosition(maxy_pos, maxx_pos, E_PLAYER );
    p->player.spawn_point = p->player.position;
    p->player.deaths = 0;
    p->player.coins_found = 0;
    p->player.coins_brought = 0;
    p->player.slow_counter = 0;
    p->round = round;
    sem_init(&p->cs, 1, 0);
}

PLAYER_PAYLOAD_T *searchPayload(pid_t key, bool searchActiveSlot){ //search for active or for empty player slot
    PLAYER_PAYLOAD_T *p;
    for(int i = 0; i  < MAX_PLAYERS; i++) {
        p = players[i];
        if (searchActiveSlot) {
            if (p->pid == key && p->used)
                return p;
        }
        else {
            if (p->pid == 0 && !p->used) {
                p->player.id = i + 1;
                return p;
            }
        }
    }
    return NULL;
}

void turn_handler(PLAYER_PAYLOAD_T* p[MAX_PLAYERS]){
    for(int i = 0; i < MAX_PLAYERS; i++){
        if(p[i]->used && p[i]->pid != 0){
            if(p[i]->player.slow_counter == 2){
                p[i]->player.slow_counter = 1;
                continue;
            }
            if(p[i]->player.slow_counter == 1) p[i]->player.slow_counter = 0;
            set_position(&p[i]->player.position, p[i]->player.dir, E_PLAYER);
            p[i]->player.dir = (POSITION_T){0,0};
        }
    }

    for(int i = 0; i < beasts_counter; i++){
        sem_wait(&beasts[i].cs);
        if(beasts[i].exist && beasts[i].state == S_SPOTTED){
            beasts[i].state = S_CHASE;
        }
        if(beasts[i].state == S_CHASE){
            bool c = set_position(&beasts[i].pos, beasts[i].dir, E_BEAST);
            if(!c){
                beasts[i].state = S_IDLE;
            }
        }
        sem_post(&beasts[i].cs);
    }
}

void connection_handler(int signum, siginfo_t* info, void* c){
    int payload = (int) info->si_value.sival_int;
    pid_t client_pid = (pid_t) info->si_pid;

    if(signum == SIGUSR1) {
        PLAYER_PAYLOAD_T *p;
        if(payload == SV_CLIENT_CNT){
            p = searchPayload(client_pid, false);
            if(p == NULL){
                sigqueue(client_pid, SIGUSR1, (union sigval){.sival_int = SV_SERVER_DECLINE});
                return;
            }
            init_player_payload(p, 29, 39,  client_pid, 0);
            sigqueue(client_pid, SIGUSR1, (union sigval){.sival_int = p->player.id });
            sem_post(con);
        }
        else if(payload >= SV_CLIENT_MV){
            p = searchPayload(client_pid, true);
            if(p == NULL){
                sigqueue(client_pid, SIGUSR1, (union sigval){.sival_int = SV_SERVER_DECLINE});
                return;
            }
            POSITION_T delta = pos_convert(payload);
            p->player.dir = delta;
        }
        else if(payload == SV_CLIENT_DCNT){
            p = searchPayload(client_pid, true);
            if(p==NULL) return;
            p->used = false;
            p->pid = 0;
            sem_destroy(&p->cs);
        }
    }
}

void sigint_handler(int signum){
    destroy_display(maze_win, info_win, legend_win);
    printf("Server shut down, freeing the memory and connections...");
    pthread_cancel(tid_seedgen);
    pthread_cancel(tid_heartbeat);
    pthread_cancel(tid_displayer);
    pthread_cancel(tid_logic);
    for(int i = 0 ; i < beasts_counter; i++) pthread_cancel(tid_beasts[i]);
    for(int i = 0;  i < MAX_BEASTS; i++) sem_close(&beasts[i].cs);
    sem_destroy(&sc->hcs);
    sem_destroy(&displayer_request);
    sem_destroy(&displayer_respond);
    sem_destroy(con);
    munmap(sc, sizeof(SERVER_HEARTBEAT_T));
    close(server_fd);
    shm_unlink(SHM_SERVERINFO);
    close_channels();
    sem_close(con);
    sem_unlink(SEM_CON);
    exit(0);
}

void init_display(){
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    curs_set(FALSE);
    timeout(500);
    maze_win =  createWindow(33, 44,0,0);
    legend_win = createWindow(11, 45,13, 44);
    info_win = createWindow(13, 45, 0, 44);
}

void display_maze(WINDOW* win){
    mvwprintw(win, 0,1, "MAZE");
    int k = (int) (sizeof(maze) / sizeof(maze[0]));
    for(int i = 0; i<k; i++) {
        for (int j = 0; j < 41; j++) {
            wchar_t c = (unsigned char)maze[i][j];
            if(c == '#')
                mvwaddch(win, i + 1, j + 1, CH_WALL);
            else
                mvwaddch(win, i + 1, j + 1, CH_CORRIDOR);
        }
    }
}

void display_players(PLAYER_PAYLOAD_T* p[MAX_PLAYERS]){
    if(p!=NULL) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if(p[i]->used)
                mvwaddch(maze_win, p[i]->player.position.y, p[i]->player.position.x, CH_PLAYER(i+1));
        }
    }
}

void display_bonuses(BONUSES_T* b){
    int curr = b->bonuses_count > MAX_BONUSES ? MAX_BONUSES : b->bonuses_count;
    for(int i = 0; i < curr; i++){
        if(b->arr[i].exist){
            mvwaddch(maze_win, b->arr[i].pos.y, b->arr[i].pos.x, b->arr[i].type);
        }
    }
}

void display_beasts(BEAST_T* b){
    if(b!=NULL) {
        for (int i = 0; i < beasts_counter; i++) {
            sem_wait(&beasts[i].cs);
            if((b+i)->exist)
                mvwaddch(maze_win, (b+i)->pos.y, (b+i)->pos.x, CH_BEAST);
            sem_post(&beasts[i].cs);
        }
    }
}

void display_campsite(POSITION_T* p){
    mvwaddch(maze_win, p->y , p->x, CH_CAMPSITE);
}

void display_bushes(POSITION_T* p){
    for(int i = 0; i<MAX_BUSHES; i++){
        mvwaddch(maze_win, (p+i)->y, (p+i)->x , CH_BUSHES);
    }
}

void display_info(WINDOW* win){
    mvwprintw(win, 0, 1, "INFO");
    mvwprintw(win, 1, 1, "Server's PID: %d", getpid());
    mvwprintw(win, 2, 1, "Campsite X/Y: %d/%d", campsite.x, campsite.y);
    mvwprintw(win, 3, 1, "Round: %d", rounds);
    mvwprintw(win, 4, 1, "Parameter: Player1 Player2 Player3 Player4");
    mvwprintw(win, 5, 1, "PID:");
    mvwprintw(win, 6, 1, "TYPE:");
    mvwprintw(win, 7, 1, "Curr X/Y:");
    mvwprintw(win, 8, 1, "Deaths:");
    mvwprintw(win, 9, 1, "Coins:");
    mvwprintw(win, 10, 1, "carried");
    mvwprintw(win, 11, 1, "brought");
    for (int i = 0, offset = 8; i< MAX_PLAYERS; i++){
        if(players[i]->used){
            mvwprintw(win, 5, 12  +  offset*(i), "%d", players[i]->pid);
            mvwprintw(win, 6, 12  +  offset*(i), "%s", "HUMAN");
            mvwprintw(win, 7, 12  +  offset*(i), "%d/%d", players[i]->player.position.x, players[i]->player.position.y);
            mvwprintw(win, 8, 12  +  offset*(i), "%d", players[i]->player.deaths);
            mvwprintw(win, 10, 12  +  offset*(i), "%d", players[i]->player.coins_found);
            mvwprintw(win, 11,12  +  offset*(i), "%d", players[i]->player.coins_brought);
        }
        else {
            mvwprintw(win, 5,  12  +  offset*(i), "-      ");
            mvwprintw(win, 6,  12  +  offset*(i), "-      ");
            mvwprintw(win, 7,  12  +  offset*(i), "  -/-  ", players[i]->player.position.x, players[i]->player.position.y);
            mvwprintw(win, 8,  12  +  offset*(i), "-      ", players[i]->player.deaths);
            mvwprintw(win, 10,  12  +  offset*(i), "-      ", players[i]->player.coins_found);
            mvwprintw(win, 11, 12  +  offset*(i), "-      ", players[i]->player.coins_brought);
        }
    }
}
