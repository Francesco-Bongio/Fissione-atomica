#ifndef ALIMENTAZIONE_H 
    #define ALIMENTAZIONE_H
    #include <sys/time.h>
    #include <time.h>
    #include <signal.h>
    #include "common.h"
   
    int N_NEW_ATOMS;  // Number of new atoms to create each interval
    int STEP; // Interval between new atom creation, in nanoseconds (1 second)
    int const_id;
    SharedCostantsData *shared_constants_data;

    void cleanup_ipc_resources();
    void sigterm_handler(int sig);
    void setup_signal_handlers();
    void create_new_atom(int initial_atomic_number);

#endif //ALIMENTAZIONE_H