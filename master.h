#ifndef MASTER_H 
    #define MASTER_H
        
    #include <sys/wait.h>
    #include <signal.h>
    #include <time.h>
    #include "common.h"

    int shm_id;
    int const_id;
    SharedCostantsData *shared_constants_data;
    SharedData *shared_data;
    int sem_id;
    int msg_id;
    int sync_sem_id; // Synchronization semaphor
    key_t key;
    struct message msg;

    /* Default Settings */
    int ENERGY_DEMAND = 0;// Energy units demanded per second
    int N_ATOMI_INIT = 5;
    int N_ATOM_MAX = 100;
    int MIN_N_ATOMICO = 1;
    int SIM_DURATION = 30;// Simulation runs for 60 seconds
    int ENERGY_EXPLODE_THRESHOLD = 1000;
    int N_NEW_ATOMS = 2;   // Number of new atoms to create each interval
    int STEP = 1000000000; // Interval between new atom creation, in nanoseconds (1 second)
    int NUM_ATOMS = 10;  // Number of atom processes that can be controlled

    void update_settings();
    void print_settings();
    void cleanup_ipc_resources();
    void terminate_all_children();
    void sigint_handler(int sig);
    void setup_ipc_resources();
    void print_stats();
    void fork_and_exec(const char *path, const char *arg1, const char *arg2);
    void start_child_processes();
    void retrieve_energy_demand();
    void monitor_termination_conditions(time_t start_time);

#endif //MASTER_H