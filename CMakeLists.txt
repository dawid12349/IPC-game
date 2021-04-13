cmake_minimum_required(VERSION 3.17)
project(gra C)

set(CMAKE_C_STANDARD 99)



set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


add_executable(gra client.c
        main.c display.h connection.h s.h)
target_link_libraries(gra PRIVATE Threads::Threads rt curses)