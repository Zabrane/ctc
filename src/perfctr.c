/* perfctr.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libperfctr.h>
#include "interpreter.h"
#include "benchmark.h"

#define ARRAY_SIZE(A)	(sizeof(A)/sizeof((A)[0]))

static struct vperfctr *self;
static struct perfctr_info info;
struct vperfctr_control control;
static struct perfctr_sum_ctrs sums[2];
struct setup {
    unsigned int nrconfigs;
    const struct perfctr_cpu_control *configs;
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

static void report_bmr(unsigned long long misses, unsigned long long branches)
{
    printf("BMR\t\t\t%f\n", (double)misses / (double)branches);
}

static const struct perfctr_cpu_control generic_configs[1] = {
    [0] = {
	.tsc_on = 1,
    },
};

static void generic_report_metrics(void)
{
    report_cycles(sums[0].tsc);
}

static const struct setup generic_setup = {
    .nrconfigs = ARRAY_SIZE(generic_configs),
    .configs = generic_configs,
    .report_metrics = generic_report_metrics,
};

#if defined(__i386__) || defined(__x86_64__)
static const struct perfctr_cpu_control amd_configs[1] = {
    [0] = {
	.tsc_on = 1,
	.nractrs = 4,
	.pmc_map = {
	    [0] = 0,
	    [1] = 1,
	    [2] = 2,
	    [3] = 3,
	},
	.evntsel = {
	    /* CPU_CLK_UNHALTED */
	    [0] = 0x76 | (1 << 16) | (1 << 22),
	    /* RETIRED_INSTRUCTIONS */
	    [1] = 0xC0 | (1 << 16) | (1 << 22),
	    /* RETIRED_BRANCHES_MISPREDICTED */
	    [2] = 0xC3 | (1 << 16) | (1 << 22),
	    /* RETIRED_BRANCHES */
	    [3] = 0xC2 | (1 << 16) | (1 << 22),
	},
    },
};

static void amd_report_metrics(void)
{
    /* Prefer CPU_CLK_UNHALTED over TSC, since the former respects USR-only CPL. */
    report_cpi(sums[0].pmc[0], sums[0].pmc[1]);
    report_bmr(sums[0].pmc[2], sums[0].pmc[3]);
}

static const struct setup amd_setup = {
    .nrconfigs = ARRAY_SIZE(amd_configs),
    .configs = amd_configs,
    .report_metrics = amd_report_metrics,
};

static const struct perfctr_cpu_control p4_configs[1] = {
    [0] = {
	.tsc_on = 1,
	.nractrs = 4,
	.pmc_map = {
	    [0] = 0x00 | (1 << 31), /* BPU_COUNTER0 */
	    [1] = 0x0C | (1 << 31), /* IQ_COUNTER0 */
	    [2] = 0x0D | (1 << 31), /* IQ_COUNTER1 */
	    [3] = 0x0E | (1 << 31), /* IQ_COUNTER2 */
	},
	.evntsel = {
	    /* BPU_CCCR0: ESCR 6 (FSB_ESCR0) */
	    [0] = (0x3 << 16) | (6 << 13) | (1 << 12),
	    /* IQ_CCCR0: ESCR 4 (CRU_ESCR0) */
	    [1] = (0x3 << 16) | (4 << 13) | (1 << 12),
	    /* IQ_CCCR1: ESCR 5 (CRU_ESCR2) */
	    [2] = (0x3 << 16) | (5 << 13) | (1 << 12),
	    /* IQ_CCCR2: ESCR 4 (CRU_ESCR1) */
	    [3] = (0x3 << 16) | (4 << 13) | (1 << 12),
	},
	.p4 = {
	    .escr = {
		/* FSB_ESCR0: GLOBAL_POWER_EVENTS, RUNNING */
		[0] = (0x13 << 25) | (1 << 9) | (1 << 2),
		/* CRU_ESCR0: INSTR_RETIRED, NBOGUSNTAG */
		[1] = (2 << 25) | (1 << 9) | (1 << 2),
		/* CRU_ESCR2: BRANCH_RETIRED, MMNP+MMNM+MMTP+MMTM */
		[2] = (6 << 25) | (0xF << 9) | (1 << 2),
		/* CRU_ESCR1: MISPRED_BRANCH_RETIRED, NBOGUS */
		[3] = (3 << 25) | (1 << 9) | (1 << 2),
	    },
	},
    },
};

static void p4_report_metrics(void)
{
    /* Prefer non-halted clockticks over non-sleep clockticks or TSC,
       since the former respects USR-only CPL. */
    report_cpi(sums[0].pmc[0], sums[0].pmc[1]);
    report_bmr(sums[0].pmc[3], sums[0].pmc[2]);
}

static const struct setup p4_setup = {
    .nrconfigs = ARRAY_SIZE(p4_configs),
    .configs = p4_configs,
    .report_metrics = p4_report_metrics,
};

static const struct perfctr_cpu_control p5mmx_configs[2] = {
    [0] = {
	.tsc_on = 1,
	.nractrs = 2,
	.pmc_map = {
	    [0] = 0,
	    [1] = 1,
	},
	.evntsel = {
	    /* NUMBER_OF_CYCLES_NOT_IN_HALT_STATE (PMC 0) MMX-only */
	    [0] = 0x30 | (2 << 6),
	    /* INSTRUCTIONS_EXECUTED (any PMC) */
	    [1] = 0x16 | (2 << 6),
	},
    },
    [1] = {
	.tsc_on = 1,
	.nractrs = 2,
	.pmc_map = {
	    [0] = 0,
	    [1] = 1,
	},
	.evntsel = {
	    /*
	     * The 1997 Intel Architecture Optimization manual (242816-003)
	     * defines BTB_HIT_RATE == BTB_HITS / BRANCHES.
	     * The miss rate can be derived from this, but the results
	     * are bogus (almost zero) on jump-indirect intensive code:
	     * it seems a jump-indirect that has an entry in the BTB with
	     * the wrong target still counts as a BTB hit.
	     *
	     * PIPELINE_FLUSHES / BRANCHES gives much more believable results.
	     * The predominant causes of PIPELINE_FLUSHES are BTB misses and
	     * mispredictions, so this makes sense.
	     */
	    /* BRANCHES (any PMC) */
	    [0] = 0x12 | (2 << 6),
	    /* PIPELINE_FLUSHES (any PMC) */
	    [1] = 0x15 | (2 << 6),
	},
    },
};

static void p5mmx_report_metrics(void)
{
    /* Prefer NUMBER_OF_CYCLES_NOT_IN_HALT_STATE over TSC,
       since the former respect USR-only CPL. */
    report_cpi(sums[0].pmc[0], sums[0].pmc[1]);
    report_bmr(sums[1].pmc[1], sums[1].pmc[0]);
}

static const struct setup p5mmx_setup = {
    .nrconfigs = ARRAY_SIZE(p5mmx_configs),
    .configs = p5mmx_configs,
    .report_metrics = p5mmx_report_metrics,
};

static const struct perfctr_cpu_control p5_configs[2] = {
    [0] = {
	.tsc_on = 1,
	.nractrs = 1,
	.pmc_map = {
	    [0] = 0,
	},
	.evntsel = {
	    /* INSTRUCTIONS_EXECUTED (any PMC) */
	    [0] = 0x16 | (2 << 6),
	},
    },
    [1] = {
	.tsc_on = 1,
	.nractrs = 2,
	.pmc_map = {
	    [0] = 0,
	    [1] = 1,
	},
	.evntsel = {
	    /* BRANCHES (any PMC) */
	    [0] = 0x12 | (2 << 6),
	    /* PIPELINE_FLUSHES (any PMC) */
	    [1] = 0x15 | (2 << 6),
	},
    },
};

static void p5_report_metrics(void)
{
    report_cpi(sums[0].tsc, sums[0].pmc[0]);
    report_bmr(sums[1].pmc[1], sums[1].pmc[0]);
}

static const struct setup p5_setup = {
    .nrconfigs = ARRAY_SIZE(p5_configs),
    .configs = p5_configs,
    .report_metrics = p5_report_metrics,
};

static const struct perfctr_cpu_control p6_configs[2] = {
    [0] = {
	.tsc_on = 1,
	.nractrs = 2,
	.pmc_map = {
	    [0] = 0,
	    [1] = 1,
	},
	.evntsel = {
	    /* CPU_CLK_UNHALTED */
	    [0] = 0x79 | (1 << 16) | (1 << 22),
	    /* INST_RETIRED */
	    [1] = 0xC0 | (1 << 16),
	},
    },
    [1] = {
	.tsc_on = 1,
	.nractrs = 2,
	.pmc_map = {
	    [0] = 0,
	    [1] = 1,
	},
	.evntsel = {
	    /*
	     * BR_MISS_PRED_RETIRED / BR_INST_RETIRED gives the miss
	     * ratio according to a 1997 Intel Optimisation manual.
	     *
	     * This only tells us about retired branches.
	     * Ideally we'd like to know about all executed
	     * branches, but only the P-M seems to be able
	     * to monitor them.
	     *
	     * BTB_MISSES and BR_BOGUS are not useful.
	     * BR_INST_DECODED is higher than BR_INST_RETIRED,
	     * but those extra branches are not necessarily
	     * ever executed.
	     */
	    /* BR_MISS_PRED_RETIRED */
	    [0] = 0xC5 | (1 << 16) | (1 << 22),
	    /* BR_INST_RETIRED */
	    [1] = 0xC4 | (1 << 16),
	},
    },
};

static void p6_report_metrics(void)
{
    /* Prefer CPU_CLK_UNHALTED over TSC, since the former respects USR-only CPL. */
    report_cpi(sums[0].pmc[0], sums[0].pmc[1]);
    report_bmr(sums[1].pmc[0], sums[1].pmc[1]);
}

static const struct setup p6_setup = {
    .nrconfigs = ARRAY_SIZE(p6_configs),
    .configs = p6_configs,
    .report_metrics = p6_report_metrics,
};

static void init_setup(void)
{
    switch (info.cpu_type) {
      case PERFCTR_X86_AMD_K7:
      case PERFCTR_X86_AMD_K8:
      case PERFCTR_X86_AMD_K8C:
	setup = &amd_setup;
	break;
      case PERFCTR_X86_INTEL_P4:
      case PERFCTR_X86_INTEL_P4M2:
      case PERFCTR_X86_INTEL_P4M3:
	setup = &p4_setup;
	break;
      case PERFCTR_X86_INTEL_P5MMX:
	setup = &p5mmx_setup;
	break;
      case PERFCTR_X86_INTEL_P5:
      case PERFCTR_X86_CYRIX_MII:
	setup = &p5_setup;
	break;
      case PERFCTR_X86_INTEL_P6:
      case PERFCTR_X86_INTEL_PII:
      case PERFCTR_X86_INTEL_PIII:
      case PERFCTR_X86_INTEL_PENTM:
	setup = &p6_setup;
	break;
      default:
	if (info.cpu_features & PERFCTR_FEATURE_RDTSC) {
	    setup = &generic_setup;
	    break;
	}
	fprintf(stderr, "cpu_type %u (%s) not supported\n",
		info.cpu_type, perfctr_info_cpu_name(&info));
	exit(1);
    }
}
#endif

#ifdef __powerpc__
static const struct perfctr_cpu_control ppc7450_configs[1] = {
    [0] = {
	.tsc_on = 1,
	.nractrs = 2,
	.pmc_map = {
	    [0] = 0, /* PMC1 */
	    [1] = 1, /* PMC2 */
	},
	.evntsel = {
	    /* COMPLETED_INSTRUCTIONS (PMC1-PMC4) */
	    [0] = 2,
	    /* PROCESSOR_CYCLES (PMC1-PMC6) */
	    [1] = 1,
	},
	.ppc = {
	    /* set FCS: freeze counters in supervisor-mode. */
	    .mmcr0 = 1 << (31-1),
	},
    },
};

static void ppc7450_report_metrics(void)
{
    /*
     * I've yet to find a way to derive believable Branch Miss rates
     * on my 7455. It seems indirect branches aren't included in the
     * relevant events.
     */
    /* Prefer PROCESSOR_CYCLES over scaled TB, since the former respects FCS. */
    report_cpi(sums[0].pmc[1], sums[0].pmc[0]);
}

static const struct setup ppc7450_setup = {
    .nrconfigs = ARRAY_SIZE(ppc7450_configs),
    .configs = ppc7450_configs,
    .report_metrics = ppc7450_report_metrics,
};

static void init_setup(void)
{
    switch (info.cpu_type) {
      case PERFCTR_PPC_7450:
	setup = &ppc7450_setup;
	break;
      default:
	if (info.cpu_features & PERFCTR_FEATURE_RDTSC) {
	    setup = &generic_setup;
	    break;
	}
	fprintf(stderr, "cpu_type %u (%s) not supported\n",
		info.cpu_type, perfctr_info_cpu_name(&info));
	exit(1);
    }       
}
#endif

static void init(void)
{
    self = vperfctr_open();
    if (!self) {
	perror("vperfctr_open");
	exit(1);
    }
    if (vperfctr_info(self, &info) < 0) {
	perror("vperfctr_info");
	exit(1);
    }
    init_setup();
}

static void each_config_run_code(void)
{
    unsigned int i;
    struct vperfctr_control control;

    for(i = 0; i < setup->nrconfigs; ++i) {
	memset(&control, 0, sizeof control);
	control.cpu_control = setup->configs[i];
	if (vperfctr_control(self, &control) < 0) {
	    perror("vperfctr_control");
	    exit(1);
	}
	run_code();
	vperfctr_read_ctrs(self, &sums[i]);
	vperfctr_stop(self);
    }
}

static void report(void)
{
    unsigned int i, j;

    printf("Perfctr Info:\n");
    perfctr_info_print(&info);
    for(i = 0; i < setup->nrconfigs; ++i) {
	printf("\nConfiguration #%u:\n", i);
	perfctr_cpu_control_print(&setup->configs[i]);
	if (setup->configs[i].tsc_on)
	    printf("tsc\t\t\t%lld\n", sums[i].tsc);
	for(j = 0; j < setup->configs[i].nractrs; ++j)
	    printf("pmc[%u]\t\t\t%lld\n", j, sums[i].pmc[j]);
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
