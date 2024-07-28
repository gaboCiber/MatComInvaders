#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "invaderstruct.h"
#include "stdbool.h"



struct FreeEnemyBlock rootBlock;
int numberOfFreeBlock;
const int TOP=10;
struct Enemy *enemyList[10];

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
    for (int i = 0; i < TOP; i++)
    {
        if( enemyList[i]->line == line && enemyList[i]->col == col)
            return i;
    }

    return -1;
    
}

int main()
{
    rootBlock.index = -2;
    rootBlock.length = -2;

    struct FreeEnemyBlock initial;
    initial.index = 0;
    initial.length = TOP;

    rootBlock.next = &initial;
    numberOfFreeBlock = 1;

    struct Enemy e1;
    e1.ch = 'Q'; 
    EnemyListInsert(&e1);

    struct Enemy e2;
    e2.ch = 'W'; 
    EnemyListInsert(&e2);

    struct Enemy e3;
    e3.ch = 'E'; 
    EnemyListInsert(&e3);

    struct Enemy e4;
    e4.ch = 'R'; 
    EnemyListInsert(&e4);

    struct Enemy e5;
    e5.ch = 'T'; 
    EnemyListInsert(&e5);

    struct FreeEnemyBlock b1;
    b1.index = 0;
    b1.length = 1;
    EnemyListRemove(&b1);

    
    struct FreeEnemyBlock b2;
    b2.index = 2;
    b2.length = 1;
    EnemyListRemove(&b2);
    
    struct FreeEnemyBlock b3;
    b3.index = 4;
    b3.length = 1;
    EnemyListRemove(&b3);

    struct FreeEnemyBlock b4;
    b4.index = 4;
    b4.length = 1;
    EnemyListRemove(&b4);

    struct FreeEnemyBlock b5;
    b5.index = 2;
    b5.length = 1;
    EnemyListRemove(&b5);

}