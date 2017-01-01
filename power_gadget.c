/*
Copyright (c) 2012, Intel Corporation

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Written by Martin Dimitrov, Carl Strickland */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

#include "rapl.h"

char         *progname;
const char   *version = "2.2";
uint64_t      num_node = 0;
uint64_t      delay_us = 1000000;
double        delay_unit = 1000000.0;

double **cum_energy_J = NULL;
struct timeval measurement_start_time, measurement_end_time;
// Not to be confused with is_supported_domain()!
double **rapl_domain_actually_supported = NULL;

int
get_rapl_energy_info(uint64_t power_domain, uint64_t node, double *total_energy_consumed)
{
    int          err;

    switch (power_domain) {
    case PKG:
        err = get_pkg_total_energy_consumed(node, total_energy_consumed);
        break;
    case PP0:
        err = get_pp0_total_energy_consumed(node, total_energy_consumed);
        break;
    case PP1:
        err = get_pp1_total_energy_consumed(node, total_energy_consumed);
        break;
    case DRAM:
        err = get_dram_total_energy_consumed(node, total_energy_consumed);
        break;
    default:
        err = MY_ERROR;
        break;
    }

    return err;
}

void
convert_time_to_string(struct timeval tv, char* time_buf)
{
    time_t sec;
    int msec;
    struct tm *timeinfo;
    char tmp_buf[9];

    sec = tv.tv_sec;
    timeinfo = localtime(&sec);
    msec = tv.tv_usec/1000;

    strftime(tmp_buf, 9, "%H:%M:%S", timeinfo);
    sprintf(time_buf, "%s:%d",tmp_buf,msec);
}

double
convert_time_to_sec(struct timeval tv)
{
    double elapsed_time = (double)(tv.tv_sec) + ((double)(tv.tv_usec)/1000000);
    return elapsed_time;
}

sigset_t get_sigset() {
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGQUIT);
  sigaddset(&set, SIGUSR1);
  return set;
}

void print_intermediate_results() {
    int i, domain;
    uint64_t freq;

    double start_seconds = convert_time_to_sec(measurement_start_time);
    double end_seconds = convert_time_to_sec(measurement_end_time);
    char end_time_string[12];
    convert_time_to_string(measurement_end_time, end_time_string);
    fprintf(stdout, "curr_time=%f (%s o'clock)\n", end_seconds, end_time_string);
    fprintf(stdout, "tot_duration=%f\n", end_seconds - start_seconds);

    if (cum_energy_J != NULL) {
        for (i = 0; i < num_node; i++) {
            if (cum_energy_J[i] == NULL) {
                continue;
            }

            for (domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
                if (is_supported_domain(domain)) {
                    char *domain_string = RAPL_DOMAIN_STRINGS[domain];
                    fprintf(stdout, "cpu%d_%s_Joules=%f\n", i, domain_string, cum_energy_J[i][domain]);
                }
            }
        }
    }
}

void handle_sigint()
{
    print_intermediate_results();
    terminate_rapl();
    exit(0);
}

void handle_signal(int sig, siginfo_t * info) {
  if (sig < 0) {
    return;

  } else if (sig == SIGINT || sig == SIGQUIT) {
    handle_sigint();

  } else if (sig == SIGUSR1) {
    print_intermediate_results();

  } else {
    printf("Didn't handle signal number %d", sig);
  }
}

void
do_print_energy_info()
{
    struct timespec signal_timelimit = { .tv_sec = 0, .tv_nsec = 10 };
    sigset_t signal_set = get_sigset();
    int i = 0;
    int domain = 0;
    int err = 0;
    uint64_t node = 0;
    double new_sample;
    double delta;
    double power;

    double prev_sample[num_node][RAPL_NR_DOMAIN];
    double power_watt[num_node][RAPL_NR_DOMAIN];
    cum_energy_J = calloc(num_node, sizeof(double*));
    for (i = 0; i < num_node; i++) {
        cum_energy_J[i] = calloc(RAPL_NR_DOMAIN, sizeof(double));
    }

    char time_buffer[32];
    struct timeval tv;
    int msec;
    uint64_t tsc;
    uint64_t freq;

    /* don't buffer if piped */
    setbuf(stdout, NULL);

    fprintf(stdout, "cpu_count=%lu\n", num_node);

    /* Read initial values */
    rapl_domain_actually_supported = calloc(num_node, sizeof(double*));
    for (i = node; i < num_node; i++) {
        rapl_domain_actually_supported[node] = calloc(RAPL_NR_DOMAIN, sizeof(double));
        for (domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
            rapl_domain_actually_supported[node][domain] = 0;

            if(is_supported_domain(domain)) {
                rapl_domain_actually_supported[node][domain] = 1;

                if (0 != get_rapl_energy_info(domain, i, &(prev_sample[i][domain]))) {
                    rapl_domain_actually_supported[node][domain] = 0;
                }
            }
        }
    }

    gettimeofday(&tv, NULL);
    measurement_start_time = tv;
    measurement_end_time = measurement_start_time;

    char time_string[12];
    convert_time_to_string(tv, time_string);
    fprintf(stdout, "start_time=%f (%s o'clock)\n", convert_time_to_sec(tv), time_string);

    sigprocmask(SIG_BLOCK, &signal_set, NULL);

    int rcvd_signal;
    siginfo_t signal_info;
    /* Begin sampling */
    while (1) {
        do {
          rcvd_signal = sigtimedwait(&signal_set, &signal_info, &signal_timelimit);
          handle_signal(rcvd_signal, &signal_info);
        } while (rcvd_signal > -1);

        for (i = node; i < num_node; i++) {
            for (domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
                if (rapl_domain_actually_supported[node][domain]) {
                    err = get_rapl_energy_info(domain, i, &new_sample);
                    delta = new_sample - prev_sample[i][domain];

                    /* Handle wraparound */
                    if (delta < 0) {
                        delta += MAX_ENERGY_STATUS_JOULES;
                    }

                    prev_sample[i][domain] = new_sample;

                    cum_energy_J[i][domain] += delta;
                }
            }
        }

        gettimeofday(&tv, NULL);
        measurement_end_time = tv;
    }
    sigprocmask(SIG_UNBLOCK, &signal_set, NULL);
}

void
usage()
{
    fprintf(stdout, "\nIntel(r) Power Gadget %s\n", version);
    fprintf(stdout, "\nUsage: \n");
    fprintf(stdout, "%s [-e [sampling delay (ms) ] optional]\n", progname);
    fprintf(stdout, "\nExample: %s -e 1000 -d 10\n", progname);
    fprintf(stdout, "\n");
}


int
cmdline(int argc, char **argv)
{
    int             opt;
    uint64_t    delay_ms_temp = 1000;

    progname = argv[0];

    while ((opt = getopt(argc, argv, "e:")) != -1) {
        switch (opt) {
        case 'e':
            delay_ms_temp = atoi(optarg);
            if(delay_ms_temp > 50) {
                delay_us = delay_ms_temp * 1000;
            } else {
                fprintf(stdout, "Sampling delay must be greater than 50 ms.\n");
                return -1;
            }
            break;
        case 'h':
            usage();
            exit(0);
            break;
        default:
            usage();
            return -1;
        }
    }
    return 0;
}

int
main(int argc, char **argv)
{
    int i = 0;
    int ret = 0;

    // First init the RAPL library
    if (0 != init_rapl()) {
        fprintf(stdout, "Init failed!\n");
        terminate_rapl();
        return MY_ERROR;
    }
    num_node = get_num_rapl_nodes_pkg();

    ret = cmdline(argc, argv);
    if (ret) {
        terminate_rapl();
        return ret;
    }

    do_print_energy_info();

    terminate_rapl();
}
