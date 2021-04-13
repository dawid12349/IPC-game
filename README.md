# IPC-game
<h3>A simple IPC pacman-like game mady by using C programming langauge.</h3>

<h3>Client view:</h3>
![clientview](https://user-images.githubusercontent.com/47251508/114604933-d442a980-9c99-11eb-80d1-5e2a0b34fb6c.png)
<h3>Server view</h3>
![serverview](https://user-images.githubusercontent.com/47251508/114605010-e886a680-9c99-11eb-8e79-7d083b16529d.png)

<b>commands for linux:</b>
<p>running a <b>server</b>: "gcc main.c -o server -lpthread -lrt -lcurses -lm && ./server"</p>
<p>running a <b>client</b>(up to 4): "gcc client.c -o client -lpthread -lrt -lcurses -lm && ./client"</p>


<b>Features:</b>
<li>turn-based</li>
<li>4 playable concurrent players</li>
<li>spawning up to 3 enemies</li>
<li>spawning up to 20 collectable treasures</li>
<li>droping loot after death</li>
<li>bushes that slow you down for 1 turn</li>
<li>a game log and monitoring interface for the server</li>

<b><ul>List of IPC tools that I used:</ul></b>
<li>threads</li>
<li>mutexes</li>
<li>unamed and named semaphores</li>
<li>shared memory</li>


