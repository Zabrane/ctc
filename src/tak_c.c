/* tak_c.c */
#include <stdio.h>

#if DEBUG
#define DPRINTF(fmt, args...)	printf(fmt, ##args)
#define NITER	1
#else
#define DPRINTF(fmt, args...)
#define NITER	255
#endif

static int tak3(int x, int y, int z)
{
 tailcall:
    DPRINTF("%d %d %d\n", x, y, z);
    if (x <= y) {
	DPRINTF("%d\n", z);
	return z;
    } else {
	int a = tak3(x-1, y, z);
	int b = tak3(y-1, z, x);
	int c = tak3(z-1, x, y);
	x = a; y = b; z = c; goto tailcall;
    }
}

static int tak0(void)
{
    return tak3(18, 12, 6);
}

static void doit(void)
{
    int i = NITER;
    do {
	tak0();
    } while (--i != 0);
}

#if STANDALONE
int main(void)
{
    doit();
    return 0;
}
#else
#include "interpreter.h"

void prepare_code(void)
{
}

void run_code(void)
{
    doit();
}
#endif
