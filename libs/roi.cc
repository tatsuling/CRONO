/*
 * 
 * */

#include <iostream>
#include "roi.h"

#ifdef ENABLE_TIMING
#include <sys/time.h>
static double time_begin;
static double time_end;
#endif

#ifdef ENABLE_PTLCALLS
#include "ptlcalls.h"
#endif

void roi_begin() {
    std::cout << ROI_PREFIX " Entering ROI" << std::endl;
    std::cout.flush();

    #ifdef ENABLE_TIMING
    struct timeval t;
    gettimeofday(&t,NULL);
    time_begin = (double)t.tv_sec+(double)t.tv_usec*1e-6;
    #endif

    #ifdef ENABLE_PTLCALLS
    ptlcall_switch_to_sim();
    #endif
}

void roi_end() {
    #ifdef ENABLE_PTLCALLS
    ptlcall_switch_to_native();
    #endif //ENABLE_PTLSIM_TRIGGER

    #if ENABLE_TIMING
    struct timeval t;
    gettimeofday(&t,NULL);
    time_end = (double)t.tv_sec+(double)t.tv_usec*1e-6;
    #endif

    std::cout << ROI_PREFIX " Leaving ROI\n" << std::endl;
    std::cout.flush();
}

