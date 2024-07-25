#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "invaderstruct.h"
#include "stdbool.h"

struct Player player;
int COLUMNS, ROWS;
pthread_mutex_t mutex;

const int TOP=10;
struct Enemy *enemyList[10];
bool ocupied[10];
int count = 0;

int getRamdomNumberInterval(int min, int max);
void getRandomPos(struct Enemy *en);
void *createEnemy(void *arg);
void SetMotherShip();
void *createMotherShip(void *arg);
void *createBullet(void *arg);
void *createPlayerThread(void *arg);

int EnemyListInsert(struct Enemy *enemy);
void EnemyListRemove(int index);
bool EnemyListIsOneLeft();
int EnemyListCheckPositions(int line, int col);

int main() 
{
    
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
    

    pthread_mutex_destroy(&mutex);
    return 0;
}

// Player
void *createPlayerThread(void *arg)
{    
    player.line = ROWS-1;
    player.col = COLUMNS/2;
    player.ch = '^';

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
            int bullet = pthread_create(&bulletThread, NULL, createBullet, NULL);
            if(bullet != 0)
            {
                perror("Error al crear el hilo bullet");
                break;
            }
        }
    }
    pthread_exit(NULL);

}

void *createBullet(void *arg)
{   
    struct Bullet bullet;
    bullet.ch = '|';
    bullet.line = ROWS-2;
    bullet.col = player.col;

    while (bullet.line > 1)
    {
        mvaddch(bullet.line, bullet.col,bullet.ch);

        int killEnemy = EnemyListCheckPositions(bullet.line, bullet.col);
        if(killEnemy >= 0)
        {
            EnemyListRemove(killEnemy);
            mvaddch(bullet.line, bullet.col,' ');
            break;
        }

        pthread_mutex_lock(&mutex);
        refresh();
        pthread_mutex_unlock(&mutex);
        usleep(50000);

        mvaddch(bullet.line, bullet.col,' ');

        bullet.line--;
    }
    
    pthread_exit(NULL);
}

// MotherShip

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

void *createMotherShip(void *arg)
{   
    while (true)
    {
        pthread_t enemyThread;       
        int en = pthread_create(&enemyThread, NULL, createEnemy, NULL);
        if(en != 0)
        {
            perror("Error al crear el hilo player");
            break;
        }

        sleep(getRamdomNumberInterval(1,10));
        
    }
    

}


// Enemies
void *createEnemy(void *arg)
{
    if(EnemyListIsOneLeft())
    {
        // Construction
        struct Enemy enemy;
        enemy.line = 3;
        enemy.col = getRamdomNumberInterval(5, COLUMNS-5);
        enemy.indexAtEnemyList = EnemyListInsert(&enemy);
        
        switch (getRamdomNumberInterval(0,2))
        {
        case 0:
            enemy.ch = '#';   
            enemy.leftRight =  (int[3]) {-1, 0 , 1};
            enemy.leftRightCount = 3;  
            enemy.upDown = (int[2]) {0 , 1};
            enemy.upDownCount = 2;
            break;
        case 1:
            enemy.ch = '&';   
            enemy.leftRight =  (int[1]) {0};
            enemy.leftRightCount = 1;  
            enemy.upDown = (int[1]) {1};
            enemy.upDownCount = 1;
            break;
        case 2:
            enemy.ch = '$';   
            enemy.leftRight =  (int[2]) {-2,2};
            enemy.leftRightCount = 2;  
            enemy.upDown = (int[1]) {1};
            enemy.upDownCount = 1;
            break;
        default:
            break;
        }
        
        // On battle
        mvaddch(enemy.line, enemy.col,enemy.ch);
    
        pthread_mutex_lock(&mutex);
        refresh();
        pthread_mutex_unlock(&mutex);

        bool insideScreen = 1;

        while (true)
        {
            mvaddch(enemy.line, enemy.col,' ');
            
            getRandomPos(&enemy);
            
            mvaddch(enemy.line, enemy.col,enemy.ch);
           
            if(enemy.indexAtEnemyList == -1)
            {
                mvaddch(enemy.line, enemy.col,' ');
                break;
            }  

            if(!(enemy.line > 0 && enemy.line < ROWS - 1 && enemy.col > 0 && enemy.col < COLUMNS - 1))
            {
                EnemyListRemove(enemy.indexAtEnemyList);
                mvaddch(enemy.line, enemy.col,' ');
                break;
            }     
            
            pthread_mutex_lock(&mutex);
            refresh();
            pthread_mutex_unlock(&mutex);
            
            usleep(500000);
        }

    
    }
    
    pthread_exit(NULL);

}

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


// EnemyList
int EnemyListInsert(struct Enemy *enemy)
{
    int i = 0;
    for (i = 0; i < 10; i++)
    {
        if(!ocupied[i])
        {
            enemyList[i] = enemy;
            ocupied[i]=true;
            count++;
            return i;
        }
    }

    return -1;
    
}

void EnemyListRemove(int index)
{
    if(index >= 0 && index < count)
    {
        enemyList[index]->indexAtEnemyList = -1;
        ocupied[index] = false;
        count--;   
    }
}

bool EnemyListIsOneLeft()
{
    return count < TOP;
}

int EnemyListCheckPositions(int line, int col)
{
    for (int i = 0; i < count; i++)
    {
        if( enemyList[i]->line == line && enemyList[i]->col == col)
            return i;
    }

    return -1;
    
}
