#include "attivatore.h" 

void lock() {
    semop(sem_id, &p_op, 1);
}

void unlock() {
    semop(sem_id, &v_op, 1);
}

void cleanup_resources() {
    if (shmdt(shared_constants_data) == -1) {
        perror("[Attivatore]: Failed to detach shared memory");
    }
    if (shmdt(shared_data) == -1) {
        perror("[Attivatore]: Failed to detach shared memory");
    }
    printf("[Attivatore]: IPC resourcese detached!\n");
    exit(0);
}

void sigterm_handler(int sig) {
        printf("[Attivatore]: SIGTERM received, cleaning up resources...\n");
        cleanup_resources();
}

void sigalrm_handler(int sig) {
    switch (sig)
    {
    case SIGALRM:
        send_split_signal();
        alarm(2);
        break;
    
    default:
        break;
    }
}

void sigusr2_handler(int sig) {
    switch (sig)
    {
    case SIGUSR2:
        alarm(5);
        printf("[Attivatore]: Received SIGUSR2 from Inibitore, pausing activations for 4 seconds\n");
        break;
    
    default:
        break;
    }
}

void setup_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = sigterm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("[Attivatore]: Error setting up signal handler");
        exit(EXIT_FAILURE);
    }
}

void send_split_signal() {
    srand(time(NULL));
    int random = rand() % 4 +1;
    for(int i=0; i<random; i++){
        lock();
            int idx = rand() % shared_data->contatore;
            pid_t target_pid = shared_data->arrayPid[idx];
            printf("[Attivatore]: Selected atom PID %d\n", target_pid);
            shared_data->activations_total=shared_data->activations_total+1;
        unlock();

        if (kill(target_pid, SIGUSR1) == -1) {
            perror("[Attivatore]: Failed to send split signal");
        } else {
            printf("[Attivatore]: Sent split signal to atom PID %d\n", target_pid);
        }
    }
}

int main() {
    setup_signal_handler(); // Setup the SIGTERM handler

    struct sigaction sa;
	bzero(&sa, sizeof(sa));
	sa.sa_handler = sigalrm_handler;
    sigaction(SIGALRM,&sa,NULL);

    struct sigaction sb;
	bzero(&sb, sizeof(sb));
	sb.sa_handler = sigusr2_handler;
    sigaction(SIGUSR2,&sb,NULL);

    sem_id = semget(SEM_KEY, 1, 0666);
    if (sem_id == -1) {
        perror("[Attivatore]: Failed to obtain semaphore");
        exit(EXIT_FAILURE);
    }

    const_id = shmget(MON_KEY, sizeof(SharedCostantsData), 0666);
    if (const_id == -1) {
        perror("[Attivatore]: Failed to obtain shared memory for SharedCostantsData");
        exit(EXIT_FAILURE);
    }
    shared_constants_data = (SharedCostantsData *)shmat(const_id, NULL, 0);
    if (shared_constants_data == (void *)-1) {
        perror("[Attivatore]: Failed to attach shared memory for SharedCostantsData");
        exit(EXIT_FAILURE);
    }
    data_id = shmget(SHM_KEY, sizeof(SharedData), 0666);
    if (data_id == -1) {
        perror("[Attivatore]: Failed to obtain shared memory for SharedData in attivatore");
        exit(EXIT_FAILURE);
    }
    shared_data = (SharedData *)shmat(data_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("[Attivatore]: Failed to attach shared memory for SharedData");
        exit(EXIT_FAILURE);
    }


    printf("[Attivatore]: Activator process started\n");
    lock();
        shared_data->pidAttivatore = getpid();
    unlock();
    srand(time(NULL));  // Seed the random number generator only once
    alarm(2); // Send SIGALRM every 2 seconds
    while (1)
    {
        pause();
    }
    
    return 0;
}
