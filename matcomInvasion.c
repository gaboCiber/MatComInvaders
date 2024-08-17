#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "invaderstruct.h"
#include "stdbool.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include "audio/backgroundSound.h"
#include "audio/laser.h"


// General
int COLUMNS, ROWS;
pthread_mutex_t mutex;
pthread_attr_t threadDetachedAttr;
bool screenOnPause = false;
bool gameClose = false;
bool levelUp = false;
int playerLevel = 1;
bool saveGame;

int getRamdomNumberInterval(int min, int max);
void screenRefresh();
void gameOver(int i);
void gameStart();

// Player 
struct Player player;
void *createBullet(void *arg);
void *createPlayerThread(void *arg);


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

// Load and Save Game
int playerHPSave;
bool FileSaveEnemyList(const char *fileToWrite);
void FileWhiteSpaceRemove(char *line);
bool FileConvertStringToInt(char *str, int *num);
bool FileLoadEnemyList(const char *fileToRead);
void FileGetRoute(char *route);
void SaveGame();

// Sound
int chanel;
void* playBulletSound(void * arg);
void* playBackgroundSound(void* arg);


// ------------------------------------------------------------------------------------//

//General

int main(int argc, char *argv[]) 
{
    if(argc > 2)
    {
        perror("Too many argmuments");
        return -1;
    }

    srand(time(0));

    pthread_mutex_init(&mutex, 0);
    pthread_attr_init(&threadDetachedAttr);
    pthread_attr_setdetachstate(&threadDetachedAttr, PTHREAD_CREATE_DETACHED);


    initscr();
    cbreak();

    getmaxyx(stdscr, ROWS, COLUMNS);
    
    keypad(stdscr, TRUE);
    curs_set(0);

    if(argc == 2)
    {
        FileLoadEnemyList(argv[1]);
        saveGame = true;
    }
    else
    {
        saveGame = false;
    }

     if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Error initializing SDL_mixer: %s\n", Mix_GetError());
        return -1;
    }


    gameStart();

    pthread_mutex_destroy(&mutex);
    pthread_attr_destroy(&threadDetachedAttr);

    endwin();
    return 0;
}

int getRamdomNumberInterval(int min, int max)
{
    return min + rand() % (max + 1 - min);
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
    
        // Set Player
        player.line = ROWS-1;
        player.col = COLUMNS/2;
        player.ch = '^';
        player.hp = player.totalHp = (saveGame) ? playerHPSave : 10 + (playerLevel * 5);
        pthread_t playerThread;
        int player = pthread_create(&playerThread, NULL, createPlayerThread, NULL);
        if(player != 0)
        {
            perror("Error al crear el hilo player");
            return;
        }
        
        // Set Memory for Enemies
        if(!saveGame)
        {
            rootBlock.index = -2;
            rootBlock.length = -2;

            struct FreeEnemyBlock *initial = (struct FreeEnemyBlock *) malloc(sizeof(struct FreeEnemyBlock));
            initial->index = 0;
            initial->length = totalNumberOfEnemiesOnBattle;

            rootBlock.next = initial;
            numberOfFreeBlock = 1;
        }
        
        saveGame = false;

        // Set MotherSip
        setMotherShip();
        totalEnemiesOnThisLevel = (10 * playerLevel);
        pthread_t mothershipThread;
        int ship = pthread_create(&mothershipThread, NULL, createMotherShipThread, NULL);
        if(ship != 0)
        {
            perror("Error al crear el hilo motherShip");
            return;
        }

        pthread_t soundThread;
        int sound = pthread_create(&soundThread, NULL, playBackgroundSound, NULL );
        if(sound != 0)
        {
            perror("Error al crear el hilo sound");
            return;
        }

        pthread_join(playerThread, NULL);
        pthread_join(mothershipThread, NULL); 
        pthread_join(soundThread, NULL); 
        EnemyListEraseAllBlocksFromMemory();
    } while (levelUp);
    

    
}

void gameOver(int i)
{
    clear();
    refresh();

    gameClose = true;
    levelUp = false;

    switch (i)
    {
        case -1:
            mvaddstr(LINES/2 - 1, COLUMNS/2 - 5, "YOU LOSE");
            break;
        case 0:
            SaveGame();
            clear();
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

// Player

void *createPlayerThread(void *arg)
{    
    clock_t bulletFire = clock();
    clock_t bulletRecall;
    double elapsedTime;

    mvaddch(player.line, player.col,player.ch);

    int inputKeyBoard;
    while((inputKeyBoard = getch()) != KEY_F(4) && inputKeyBoard != ERR && !gameClose)
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
            bulletRecall = clock();
            elapsedTime = ( (double) bulletRecall - bulletFire );
            if( elapsedTime >= 5000)
            {
                bulletFire = clock();
                pthread_t bulletThread, bulletSound;
                
                int bullet = pthread_create(&bulletThread, &threadDetachedAttr, createBullet, NULL);
                if(bullet != 0)
                {
                    perror("Error al crear el hilo bullet");
                    break;
                }

                int sound = pthread_create(&bulletSound, &threadDetachedAttr, playBulletSound, NULL);
                if(sound != 0)
                {
                    perror("Error al crear el hilo sound");
                    break;
                }
            }
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
                struct EnemyToBield* toBield = (struct EnemyToBield *) malloc(sizeof(struct EnemyToBield));
                toBield->col = getRamdomNumberInterval(5, COLUMNS-5);
                toBield->shipModel = model;
                toBield->indexAtEnemyList = -1;
                
                pthread_t enemyThread;       
                int en = pthread_create(&enemyThread, &threadDetachedAttr, createEnemyThread, (void *) toBield);
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
    enemy->line = toBield->line;
    enemy->col = toBield->col;
    enemy->shipModel = toBield->shipModel;
    enemy->indexAtEnemyList = toBield->indexAtEnemyList;
    enemy->movementIndex = 0;
    free(toBield);
    EnemyListInsert(enemy);
    
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
            enemy->ch = '$';   
            enemy->leftRight =  (int[2]) {-2,2};
            enemy->leftRightCount = 2;  
            enemy->upDown = (int[1]) {1};
            enemy->upDownCount = 1;
            break;
        case 2:
            enemy->ch = '#';   
            enemy->leftRight = (int[3]) {-1, 0, 1};
            enemy->leftRightCount = 3;  
            enemy->upDown = (int[2]) {1, 0};
            enemy->upDownCount = 2;
            break;
        default:
            break;
    }
    
    // On battle
    mvaddch(enemy->line, enemy->col,enemy->ch);

    pthread_mutex_lock(&mutex);
    screenRefresh();
    pthread_mutex_unlock(&mutex);

    while (!gameClose)
    {
        
        mvaddch(enemy->line, enemy->col,' ');
        enemy->line += enemy->upDown[enemy->movementIndex % enemy->upDownCount];
        enemy->col += enemy->leftRight[enemy->movementIndex % enemy->leftRightCount];
        enemy->movementIndex++;
        
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
    if(enemy->indexAtEnemyList >= 0 && enemy->indexAtEnemyList < totalNumberOfEnemiesOnBattle)
    {
        enemyList[enemy->indexAtEnemyList] = enemy;
    }
    else if(numberOfEnemiesOnBattle < totalNumberOfEnemiesOnBattle)
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


// Load and Save Game
bool FileSaveEnemyList(const char *fileToWrite)
{
    FILE *file = fopen(fileToWrite, "w");
    if(file == NULL)
    {
        printf("Error al crear el archivo\n");
        return false;
    }

    fprintf(file, "playerLevel: %d\n", playerLevel); 
    fprintf(file, "playerHP: %d\n", player.hp);
    
    fprintf(file, "numberOfFreeBlock: %d\n", numberOfFreeBlock);
    struct FreeEnemyBlock *iterator = &rootBlock;
    for (int i = 0; i <= numberOfFreeBlock; i++)
    {
        fprintf(file, "[\n");
        fprintf(file, "\tindex: %d\n", iterator->index);
        fprintf(file, "\tlength: %d\n", iterator->length);
        fprintf(file, "]\n");

        iterator = iterator->next;
    }
    
    fprintf(file, "numberOfEnemiesOnBattle: %d\n",numberOfEnemiesOnBattle);
    for (int i = 0; i < totalNumberOfEnemiesOnBattle; i++)
    {
        if(enemyList[i] != NULL)
        {
            fprintf(file, "[\n");
            fprintf(file, "\tcol: %d\n", enemyList[i]->col);
            fprintf(file, "\tline: %d\n", enemyList[i]->line);
            fprintf(file, "\tshipModel: %d\n", enemyList[i]->shipModel);
            fprintf(file, "\tindexAtEnemyList: %d\n", enemyList[i]->indexAtEnemyList);
            fprintf(file, "]\n");
        }
    }

    fclose(file);
    return true;

}

void FileWhiteSpaceRemove(char *line)
{
    int length = strlen(line);
    char result[length + 1];

    int j = 0;
    for (int i = 0; i < length - 1; i++)
    {
        if(line[i] != ' ' && line[i] != '\n' && line[i] != '\t')
            result[j++] = line[i];
    }

    while (j <= length)
    {
        result[j++] = '\0';
    }
    
    strcpy(line, result);
}

bool FileConvertStringToInt(char *str, int *num)
{
    char *endptr;
    long int number = strtol(str, &endptr, 10);

    if(*endptr == '\0')
    {
        *num = (int) number;
        return true;
    }
    
    return false;
}

bool FileLoadEnemyList(const char *fileToRead)
{
    FILE *file = fopen(fileToRead, "r");
    if(file == NULL)
    {
        printf("Error al crear el archivo\n");
        return false;
    }

    char line[100];
    
    bool loadingEnemy = false;
    int realNumberOfEnemies = 0;
    struct EnemyToBield *toBield;


    bool loadingBlock = false;
    int realNumberOfBlock = 0;
    struct FreeEnemyBlock *block = NULL, *save = NULL;
    
    while(fgets(line, sizeof(line), file) != NULL)
    {
        FileWhiteSpaceRemove(line);
        char  *propertyName, *propertyNumber;
        
        propertyName = strtok(line, ":"); 
        propertyNumber = strtok(NULL, ":");  
        
        int num;


        if(propertyName == "\n" || propertyName == NULL)
            continue;

        if(propertyNumber != NULL && !FileConvertStringToInt(propertyNumber, &num))
            return false;

        
        if(strcmp(propertyName, "[") == 0)
        {
            if(loadingEnemy)
                toBield = (struct EnemyToBield *) malloc(sizeof(struct EnemyToBield));
            else if(loadingBlock)
            {
                if(save != NULL)
                {
                    block = (struct FreeEnemyBlock *) malloc(sizeof(struct FreeEnemyBlock));
                    save->next = block; 
                }
                else
                    block = &rootBlock;   
            }
            else 
                return false;

            
            continue;
        }
        else if(strcmp(propertyName, "]") == 0)
        {
            if(loadingEnemy)
            {
                realNumberOfEnemies++;
                pthread_t enemyThread;       
                int en = pthread_create(&enemyThread, &threadDetachedAttr, createEnemyThread, (void *) toBield);
                if(en != 0)
                {
                    perror("Error al crear el hilo player");
                    break;
                }
            }
            else
            {
                save = block;
                realNumberOfBlock++;
            }
        }
        else if(strcmp(propertyName, "playerLevel") == 0)
        {
            playerLevel = num;
        }
        else if(strcmp(propertyName, "playerHP") == 0)
        {
            playerHPSave = num;
        }
        else if(strcmp(propertyName, "numberOfEnemiesOnBattle") == 0)
        {
            loadingEnemy = true;
            loadingBlock = false;
            numberOfEnemiesOnBattle = num;
        }
        else if(strcmp(propertyName, "col") == 0 )
        {
            toBield->col = num;
        }
        else if(strcmp(propertyName, "line") == 0)
        {
            toBield->line = num;
        }
        else if(strcmp(propertyName, "shipModel") == 0)
        {
            toBield->shipModel = num;
        }
        else if(strcmp(propertyName, "indexAtEnemyList") == 0)
        {
            toBield->indexAtEnemyList = num;
        }
        else if(strcmp(propertyName, "numberOfFreeBlock") == 0)
        {
            numberOfFreeBlock = num;
            loadingBlock = true;
            loadingEnemy = false;
        }
        else if(strcmp(propertyName, "index") == 0)
        {
            block->index = num;
        }
        else if(strcmp(propertyName, "length") == 0)
        {
            block->length = num;
        }
        else
        {
            return false;
        }

    }

    if(realNumberOfEnemies != numberOfEnemiesOnBattle || realNumberOfBlock != numberOfFreeBlock + 1)
        return false;

    fclose(file);
    return true;
}

void FileGetRoute(char *route)
{

} 

void SaveGame()
{
    start_color();
    mvaddstr(LINES/2 - 1, COLUMNS/2 - 13, "Do you want to save the game: ");
    int input = 0;
    bool save, first = true;
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    
    while(input == KEY_ENTER || input != 10)
    {
        if(input == KEY_LEFT || first)
        {
            attron(COLOR_PAIR(1));
            mvaddstr(LINES/2, COLUMNS/2 - 10, "YES");
            attroff(COLOR_PAIR(1));
            mvaddstr(LINES/2, COLUMNS/2 + 10, "NO");
            save = true;
        }
        else if(input == KEY_RIGHT)
        {
            mvaddstr(LINES/2, COLUMNS/2 - 10, "YES");
            attron(COLOR_PAIR(1));
            mvaddstr(LINES/2, COLUMNS/2 + 10, "NO");
            attroff(COLOR_PAIR(1));
            save = false;
        }

        refresh();
        first = false;
        input = getch();
    } 

    if(save)
    {
        int actualLine = LINES/2 + 2;

        mvaddstr(actualLine, COLUMNS/2 - 20, "Type a name for the file (max 15 char): ");
        refresh();
        
        char name[20];
        int index = 0;
        int actualColumn = COLUMNS/2 - 20 + 40;
    
        curs_set(1);
        refresh();
        while(true)
        {           
            input = getch();

            if(input == 10)
            {
                if(index == 0)
                    move(actualLine, actualColumn);
                else
                    break;
            }
            else if(input == KEY_BACKSPACE)
            {
                if(index > 0)
                    index--;
                else
                    move(actualLine, actualColumn);
                    
                addch(' ');
                move(actualLine, actualColumn + index);
            }
            else if(index >= 15)
            {
                //addch(' ');
                move(actualLine, actualColumn + 15);
                addch(' ');
                move(actualLine, actualColumn + 15);
            }
            else
                name[index++] = (char) input; 
        }
        
        if(index < 4 || name[index - 4] != '.' || name[index - 3] != 't' || name[index - 2] != 'x' || name[index - 1] != 't')
        {
            name[index++] = '.';
            name[index++] = 't';
            name[index++] = 'x';
            name[index++] = 't';
        }

        name[index] = '\0';

        FileSaveEnemyList(name);
    }
}

// Sound
void* playBackgroundSound(void* arg) {
    
    while (!gameClose)
    {
        SDL_RWops* rw = SDL_RWFromMem(backsound_cut_mp3,  backsound_cut_mp3_len);
        Mix_Chunk *sound = Mix_LoadWAV_RW(rw, 1);
   
        if (!sound) {
            printf("Error loading sound: %s\n", Mix_GetError());
            return NULL;
        }
        chanel = Mix_PlayChannel(-1, sound, 0);
        
        while(!gameClose)
            SDL_Delay(100);
        
        Mix_HaltChannel(chanel);
        Mix_FreeChunk(sound);
    }
    
    pthread_exit(NULL);
}

void* playBulletSound(void * arg)
{
    SDL_RWops* rw = SDL_RWFromMem(laser_wav, laser_wav_len);
    Mix_Chunk *sound = Mix_LoadWAV_RW(rw, 1);
    
    if (!sound) {
        printf("Error loading sound: %s\n", Mix_GetError());
        return NULL;
    }
    
    Mix_PlayChannel(-1, sound, 0);
    
    SDL_Delay(4000);
    Mix_FreeChunk(sound);

    pthread_exit(NULL);
}