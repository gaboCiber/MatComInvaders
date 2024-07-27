#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "invaderstruct.h"
#include "stdbool.h"

struct HangarNode
{
    struct Enemy *enemy;
    struct HangarNode *next;
};

struct HangarNode hangarRoot;
int enemiesInConstruction = 0;
bool enemyReadyToBattle = false;

void HangarInsert(struct HangarNode *newEnemy)
{
    struct HangarNode *iterator = &hangarRoot;
    int count = 0;

    while (count < enemiesInConstruction)
    {
        if(newEnemy->enemy->constructionTime < iterator->next->enemy->constructionTime)
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

struct Enemy *HangarUpdate()
{
    struct Enemy *enemyToReturn;
    enemyReadyToBattle = false;
    
    if(enemiesInConstruction > 0)
    {

        if(hangarRoot.next->enemy->constructionTime <= 1)
        {
            enemyToReturn = hangarRoot.next->enemy;
            hangarRoot.next = hangarRoot.next->next;
            enemiesInConstruction--;
            enemyReadyToBattle = true;
        }
        else
        {
            hangarRoot.next->enemy->constructionTime--;     
        }
        
        
    }   
    
    return enemyToReturn;
}

int getRamdomNumberInterval(int min, int max)
{
    return min + rand() % (max + 1 - min);
}

struct Enemy BuildEnemy(int num)
{
    // Construction
    struct Enemy enemy;
    enemy.line = 3;
    enemy.col = getRamdomNumberInterval(5, 10);
    enemy.indexAtEnemyList = 3;
    
    switch (getRamdomNumberInterval(0,2))
    {
        case 0:
            enemy.ch = '#';   
            enemy.leftRight =  (int[3]) {-1, 0 , 1};
            enemy.leftRightCount = 3;  
            enemy.upDown = (int[2]) {0 , 1};
            enemy.upDownCount = 2;
            enemy.constructionTime=2;
            break;
        case 1:
            enemy.ch = '&';   
            enemy.leftRight =  (int[1]) {0};
            enemy.leftRightCount = 1;  
            enemy.upDown = (int[1]) {1};
            enemy.upDownCount = 1;
            enemy.constructionTime=3;
            break;
        case 2:
            enemy.ch = '$';   
            enemy.leftRight =  (int[2]) {-2,2};
            enemy.leftRightCount = 2;  
            enemy.upDown = (int[1]) {1};
            enemy.upDownCount = 1;
            enemy.constructionTime=1;
            break;
        default:
            break;
    }

    return enemy;
}

int main(){
    
    srand(time(0));
    struct Enemy *enemy;

    struct Enemy e0;
    e0.ch = 'Q';
    hangarRoot.enemy = &e0;
    hangarRoot.enemy->constructionTime = -1;

    
    struct Enemy e1;
    e1 = BuildEnemy(1);
    struct HangarNode n1;
    n1.enemy = &e1;
    HangarInsert(&n1);
    enemy = HangarUpdate();
    
    struct Enemy e2;
    e2 = BuildEnemy(2);
    struct HangarNode n2;
    n2.enemy = &e2;
    HangarInsert(&n2);
    enemy = HangarUpdate();

    struct Enemy e3;
    e3 = BuildEnemy(3);
    struct HangarNode n3;
    n3.enemy = &e3;
    HangarInsert(&n3);
    enemy = HangarUpdate();

    struct Enemy e4;
    e4 = BuildEnemy(4);
    struct HangarNode n4;
    n4.enemy = &e4;
    HangarInsert(&n4);
    enemy = HangarUpdate();

    struct Enemy e5;
    e5 = BuildEnemy(5);
    struct HangarNode n5;
    n5.enemy = &e5;
    HangarInsert(&n5);
    enemy = HangarUpdate();

    struct Enemy e6;
    e6 = BuildEnemy(6);
    struct HangarNode n6;
    n6.enemy = &e6;
    HangarInsert(&n6);
    enemy = HangarUpdate();
}