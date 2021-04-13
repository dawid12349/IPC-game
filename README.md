# IPC-game
A simple IPC pacman-like game mady by using C programming langauge.

commands for linux:
running a server: "gcc main.c -o server -lpthread -lrt -lcurses -lm && ./server"

running a client(up to 4): "gcc client.c -o client -lpthread -lrt -lcurses -lm && ./client"


Features:

turn-based

4 playable concurrent players

spawning up to 3 enemies

spawning up to 20 collectable treasures

droping loot after death

bushes that slow you down for 1 turn

a game log and monitoring interface for the server

List of IPC tools that I used:

threads

mutexes

unnamed semaphores and named semaphores

shared memory

