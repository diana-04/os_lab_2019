#include <stdlib.h>

struct SumArgs {
  int *array;
  int begin;
  int end;
};

int Sum(const struct SumArgs *);
void *ThreadSum(void *);