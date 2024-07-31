#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "../invaderstruct.h"
#include "stdbool.h"
#include <stdio.h>
#include <string.h>

// ----------------------------------------------------------------------------------- //

struct FreeEnemyBlock rootBlock;
int numberOfFreeBlock;
int TOP=10;
struct Enemy *enemyList[10];
int numberOfEnemiesOnBattle;

int getRamdomNumberInterval(int min, int max)
{
    return min + rand() % (max + 1 - min);
}


void desingEnemy(struct Enemy *enemy)
{
    enemy->line = 3;
    enemy->col = getRamdomNumberInterval(5, 10);
    
    switch ( (int) (intptr_t) getRamdomNumberInterval(0, 2) )
    {
        case 0:
            enemy->ch = '&';   
            enemy->leftRight =  (int[1]) {0};
            enemy->leftRightCount = 1;  
            enemy->upDown = (int[1]) {1};
            enemy->upDownCount = 1;
            enemy->shipModel = 0;
            break;
        case 1:
            enemy->ch = '#';   
            enemy->leftRight =  (int[3]) {-1, 0 , 1};
            enemy->leftRightCount = 3;  
            enemy->upDown = (int[2]) {0 , 1};
            enemy->upDownCount = 2;
            enemy->shipModel = 1;
            break;
        case 2:
            enemy->ch = '$';   
            enemy->leftRight =  (int[2]) {-2,2};
            enemy->leftRightCount = 2;  
            enemy->upDown = (int[1]) {1};
            enemy->upDownCount = 1;
            enemy->shipModel = 2;
            break;
        default:
            break;
    }
}

void EnemyListInsert(struct Enemy *enemy)
{
    if(numberOfEnemiesOnBattle < TOP)
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
    if(numberOfEnemiesOnBattle > 0 && index >= 0 && index < TOP)
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
    return numberOfEnemiesOnBattle < TOP;
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

void EnemyListEraseAllBlocksFromMemory()
{
    struct FreeEnemyBlock *iterator = rootBlock.next;
    for (int i = 0; i < TOP; i++)
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

void EnemyListRamdomExample()
{
    rootBlock.index = -2;
    rootBlock.length = -2;

    struct FreeEnemyBlock *initial = (struct FreeEnemyBlock *) malloc(sizeof(struct FreeEnemyBlock));
    initial->index = 0;
    initial->length = TOP;

    rootBlock.next = initial;
    numberOfFreeBlock = 1;

    bool ocu[TOP];
    for (int i = 0; i < TOP; i++)
    {
        ocu[i] = false;
    }
    
    int count = 0;
    for (int i = 0; i < 20; i++)
    {
        if(rand() % 2)
        {
            struct Enemy *en = (struct Enemy *) malloc(sizeof(struct Enemy));
            desingEnemy(en);
            EnemyListInsert(en);
            ocu[en->indexAtEnemyList] = true;
        }
        else 
        {           
            int index = 0;
            if(numberOfEnemiesOnBattle > 0)
            {
                do {
                    index = rand() % 10;
                } while (!ocu[index]);
            }     
            
            ocu[index] = false;
            EnemyListRemove(index);
        }
    }
    //EnemyListEraseAllBlocksFromMemory();
}


// ----------------------------------------------------------------------------------- //

bool FileSaveEnemyList(const char *fileToWrite)
{
    FILE *file = fopen(fileToWrite, "w");
    if(file == NULL)
    {
        printf("Error al crear el archivo\n");
        return false;
    }

    fprintf(file, "TOP: %d\n", TOP);
    
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
    for (int i = 0; i < TOP; i++)
    {
        if(enemyList[i] != NULL)
        {
            fprintf(file, "[\n");
            fprintf(file, "\tcol: %d\n", enemyList[i]->col);
            fprintf(file, "\tline: %d\n", enemyList[i]->line);
            fprintf(file, "\tindexAtEnemyList: %d\n", enemyList[i]->indexAtEnemyList);
            fprintf(file, "\tshipModel: %d\n", enemyList[i]->shipModel);
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

bool FileconvertStringToInt(char *str, int *num)
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
    struct Enemy *enemy;

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

        if(propertyNumber != NULL && !FileconvertStringToInt(propertyNumber, &num))
            return false;

        
        if(strcmp(propertyName, "[") == 0)
        {
            if(loadingEnemy)
                enemy = (struct Enemy *) malloc(sizeof(struct Enemy));
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
                enemyList[enemy->indexAtEnemyList] = enemy;
            }
            else
            {
                save = block;
                realNumberOfBlock++;
            }
        }
        else if(strcmp(propertyName, "TOP") == 0)
        {
            TOP = num;
        }
        else if(strcmp(propertyName, "numberOfEnemiesOnBattle") == 0)
        {
            loadingEnemy = true;
            loadingBlock = false;
            numberOfEnemiesOnBattle = num;
        }
        else if(strcmp(propertyName, "col") == 0 )
        {
            enemy->col = num;
        }
        else if(strcmp(propertyName, "line") == 0)
        {
            enemy->line = num;
        }
        else if(strcmp(propertyName, "indexAtEnemyList") == 0)
        {
            enemy->indexAtEnemyList = num;
        }
        else if(strcmp(propertyName, "shipModel") == 0)
        {
            enemy->shipModel = num;
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

int main()
{
    srand(time(0));
    
    EnemyListRamdomExample();
    
    FileSaveEnemyList("save.txt");
    
    EnemyListEraseAllBlocksFromMemory();

    FileLoadEnemyList("save.txt");

    EnemyListEraseAllBlocksFromMemory();
    return 0;
}