/* solaris.c */
#include <sys/types.h>
#include <sys/processor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libcpc.h>
#include "interpreter.h"
#include "benchmark.h"

#define ARRAY_SIZE(A)	(sizeof(A)/sizeof((A)[0]))

struct setup {
    unsigned int nrconfigs;
    cpc_event_t *configs;
    void (*report_metrics)(void);
};
static const struct setup *setup;

static void report_cycles(unsigned long long cycles)
{
    printf("Cycles\t\t\t%llu\n", cycles);
}

static void report_insns(unsigned long long insns)
{
    printf("Instructions\t\t%llu\n", insns);
}

static void report_cpi(unsigned long long cycles, unsigned long long insns)
{
    report_cycles(cycles);
    report_insns(insns);
    printf("CPI\t\t\t%f\n", (double)cycles / (double)insns);
}

#ifdef __sparc__
static cpc_event_t us1_configs[1] = {
    [0] = {
	.ce_cpuver = CPC_ULTRA1, /* SIGSEGV if not set */
	/* PIC1 == Cycle_cnt, PIC0 == Instr_cnt, UT */
	.ce_pcr = (0x00 << 11) | (0x01 << 4) | (1 << 2),
    },
};

static void us1_report_metrics(void)
{
    /* Prefer Cycle_cnt over %tick, since the former respects UT. */
    report_cpi(us1_configs[0].ce_pic[1], us1_configs[0].ce_pic[0]);
}

static const struct setup us1_setup = {
    .nrconfigs = ARRAY_SIZE(us1_configs),
    .configs = us1_configs,
    .report_metrics = us1_report_metrics,
};

static void init_setup(int cpuver)
{
    switch (cpuver) {
      case CPC_ULTRA3_I:
	/*
	 * (Dispatch0_mispred + Dispatch0_br_target) / Cycles
	 * gives very low values. I don't trust them.
	 *
	 * IU_Br_{miss,Count}_{taken,untaken} appear to
	 * only count conditional branch instructions, not
	 * returns or jump-indirect instructions. Useless.
	 *
	 * So on USIIIi we fall back to plain US metrics.
	 */
      case CPC_ULTRA2:
      case CPC_ULTRA1:
	setup = &us1_setup;
	break;
      default:
	fprintf(stderr, "cpuver %d (%s) not supported\n",
		cpuver, cpc_getcciname(cpuver));
	exit(1);
    }
}
#endif

static void init(void)
{
    int cpuver;

    if (cpc_access() < 0) {
	perror("cpc_access()");
	exit(1);
    }
    cpuver = cpc_getcpuver();
    init_setup(cpuver);
}

static void each_config_run_code(void)
{
    unsigned int i;

    for(i = 0; i < setup->nrconfigs; ++i) {
	if (cpc_bind_event(&setup->configs[i], 0) < 0) {
	    perror("cpc_bind_event()");
	    exit(1);
	}
	run_code();
	if (cpc_take_sample(&setup->configs[i]) < 0)
	    perror("cpc_take_sample()");
	cpc_rele();
    }
}

#if 0
static void walk_name_action(void *arg, int regno, const char *name, uint8_t bits)
{
    printf("#define PIC%d_%s\t%#x\n", regno, name, bits);
}

static void do_walk_names(cpuver)
{
    uint_t npic, i;

    npic = cpc_getnpic(cpuver);
    for(i = 0; i < npic; ++i)
	cpc_walk_names(cpuver, i, NULL, walk_name_action);
}
#endif

static void report(void)
{
    unsigned int i, j;
    processor_info_t pinfo;
    int cpuver = cpc_getcpuver();

    printf("CPC Info:\n");
    printf("cpc_version(%u) -> %u\n", CPC_VER_NONE, cpc_version(CPC_VER_NONE));
    printf("cpc_getcpuver -> %d\n", cpuver);
    printf("cpc_getcciname(%d) -> '%s'\n", cpuver, cpc_getcciname(cpuver));
    printf("cpc_getnpic(%d) -> %u\n", cpuver, cpc_getnpic(cpuver));
    printf("cpc_getusage(%d) -> '%s'\n", cpuver, cpc_getusage(cpuver));
#if 0
    printf("cpc_getcpuref(%d) -> '%s'\n", cpuver, cpc_getcpuref(cpuver));
    do_walk_names(cpuver);
#endif
    processor_info(getcpuid(), &pinfo);
    printf("Processor Info:\n");
    printf("pi_processor_type -> '%s'\n", pinfo.pi_processor_type);
    printf("pi_clock -> %d MHz\n", pinfo.pi_clock);
    for(i = 0; i < setup->nrconfigs; ++i) {
	printf("\nConfiguration #%u:\n", i);
	printf("%s\n", cpc_eventtostr(&setup->configs[i]));
	printf("%%tick\t\t\t%lld\n", setup->configs[i].ce_tick);
	for(j = 0; j < 2; ++j)
	    printf("%%pic[%u]\t\t\t%lld\n", j, setup->configs[i].ce_pic[j]);
    }
    printf("\nMetrics:\n");
    (*setup->report_metrics)();
}

void benchmark_code(void)
{
    init();
    each_config_run_code();
    report();
}
