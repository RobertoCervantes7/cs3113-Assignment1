#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

#define SHM_KEY 12345 // Shared memory key
#define SEM_KEY 12346  // Semaphore key

void process1(int *total) {
    for (int i = 0; i < 100000; i++) {
        (*total)++;
    }
    printf("From Process 1: counter = %d.\n", *total);
}

void process2(int *total) {
    for (int i = 0; i < 200000; i++) {
        (*total)++;
    }
    printf("From Process 2: counter = %d.\n", *total);
}

void process3(int *total) {
    for (int i = 0; i < 300000; i++) {
        (*total)++;
    }
    printf("From Process 3: counter = %d.\n", *total);
}

void process4(int *total) {
    for (int i = 0; i < 500000; i++) {
        (*total)++;
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

    // Create a semaphore for synchronization
    sem_t *sem = sem_open("/my_semaphore", O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

    // Create 4 child processes
    pid_t pids[4];
    for (int i = 0; i < 4; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            // Child process
            for (int j = 0; j < (i + 1) * 100000; j++) {
                sem_wait(sem); // Wait for the semaphore
                (*total)++;    // Critical section
                sem_post(sem); // Signal the semaphore
            }
            printf("From Process %d: counter = %d.\n", i + 1, *total);
            exit(0);
        }
    }

    // Parent process waits for each child to finish
    for (int i = 0; i < 4; i++) {
        int status;
        pid_t child_pid = wait(&status);
        printf("Child with ID: %d has just exited.\n", child_pid);
    }

    // Detach and remove the shared memory
    shmdt(total);
    shmctl(shmid, IPC_RMID, NULL);
    
    // Close and unlink the semaphore
    sem_close(sem);
    sem_unlink("/my_semaphore");

    printf("End of Program.\n");

    return 0;
}

