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

#include <sys/resource.h>

#define stringify_value(x) #x
#define stringify(x) stringify_value(x)

void roi_begin() {
    std::cout << stringify(ROI_PREFIX) " Entering ROI" << std::endl;
    std::cout.flush();

    #ifdef ENABLE_TIMING
    struct timeval t;
    gettimeofday(&t,NULL);
    time_begin = (double)t.tv_sec+(double)t.tv_usec*1e-6;
    #endif

    #ifdef ENABLE_PTLCALLS
    char * snapshot_name = getenv("SNAPSHOT");
    if ( snapshot_name ) {
        ptlcall_checkpoint_and_shutdown( snapshot_name );
    }
    #endif

    #ifdef ENABLE_PTLCALLS
    char * enter_sim = getenv("SIMULATE");
    if ( enter_sim && enter_sim[0] == '1' ) {
        std::cout << stringify(ROI_PREFIX) " Entering SIMULATION" << std::endl;
        std::cout.flush();

        ptlcall_switch_to_sim();
    }
    #endif
}

void roi_end() {
    #ifdef ENABLE_PTLCALLS
    char * enter_sim = getenv("SIMULATE");
    if ( enter_sim && enter_sim[0] == '1' ) {
        ptlcall_switch_to_native();
    }
    #endif

    #if ENABLE_TIMING
    struct timeval t;
    gettimeofday(&t,NULL);
    time_end = (double)t.tv_sec+(double)t.tv_usec*1e-6;
    #endif

    std::cout << stringify(ROI_PREFIX) " Leaving ROI" << std::endl;
    std::cout.flush();

    std::cout << stringify(ROI_PREFIX) " Runtime: " << (time_end - time_begin) << std::endl;

    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    
    std::cout << stringify(ROI_PREFIX) " Memory: " << (double)usage.ru_maxrss/1024 << " MB" << std::endl;
}

