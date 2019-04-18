#ifndef TRAINLIST_H
#define TRAINLIST_H
#include "Train.h"

class TrainList {
  public:
    TrainList(void);
    void insert(train_t * train);
    void removeAll();
    train_t getFirst();
  private:
    train_t * trainList;

};

#endif
