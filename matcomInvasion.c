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

// Start Screen
void startScreen();
void drawMatCom(int x, int y);
void drawSpaceship(int x, int y);
void drawAlien(int x, int y);

// Player 
struct Player player;
void *createBullet(void *arg);
void *createPlayerThread(void *arg);


// MotherShip and Enemies 
void drawMotherShip();
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
int remainingEnemiesOnThisLevel;
int enemiesInConstruction = 0;
int leafPriority = 0;
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
int FileGetRoute(int actualLine, char *route);
void FileSaveGame();
int TypeOnScreen(int actualLine, int actualColumn, char *type, int typeLength);

// Sound
int chanel;
void* playBulletSound(void * arg);
void* playBackgroundSound(void* arg);

// Settings
int enemySpeed = 0, contructionSpeed = 0, difficulty = 0;
int enemySpeedTime = 500000 ;
int constructionSpeedTime = 3000000;
int priorityBoost = 3;
void ShowSettings();
void ShowDifficulty(int enemySpeed, int contructionSpeed, int difficulty, int opt);
void ChooseDifficulty(int x, int y, int difficulty, bool color);
void ChangeSettings(int enemySpeed, int contructionSpeed, int difficulty);

// ------------------------------------------------------------------------------------//

//General

int main(int argc, char *argv[]) 
{
    if(argc > 1)
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
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);

    // if(argc == 2)
    // {
    //     FileLoadEnemyList(argv[1]);
    //     saveGame = true;
    // }
    // else
    // {
    //     saveGame = false;
    // }

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Error initializing SDL_mixer: %s\n", Mix_GetError());
        return -1;
    }

    startScreen();
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
    char hp[12], thp[12], level[12], totalEnemy[12], remainEnemy[12];
    sprintf(hp, "%d", player.hp);
    sprintf(thp, "%d", player.totalHp);
    sprintf(level, "%d", playerLevel);
    sprintf(remainEnemy, "%d", remainingEnemiesOnThisLevel);
    sprintf(totalEnemy, "%d", totalEnemiesOnThisLevel);

    char levelStr[15] = "Level:";
    strcat(levelStr, level);

    char playerStr[30] = "Player HP: ";
    strcat(playerStr, hp);
    strcat(playerStr, " / ");
    strcat(playerStr, thp);
    strcat(playerStr, " ");

    char enemyStr[30] = "Enemies: ";
    strcat(enemyStr, remainEnemy);
    strcat(enemyStr, " / ");
    strcat(enemyStr, totalEnemy);
    strcat(enemyStr, " ");
    
    drawMotherShip();
    mvaddstr(0, 5, levelStr);
    mvaddstr(0, COLUMNS/2 - 11, playerStr);
    mvaddstr(0, COLUMNS - 20, enemyStr);
    
    //refresh();
    wrefresh(stdscr);
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
        

        // Set MotherSip
        drawMotherShip();
        totalEnemiesOnThisLevel = (10 * playerLevel);

        if(!saveGame)
        {
            rootBlock.index = -2;
            rootBlock.length = -2;

            struct FreeEnemyBlock *initial = (struct FreeEnemyBlock *) malloc(sizeof(struct FreeEnemyBlock));
            initial->index = 0;
            initial->length = totalNumberOfEnemiesOnBattle;

            rootBlock.next = initial;
            numberOfFreeBlock = 1;
            
            remainingEnemiesOnThisLevel = totalEnemiesOnThisLevel;
        }
        
        saveGame = false;
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
            FileSaveGame();
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


// Start Screen
void startScreen()
{
    drawSpaceship(5, ROWS / 2 - 9);
    drawMatCom(COLUMNS / 2 - 30, ROWS / 2 - 10);
    drawAlien(COLUMNS - 20, ROWS / 2 - 8);
    refresh();

    int input = 0;
    bool save, first = true;
    
    while(input == KEY_ENTER || input != 10)
    {
        if(input == KEY_LEFT || first)
        {
            attron(COLOR_PAIR(1));
            mvaddstr(ROWS/2 + 10, COLUMNS/2 - 10, "PLAY");
            attroff(COLOR_PAIR(1));
            mvaddstr(ROWS/2 + 10, COLUMNS/2 + 10, "LOAD");
            saveGame = false;
        }
        else if(input == KEY_RIGHT)
        {
            mvaddstr(ROWS/2 + 10, COLUMNS/2 - 10, "PLAY");
            attron(COLOR_PAIR(1));
            mvaddstr(ROWS/2 + 10, COLUMNS/2 + 10, "LOAD");
            attroff(COLOR_PAIR(1));
            saveGame = true;
        }

        refresh();
        first = false;
 
        input = getch();
    }

    if(saveGame)
    {
        curs_set(1);
        char route[55];
        FileGetRoute(ROWS/2 + 12, route);
        FileLoadEnemyList(route);
        saveGame = true;
        curs_set(0);
    }
    
}

void drawMatCom(int x, int y) {
    mvprintw(y, x, "      __  __      _    _____  ____  _____   __  __");
    mvprintw(y + 1, x, "     |  \\/  |    / \\  |_   _|/ ___ /  _  \\ |  \\/  |");
    mvprintw(y + 2, x, "     | |\\/| |   / _ \\   | |  | |   | | | | | |\\/| |");
    mvprintw(y + 3, x, "     | |  | |  / ___ \\  | |  | |__ | |_| | | |  | |");
    mvprintw(y + 4, x, "     |_|  |_| /_/   \\_\\ |_|  \\____ \\_____/ |_|  |_|");
    mvprintw(y + 5, x, "                                                            ");
    mvprintw(y + 6, x, " ___   _   _ __       __  _     _____   ___   _____   _   _");
    mvprintw(y + 7, x, "|_ _| | \\ | |\\ \\     / / / \\   / ____| |_ _| /  _  \\ | \\ | |");
    mvprintw(y + 8, x, " | |  |  \\| | \\ \\   / / / _ \\  |(___    | |  | | | | |  \\| |");
    mvprintw(y + 9, x, " | |  | |\\  |  \\ \\_/ / / ___ \\  ____)|  | |  | |_| | | |\\  | ");
   mvprintw(y + 10, x, "|___| |_| \\_|   \\___/ / /   \\_\\|_____/ |___| \\_____/ |_| \\_| ");
}

void drawSpaceship(int x, int y) {
    mvprintw(y, x, "     /\\");
    mvprintw(y + 1, x, "    /  \\");
    mvprintw(y + 2, x, "   /____\\");
    mvprintw(y + 3, x, "  |      |");
    mvprintw(y + 4, x, "  |  __  |");
    mvprintw(y + 5, x, "  | |  | |");
    mvprintw(y + 6, x, "  | |__| |");
    mvprintw(y + 7, x, "  |______|");
    mvprintw(y + 8, x, " /|      |\\");
    mvprintw(y + 9, x, "/_|______|_\\");
}

void drawAlien(int x, int y) {
    mvprintw(y, x, "   _____ ");
    mvprintw(y + 1, x, "  /     \\");
    mvprintw(y + 2, x, " | () () |");
    mvprintw(y + 3, x, "  \\  ^  /");
    mvprintw(y + 4, x, "   |||||");
    mvprintw(y + 5, x, "   |||||");
}

// Player

void *createPlayerThread(void *arg)
{    
    clock_t bulletFire = clock();
    clock_t bulletRecall;
    long elapsedTime;

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
            elapsedTime =  bulletRecall - bulletFire;

            if( elapsedTime >= 20500)
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
            //drawMotherShip();
            screenOnPause = false;
            //refresh();
            screenRefresh();

            pthread_mutex_unlock(&mutex);
        }
        else if(inputKeyBoard == 'q')
        {
            pthread_mutex_lock(&mutex);
            gameOver(0);
            pthread_mutex_unlock(&mutex);

            break;
        }
        else if(inputKeyBoard == 's')
        {
            pthread_mutex_lock(&mutex);
            ShowSettings();
            pthread_mutex_unlock(&mutex);
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
            remainingEnemiesOnThisLevel--;
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

void drawMotherShip()
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
        if(remainingEnemiesOnThisLevel == 0)
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
                toBield->line = 3;
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

            usleep(constructionSpeedTime);        
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
            enemy->ch = 'O';   
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
            remainingEnemiesOnThisLevel--;

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
        
        usleep(enemySpeedTime);
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
        if(leafPriority == priorityBoost && enemiesInConstruction > 1)
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
    
    fprintf(file, "remainingEnemiesOnThisLevel: %d\n",remainingEnemiesOnThisLevel);
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

    fprintf(file, "enemySpeed: %d\n",enemySpeed);
    fprintf(file, "contructionSpeed: %d\n",contructionSpeed);
    fprintf(file, "difficulty: %d\n", difficulty);

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
        else if(strcmp(propertyName, "enemySpeed") == 0)
        {
            enemySpeed = num;
        }
        else if(strcmp(propertyName, "difficulty") == 0)
        {
            difficulty = num;
        }
        else if(strcmp(propertyName, "contructionSpeed") == 0)
        {
            contructionSpeed = num;
        }
        else if(strcmp(propertyName, "remainingEnemiesOnThisLevel") == 0)
        {
            remainingEnemiesOnThisLevel = num;
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

    ChangeSettings(enemySpeed, contructionSpeed, difficulty);
    
    fclose(file);
    return true;
}

void FileSaveGame()
{ 
    move(0,0);
    wclear(stdscr);

    mvaddstr(LINES/2 - 1, COLUMNS/2 - 13, "Do you want to save the game: ");
    int input = 0;
    bool save, first = true;
    
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
        curs_set(1);

        // File Name
        int actualLine = LINES/2 + 2;
        mvaddstr(actualLine, COLUMNS/2 - 20, "Type a name for the file (max 15 char): ");
        
        char name[20];
        int actualColumn = COLUMNS/2 - 20 + 40;
    
        refresh();
        int index = TypeOnScreen(actualLine, actualColumn, name, 15);  
        if(index < 4 || name[index - 4] != '.' || name[index - 3] != 't' || name[index - 2] != 'x' || name[index - 1] != 't')
        {
            name[index++] = '.';
            name[index++] = 't';
            name[index++] = 'x';
            name[index++] = 't';
        }
        name[index] = '\0';


        // File Route
        actualLine++;
        char route[55];
        index = FileGetRoute(actualLine, route);
        
        if(route[index - 1] != '/')
        {
            route[index++] = '/';   
            route[index] = '\0';
        }

        strcat(route, name);
        FileSaveEnemyList(route);
    }
}

int FileGetRoute(int actualLine, char *route)
{
    int index = 0;

    while (true)
    {
        mvaddstr(actualLine, COLUMNS/2 - 20, "Type the route for the file (max 50 char): ");
        int actualColumn = COLUMNS/2 - 20 + 42;

        refresh();
        index = TypeOnScreen(actualLine, actualColumn, route, 50);  
        route[index] = '\0';

        if(access(route, F_OK) != -1)
            break;
        
        mvaddstr(actualLine + 1,COLUMNS/2 - 5, "Wrong route");
        refresh();

        sleep(1);

        move(actualLine + 1, 0);
        wclrtoeol(stdscr);
        move(actualLine, actualColumn);
        wclrtoeol(stdscr);
    }

    return index;
}

int TypeOnScreen(int actualLine, int actualColumn, char *type, int typeLength)
{
    int input;
    int index = 0;
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
        else if(index >= typeLength)
        {
            //addch(' ');
            move(actualLine, actualColumn + typeLength);
            addch(' ');
            move(actualLine, actualColumn + typeLength);
        }
        else
            type[index++] = (char) input; 
    }

    return index;
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
        
        int duration = 0;
        while(!gameClose && duration < 90000)
        {
            duration+=100;
            SDL_Delay(100);
        }
        
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

// Settings

void ChooseDifficulty(int x, int y, int difficulty, bool color)
{
    mvaddstr(LINES/2 + x, COLUMNS/2 + y, "           ");
    
    if(color)
        attron(COLOR_PAIR(1));

    switch (difficulty)
    {
        case 0:
            mvaddstr(LINES/2+ x, COLUMNS/2 - 5  + y, " Low ");
            break;
        case 1:
            mvaddstr(LINES/2 + x, COLUMNS/2 - 5 + y, " Medium ");
            break;
        case 2:
            mvaddstr(LINES/2 + x, COLUMNS/2 -5 + y, " High ");
            break;
    default:
        break;
    }

    if(color)
        attroff(COLOR_PAIR(1));
}

void ShowDifficulty(int enemySpeed, int contructionSpeed, int difficulty, int opt)
{
    ChooseDifficulty(0, 17, enemySpeed, opt == 0);
    ChooseDifficulty(1, 22,contructionSpeed, opt == 1);
    ChooseDifficulty(2, 12, difficulty, opt == 2);
    //refresh();
}

void ShowSettings()
{
    wclear(stdscr);

    mvaddstr(LINES/2 - 2, COLUMNS/2 - 15, "Settings Options (Press Enter to Save)");
    mvaddstr(LINES/2, COLUMNS/2 - 10, "1 - Enemies's speed:");
    mvaddstr(LINES/2 + 1, COLUMNS/2 - 10, "2 - Construction's speed:");
    mvaddstr(LINES/2 + 2, COLUMNS/2 - 10, "3 - Difficulty:");
    
    int input = 0;
    int opt = 0;

    ShowDifficulty(enemySpeed, contructionSpeed, difficulty, opt);

    while( (input = getch()) != 10)
    {
        char css[12];
        sprintf( css, "%d" , input);
        mvaddstr( 0,0,css);

        if(input == 259) // Up
            opt += (opt >= 0) ? -1 : 0;
        else if(input == 258) // Down
            opt += (opt < 3) ? 1: 0;
        else if(input == 260 || input == 261) //Left and Right
        {
            int mov = (input == 260) ? -1 : 1; //Left

            switch (opt)
            {
                case 0:
                    enemySpeed += ( (enemySpeed == 0 && mov == -1 )|| (enemySpeed == 2 && mov == 1)) ? 0 : mov;
                    break;
                case 1:
                    contructionSpeed += ( (contructionSpeed == 0 && mov == -1 )|| (contructionSpeed == 2 && mov == 1)) ? 0 : mov;
                    break;
                case 2:
                    difficulty += ( (difficulty == 0 && mov == -1 )|| (difficulty == 2 && mov == 1)) ? 0 : mov;
                    break;
            }

        }

        wclear(stdscr);
        mvaddstr(LINES/2 - 2, COLUMNS/2 - 15, "Settings Options (Press Enter to Save)");
        mvaddstr(LINES/2, COLUMNS/2 - 10, "1 - Enemies's speed:");
        mvaddstr(LINES/2 + 1, COLUMNS/2 - 10, "2 - Construction's speed:");
        mvaddstr(LINES/2 + 2, COLUMNS/2 - 10, "3 - Difficulty:");
        ShowDifficulty(enemySpeed, contructionSpeed, difficulty, opt);
    } 

    ChangeSettings(enemySpeed, contructionSpeed, difficulty);
    clear();
}

void ChangeSettings(int enemySpeed, int contructionSpeed, int difficulty)
{
    enemySpeedTime = 500000 - (enemySpeed * 80000);
    constructionSpeedTime = 3000000 - (contructionSpeed * 1000000);
    priorityBoost = 3 - difficulty; 
}