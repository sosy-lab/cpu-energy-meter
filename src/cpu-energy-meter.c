/*
Copyright (c) 2012, Intel Corporation
Copyright (c) 2015-2018, Dirk Beyer

Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions
and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials provided with
the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse
or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <assert.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "rapl.h"
#include "util.h"

const char *progname = "CPU Energy Meter"; // will be overwritten when parsing the command line
const char *version = "1.1-dev";

int num_node = 0;
uint64_t delay = 0;
uint64_t delay_unit = 1000000000; // unit in nanoseconds
uint64_t print_rawtext = 0;

double **cum_energy_J = NULL;
struct timeval measurement_start_time, measurement_end_time;

void convert_time_to_string(struct timeval tv, char *time_buf) {
  time_t sec;
  int msec;
  struct tm *timeinfo;
  char tmp_buf[9];

  sec = tv.tv_sec;
  timeinfo = localtime(&sec);
  msec = tv.tv_usec / 1000;

  strftime(tmp_buf, 9, "%H:%M:%S", timeinfo);
  sprintf(time_buf, "%s:%d", tmp_buf, msec);
}

double convert_time_to_sec(struct timeval tv) {
  double elapsed_time = (double)(tv.tv_sec) + ((double)(tv.tv_usec) / 1000000);
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

void print_header(int socket) {
  double start_seconds = convert_time_to_sec(measurement_start_time);
  double end_seconds = convert_time_to_sec(measurement_end_time);

  if (print_rawtext) {
    fprintf(stdout, "\ncpu_count=%d\n", num_node);
    fprintf(stdout, "duration_seconds=%f\n", end_seconds - start_seconds);
  } else {
    fprintf(stdout, "\b\b+--------------------------------------+\n");
    fprintf(stdout, "| %-21s %12s %u |\n", "CPU-Energy-Meter", "Socket", socket);
    fprintf(stdout, "+--------------------------------------+\n");
    fprintf(stdout, "%-19s %14.6lf %s\n", "Duration", end_seconds - start_seconds, "sec");
  }
}

void print_domain(int socket, int domain) {
  if (cum_energy_J[socket][domain] == 0.0) {
    // The values in the double-array were explicitly initialized with 0.0, that's why we can safely
    // check with '=='-equality here
    return;
  }

  char *domain_string;
  if (print_rawtext) {
    domain_string = RAPL_DOMAIN_STRINGS[domain];
    fprintf(stdout, "cpu%d_%s_joules=%f\n", socket, domain_string, cum_energy_J[socket][domain]);
  } else {
    domain_string = RAPL_DOMAIN_FORMATTED_STRINGS[domain];
    fprintf(stdout, "%-19s %14.6f %s\n", domain_string, cum_energy_J[socket][domain], "Joule");
  }
}

void print_intermediate_results() {
  int domain;

  if (cum_energy_J != NULL) {
    for (int i = 0; i < num_node; i++) {
      if (cum_energy_J[i] == NULL) {
        continue;
      }

      print_header(i);

      for (domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
        if (is_supported_domain(domain)) {
          print_domain(i, domain);
        }
      }
    }
  }
}

// Returns 1 if the process is supposed to continue, 0 if the process is supposed to stop
int handle_signal(int sig) {
  if (sig < 0) {
    return 1;

  } else if (sig == SIGINT || sig == SIGQUIT) {
    print_intermediate_results();
    return 0;

  } else if (sig == SIGUSR1) {
    print_intermediate_results();
    return 1;

  } else {
    printf("Didn't handle signal number %d", sig);
    return 1;
  }
}

void compute_msr_probe_interval_time(struct timespec *signal_timelimit) {
  if (delay) {
    // delay set by user; i.e. use the according values and return
    long seconds = delay / delay_unit;
    long nano_seconds = delay % delay_unit;

    signal_timelimit->tv_sec = seconds;
    signal_timelimit->tv_nsec = nano_seconds;
    return;
  }

  long seconds = get_maximum_read_interval();
  signal_timelimit->tv_sec = seconds;
  signal_timelimit->tv_nsec = 0;
}

void do_print_energy_info() {
  struct timespec signal_timelimit;
  compute_msr_probe_interval_time(&signal_timelimit);
  if (debug_enabled) {
    fprintf(stdout, "[DEBUG] Interval time of msr probes set to %lds, %ldns:\n",
            signal_timelimit.tv_sec, signal_timelimit.tv_nsec);
  }

  sigset_t signal_set = get_sigset();
  int domain = 0;
  int node = 0;
  double new_sample;
  double delta;

  double prev_sample[num_node][RAPL_NR_DOMAIN];
  cum_energy_J = calloc(num_node, sizeof(double *));
  for (int i = 0; i < num_node; i++) {
    cum_energy_J[i] = calloc(RAPL_NR_DOMAIN, sizeof(double));
  }

  struct timeval tv;

  /* don't buffer if piped */
  setbuf(stdout, NULL);

  /* Read initial values */
  for (int i = node; i < num_node; i++) {
    for (domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
      if (is_supported_domain(domain)) {
        get_total_energy_consumed(i, domain, &prev_sample[i][domain]);
      }
    }
  }

  gettimeofday(&tv, NULL);
  measurement_start_time = tv;
  measurement_end_time = measurement_start_time;

  char time_string[12];
  convert_time_to_string(tv, time_string);
  // fprintf(stdout, "start_time=%f (%s o'clock)\n", convert_time_to_sec(tv), time_string);

  int rcvd_signal;
  int do_continue = 1;
  siginfo_t signal_info;
  /* Begin sampling */
  while (do_continue) {
    // If a signal is received, perform one probe before handling it.
    rcvd_signal = sigtimedwait(&signal_set, &signal_info, &signal_timelimit);

    for (int i = node; i < num_node; i++) {
      for (domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
        if (is_supported_domain(domain)) {
          get_total_energy_consumed(i, domain, &new_sample);
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
    if (rcvd_signal != -1) {
      do_continue = handle_signal(rcvd_signal);
    }
  }
}

void usage() {
  fprintf(stdout, "\n");
  fprintf(stdout, "CPU Energy Meter v%s\n", version);
  fprintf(stdout, "\n");
  fprintf(stdout, "Usage: %s [OPTION]...\n", progname);
  fprintf(stdout, "  %-20s %s\n", "-d", "print additional debug information to the output");
  fprintf(stdout, "  %-20s %s\n", "-e=MILLISEC", "set the sampling delay in ms");
  fprintf(stdout, "  %-20s %s\n", "-h", "show this help text");
  fprintf(stdout, "  %-20s %s\n", "-r", "print the output as raw-text");
  fprintf(stdout, "\n");
  fprintf(stdout, "Example: %s -r\n", progname);
  fprintf(stdout, "\n");
}

int read_cmdline(int argc, char **argv) {
  int opt;
  uint64_t delay_ms_temp = 1000;

  progname = argv[0];

  while ((opt = getopt(argc, argv, "de:hr")) != -1) {
    switch (opt) {
    case 'd':
      debug_enabled = 1;
      break;
    case 'e':
      delay_ms_temp = atoi(optarg);
      if (delay_ms_temp > 50) {
        delay = delay_ms_temp * 1000000; // delay in ns
      } else {
        fprintf(stdout, "Sampling delay must be greater than 50 ms.\n");
        return -1;
      }
      break;
    case 'h':
      usage();
      exit(0);
      break;
    case 'r':
      print_rawtext = 1;
      break;
    default:
      usage();
      return -1;
    }
  }
  return 0;
}

int main(int argc, char **argv) {
  if (0 != read_cmdline(argc, argv)) {
    // Error occured while reading command line
    return MY_ERROR;
  }

  sigset_t signal_set = get_sigset();
  sigprocmask(SIG_BLOCK, &signal_set, NULL);
  // First init the RAPL library
  if (0 != init_rapl()) {
    fprintf(stdout, "Init failed!\n");
    terminate_rapl();
    return MY_ERROR;
  }

  drop_root_privileges_by_id(UID_NOBODY, GID_NOGROUP);
  drop_capabilities();

  num_node = get_num_rapl_nodes();
  if (debug_enabled) {
    fprintf(stdout, "[DEBUG] Number of nodes detected: %d\n", num_node);
  }

  do_print_energy_info();

  terminate_rapl();
  sigprocmask(SIG_UNBLOCK, &signal_set, NULL);
  return 0;
}
