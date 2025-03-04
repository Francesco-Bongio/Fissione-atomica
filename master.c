#include "master.h"

time_t start_time;
int scelta;

/* All'inizio il processo master raccoglie le impostazioni dal file settings.txt. 
   Se il file non esiste viene creato un nuovo file settings.txt contenente le impostazioni di default.    */
void update_settings() {
    FILE *settings = fopen("settings.txt", "r");
    if (settings) {
        fscanf(settings, "ENERGY_DEMAND = %d\n", &ENERGY_DEMAND);
        fscanf(settings, "N_ATOMI_INIT = %d\n", &N_ATOMI_INIT);
        fscanf(settings, "N_ATOM_MAX = %d\n", &N_ATOM_MAX);
        fscanf(settings, "MIN_N_ATOMICO = %d\n", &MIN_N_ATOMICO);
        fscanf(settings, "SIM_DURATION = %d\n", &SIM_DURATION);
        fscanf(settings, "ENERGY_EXPLODE_THRESHOLD = %d\n", &ENERGY_EXPLODE_THRESHOLD);
        fscanf(settings, "N_NEW_ATOMS = %d\n", &N_NEW_ATOMS);
        fscanf(settings, "STEP = %d\n", &STEP);
        fscanf(settings, "NUM_ATOMS = %d\n", &NUM_ATOMS);
        fclose(settings);
    } else {
        FILE *default_settings = fopen("settings.txt", "w");
        fprintf(default_settings, "ENERGY_DEMAND = %d\n", ENERGY_DEMAND);
        fprintf(default_settings, "N_ATOMI_INIT = %d\n", N_ATOMI_INIT);
        fprintf(default_settings, "N_ATOM_MAX = %d\n", N_ATOM_MAX);
        fprintf(default_settings, "MIN_N_ATOMICO = %d\n", MIN_N_ATOMICO);
        fprintf(default_settings, "SIM_DURATION = %d\n", SIM_DURATION);
        fprintf(default_settings, "ENERGY_EXPLODE_THRESHOLD = %d\n", ENERGY_EXPLODE_THRESHOLD);
        fprintf(default_settings, "N_NEW_ATOMS = %d\n", N_NEW_ATOMS);
        fprintf(default_settings, "STEP = %d\n", STEP);
        fprintf(default_settings, "NUM_ATOMS = %d\n", NUM_ATOMS);
        fclose(default_settings);
    }
    return;
}

/* Successivamente si stampano le configurazioni lette */
void print_settings() {
    printf("[Master]: reading data...\n");
    printf("\t - ENERGY_DEMAND = %d\n", ENERGY_DEMAND);
    printf("\t - N_ATOMI_INIT = %d\n", N_ATOMI_INIT);
    printf("\t - N_ATOM_MAX = %d\n", N_ATOM_MAX);
    printf("\t - MIN_N_ATOMICO = %d\n", MIN_N_ATOMICO);
    printf("\t - SIM_DURATION = %d\n", SIM_DURATION);
    printf("\t - ENERGY_EXPLODE_THRESHOLD = %d\n", ENERGY_EXPLODE_THRESHOLD);
    printf("\t - N_NEW_ATOMS = %d\n", N_NEW_ATOMS);
    printf("\t - STEP = %d\n", STEP);
    printf("\t - NUM_ATOMS = %d\n", NUM_ATOMS);
    return;
}

void cleanup_ipc_resources() {
    if (shmdt(shared_data) == -1) {
        perror("[Master]: shmdt(shared_data) failed!");
    }
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("[Master]: shmctl(shm_id, IPC_RMID, NULL) failed!");
    }
    if (shmdt(shared_constants_data) == -1) {
        perror("[Master]: shmdt(shared_constants_data) failed!");
    }
    if (shmctl(const_id, IPC_RMID, NULL) == -1) {
        perror("[Master]: shmctl(const_id, IPC_RMID, NULL) failed!");
    }
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("[Master]: semctl(sem_id, 0, IPC_RMID) failed!");
    }
    if (semctl(sync_sem_id, 0, IPC_RMID) == -1) {
        perror("[Master]: semctl(sync_sem_id, 0, IPC_RMID) failed!");
    }
    if (msgctl(msg_id, IPC_RMID, NULL) == -1) {
        perror("[Master]: msgctl(msg_id, IPC_RMID, NULL) failed!");
    }
}

void terminate_all_children() {
    int status;
    kill(-getpid(), SIGTERM);  // Kill the entire process group
    while (waitpid(-1, &status, 0) > 0);
}

void sigint_handler(int sig) {
    terminate_all_children();
    cleanup_ipc_resources();
    printf("[Master]: All child processes terminated and resources cleaned up!\n");
    exit(0);
}

void setup_ipc_resources() {
     shm_id = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    const_id = shmget(MON_KEY, sizeof(SharedCostantsData), IPC_CREAT | 0666);
    shared_constants_data = (SharedCostantsData *)shmat(const_id, NULL, 0);
    sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    semctl(sem_id, 0, SETVAL, 1);
    key=ftok("settings.txt", 'A');
    msg_id = msgget(MSG_KEY, IPC_CREAT | 0666);

    // Initialize synchronization semaphore with 0 (all processes will wait)
    sync_sem_id = semget(SYNC_KEY, 1, IPC_CREAT | 0666);
    semctl(sync_sem_id, 0, SETVAL, 0);
    lock();
    shared_data->contatore=0;
    unlock();
}

void lock() {
    semop(sem_id, &p_op, 1);
}

void unlock() {
    semop(sem_id, &v_op, 1);
}

void print_stats() {
    lock();
    printf("--------------------\n");
    printf("Activations: Total=%d\n", shared_data->activations_total);
    printf("Splits: Total=%d\n", shared_data->splits_total);
    printf("Energy Produced: Total=%d\n", shared_data->energy_produced_total);
    printf("Energy Consumed: Total=%d\n", shared_data->energy_consumed_total);
    printf("Waste: Total=%d\n", shared_data->waste_total);
    printf("Contatore: %d\n", shared_data->contatore);
    unlock();
}


void fork_and_exec(const char *path, const char *arg1, const char *arg2) {
    pid_t pid = fork();
    if (pid == 0) {  // Child process
        // Wait on the synchronization semaphore
        struct sembuf wait_op = {0, -1, 0};
        semop(sync_sem_id, &wait_op, 1);
        execl(path, path, arg1, arg2, NULL);
        perror("[Master]: Failed to exec child process");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {  // Error
        perror("[Master]: Meltdown; failed to fork!");
        cleanup_ipc_resources();
        exit(EXIT_FAILURE);
    } else {  // Parent process
    }
}

void start_child_processes() {
    if(scelta==1){
        struct sembuf signal_op = {0, N_ATOMI_INIT + 3, 0}; // N_ATOMI_INIT processes plus others
        semop(sync_sem_id, &signal_op, 1);
    }else{
        struct sembuf signal_op = {0, N_ATOMI_INIT + 2, 0}; // N_ATOMI_INIT processes plus others
        semop(sync_sem_id, &signal_op, 1);
    }
}


void retrieve_energy_demand() {
    lock();
    shared_data->energy_consumed_total += ENERGY_DEMAND;
    shared_data->energy_produced_total=shared_data->energy_produced_total-ENERGY_DEMAND;
    unlock();
}

void monitor_termination_conditions(time_t start_time) {
    if (time(NULL) - start_time > 2){

        lock();
        int energy_produced = shared_data->energy_produced_total;
        unlock();
        if (energy_produced > ENERGY_EXPLODE_THRESHOLD) {
            printf("\t -> [Simulation ends: Explosion occurred!]\n");
             cleanup_ipc_resources();
            terminate_all_children();
            exit(0);
        }


        // timeout
        if (time(NULL) - start_time >= SIM_DURATION) {
            printf("\t -> [Simulation ends: Timeout reached!]\n");
             cleanup_ipc_resources();
            terminate_all_children();
            exit(0);
        }


        // blackout
        if (energy_produced<ENERGY_DEMAND) {
            printf("\t -> [Simulation ends: Blackout occurred due to insufficient energy!]\n");
             cleanup_ipc_resources();
            terminate_all_children();
            exit(0);
        }

        // Retrieve energy demand and check for blackout condition
        if (time(NULL) - start_time > 2){
        retrieve_energy_demand();  // Update energy consumption every cycle
        }

        //meltdown
        if( msgrcv(msg_id, &msg, sizeof(msg.mtext), 0, IPC_NOWAIT)){
            if (msg.mtext[0] == 'M') {
                printf("\t -> [Simulation ends: Meltdown occurred!]\n");
                cleanup_ipc_resources();
                terminate_all_children();
                exit(0);
            }
        }
    }
}
pid_t inhibitor_pid = 0;

void sigusr2_handler(int sig) {
    if (inhibitor_pid != 0) {
        kill(inhibitor_pid, SIGUSR1); // Toggle the inhibitor process
    }
}

void start_inhibitor() {
    if (inhibitor_pid != 0) {
        fprintf(stderr, "[Master]: Inhibitor process already running!\n");
        return;
    }
    fork_and_exec("./inibitore", NULL, NULL);
}

void stop_inhibitor() {
    if (inhibitor_pid != 0) {
        kill(inhibitor_pid, SIGTERM); // Terminate the inhibitor process
        waitpid(inhibitor_pid, NULL, 0); // Wait for termination
        inhibitor_pid = 0;
        printf("[Master]: Inhibitor process stopped\n");
    } else {
        fprintf(stderr, "[Master]: Inhibitor process is not running!\n");
    }
}

void sigalrm_handler(int sig) {
    switch (sig)
    {
    case SIGALRM:
        print_stats();
         monitor_termination_conditions(start_time);
        alarm(1); // Reimposta l'allarme per il prossimo secondo
        break;
    
    default:
        break;
    }
}

int main() {
    system("clear");
    printf("Vuoi attivare l'inibitore? 1 sì 0 no \n");
    scanf("%d", &scelta);
    printf("[Master - PID:%d]: started!\n", getpid());
    update_settings();
    print_settings();
     struct sigaction sa;
	bzero(&sa, sizeof(sa));
	sa.sa_handler = sigalrm_handler;
    sigaction(SIGALRM,&sa,NULL);

    signal(SIGINT, sigint_handler);
    signal(SIGUSR2, sigusr2_handler); // Handle SIGUSR2 to toggle the inhibitor

    setup_ipc_resources();

    shared_constants_data->energy_demand=ENERGY_DEMAND;
    shared_constants_data->energy_explode_treshold=ENERGY_EXPLODE_THRESHOLD;
    shared_constants_data->min_n_atomico=MIN_N_ATOMICO;
    shared_constants_data->n_atomi_init=N_ATOMI_INIT;
    shared_constants_data->n_max_atom=N_ATOM_MAX;
    shared_constants_data->n_new_atoms=N_NEW_ATOMS;
    shared_constants_data->num_atoms=NUM_ATOMS;
    shared_constants_data->simulation_duration=SIM_DURATION;
    shared_constants_data->step=STEP;

    setpgid(0, 0);  // Set process group ID to PID to allow sending signals to all child processes

    char str_max[10];   // Ensure buffer is large enough
    char str_min[10];   // Ensure buffer is large enough

    sprintf(str_max, "%d", N_ATOM_MAX);
    sprintf(str_min, "%d", MIN_N_ATOMICO);

    // Create child processes
    for (int i = 0; i < N_ATOMI_INIT; i++) {
        fork_and_exec("./atomo", str_max, str_min); // Assuming each atom process takes no initial argument
    }
    fork_and_exec("./alimentazione", NULL, NULL);
    fork_and_exec("./attivatore", NULL, NULL);
    // Optionally start the inhibitor process if specified in settings or command-line argument
    if (scelta == 1) {
        printf("[Inhibitor]: Activated--------------------------------\n");
        start_inhibitor();
    } else {
        printf("[Inhibitor]: Deactivated------------------------------\n");
    }

    start_child_processes();

    printf("[Master]: simulation started!\n");
    start_time = time(NULL);


    // Imposta il primo allarme
    alarm(1);

    // Rimani nel ciclo infinito, l'esecuzione del corpo del ciclo è gestita dal signal handler sigalrm_handler
    while (1) {
        pause(); // Attende che un segnale venga consegnato
    }


    return 0;
}