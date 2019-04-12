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

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "rapl.h"
#include "util.h"

const char *progname = "CPU Energy Meter"; // will be overwritten when parsing the command line
const char * const version = "1.1";

// Configuration (from command-line parameters)
static uint64_t delay = 0;
static const uint64_t delay_unit = 1000000000; // unit in nanoseconds
static int print_rawtext = 0;

static double convert_time_to_sec(struct timeval tv) {
  double elapsed_time = (double)(tv.tv_sec) + ((double)(tv.tv_usec) / 1000000);
  return elapsed_time;
}

/**
 * Create set with signals that we care about.
 */
static sigset_t get_sigset() {
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGUSR1);
  return set;
}

/**
 * Print header of measurements.
 */
static void print_global_header(int num_node, double duration) {
  if (print_rawtext) {
    fprintf(stdout, "\ncpu_count=%d\n", num_node);
    fprintf(stdout, "duration_seconds=%f\n", duration);
  }
}

/**
 * Print socket-specific header of measurements.
 */
static void print_header(int socket, double duration) {
  if (!print_rawtext) {
    fprintf(stdout, "\b\b+--------------------------------------+\n");
    fprintf(stdout, "| CPU Energy Meter            Socket %u |\n", socket);
    fprintf(stdout, "+--------------------------------------+\n");
    fprintf(stdout, "%-19s %14.6lf sec\n", "Duration", duration);
  }
}

static void print_value(int socket, int domain, double value_J) {
  if (value_J == 0.0) {
    // Sometimes measurement seems to work but energy consumption is 0.
    // This means an unsupported domain, because even for short measurements value would be larger.
    return;
  }

  const char *domain_string;
  if (print_rawtext) {
    domain_string = RAPL_DOMAIN_STRINGS[domain];
    fprintf(stdout, "cpu%d_%s_joules=%f\n", socket, domain_string, value_J);
  } else {
    domain_string = RAPL_DOMAIN_FORMATTED_STRINGS[domain];
    fprintf(stdout, "%-19s %14.6f Joule\n", domain_string, value_J);
  }
}

static void print_results(
    int num_node,
    double cum_energy_J[num_node][RAPL_NR_DOMAIN],
    struct timeval measurement_start_time,
    struct timeval measurement_end_time) {

  const double duration =
      convert_time_to_sec(measurement_end_time) - convert_time_to_sec(measurement_start_time);
  print_global_header(num_node, duration);

  for (int i = 0; i < num_node; i++) {
    print_header(i, duration);

    for (int domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
      if (is_supported_domain(domain)) {
        print_value(i, domain, cum_energy_J[i][domain]);
      }
    }
  }
}

static struct timespec compute_msr_probe_interval_time() {
  struct timespec signal_timelimit;
  if (delay) {
    // delay set by user; i.e. use the according values and return
    signal_timelimit.tv_sec = delay / delay_unit;
    signal_timelimit.tv_nsec = delay % delay_unit;
  } else {
    signal_timelimit.tv_sec = get_maximum_read_interval();
    signal_timelimit.tv_nsec = 0;
  }
  DEBUG("Interval time of msr probes set to %lds, %ldns.",
      signal_timelimit.tv_sec, signal_timelimit.tv_nsec);
  return signal_timelimit;
}

/**
 * Read measurements and write them to current_measurements.
 * If cum_energy_J is not NULL, read previous measurements from current_measurements
 * and accumulate delta in cum_energy_J.
 */
static int read_measurements(
    int num_node,
    double current_measurements[num_node][RAPL_NR_DOMAIN],
    double cum_energy_J[num_node][RAPL_NR_DOMAIN],
    struct timeval *measurement_time) {
  int result = 0;
  for (int i = 0; i < num_node; i++) {
    for (int domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
      if (is_supported_domain(domain)) {
        double new_sample;
        if (get_total_energy_consumed(i, domain, &new_sample) != 0) {
          warnx("Measuring domain %s of CPU %d failed.", RAPL_DOMAIN_FORMATTED_STRINGS[domain], i);
          result = 1;
          continue; // at least continue reading other domains
        }

        if (cum_energy_J != NULL) {
          double delta = new_sample - current_measurements[i][domain];

          /* Handle wraparound */
          if (delta < 0) {
            delta += MAX_ENERGY_STATUS_JOULES;
          }

          cum_energy_J[i][domain] += delta;
        }

        current_measurements[i][domain] = new_sample;
      }
    }
  }

  gettimeofday(measurement_time, NULL);
  return result;
}

static int measure_and_print_results() {
  const int num_node = get_num_rapl_nodes();
  struct timeval measurement_start_time, measurement_end_time;
  double prev_sample[num_node][RAPL_NR_DOMAIN];

  // Read initial values
  if (read_measurements(num_node, prev_sample, NULL, &measurement_start_time) != 0) {
    return 1;
  }

  double cum_energy_J[num_node][RAPL_NR_DOMAIN];
  memset(cum_energy_J, 0, sizeof(cum_energy_J));
  const struct timespec signal_timelimit = compute_msr_probe_interval_time();
  const sigset_t signal_set = get_sigset();

  // Actual measurement loop
  while (true) {
    // Wait for signal or timeout
    const int rcvd_signal = sigtimedwait(&signal_set, NULL, &signal_timelimit);

    // handle errors
    if (rcvd_signal == -1) {
      if (errno == EAGAIN) {
        DEBUG("Time limit elapsed, reading values to ensure overflows are detected.%s", "");
      } else if (errno == EINTR) {
        // interrupted, just try again
      } else {
        warn("Waiting for signal failed.");
        return 1;
      }
    }

    // make sure to read in each iteration, otherwise we might miss overflows
    if (read_measurements(num_node, prev_sample, cum_energy_J, &measurement_end_time) != 0) {
      return 1;
    }

    // handle signals
    if (rcvd_signal != -1) {
      DEBUG("Received signal %d.", rcvd_signal);
      if (rcvd_signal == SIGINT) {
        print_results(num_node, cum_energy_J, measurement_start_time, measurement_end_time);
        break;

      } else if (rcvd_signal == SIGUSR1) {
        print_results(num_node, cum_energy_J, measurement_start_time, measurement_end_time);

      } else {
        warnx("Received unexpected signal %d", rcvd_signal);
        return 1;
      }
    }
  }

  return 0;
}

static void usage(FILE *target) {
  fprintf(target, "\n");
  fprintf(target, "CPU Energy Meter v%s\n", version);
  fprintf(target, "\n");
  fprintf(target, "Usage: %s [OPTION]...\n", progname);
  fprintf(target, "  %-20s %s\n", "-d", "print additional debug information to the output");
  fprintf(target, "  %-20s %s\n", "-e=MILLISEC", "set the sampling delay in ms");
  fprintf(target, "  %-20s %s\n", "-h", "show this help text");
  fprintf(target, "  %-20s %s\n", "-r", "print the output as raw-text");
  fprintf(target, "\n");
  fprintf(target, "Example: %s -r\n", progname);
  fprintf(target, "\n");
}

static int read_cmdline(int argc, char **argv) {
  progname = argv[0];

  int opt;
  while ((opt = getopt(argc, argv, "de:hr")) != -1) {
    switch (opt) {
    case 'd':
      enable_debug();
      break;
    case 'e': {
      uint64_t delay_ms_temp = atoi(optarg);
      if (delay_ms_temp > 50) {
        delay = delay_ms_temp * 1000000; // delay in ns
      } else {
        fprintf(stderr, "Sampling delay must be greater than 50 ms.\n");
        return -1;
      }
      break;
    }
    case 'h':
      usage(stdout);
      exit(0);
      break;
    case 'r':
      print_rawtext = 1;
      break;
    default:
      usage(stderr);
      return -1;
    }
  }
  if (optind < argc) {
    warnx("no positional argument expected");
    usage(stderr);
    return -1;
  }
  return 0;
}

int main(int argc, char **argv) {
  // Check cmdline first because we don't want to attempt accessing MSRs if -h is given.
  if (0 != read_cmdline(argc, argv)) {
    return 1;
  }
  int result = 0;

  // Block signals as fast as possible to ensure proper results if we get a signal soon
  const sigset_t signal_set = get_sigset();
  if (sigprocmask(SIG_BLOCK, &signal_set, NULL)) {
    err(1, "Failed to block signals");
  }

  // Initialize RAPL
  if (0 != init_rapl()) {
    fprintf(stderr, "Cannot access RAPL!\n");
    result = 1;
    goto out;
  }

  drop_root_privileges_by_id(UID_NOBODY, GID_NOGROUP);
  drop_capabilities();

  // Turn off buffering to ensure intermediate results are not delayed
  setbuf(stdout, NULL);

  result = measure_and_print_results();

out:
  terminate_rapl();
  sigprocmask(SIG_UNBLOCK, &signal_set, NULL);
  return result;
}
