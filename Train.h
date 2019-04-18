#ifndef TRAIN_H
#define TRAIN_H

typedef struct train {
  train * next;  // pointer to next train in the list
  char from[4]; // from CRS
  char to[4]; // to CRS
  unsigned int scheduled; // departure time in minutes
  unsigned int estimated;
  int status;
} train_t;


#endif
