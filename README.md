# IPC-game
<h3>A simple IPC pacman-like game mady by using C programming langauge.</h3>

<b>commands for linux:</b>
running a server: "gcc main.c -o server -lpthread -lrt -lcurses -lm && ./server"
running a client(up to 4): "gcc client.c -o client -lpthread -lrt -lcurses -lm && ./client"

<ul><b>Features:</b></ul>
<li>turn-based</li>
<li>4 playable concurrent players</li>
<li>spawning up to 3 enemies</li>
<li>spawning up to 20 collectable treasures</li>
<li>droping loot after death</li>
<li>bushes that slow you down for 1 turn</li>
<li>a game log and monitoring interface for the server</li>

List of IPC tools that I used:

threads

mutexes

unnamed semaphores and named semaphores

shared memory

