#ifndef SUM_H
#define SUM_H

struct SumArgs {
  int *array;
  int begin;
  int end;
};

int Sum(struct SumArgs *args);

#endif