#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "invaderstruct.h"
#include "stdbool.h"


struct HangarNode hangarRoot;
struct HangarNode *hangarPreLeaf;
int enemiesInConstruction = 0;
int priority = 0;
bool enemyReadyToBattle = false;

int getRamdomNumberInterval(int min, int max)
{
    return min + rand() % (max + 1 - min);
}

void HangarInsert(struct HangarNode *newEnemy)
{
    struct HangarNode *iterator = &hangarRoot;
    int count = 0;

    while (true)
    {
        if(count == enemiesInConstruction)
        {
            hangarPreLeaf = iterator;
            iterator->next = newEnemy;
            break;
        }

        if(newEnemy->ship->buildTime < iterator->next->ship->buildTime)
        {
            if(iterator == hangarPreLeaf)
            {
                hangarPreLeaf = newEnemy;
            }

            iterator->next = newEnemy;
            newEnemy->next = iterator->next;
            break;
        }

        iterator = iterator->next;
        count++;
    }

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
        else if(priority == 2)
        {
            priority = -1;
            struct HangarNode boost;
            boost.ship->shipModel = hangarPreLeaf->next->ship->shipModel;
            boost.ship->buildTime = hangarPreLeaf->next->ship->buildTime - 1;
            hangarPreLeaf->next = boost.next;
            HangarInsert(&boost);
        }
        else
        {
            hangarRoot.next->ship->buildTime--;     
        }
        
        
    }   
    
    priority++;
    return enemyToReturn;
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

int main(){
    
    srand(time(0));
    struct EnemyDesing *enemy;

    struct EnemyDesing e0;
    e0.buildTime = -1;
    e0.shipModel =-1;
    hangarRoot.ship = &e0;
    
    struct EnemyDesing e1;
    desingEnemy(&e1);
    struct HangarNode n1;
    n1.ship = &e1;
    HangarInsert(&n1);
    enemy = HangarBuild();
    
    struct EnemyDesing e2;
    desingEnemy(&e2);
    struct HangarNode n2;
    n2.ship = &e2;
    HangarInsert(&n2);
    enemy = HangarBuild();

    struct EnemyDesing e3;
    desingEnemy(&e3);
    struct HangarNode n3;
    n3.ship = &e3;
    HangarInsert(&n3);
    enemy = HangarBuild();

    struct EnemyDesing e4;
    desingEnemy(&e4);
    struct HangarNode n4;
    n4.ship = &e4;
    HangarInsert(&n4);
    enemy = HangarBuild();

    struct EnemyDesing e5;
    desingEnemy(&e5);
    struct HangarNode n5;
    n5.ship = &e5;
    HangarInsert(&n5);
    enemy = HangarBuild();
}