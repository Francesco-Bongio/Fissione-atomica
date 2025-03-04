#include "atomo.h" 

void setup_ipc() {
    shm_id = shmget(SHM_KEY, sizeof(SharedData), 0666);
    if (shm_id == -1) {
        perror("[Atomo]: Failed to obtain shared memory for SharedData in atomo");
        exit(EXIT_FAILURE);
    }
    shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("[Atomo]: Failed to attach shared memory for SharedData");
        exit(EXIT_FAILURE);
    }

    sem_id = semget(SEM_KEY, 1, 0666);
    if (sem_id == -1) {
        perror("[Atomo]: Failed to obtain semaphore");
        exit(EXIT_FAILURE);
    }
    key = ftok("settings.txt", 'A');
    msg_id = msgget(key, 0666);
}

void cleanup_ipc() {
    if(shmdt(shared_data) == -1) {
        perror("[Atomo]: Failed to detach shared memory");
    }
    
}

void handle_sigint(int sig) {
    cleanup_ipc();
    exit(EXIT_SUCCESS);
}

void lock() {
    semop(sem_id, &p_op, 1);
}

void unlock() {
    semop(sem_id, &v_op, 1);
}


void update_statistics(int energy, int waste) {
    lock();
    shared_data->energy_produced_total += energy;
    shared_data->waste_total += waste;
    shared_data->splits_total=shared_data->splits_total+1;
    unlock();
}

int function_energy_released(int parent_new_atomic_number, int child_atomic_number) {
    return parent_new_atomic_number * child_atomic_number - (parent_new_atomic_number > child_atomic_number ? parent_new_atomic_number : child_atomic_number);
}

void split(int sig) {
    if (atomic_number <= MIN_N_ATOMICO) {
        update_statistics(0, 1);
        exit(EXIT_SUCCESS);
    }

    int child_atomic_number = atomic_number/2;
    int parent_new_atomic_number = atomic_number - child_atomic_number;
    int energy_released = function_energy_released(parent_new_atomic_number, child_atomic_number);

    pid_t pid = fork();
    if (pid == 0) {  // Child atom
        atomic_number = child_atomic_number;
    } else if (pid > 0) {  // Parent atom continues
        atomic_number = parent_new_atomic_number;
        update_statistics(energy_released, 0);
    } else {
        perror("[Atomo]: Meltdown; failed to fork new atom!");
        msgsnd(msg_id, "M", 1, 0);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {  // Check if the correct number of arguments is passed
        fprintf(stderr, "Usage: %s <N_ATOM_MAX> <MIN_N_ATOMICO>\n", argv[0]);
        return EXIT_FAILURE;
    }

    N_MAX_ATOMS = atoi(argv[1]);
    MIN_N_ATOMICO = atoi(argv[2]);    

    srand(getpid());
    atomic_number = (rand() % N_MAX_ATOMS) + 1;
    if (atomic_number < 1 || atomic_number > N_MAX_ATOMS) {
        fprintf(stderr, "[Atomo]: Atomic number out of bounds. Must be between 1 and %d.\n", N_MAX_ATOMS);
        exit(EXIT_FAILURE);
    }

    // Seed for random number generation
    srand(time(NULL) + getpid());

    setup_ipc();
    lock();
    shared_data->arrayPid[shared_data->contatore]=getpid();
    shared_data->contatore=shared_data->contatore+1;
    unlock();

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = split;
    sigaction(SIGUSR1, &sa, NULL);

    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = handle_sigint;
    sigaction(SIGTERM, &sa_int, NULL);
    
    while (1) {
        //check_for_messages();
        pause();  // Wait for signal to split
    }

    cleanup_ipc();
    return 0;
}

