#include "sum.h"

#include <stdio.h>
#include <stdlib.h>

int Sum(struct SumArgs *args) {
    int sum = 0;
    int i;
    for (i=(*(args)).begin; i<(*(args)).end;i++)
    {
        sum+=*((*(args)).array+i);
    }
    return sum;
}