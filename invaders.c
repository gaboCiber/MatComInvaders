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
int COLUMNS, ROWS;
pthread_mutex_t mutex;

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
    enemy.col = getRamdomNumberInterval(5, COLUMNS-5);
    enemy.ch = '#';   
    enemy.leftRight = &enemyMovRightLeft;
    enemy.leftRightCount = 3;  
    enemy.upDown = &enemyMoveUpDown;
    enemy.upDownCount = 2;

    mvaddch(enemy.line, enemy.col,enemy.ch);

    pthread_mutex_lock(&mutex);
    refresh();
    pthread_mutex_unlock(&mutex);

    bool insideScreen = 1;

    while (insideScreen)
    {
        mvaddch(enemy.line, enemy.col,' ');
        
        getRandomPos(&enemy);

        mvaddch(enemy.line, enemy.col,enemy.ch);

        insideScreen = enemy.line > 0 && enemy.line < ROWS && enemy.col > 0 && enemy.col < COLUMNS;
        
        pthread_mutex_lock(&mutex);
        refresh();
        pthread_mutex_unlock(&mutex);
        
        usleep(500000);
    }

    pthread_exit(NULL);
}

void SetMotherShip()
{
    mvaddch(0,0,'\\');
    
    mvaddch(1,1,'\\');

    int col = 2;
    while (col < COLUMNS - 1)
    {
        mvaddch(1,col,'_');
        col++;
    }
    
    mvaddch(0,COLUMNS-1,'/');

    mvaddch(1,COLUMNS-2,'/');
    
    refresh();
}

void createMotherShip(void *arg)
{   
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
    bullet.line = ROWS-2;
    bullet.col = player.col;

    while (bullet.line > 1)
    {
        mvaddch(bullet.line, bullet.col,bullet.ch);

        pthread_mutex_lock(&mutex);
        refresh();
        pthread_mutex_unlock(&mutex);
        usleep(50000);

        mvaddch(bullet.line, bullet.col,' ');

        bullet.line--;
    }
    
    pthread_exit(NULL);
}

void createPlayerThread(void *arg)
{    
    player.line = ROWS-1;
    player.col = COLUMNS/2;
    player.ch = '^';
    int bulletIndex = 0;

    mvaddch(player.line, player.col,player.ch);

    int inputKeyBoard;
    while ((inputKeyBoard = getch()) != KEY_F(4) && inputKeyBoard != ERR)
    {
        if( (inputKeyBoard == KEY_RIGHT || inputKeyBoard == KEY_LEFT) )
        {            
            mvaddch(player.line, player.col,' ');

            player.col += (inputKeyBoard == KEY_RIGHT) ? 1 : -1;

            if(player.col <= 0)
                player.col = 1;
            else if (player.col >= COLUMNS)
                player.col = COLUMNS - 1;

            mvaddch(player.line, player.col,player.ch);
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
    pthread_mutex_init(&mutex, 0);

    initscr();
    cbreak();

    getmaxyx(stdscr, ROWS, COLUMNS);
    
    keypad(stdscr, TRUE);
    curs_set(0);

    SetMotherShip();
    pthread_t mothership;
    int ship = pthread_create(&mothership, NULL, createMotherShip, NULL);
    if(ship != 0)
    {
        perror("Error al crear el hilo player");
        return 1;
    }
    
    pthread_t playerThread;
    int player = pthread_create(&playerThread, NULL, createPlayerThread, NULL);
    if(player != 0)
    {
        perror("Error al crear el hilo player");
        return 1;
    }


    pthread_join(playerThread, NULL);
    pthread_join(mothership, NULL); 
    endwin();

    pthread_mutex_destroy(&mutex);
    return 0;
}