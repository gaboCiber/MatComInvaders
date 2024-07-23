#include "stdlib.h"
#include "stdbool.h"
#include "stdio.h"
#include "invaderstruct.h"

const int TOP=2;
struct Enemy *enemyList[TOP];
bool ocupied[TOP];
int count = 0;

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

/*int main()
{
    struct Enemy en1;
    en1.col = 1;
    en1.line = 2;

    en1.indexAtEnemyList = EnemyListInsert(&en1);

    struct Enemy en2;
    en2.col = 5;
    en2.line = 2;
    en2.indexAtEnemyList = EnemyListInsert(&en2);

    for (int i = 0; i < count; i++)
    {
        int c = EnemyListCheckPositions(i,i);
        if( c == -1 )
            break;
    }
    



}*/
