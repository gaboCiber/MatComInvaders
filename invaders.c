#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "invaderstruct.h"

const int TOTALBULLET = 10;
struct Player player;
struct Bullet bulletArr[10];


void SetScreen()
{
    keypad(stdscr, TRUE);
    curs_set(0);

    move(0,(COLS/2)-9);
    addstr("--MatCOM INVADERS--");
    move (0,1);
    addstr("SCORE: ");
    
    refresh();
}

void createBullet(void *arg)
{   
    struct Bullet bullet = bulletArr[(int) arg];
    bullet.ch = '|';
    bullet.line = LINES-2;
    bullet.col = player.col;

    while (bullet.line > 1)
    {
        move(bullet.line, bullet.col);
        addch(bullet.ch);

        refresh();
        usleep(50000);

        move(bullet.line, bullet.col);
        addch(' ');

        bullet.line--;
    }
    
    pthread_exit(NULL);
}

void createPlayerThread(void *arg)
{
    
    player.line = LINES-1;
    player.col = COLS/2;
    player.ch = '^';
    int bulletIndex = 0;

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
        else if(inputKeyBoard == KEY_UP)
        {
            pthread_t bulletThread;
            int bullet = pthread_create(&bulletThread, NULL, createBullet, (void *) bulletIndex);
            if(bullet != 0)
            {
                perror("Error al crear el hilo bullet");
                return 1;
            }
            bulletIndex = (bulletIndex + 1) % TOTALBULLET;
        }
    }
    pthread_exit(NULL);

}

int main() {
    initscr(); // Inicializa ncurses
    
    SetScreen();
    
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