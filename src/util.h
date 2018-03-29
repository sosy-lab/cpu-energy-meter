/*
Copyright (c) 2003, O'Reilly: Taken from "Secure Programming Cookbook for C and C++"
(https://resources.oreilly.com/examples/9780596003944/)
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

#ifndef _h_util
#define _h_util

#include <sys/capability.h>

static const uid_t UID_NOBODY = 65534;
static const gid_t GID_NOGROUP = 65534;

enum { TEMPORARY = 0, PERMANENT = 1 };

/*
 * The documentation regarding the capabilities was taken from the linux manual pages (i.e.,
 * http://man7.org/linux/man-pages/man3/cap_get_proc.3.html and
 * http://man7.org/linux/man-pages/man3/cap_clear.3.html) [links from Dec. 14, 2017].
 *
 * Note that in order to execute the code on linux, the 'libcap-dev'-package needs to be available
 * on the working machine.
 */

/*
 * Drop all capabilities that the process is currently in possession of.
 *
 * Return 0 on success, or -1 otherwise.
 */
int drop_capabilities();

/*
 * Documentation and source code for dropping and restoring the root privileges can be found at
 * https://www.safaribooksonline.com/library/view/secure-programming-cookbook/0596003943/ch01s03.html
 * [link from Nov. 28, 2017]
 */

/**
 * Drop any extra group or user privileges either permanently or temporarily, depending on the value
 * of the argument. If a nonzero value is passed, privileges will be dropped permanently; otherwise,
 * the privilege drop is temporary. Custom values can be specified for uid and gid to be taken as
 * new id in the process.
 */
void drop_root_privileges_by_id(int permanent, uid_t uid, gid_t gid);

/**
 * Drop any extra group or user privileges either permanently or temporarily, depending on the value
 * of the argument. If a nonzero value is passed, privileges will be dropped permanently; otherwise,
 * the privilege drop is temporary.
 */
void drop_root_privileges(int permanent);

/**
 *  Restore privileges to what they were at the last call to spc_drop_privileges().
 */
void restore_root_privileges(void);

#endif /* _h_util */
