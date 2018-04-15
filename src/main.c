/* main.c */
#include "interpreter.h"
#include "benchmark.h"

int main(void)
{
    prepare_code();
    benchmark_code();
    return 0;
}
