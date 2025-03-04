#include "alimentazione.h"

void cleanup_ipc_resources() {
    if (shmdt(shared_constants_data) == -1) { // Detach shared memory
        perror("[Alimentazione]: Failed to detach shared memory");
    }
    printf("[Alimentazione]: IPC resources detached!\n");
    exit(0);
}

void sigterm_handler(int sig) {
    printf("[Alimentazione]: SIGTERM received, cleaning up resources...\n");
    cleanup_ipc_resources();
}

void setup_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = sigterm_handler;  // Set the handler function
    sigemptyset(&sa.sa_mask);         // No signals blocked during the execution of sigterm_handler
    sa.sa_flags = 0;                  // No flags
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("[Alimentazione]: Failed to setup signal handler for SIGTERM");
        exit(EXIT_FAILURE);
    }
}

void create_new_atom(int initial_atomic_number) {
    int N_ATOM_MAX = shared_constants_data->n_max_atom;
    int MIN_N_ATOMICO = shared_constants_data->min_n_atomico;
    char str_max[10];   // Ensure buffer is large enough
    char str_min[10];   // Ensure buffer is large enough

    sprintf(str_max, "%d", N_ATOM_MAX);
    sprintf(str_min, "%d", MIN_N_ATOMICO);

    pid_t pid = fork();
    if (pid == 0) { // Child process
        char arg[10];
        sprintf(arg, "%d", initial_atomic_number); // Convert integer to string for execl argument
        execl("./atomo", "atomo", str_max, str_min, NULL);
        perror("[Alimentazione]: Failed to start new atom process");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("[Alimentazione]: Meltdown; fork failed!");
        exit(EXIT_FAILURE);
    }
}

// Variabili globali per gestire il numero di nuovi atomi e il tempo di intervallo
int N_NEW_ATOMS;
int STEP;

// Gestore di segnali per SIGALRM
void sigalrm_handler(int sig) {
    for (int i = 0; i < N_NEW_ATOMS; i++) {
        create_new_atom(10); // Start each new atom with an atomic number of 10
    }
    alarm(STEP); // Reimposta l'allarme per il prossimo intervallo
}

void setup_alarm_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = sigalrm_handler;  // Set the handler function
    sigemptyset(&sa.sa_mask);         // No signals blocked during the execution of sigalrm_handler
    sa.sa_flags = 0;                  // No flags
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("[Alimentazione]: Failed to setup signal handler for SIGALRM");
        exit(EXIT_FAILURE);
    }
}

int main() {
    setup_signal_handlers(); // Setup signal handlers before anything else

    const_id = shmget(MON_KEY, sizeof(SharedCostantsData), 0666);
    if (const_id == -1) {
        perror("[Alimentazione]: Failed to obtain shared memory for SharedCostantsData");
        exit(EXIT_FAILURE);
    }
    shared_constants_data = (SharedCostantsData *)shmat(const_id, NULL, 0);
    if (shared_constants_data == (void *)-1) {
        perror("[Alimentazione]: Failed to attach shared memory for SharedCostantsData");
        exit(EXIT_FAILURE);
    }

    N_NEW_ATOMS = shared_constants_data->n_new_atoms;
    STEP = shared_constants_data->step / 1000000000; // Convert nanoseconds to seconds

    setup_alarm_signal_handler(); // Setup signal handler for alarm
    printf("[Alimentazione]: Feeding process started\n");
    printf("[Alimentazione]: N_NEW_ATOMS vale: %d\n", N_NEW_ATOMS);

    alarm(STEP); // Imposta il primo allarme

    // Rimani nel ciclo infinito, l'esecuzione del corpo del ciclo è gestita dal signal handler sigalrm_handler
    while (1) {
        pause(); // Attende che un segnale venga consegnato
    }

    return 0;
}
