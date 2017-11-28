/*
Copyright (c) 2012, Intel Corporation

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <sys/param.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <grp.h>
#include <stdio.h>

#include "util.h"

static int   orig_ngroups = -1;
static gid_t orig_gid = -1;
static uid_t orig_uid = -1;
static gid_t orig_groups[NGROUPS_MAX];

/**
 * Drop privileges permanently in case a nonzero value is passed; otherwise, the 
 * privilege drop is temporary.
 * 
 * Warning: 
 * If any problems are encountered in attempting to perform the task, abort()
 * is called, terminating the process immediately. If any manipulation of privileges
 * cannot complete successfully, it's safest to assume that the process is in an
 * unknown state, and you should not allow it to continue.
 */
void drop_root_privileges(int permanent) {
    gid_t newgid = getgid(), oldgid = getegid();
    uid_t newuid = getuid(), olduid = geteuid();
   
    if (!permanent) {
        /* Save information about the privileges that are being dropped so that they
         * can be restored later.
         */
        orig_gid = oldgid;
        orig_uid = olduid;
        orig_ngroups = getgroups(NGROUPS_MAX, orig_groups);
    }
   
    /* If root privileges are to be dropped, be sure to pare down the ancillary
     * groups for the process before doing anything else because the setgroups()
     * system call requires root privileges.  Drop ancillary groups regardless of
     * whether privileges are being dropped temporarily or permanently.
     */
    if (!olduid) 
        setgroups(1, &newgid);
   
    if (newgid != oldgid) {
#if !defined(linux)
        setegid(newgid);
        if (permanent && setgid(newgid) == -1)
            abort();
#else
        if (setregid((permanent ? newgid : -1), newgid) == -1)
            abort();
#endif
    }
   
    if (newuid != olduid) {
#if !defined(linux)
        seteuid(newuid);
        if (permanent && setuid(newuid) == -1) 
            abort();
#else
        if (setreuid((permanent ? newuid : -1), newuid) == -1)
            abort();
#endif
    }
   
    /* verify that the changes were successful */
    if (permanent) {
        if (newgid != oldgid && (setegid(oldgid) != -1 || getegid() != newgid))
            abort();
        if (newuid != olduid && (seteuid(olduid) != -1 || geteuid() != newuid))
            abort();
    } else {
        if (newgid != oldgid && getegid() != newgid)
            abort();
        if (newuid != olduid && geteuid() != newuid)
            abort();
    }
}

/**
 * Restore privileges to what they were at the last call to spc_drop_privileges().
 * 
 * Warning: 
 * If any problems are encountered in attempting to perform the task, abort()
 * is called, terminating the process immediately. If any manipulation of privileges
 * cannot complete successfully, it's safest to assume that the process is in an
 * unknown state, and you should not allow it to continue.
 */
void restore_root_privileges(void) {
    if (geteuid() != orig_uid)
        if (seteuid(orig_uid) == -1 || geteuid() != orig_uid)
            abort();
    
    if (getegid() != orig_gid)
        if (setegid(orig_gid) == -1 || getegid() != orig_gid)
            abort();
    
    if (!orig_uid)
        setgroups(orig_ngroups, orig_groups);
}