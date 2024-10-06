#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SHM_KEY 12345 // Shared memory key

void process1(int *total, sem_t *sem) {
    for (int i = 0; i < 100000; i++) {
        sem_wait(sem); // Wait for access to critical section
        (*total)++;
        sem_post(sem); // Signal that the critical section is free
    }
    printf("From Process 1: counter = %d.\n", *total);
}

void process2(int *total, sem_t *sem) {
    for (int i = 0; i < 200000; i++) {
        sem_wait(sem); // Wait for access to critical section
        (*total)++;
        sem_post(sem); // Signal that the critical section is free
    }
    printf("From Process 2: counter = %d.\n", *total);
}

void process3(int *total, sem_t *sem) {
    for (int i = 0; i < 300000; i++) {
        sem_wait(sem); // Wait for access to critical section
        (*total)++;
        sem_post(sem); // Signal that the critical section is free
    }
    printf("From Process 3: counter = %d.\n", *total);
}

void process4(int *total, sem_t *sem) {
    for (int i = 0; i < 500000; i++) {
        sem_wait(sem); // Wait for access to critical section
        (*total)++;
        sem_post(sem); // Signal that the critical section is free
    }
    printf("From Process 4: counter = %d.\n", *total);
}

int main() {
    int shmid;
    int *total;

    // Create shared memory segment
    shmid = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    // Attach the shared memory segment to the parent process
    total = (int *)shmat(shmid, NULL, 0);
    if (total == (int *)-1) {
        perror("shmat failed");
        exit(1);
    }

    *total = 0; // Initialize shared memory

    // Create semaphore for synchronization
    sem_t *sem = sem_open("sem", O_CREAT, 0644, 1); // Initialize semaphore with value 1

    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

    // Create 4 child processes
    pid_t pids[4];

    pids[0] = fork();
    if (pids[0] == 0) {
        process1(total, sem);
        exit(0);
    }

    pids[1] = fork();
    if (pids[1] == 0) {
        process2(total, sem);
        exit(0);
    }

    pids[2] = fork();
    if (pids[2] == 0) {
        process3(total, sem);
        exit(0);
    }

    pids[3] = fork();
    if (pids[3] == 0) {
        process4(total, sem);
        exit(0);
    }

    // Parent process waits for each child to finish before printing exit messages
    for (int i = 0; i < 4; i++) {
        wait(NULL); // Wait for each child process to finish
    }

    // Final output of exit messages for each child process
    for (int i = 0; i < 4; i++) {
        printf("Child with ID: %d has just exited.\n", pids[i]);
    }

    // Detach and remove the shared memory
    shmdt(total);
    shmctl(shmid, IPC_RMID, NULL);

    // Close and unlink semaphore
    sem_close(sem);
    sem_unlink("sem");

    printf("End of Simulation.\n");

    return 0;
}


