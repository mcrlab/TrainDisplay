#include <stddef.h>
#include <stdlib.h>
#include "TrainList.h"
#include "Train.h"

TrainList::TrainList(void){
  
}

void TrainList::insert(train_t * nt){
  train_t **t = &this->trainList;
  while(*t){
    t = &(*t)->next;
  }
  nt->next = *t;
  *t = nt;
}

void TrainList::removeAll(){
  train_t * current = this->trainList;
  train_t * temp_node = NULL;
  
  while (current != NULL) {
      temp_node = current;
      current = temp_node->next;
      free(temp_node);
  }
  trainList = NULL;
}

train_t TrainList::getFirst(){
  return * this->trainList;
}


