#ifndef ATTIVATORE_H 
    #define ATTIVATORE_H

    #include <signal.h>
    #include <time.h>
    #include "common.h"

    int const_id, data_id;
    SharedCostantsData *shared_constants_data;
    SharedData *shared_data;
    int sem_id;

    void cleanup_ipc_resources();
    void sigterm_handler(int sig);
    void setup_signal_handlers();
    void send_split_signal();

#endif //ATTIVATORE_H