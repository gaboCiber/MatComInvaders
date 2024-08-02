#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "invaderstruct.h"
#include "stdbool.h"
#include <SDL2/SDL.h>


// General
int COLUMNS, ROWS;
pthread_mutex_t mutex;
pthread_attr_t threadDetachedAttr;
bool screenOnPause = false;
bool gameClose = false;
bool levelUp = false;
int playerLevel = 1;

int getRamdomNumberInterval(int min, int max);
void getRandomPos(struct Enemy *en);
void screenRefresh();
void gameOver(int i);
void gameStart();


// Player 
struct Player player;
void *createBullet(void *arg);
void *createPlayerThread(void *arg);
//void bulletSound();

// MotherShip and Enemies 
void setMotherShip();
void *createMotherShipThread(void *arg);
void *createEnemyThread(void *arg);


// EnemyList 
struct FreeEnemyBlock rootBlock;
int numberOfFreeBlock;
const int totalNumberOfEnemiesOnBattle=10;
struct Enemy *enemyList[10];
int numberOfEnemiesOnBattle;
void EnemyListInsert(struct Enemy *enemy);
void EnemyListRemove(int index);
bool EnemyListIsOneLeft();
int EnemyListCheckPositions(int line, int col);
void EnemyListEraseAllBlocksFromMemory();

// Hangar 
struct HangarNode hangarRoot;
int totalEnemiesOnThisLevel;
int enemiesInConstruction = 0;
int leafPriority = 0;
const int PRIORITY = 2;
void HangarInsert(struct HangarNode *newEnemy);
int HangarBuild();
void desingEnemy(struct HangarNode* en);
void destroyUnbuildEnemies();

// ------------------------------------------------------------------------------------//

int main() 
{
    
    srand(time(0));

    pthread_mutex_init(&mutex, 0);
    pthread_attr_init(&threadDetachedAttr);
    pthread_attr_setdetachstate(&threadDetachedAttr, PTHREAD_CREATE_DETACHED);

    initscr();
    cbreak();

    getmaxyx(stdscr, ROWS, COLUMNS);
    
    keypad(stdscr, TRUE);
    curs_set(0);

    gameStart();

    pthread_mutex_destroy(&mutex);
    pthread_attr_destroy(&threadDetachedAttr);

    endwin();
    return 0;
}

// ------------------------------------------------------------------------------------//

// Player
void *createPlayerThread(void *arg)
{    
    mvaddch(player.line, player.col,player.ch);

    int inputKeyBoard;
    while ((inputKeyBoard = getch()) != KEY_F(4) && inputKeyBoard != ERR && !gameClose)
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
            int bullet = pthread_create(&bulletThread, &threadDetachedAttr, createBullet, NULL);
            if(bullet != 0)
            {
                perror("Error al crear el hilo bullet");
                break;
            }

            // pthread_mutex_lock(&mutex);
            // bulletSound();
            // pthread_mutex_unlock(&mutex);

        }
        else if(inputKeyBoard == 'p')
        {
            pthread_mutex_lock(&mutex);
            
            clear();
            mvaddstr(LINES/2 - 1, COLUMNS/2, "PAUSE");
            mvaddstr(LINES/2, COLUMNS/2 - 10, "Press any key to continue");
            screenOnPause = true;
            refresh();

            getch();
            
            clear();
            setMotherShip();
            screenOnPause = false;
            refresh();
            
            pthread_mutex_unlock(&mutex);
        }
        else if(inputKeyBoard == 'q')
        {
            pthread_mutex_lock(&mutex);
            gameOver(0);
            pthread_mutex_unlock(&mutex);

            break;
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

    while (bullet.line > 1 && !gameClose)
    {
        mvaddch(bullet.line, bullet.col,bullet.ch);

        int killEnemy = EnemyListCheckPositions(bullet.line, bullet.col);
        if(killEnemy >= 0)
        {
            mvaddch(bullet.line, bullet.col,' ');
            EnemyListRemove(killEnemy);
            break;
        }

        pthread_mutex_lock(&mutex);       
        screenRefresh();
        pthread_mutex_unlock(&mutex);
        
        usleep(50000);

        mvaddch(bullet.line, bullet.col,' ');

        bullet.line--;
    }
    
    pthread_exit(NULL);
}
/*
void bulletSound()
{
    SDL_Init(SDL_INIT_AUDIO);
    
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8 *wavBuffer;

    SDL_LoadWAV("../src/laser-zap-90575.wav", &wavSpec, &wavBuffer, &wavLength);

    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

    int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
    SDL_PauseAudioDevice(deviceId, 0);

    SDL_Delay(5000);

    SDL_CloseAudioDevice(deviceId);
    SDL_FreeWAV(wavBuffer);
    SDL_Quit();

}
*/

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
    int totalNumberOfEnemiesInHangar = 0;
    while (!gameClose)
    {
        if(numberOfEnemiesOnBattle == 0 && totalNumberOfEnemiesInHangar == totalEnemiesOnThisLevel)
        {
            pthread_mutex_lock(&mutex);
            gameOver(1);
            pthread_mutex_unlock(&mutex);

            break;
        }

        if(EnemyListIsOneLeft() && !screenOnPause)
        {
            if(totalNumberOfEnemiesInHangar < totalEnemiesOnThisLevel)
            {
                struct HangarNode *hangar = (struct HangarNode*) malloc(sizeof(struct HangarNode));
                desingEnemy(hangar);
                HangarInsert(hangar);
                totalNumberOfEnemiesInHangar++;
            }

            int model = HangarBuild();
            
            if(model >= 0)
            {
                struct EnemyToBield enemy;
                enemy.shipModel = model;
                enemy.isOnBattle = false;
                
                pthread_t enemyThread;       
                int en = pthread_create(&enemyThread, &threadDetachedAttr, createEnemyThread, (void *) &enemy);
                if(en != 0)
                {
                    perror("Error al crear el hilo player");
                    break;
                }
            }

            usleep(3000000);        
        }
              
    }

    pthread_exit(NULL);
    
}

void *createEnemyThread(void *arg)
{
    struct EnemyToBield *toBield = (struct EnemyToBield*) arg;

    // Construction
    struct Enemy *enemy = (struct Enemy *) malloc(sizeof(struct Enemy));
    
    if(toBield->isOnBattle)
    {
        enemy = enemyList[toBield->indexAtEnemyList];
    }
    else
    {
        enemy->line = 3;
        enemy->col = getRamdomNumberInterval(5, COLUMNS-5);
        enemy->shipModel = toBield->shipModel;
        EnemyListInsert(enemy);
    }   
    
    switch ( enemy->shipModel )
    {
        case 0:
            enemy->ch = '&';   
            enemy->leftRight =  (int[1]) {0};
            enemy->leftRightCount = 1;  
            enemy->upDown = (int[1]) {1};
            enemy->upDownCount = 1;
            break;
        case 1:
            enemy->ch = '#';   
            enemy->leftRight =  (int[3]) {-1, 0 , 1};
            enemy->leftRightCount = 3;  
            enemy->upDown = (int[2]) {0 , 1};
            enemy->upDownCount = 2;
            break;
        case 2:
            enemy->ch = '$';   
            enemy->leftRight =  (int[2]) {-2,2};
            enemy->leftRightCount = 2;  
            enemy->upDown = (int[1]) {1};
            enemy->upDownCount = 1;
            break;
        default:
            break;
    }
    
    // On battle
    mvaddch(enemy->line, enemy->col,enemy->ch);

    pthread_mutex_lock(&mutex);
    screenRefresh();
    pthread_mutex_unlock(&mutex);

    bool insideScreen = 1;

    while (!gameClose)
    {
        mvaddch(enemy->line, enemy->col,' ');
        
        getRandomPos(enemy);
        
        mvaddch(enemy->line, enemy->col,enemy->ch);
        
        if(enemy->indexAtEnemyList == -1)
        {
            mvaddch(enemy->line, enemy->col,' ');
            break;
        }  

        if(enemy->line > LINES - 2 )
        {
            pthread_mutex_lock(&mutex);           
            player.hp -= enemy->shipModel + 1;

            if(player.hp <= 0)
            {
                gameOver(-1);
                break;   
            }
            pthread_mutex_unlock(&mutex);
        }

        
        if(!(enemy->line > 0 && enemy->line < ROWS - 1 && enemy->col > 0 && enemy->col < COLUMNS - 1))
        {
            pthread_mutex_lock(&mutex);           
            mvaddch(enemy->line, enemy->col,' ');
            EnemyListRemove(enemy->indexAtEnemyList);
            free(enemy);
            
            pthread_mutex_unlock(&mutex);
            break;
        }     
        
        pthread_mutex_lock(&mutex);
        screenRefresh();
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
        if(newEnemy->buildTime < iterator->next->buildTime)
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

int HangarBuild()
{    
    if(enemiesInConstruction > 0)
    {
        if(leafPriority == PRIORITY && enemiesInConstruction > 1)
        {
            struct HangarNode *iterator = &hangarRoot;

            for(int count = 0; count < enemiesInConstruction - 1; count++)
            {
                iterator = iterator->next;
            }

            iterator->next->buildTime--;
            struct HangarNode *boost = iterator->next;
            iterator->next = NULL;
            enemiesInConstruction--;
            leafPriority = -1;
            HangarInsert(boost);
            
        }
        
        if(hangarRoot.next->buildTime <= 1)
        {
            struct HangarNode *builtEnemy = hangarRoot.next;
            int model = builtEnemy->shipModel;
            hangarRoot.next = builtEnemy->next;
            enemiesInConstruction--;
            free(builtEnemy);
            return model;
        }
        else
        {
            hangarRoot.next->buildTime--;     
        }
        
        
    }   
    
    leafPriority++;
    return -1;
}

void desingEnemy(struct HangarNode* en)
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
            en->buildTime = 3;
            break;
        default:
            break;
    }
}

void destroyUnbuildEnemies()
{
    struct HangarNode *iterator = hangarRoot.next;
    struct HangarNode *toDestroy;
    for (int count = 0; count < enemiesInConstruction; count++)
    {
        toDestroy = iterator;
        iterator = iterator->next;
        free(toDestroy);
    }
}

// EnemyList
void EnemyListInsert(struct Enemy *enemy)
{
    if(numberOfEnemiesOnBattle < totalNumberOfEnemiesOnBattle)
    {
        enemyList[rootBlock.next->index] = enemy;
        enemy->indexAtEnemyList = rootBlock.next->index;

        if(rootBlock.next->length == 1)
        {
            struct FreeEnemyBlock *blockToErase = rootBlock.next;
            rootBlock.next = (numberOfFreeBlock > 1) ? rootBlock.next->next : NULL;
            numberOfFreeBlock--;
            free(blockToErase);
        }
        else
        {
            rootBlock.next->index++;
            rootBlock.next->length--;
        }

        numberOfEnemiesOnBattle++;
    }
    
}

void EnemyListRemove(int index)
{
    if(numberOfEnemiesOnBattle > 0 && index >= 0 && index < totalNumberOfEnemiesOnBattle)
    {
        struct FreeEnemyBlock *newBlock = (struct FreeEnemyBlock *) malloc(sizeof(struct FreeEnemyBlock));
        newBlock->index = index;
        newBlock->length = 1;

        enemyList[index]->indexAtEnemyList = -1;
        enemyList[index] = NULL;

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
                    free(newBlock);
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
                    
                    struct FreeEnemyBlock *fbAfter = iterator->next;
                    iterator->next = fbAfter->next;
                    numberOfFreeBlock--;

                    free(newBlock);
                    free(fbAfter);
                }
                else if(before)
                {
                    iterator->length++;
                    free(newBlock);
                }
                else if (after)
                {
                    iterator->next->index--;
                    iterator->next->length++;
                    free(newBlock);
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

        numberOfEnemiesOnBattle--;  
    }

}

bool EnemyListIsOneLeft()
{
    return numberOfEnemiesOnBattle < totalNumberOfEnemiesOnBattle;
}

int EnemyListCheckPositions(int line, int col)
{
    struct FreeEnemyBlock *iterator = &rootBlock;
    int count = 0;
    for (int i = 0; i < totalNumberOfEnemiesOnBattle; i++)
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

void EnemyListEraseAllBlocksFromMemory()
{
    struct FreeEnemyBlock *iterator = rootBlock.next;
    for (int i = 0; i < totalNumberOfEnemiesOnBattle; i++)
    {
        if(numberOfFreeBlock > 0 && i == iterator->index)
        {
            i+= iterator->length - 1;
            struct FreeEnemyBlock *toErase = iterator;
            iterator = iterator->next;
            free(toErase);
            numberOfFreeBlock--;
            continue;
        }

        free(enemyList[i]);
        enemyList[i] = NULL;
    }

    rootBlock.next = NULL;
    numberOfEnemiesOnBattle = 0;

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

void screenRefresh()
{
    char hp[12], thp[12], level[12], totalEnemy[12];
    sprintf(hp, "%d", player.hp);
    sprintf(thp, "%d", player.totalHp);
    sprintf(level, "%d", playerLevel);
    sprintf(totalEnemy, "%d", totalEnemiesOnThisLevel);


    setMotherShip();
    mvaddstr(0, 3, "Level:");
    mvaddstr(0, 10, level);
    mvaddstr(0, COLUMNS/2 - 11, "Player HP: ");
    mvaddstr(0, COLUMNS/2, "        ");
    mvaddstr(0, COLUMNS/2, hp);
    mvaddstr(0, COLUMNS/2 + 2, "/");
    mvaddstr(0, COLUMNS/2 + 3,thp);
    mvaddstr(0, COLUMNS -15, "Enemies: ");
    mvaddstr(0, COLUMNS - 5, totalEnemy);
    refresh();
}

void gameStart()
{
    do 
    {
        clear();
        refresh();
        screenOnPause = false;
        gameClose = false;
        levelUp = false;
    
        // Set Memory for Enemies
        rootBlock.index = -2;
        rootBlock.length = -2;

        struct FreeEnemyBlock *initial = (struct FreeEnemyBlock *) malloc(sizeof(struct FreeEnemyBlock));
        initial->index = 0;
        initial->length = totalNumberOfEnemiesOnBattle;

        rootBlock.next = initial;
        numberOfFreeBlock = 1;

        // Set MotherSip
        setMotherShip();
        totalEnemiesOnThisLevel = (10 * playerLevel);
        pthread_t mothership;
        int ship = pthread_create(&mothership, NULL, createMotherShipThread, NULL);
        if(ship != 0)
        {
            perror("Error al crear el hilo player");
            return;
        }
        
        // Set Player
        player.line = ROWS-1;
        player.col = COLUMNS/2;
        player.ch = '^';
        player.hp = player.totalHp = 10 + (playerLevel * 5);
        pthread_t playerThread;
        int player = pthread_create(&playerThread, NULL, createPlayerThread, NULL);
        if(player != 0)
        {
            perror("Error al crear el hilo player");
            return;
        }

        //screenRefresh();

        pthread_join(playerThread, NULL);
        pthread_join(mothership, NULL); 
        EnemyListEraseAllBlocksFromMemory();
    } while (levelUp);
    

    
}

void gameOver(int i)
{
    clear();
    gameClose = true;
    levelUp = false;

    switch (i)
    {
        case -1:
            mvaddstr(LINES/2 - 1, COLUMNS/2 - 5, "YOU LOSE");
            break;
        case 0:
            mvaddstr(LINES/2 - 1, COLUMNS/2 - 5, "SEE YOU");
            break;
        case 1:
            mvaddstr(LINES/2 - 1, COLUMNS/2 - 5, "YOU WIN");
            levelUp = true;
            playerLevel++;
            break;
        default:
            break;
    }

    refresh();
    sleep(1);
}