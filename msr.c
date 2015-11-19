/*
Copyright (c) 2012, Intel Corporation

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Written by Martin Dimitrov, Carl Strickland */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "msr.h"


/*
 * read_msr
 *
 * Will return 0 on success and MY_ERROR on failure.
 */
int
read_msr(int       cpu,
         uint64_t  address,
         uint64_t *value)
{
    int   err = 0;
    char  msr_path[32];
    FILE *fp;

    sprintf(msr_path, "/dev/cpu/%d/msr", cpu);
    err = ((fp = fopen(msr_path, "r")) == NULL);
    if (!err)
        err = (fseek(fp, address, SEEK_CUR) != 0);
    if (!err)
        err = (fread(value, sizeof(uint64_t), 1, fp) != 1);
    if (fp != NULL)
        fclose(fp);
    return err;
}



/*
 * write_msr
 *
 * Will return 0 on success and MY_ERROR on failure.
 */

int
write_msr(int      cpu,
          uint64_t address,
          uint64_t value)
{
    int   err = 0;
    char  msr_path[32];
    FILE *fp;

    sprintf(msr_path, "/dev/cpu/%d/msr", cpu);
    err = ((fp = fopen(msr_path, "w")) == NULL);
    if (!err)
        err = (fseek(fp, address, SEEK_CUR) != 0);
    if (!err)
        err = (fwrite(&value, sizeof(uint64_t), 1, fp) != 1);
    if (fp != NULL)
        fclose(fp);
    return err;
}

