#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "invaderstruct.h"
#include "stdbool.h"

// General
int COLUMNS, ROWS;
pthread_mutex_t mutex;

int getRamdomNumberInterval(int min, int max);
void getRandomPos(struct Enemy *en);

// Player 
struct Player player;
void *createBullet(void *arg);
void *createPlayerThread(void *arg);

// MotherShip and Enemies 
void setMotherShip();
void *createMotherShipThread(void *arg);
void desingEnemy(struct EnemyDesing* en);
void *createEnemy(void *arg);


// EnemyList 
struct FreeEnemyBlock rootBlock;
int numberOfFreeBlock;
const int TOP=10;
struct Enemy *enemyList[10];
void EnemyListInsert(struct Enemy *enemy);
void EnemyListRemove(struct FreeEnemyBlock *newBlock);
bool EnemyListIsOneLeft();
int EnemyListCheckPositions(int line, int col);

// Hangar 
struct HangarNode hangarRoot;
struct EnemyDesing *enemyShip;
int enemiesInConstruction = 0;
bool enemyReadyToBattle = false;
void HangarInsert(struct HangarNode *newEnemy);
struct EnemyDesing *HangarBuild();

// ------------------------------------------------------------------------------------//

int main() 
{
    
    srand(time(0));
    pthread_mutex_init(&mutex, 0);

    initscr();
    cbreak();

    getmaxyx(stdscr, ROWS, COLUMNS);
    
    keypad(stdscr, TRUE);
    curs_set(0);

    // Set Memory For Enemies
    rootBlock.index = -2;
    rootBlock.length = -2;

    struct FreeEnemyBlock initial;
    initial.index = 0;
    initial.length = TOP;

    rootBlock.next = &initial;
    numberOfFreeBlock = 1;

    // Create MotherShip and Enemies

    setMotherShip();
    pthread_t mothership;
    int ship = pthread_create(&mothership, NULL, createMotherShipThread, NULL);
    if(ship != 0)
    {
        perror("Error al crear el hilo player");
        return 1;
    }
    
    // Create Player
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

// ------------------------------------------------------------------------------------//

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
            struct FreeEnemyBlock free;
            free.index = killEnemy;
            free.length = 1;
            EnemyListRemove(&free);
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

// MotherShip and Enemies

void setMotherShip()
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

void *createMotherShipThread(void *arg)
{   
    while (true)
    {
        if(EnemyListIsOneLeft())
        {
            struct EnemyDesing newEnemy;
            desingEnemy(&newEnemy);

            struct HangarNode newShip;
            newShip.ship = &newEnemy;
            HangarInsert(&newShip);
            enemyShip = HangarBuild();
            
            if(enemyReadyToBattle)
            {
                pthread_t enemyThread;       
                int en = pthread_create(&enemyThread, NULL, createEnemy, (void *) (intptr_t) enemyShip->shipModel);
                if(en != 0)
                {
                    perror("Error al crear el hilo player");
                    break;
                }

                enemyReadyToBattle = false;
            }

            usleep(1000000);
           
        }
              
    }
    

}

void desingEnemy(struct EnemyDesing* en)
{ 
    switch (getRamdomNumberInterval(0,2))
    {
        case 0:
            en->shipModel = 0;
            en->buildTime = 1;
            break;
        case 1:
            en->shipModel = 1;
            en->buildTime = 2;
            break;
        case 2:
            en->shipModel = 2;
            en->buildTime = 4;
            break;
        default:
            break;
    }
}

void *createEnemy(void *arg)
{
    // Construction
    struct Enemy enemy;
    enemy.line = 3;
    enemy.col = getRamdomNumberInterval(5, COLUMNS-5);

    switch ( (int) (intptr_t) arg )
    {
        case 0:
            enemy.ch = '&';   
            enemy.leftRight =  (int[1]) {0};
            enemy.leftRightCount = 1;  
            enemy.upDown = (int[1]) {1};
            enemy.upDownCount = 1;
            break;
        case 1:
            enemy.ch = '#';   
            enemy.leftRight =  (int[3]) {-1, 0 , 1};
            enemy.leftRightCount = 3;  
            enemy.upDown = (int[2]) {0 , 1};
            enemy.upDownCount = 2;
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
    
    EnemyListInsert(&enemy);

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
            struct FreeEnemyBlock free;
            free.index = enemy.indexAtEnemyList;
            free.length = 1;
            EnemyListRemove(&free);
            mvaddch(enemy.line, enemy.col,' ');
            break;
        }     
        
        pthread_mutex_lock(&mutex);
        refresh();
        pthread_mutex_unlock(&mutex);
        
        usleep(500000);
    }

    pthread_exit(NULL);

}

// Hangar
void HangarInsert(struct HangarNode *newEnemy)
{
    struct HangarNode *iterator = &hangarRoot;
    int count = 0;

    while (count < enemiesInConstruction)
    {
        if(newEnemy->ship->buildTime < iterator->next->ship->buildTime)
        {
            newEnemy->next = iterator->next;
            break;
        }

        iterator = iterator->next;
        count++;
    }

    iterator->next = newEnemy;
    enemiesInConstruction++;
}

struct EnemyDesing *HangarBuild()
{
    struct EnemyDesing *enemyToReturn;
    enemyReadyToBattle = false;
    
    if(enemiesInConstruction > 0)
    {

        if(hangarRoot.next->ship->buildTime <= 1)
        {
            enemyToReturn = hangarRoot.next->ship;
            hangarRoot.next = hangarRoot.next->next;
            enemiesInConstruction--;
            enemyReadyToBattle = true;
        }
        else
        {
            hangarRoot.next->ship->buildTime--;     
        }
        
        
    }   
    
    return enemyToReturn;
}

// EnemyList
void EnemyListInsert(struct Enemy *enemy)
{
    if(numberOfFreeBlock > 0)
    {
        enemyList[rootBlock.next->index] = enemy;
        enemy->indexAtEnemyList = rootBlock.next->index;

        if(rootBlock.next->length == 1)
        {
            rootBlock.next = (numberOfFreeBlock > 1) ? rootBlock.next->next : NULL;
            numberOfFreeBlock--;
        }
        else
        {
            rootBlock.next->index++;
            rootBlock.next->length--;
        }
    }
    
}

void EnemyListRemove(struct FreeEnemyBlock *newBlock)
{
    if(newBlock->index >= 0 && newBlock->index < TOP)
    {
        enemyList[newBlock->index]->indexAtEnemyList = -1;

        int count = 0;
        struct FreeEnemyBlock * iterator = &rootBlock;
        
        while (true)
        {
            bool before = iterator->index + iterator->length == newBlock->index;
            
            if(count == numberOfFreeBlock)
            {
                if(before)
                {
                    iterator->length++;
                }
                else
                {
                    iterator->next = newBlock; 
                    numberOfFreeBlock++; 
                }

                break;
            }
            else if(newBlock->index < iterator->next->index)
            {
                bool after = newBlock->index + 1 == iterator->next->index;

                if(before && after)
                {
                    iterator->length += 1 + iterator->next->length;
                    iterator->next = iterator->next->next;
                    numberOfFreeBlock--;
                }
                else if(before)
                {
                    iterator->length++;
                }
                else if (after)
                {
                    iterator->next->index--;
                    iterator->next->length++;
                }
                else
                {
                    newBlock->next = iterator->next;
                    iterator->next = newBlock;
                    numberOfFreeBlock++;
                } 

                break; 
            }   

            iterator = iterator->next;
            count++;        
        }    
    }

}

bool EnemyListIsOneLeft()
{
    return numberOfFreeBlock > 0;
}

int EnemyListCheckPositions(int line, int col)
{
    struct FreeEnemyBlock *iterator = &rootBlock;
    int count = 0;
    for (int i = 0; i < TOP; i++)
    {
        if(count < numberOfFreeBlock && i == iterator->next->index)
        {
            i += iterator->next->length - 1;
            iterator = iterator->next;
            count++;
            continue;
        }
        
        if( enemyList[i]->line == line && enemyList[i]->col == col)
            return i;
    }

    return -1;
    
}


// General
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

