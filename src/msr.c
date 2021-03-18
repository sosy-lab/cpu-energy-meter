// This file is part of CPU Energy Meter,
// a tool for measuring energy consumption of Intel CPUs:
// https://github.com/sosy-lab/cpu-energy-meter
//
// SPDX-FileCopyrightText: 2012 Intel Corporation
// SPDX-FileCopyrightText: 2015-2021 Dirk Beyer <https://www.sosy-lab.org>
//
// SPDX-License-Identifier: BSD-3-Clause

#include "msr.h"
#include "util.h"

#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int *fds;
static int fds_size = 0;

int open_msr_fd(int num_nodes, int (*pkg_map)(int)) {
  assert(fds_size == 0);
  assert(fds == NULL);
  int result = 0;

  fds_size = num_nodes;
  fds = calloc(fds_size, sizeof(int));

  for (int node = 0; node < fds_size; node++) {
    char msr_path[32];
    sprintf(msr_path, "/dev/cpu/%u/msr", pkg_map(node));
    DEBUG("Using %s for accessing MSR of socket %d.", msr_path, node);
    int fd = open(msr_path, O_RDONLY);
    if (fd == -1) {
      warn("Could not open %s", msr_path);
      result = -1;
    }

    fds[node] = fd;
  }

  return result;
}

int read_msr(int node, off_t address, uint64_t *value) {
  assert(node < fds_size);

  int fd = fds[node];
  if (fd == -1) {
    return -1; // had failed to open
  }

  if (lseek(fd, address, SEEK_SET) < 0) {
    warn("Could not seek to address 0x%lX for reading from MSR for CPU %u", address, node);
    return -1;
  }

  if (read(fd, value, sizeof(uint64_t)) != sizeof(uint64_t)) {
    // expected if hardware does not support this domain
    // warn("Could not read from address 0x%lX of MSR for CPU %u", address, node);
    return -1;
  }

  return 0;
}

void close_msr_fd() {
  if (fds == NULL) {
    return;
  }

  for (int node = 0; node < fds_size; node++) {
    if (fds[node] != -1) {
      close(fds[node]);
    }
  }
  free(fds);
  fds = NULL;
  fds_size = 0;
}
