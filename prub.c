#include "stdbool.h"
#include "invaderstruct.h"
#include "stdio.h"

struct HangarNode
{
    struct Enemy *enemy;
    struct HangarNode *next;
};

struct HangarNode head;
int enemyToBuildCount = 0;
bool enemyReadyToBattle = false;


void HangarInsert(struct Enemy *enemyToBuild)
{

    int count = 0;

    struct HangarNode *iterator = &head;
    while (true)
    {
        if(count == enemyToBuildCount)
        {
            iterator->next = (struct HangarNode*) NULL;
            iterator->next->enemy = enemyToBuild;
            enemyToBuildCount++;
            break;
        }      
        else if(enemyToBuild->constructionTime > iterator->next->enemy->constructionTime)
        {
            iterator = iterator->next;
            count++;
        }
        else
        {
            struct HangarNode toInsert;
            toInsert.enemy = enemyToBuild;
            toInsert.next = iterator->next;
            iterator->next = &toInsert;
            enemyToBuildCount++;
            break;
        }
    }
      

}

void HangarInsertRec(struct HangarNode *head, struct Enemy *enemyToBuild, int count)
{
    if(count == enemyToBuildCount)
    {
        struct HangarNode actual;
        actual.enemy = enemyToBuild;
        head->next = &actual;
        enemyToBuildCount++;
    }

    else if(enemyToBuild->constructionTime < head->next->enemy->constructionTime)
    {
        struct HangarNode actual;
        struct HangarNode nextToHead = *head->next;
        actual.enemy = enemyToBuild;
        head->next = &actual;
        actual.next = &nextToHead;
        enemyToBuildCount++;
    }
    else
    {
        HangarInsertRec(head-> next, enemyToBuild, count + 1);
    }
}

struct Enemy *HangarUpdate()
{
    struct Enemy *enemyToReturn;
    enemyReadyToBattle = false;
    
    if(enemyToBuildCount > 0)
    {

        if(head.next->enemy->constructionTime <= 1)
        {
            enemyToReturn = head.next->enemy;
            head.next = head.next->next;
            enemyToBuildCount--;
            enemyReadyToBattle = true;
        }
        else
        {
            head.next->enemy->constructionTime--;     
        }
        
        
    }   
    
    return enemyToReturn;
}

int main(){
    
    struct Enemy *enemy;

    struct Enemy e0;
    e0.ch = 'Q';
    head.enemy = &e0;
    head.enemy->constructionTime = -1;

    struct Enemy e1;
    e1.ch = 'W';
    e1.constructionTime=3;
    //HangarInsertRec(&head, &e1, 0);
    HangarInsert(&e1);
    enemy = HangarUpdate();
    
    struct Enemy e2;
    e2.ch = 'E';
    e2.constructionTime=5;
    //HangarInsertRec(&head, &e2, 0);
    HangarInsert(&e2);
    enemy = HangarUpdate();

    struct Enemy e3;
    e3.ch = 'R';
    e3.constructionTime=1;
    //HangarInsertRec(&head, &e3, 0);
    HangarInsert(&e3);
    enemy = HangarUpdate();

    struct Enemy e4;
    e4.ch = 'T';
    e4.constructionTime=6;
    //angarInsertRec(&head, &e4, 0);
    HangarInsert(&e4);
    enemy = HangarUpdate();

    struct Enemy e5;
    e5.ch = 'Y';
    e5.constructionTime=4;
    //HangarInsertRec(&head, &e5, 0);
    HangarInsert(&e5);
    enemy = HangarUpdate();

    struct Enemy e6;
    e6.ch = 'U';
    e6.constructionTime=2;
    //HangarInsertRec(&head, &e6, 0);
    HangarInsert(&e6);
    enemy = HangarUpdate();
    

}