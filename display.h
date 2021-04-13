//
// Created by dawid12349 on 2/14/21.
//

#ifndef GRA_DISPLAY_H
#define GRA_DISPLAY_H
#include <curses.h>
WINDOW* createWindow(int height, int width, int x, int y){
    WINDOW *newWin = newwin(height,width,x,y);
    box(newWin, 0, 0);
    return newWin;
}
void refresh_display(WINDOW *win1, WINDOW* win2, WINDOW* win3){
    refresh();
    wrefresh(win1);
    wrefresh(win2);
    wrefresh(win3);
}
void destroy_display(WINDOW *win1, WINDOW* win2, WINDOW* win3){
    if(win1) delwin(win1);
    if(win2) delwin(win2);
    if(win3) delwin(win3);
    endwin();
}

void display_legend(WINDOW * win){
    mvwprintw(win, 0, 1, "LEGEND");
    mvwprintw(win, 1, 1, "1234 -  Players");
    mvwaddch(win, 2, 1, CH_WALL);
    mvwprintw(win, 2, 3, "   -  Wall");
    mvwprintw(win, 3, 1, "#    -  Bushes (slow down)");
    mvwprintw(win, 4, 1, "c    -  One coin");
    mvwprintw(win, 5, 1, "t    -  Treasure (10 coins)");
    mvwprintw(win, 6, 1, "T    -  large treasure (50 coins)");
    mvwprintw(win, 7, 1, "D    -  Dropped treasure");
    mvwprintw(win, 8, 1, "A    -  Campsite");
    mvwprintw(win, 9, 1, "*    -  Beast");
}



#endif //GRA_DISPLAY_H
