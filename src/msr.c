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
