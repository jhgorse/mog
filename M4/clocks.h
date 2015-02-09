///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file clocks.h
///
/// Copyright (c) 2015, BoxCast, Inc. All rights reserved.
///
/// This library is free software; you can redistribute it and/or modify it under the terms of the
/// GNU Lesser General Public License as published by the Free Software Foundation; either version
/// 3.0 of the License, or (at your option) any later version.
///
/// This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
/// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
/// the GNULesser General Public License for more details.
///
/// You should have received a copy of the GNU Lesser General Public License along with this
/// library; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
/// Boston, MA 02110-1301 USA
///
/// @brief This file declares Posix clock functions that may be implemented in platform-specific
/// ways.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __CLOCKS_H__
#define __CLOCKS_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __APPLE__
// On Apple, clock_gettime is not defined. We define it here in terms of available functions.
#include <sys/types.h>
#include <sys/_types/_timespec.h>
#include <mach/mach.h>
#include <mach/clock.h>

#define CLOCK_MONOTONIC SYSTEM_CLOCK

typedef int clockid_t;

static inline int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), clk_id, &cclock);
	kern_return_t retval = clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	tp->tv_sec = mts.tv_sec;
	tp->tv_nsec = mts.tv_nsec;
    return retval;
}

#else
// On non-Apple, we assume Posix clock functions are available via time.h.
#include <time.h>
#endif

#ifdef __cplusplus

// Comparison functions for struct timespec
static inline bool operator< (const struct timespec& lhs, const struct timespec& rhs){return (lhs.tv_sec < rhs.tv_sec) || (lhs.tv_nsec < rhs.tv_nsec);}
static inline bool operator> (const struct timespec& lhs, const struct timespec& rhs){return rhs < lhs;}
static inline bool operator<=(const struct timespec& lhs, const struct timespec& rhs){return !(lhs > rhs);}
static inline bool operator>=(const struct timespec& lhs, const struct timespec& rhs){return !(lhs < rhs);}
static inline bool operator==(const struct timespec& lhs, const struct timespec& rhs){return (lhs.tv_sec == rhs.tv_sec) && (lhs.tv_nsec == rhs.tv_nsec);}
static inline bool operator!=(const struct timespec& lhs, const struct timespec& rhs){return !(lhs == rhs);}

#endif

#ifdef __cplusplus
}
#endif

#endif // __CLOCKS_H__