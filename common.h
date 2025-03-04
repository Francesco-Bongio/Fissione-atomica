#ifndef COMMON_H 
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/types.h>

// Define keys for IPC
#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define MSG_KEY 0x9ABC
#define MON_KEY 0xDEF0
#define SYNC_KEY 0xDEF1

// Shared data structure
typedef struct {
    int activations_total;
    int splits_total;
    int energy_produced_total;
    int energy_consumed_total;
    int waste_total;
    pid_t arrayPid[512];
    int contatore;
    pid_t pidAttivatore;
} SharedData;

// Shared data structure for constants
typedef struct {
    int num_atoms;
    int n_new_atoms;
    int step;
    int semop_size;
    int energy_explode_treshold;
    int simulation_duration;
    int energy_demand;
    int n_atomi_init;
    int n_max_atom;
    int min_n_atomico;
} SharedCostantsData;

// Message structure for message queue
typedef struct {
    long mtype;
    char mtext[100];  // Increased size for more complex data
} Message;

typedef struct {
    long mtype;    // Message type, for process control
    char mtext[1]; // Message text can be extended if needed
} ControlMessage;

struct message {
    long mtype;
    char mtext[100];
};

void setup_shared_resources();
void cleanup_shared_resources();

void lock();
void unlock();

struct sembuf p_op = {0, -1, SEM_UNDO}; // P operation
struct sembuf v_op = {0, 1, SEM_UNDO};  // V operation

#endif // COMMON_H
