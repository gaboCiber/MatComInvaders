#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "../invaderstruct.h"
#include "stdbool.h"

struct FreeEnemyBlock rootBlock;
int numberOfFreeBlock;
int totalNumberOfEnemiesOnBattle=10;
struct Enemy *enemyList[10];
int numberOfEnemiesOnBattle;

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

}

void EnemyListRamdomExample()
{
    rootBlock.index = -2;
    rootBlock.length = -2;

    struct FreeEnemyBlock *initial = (struct FreeEnemyBlock *) malloc(sizeof(struct FreeEnemyBlock));
    initial->index = 0;
    initial->length = totalNumberOfEnemiesOnBattle;

    rootBlock.next = initial;
    numberOfFreeBlock = 1;

    bool ocu[totalNumberOfEnemiesOnBattle];
    for (int i = 0; i < totalNumberOfEnemiesOnBattle; i++)
    {
        ocu[i] = false;
    }
    
    int count = 0;
    for (int i = 0; i < 20; i++)
    {
        if(rand() % 2)
        {
            struct Enemy *en = (struct Enemy *) malloc(sizeof(struct Enemy));
            en->ch = (char) ((int) 'A' + i); 
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

    printf("Sucess \n");

    //EnemyListEraseAllBlocksFromMemory();
}

/*int main()
{
    srand(time(0));
    EnemyListRamdomExample();
}*/