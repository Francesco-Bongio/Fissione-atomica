#ifndef ATOMO_H 
    #define ATOMO_H

    #include <sys/wait.h>
    #include <signal.h>
    #include <time.h>
    #include <string.h>
    #include "common.h"

    int shm_id;
    int const_id;
    int msg_id;
    SharedData *shared_data;
    int sem_id;
    int atomic_number;
    int N_MAX_ATOMS; // Maximum atomic number for new atoms
    int MIN_N_ATOMICO;
    key_t key;

    void setup_ipc();
    void cleanup_ipc();
    void handle_sigint(int sig);
    void update_statistics(int energy, int waste);
    void split(int sig);
    int function_energy_released(int parent_new_atomic_number, int child_atomic_number);

#endif //ATOMO_H