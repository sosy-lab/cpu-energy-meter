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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "msr.h"

int *fds;
size_t fds_size;

/*
 * open_msr_fd
 *
 * Returns 0 on success and MY_ERROR, if at least one msr-file fails to open.
 */
int open_msr_fd(uint64_t num_nodes) {
  int err = 0;
  int fd = 0;
  char msr_path[32];

  fds_size = num_nodes;
  fds = malloc(fds_size * sizeof(int));

  for (size_t i = 0; i < fds_size; i++) {
    sprintf(msr_path, "/dev/cpu/%ld/msr", i);
    fd = open(msr_path, O_RDONLY);
    fds[i] = fd;

    if (fd == -1) {
      err = MY_ERROR;
    }
  }

  return err;
}

/*
 * read_msr
 *
 * Will return 0 on success and MY_ERROR on failure.
 */
int read_msr(int cpu, uint64_t address, uint64_t *value) {
  int err = 0;
  FILE *fp;

  // dup is used here to clone the fd. This way, we can close the stream afterwards, while we still
  // retain an open file descriptor.
  fp = fdopen(dup(fds[cpu]), "r");
  err = fp == NULL;
  if (!err) {
    err = (fseek(fp, address, SEEK_SET) != 0);
  }
  if (!err) {
    err = (fread(value, sizeof(uint64_t), 1, fp) != 1);
  }
  if (fp != NULL) {
    fclose(fp);
  }
  return err;
}

/*
 * close_msr_fd
 *
 * Close each file descriptor and free the allocated memory for the fds-array.
 */
void close_msr_fd() {
  for (size_t i = 0; i < fds_size; i++) {
    if (fds[i] >= 0) {
      close(fds[i]);
    }
  }
  free(fds);
}
