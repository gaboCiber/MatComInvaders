#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "invaderstruct.h"

const int TOTALBULLET = 10;
struct Player player;
struct Bullet bulletArr[10];
int enemyMovRightLeft[] = {-1, 0 , 1};
int enemyMoveUpDown[] = {0 , 1};

int getRamdomNumberInterval(int min, int max)
{
    return min + rand() % (max + 1 - min);
}

void getRandomPos(struct Enemy *en)
{
    int random = rand();
    en->col += *(en->leftRight + (rand() % en->leftRightCount));
    en->line += *(en->upDown + (rand() % en->upDownCount));
}

void createEnemy(void *arg)
{
    struct Enemy enemy;
    enemy.line = 3;
    enemy.col = getRamdomNumberInterval(5, COLS-5);
    enemy.ch = '#';   
    enemy.leftRight = &enemyMovRightLeft;
    enemy.leftRightCount = 3;  
    enemy.upDown = &enemyMoveUpDown;
    enemy.upDownCount = 2;

    move(enemy.line, enemy.col);
    addch(enemy.ch);

    refresh();

    bool insideScreen = 1;

    while (insideScreen)
    {
        move(enemy.line, enemy.col);
        addch(' ');
        
        getRandomPos(&enemy);

        move(enemy.line, enemy.col);
        addch(enemy.ch);

        insideScreen = enemy.line > 0 && enemy.line < LINES && enemy.col > 0 && enemy.col < COLS;
        refresh();
        sleep(1);
    }

    pthread_exit(NULL);
}

void createMotherShip(void *arg)
{   
    move(0,0);
    addch('\\');
    
    move(1,1);
    addch('\\');

    int col = 2;
    while (col < COLS - 1)
    {
        move(1,col);
        addch('_');
        col++;
    }
    move(0,COLS-1);
    addch('/');

    move(1,COLS-2);
    addch('/');
    
    refresh();

    while (true)
    {
        sleep(getRamdomNumberInterval(1,10));
        pthread_t enemyThread;       
        int enemy = pthread_create(&enemyThread, NULL, createEnemy, NULL);
        if(enemy != 0)
        {
            perror("Error al crear el hilo player");
            return 1;
        }
    }
    

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
    srand(time(0));
    
    initscr(); // Inicializa ncurses
    keypad(stdscr, TRUE);
    curs_set(0);

    /*pthread_t mothership;
    int ship = pthread_create(&mothership, NULL, createMotherShip, NULL);
    if(ship != 0)
    {
        perror("Error al crear el hilo player");
        return 1;
    }*/
    
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