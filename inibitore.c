#include <signal.h>
#include <time.h>
#include <math.h>
#include "common.h"
#include "master.h"

int shm_id;
int const_id;
SharedData *shared_data;
SharedCostantsData *shared_constants_data;
int sem_id;
int running = 1; // Control variable for stopping and restarting
int inhibitor_active = 1; // Control variable for inhibitor process activation
int max_energia_split;
int max_energia_totale;


void lock() {
    semop(sem_id, &p_op, 1);
}

void unlock() {
    semop(sem_id, &v_op, 1);
}

void sigusr1_handler(int sig) {
    running = !running; // Toggle running state
    if (running) {
        printf("[Inhibitor]: Resumed--------------------------------\n");
    } else {
        printf("[Inhibitor]: Paused---------------------------------\n");
    }
}

void calcolo_energie_max(){
    max_energia_split=pow((shared_constants_data->n_max_atom/2),2)-(shared_constants_data->n_max_atom/2);
    max_energia_totale=(shared_constants_data->energy_explode_treshold-(max_energia_split*2));

}

void adaptive_inhibition() {
    // Monitor system state and adjust inhibitory actions accordingly
    if (inhibitor_active) {

        // Adjust absorption based on energy produced
        int energy_threshold_high = 1000; // High energy threshold
        int energy_threshold_low = 100; // Low energy threshold
        if (shared_data->energy_produced_total > energy_threshold_high) {
            int energy_to_absorb = shared_data->energy_produced_total / 10; // Absorb 10% of energy
            shared_data->energy_produced_total -= energy_to_absorb;
            printf("[Inhibitor]: Absorbed %d energy\n", energy_to_absorb);
        } else if (shared_data->energy_produced_total > energy_threshold_low) {
            int energy_to_absorb = shared_data->energy_produced_total / 10; // Absorb 5% of energy
            shared_data->energy_produced_total -= energy_to_absorb;
            printf("[Inhibitor]: Absorbed %d energy\n", energy_to_absorb);
        }
        if (shared_data->energy_produced_total > max_energia_totale){
           shared_data->energy_produced_total=shared_data->energy_produced_total-max_energia_split;
            printf("[Inibitore]: Assorbita energia tanta \n");
        }

        // Adjust splitting limitation based on splits
        int split_ultima_volta=0;
       if(split_ultima_volta!=shared_data->splits_total&&shared_data->splits_total % 2==0){
        int splits_threshold_high = 0; // High splits threshold
        split_ultima_volta=shared_data->splits_total;
        if (shared_data->splits_total > splits_threshold_high) {
            if (rand() % 3 == 0) { // 33% chance of limiting splits
                printf("[Inhibitor]: Limiting splits\n");
                kill(shared_data->pidAttivatore, SIGUSR2); // Send inhibitor deactivation signal
            }
        }
        }
    }
    }

void cleanup_ipc_resources() {
    if (shmdt(shared_constants_data) == -1) { // Detach shared memory
        perror("[Inibitore]: Failed to detach shared memory");
    }
    printf("[Inibitore]: IPC resources detached!\n");

    if(shmdt(shared_data) == -1) {
        perror("[Inibitore]: Failed to detach shared memory");
    }
    printf("[Inibitore]: IPC resources detached!\n");
    exit(0);
}

void sigterm_handler(int sig) {
    printf("[Inibitore]: SIGTERM received, cleaning up resources...\n");
    cleanup_ipc_resources();
}

void handle_alarm(int sig){
    lock();
    adaptive_inhibition();
    unlock();
    alarm(1);
}

int main() {
    signal(SIGUSR1, sigusr1_handler); // Handle pause/resume signal
    //signal(SIGUSR2, sigusr2_handler); // Handle inhibitor activation/deactivation signal
    signal(SIGTERM, sigterm_handler); // Handle SIGTERM signal
    printf("Il PID del processo Inibitore è: %d\n", getpid());

    // Setup shared memory and semaphore
    shm_id = shmget(SHM_KEY, sizeof(SharedData), 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }
    shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }
    sem_id = semget(SEM_KEY, 1, 0666);
    if (sem_id == -1) {
        perror("semget failed");
        exit(1);
    }

    const_id = shmget(MON_KEY, sizeof(SharedCostantsData), 0666);
    if (const_id == -1) {
        perror("shmget failed");
        exit(1);
    }
    shared_constants_data = (SharedCostantsData *)shmat(const_id, NULL, 0);
    if (shared_constants_data == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }
    calcolo_energie_max();
    printf("Stampo max energia split: %d\n", max_energia_split);

    srand(time(NULL)); // Seed for random number generation
    
    struct sigaction sa;
	bzero(&sa, sizeof(sa));
	sa.sa_handler = handle_alarm;
    sigaction(SIGALRM,&sa,NULL);


    alarm(1); // Send SIGALRM every 1 second
    while (1) {
        if (running) {
            pause();
        }
    }

    return 0;
}
