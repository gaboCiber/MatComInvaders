#include "invaderstruct.h"

const int TOP=2;
struct Enemy *enemyList[TOP];
bool ocupied[TOP];
int count = 0;

int EnemyListInsert(struct Enemy *enemy);

void EnemyListRemove(int index);

bool EnemyListIsOneLeft();

int EnemyListCheckPositions(int line, int col);