#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "invaderstruct.h"

struct Player player;

void createPlayerThread(void *arg)
{
    player.line = LINES-1;
    player.col = COLS/2;
    player.ch = '^';

    move(player.line, player.col);
    addch(player.ch);

    int inputKeyBoard;
    while ((inputKeyBoard = getch()) != KEY_F(4) && inputKeyBoard != ERR)
    {
        if( (inputKeyBoard == KEY_RIGHT || inputKeyBoard == KEY_LEFT) )
        {
            
            move(player.line, player.col);
            addch(' ');

            player.col += (inputKeyBoard == KEY_RIGHT) ? 1 : -1;

            if(player.col <= 0)
                player.col = 1;
            else if (player.col >= COLS)
                player.col = COLS - 1;

            move(player.line, player.col);
            addch(player.ch);
        }
    }

    pthread_exit(NULL);

}

int main() {
    initscr(); // Inicializa ncurses
    keypad(stdscr, TRUE);
    curs_set(0);

    pthread_t playerThread;
    int player = pthread_create(&playerThread, NULL, createPlayerThread, NULL);
    if(player != 0)
    {
        perror("Error al crear el hilo player");
        return 1;
    }

    // Detiene ncurses y restaura el estado del terminal
    pthread_join(playerThread, NULL);
    endwin();

    return 0;
}