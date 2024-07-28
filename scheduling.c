#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "invaderstruct.h"
#include "stdbool.h"

struct HangarNode
{
    int shipModel;
    int buildTime;
    struct HangarNode *next;
};

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

int main(){
    
    srand(time(0));

    hangarRoot.buildTime = -1;
    hangarRoot.shipModel = -1;

    int num0 = 0;
    int num1 = 0;
    int num2 = 0;

    for (int i = 0; i < 20; i++)
    {  
        struct HangarNode *newEnemy = (struct HangarNode*) malloc(sizeof(struct HangarNode));
        desingEnemy(newEnemy);
        int ship = newEnemy->shipModel;
        HangarInsert(newEnemy);
        int model = HangarBuild();
        printf("%d \t %d \n" , ship , model);

        num0 += (model == 0) ? 1 : 0;
        num1 += (model == 1) ? 1 : 0;
        num2 += (model == 2) ? 1 : 0;

    }

    printf("\n0: %d \t 1: %d \t 2: %d \n", num0, num1, num2);

    destroyUnbuildEnemies();
}