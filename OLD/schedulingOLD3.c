#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "invaderstruct.h"
#include "stdbool.h"



struct HangarNode hangarRoot;
int enemiesInConstruction = 0;
int leafPriority = 0;
const int PRIORITY = 2;

int getRamdomNumberInterval(int min, int max)
{
    return min + rand() % (max + 1 - min);
}

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

            iterator->next->ship->buildTime--;
            struct HangarNode *boost = iterator->next;
            iterator->next = NULL;
            enemiesInConstruction--;
            leafPriority = -1;
            HangarInsert(boost);
            
        }
        else if(hangarRoot.next->ship->buildTime <= 1)
        {
            int model = hangarRoot.next->ship->shipModel;
            hangarRoot.next = hangarRoot.next->next;
            enemiesInConstruction--;
            return model;
        }
        else
        {
            hangarRoot.next->ship->buildTime--;     
        }
        
        
    }   
    
    leafPriority++;
    return -1;
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
    int enemy;

    struct EnemyDesing e0;
    e0.buildTime = -1;
    e0.shipModel =-1;
    hangarRoot.ship = &e0;
         
    for (int i = 0; i < 10; i++)
    {
        struct EnemyDesing *e1 = (struct EnemyDesing*) malloc(sizeof(struct EnemyDesing));
        desingEnemy(e1);
        struct HangarNode *n1 = (struct HangarNode*) malloc(sizeof(struct HangarNode));
        n1->ship = e1;
        HangarInsert(n1);
        int model = HangarBuild();
        printf("%d \t %d \n" , n1->ship->shipModel , model);
    }
}