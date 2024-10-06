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

void process1(int *total) {
    for (int i = 0; i < 100000; i++) {
        (*total)++;
    }
}

void process2(int *total) {
    for (int i = 0; i < 200000; i++) {
        (*total)++;
    }
}

void process3(int *total) {
    for (int i = 0; i < 300000; i++) {
        (*total)++;
    }
}

void process4(int *total) {
    for (int i = 0; i < 500000; i++) {
        (*total)++;
    }
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

    // Create semaphores for synchronization
    sem_t *sem1 = sem_open("sem1", O_CREAT, 0644, 0);
    sem_t *sem2 = sem_open("sem2", O_CREAT, 0644, 0);
    sem_t *sem3 = sem_open("sem3", O_CREAT, 0644, 0);

    if (sem1 == SEM_FAILED || sem2 == SEM_FAILED || sem3 == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

    // Create 4 child processes
    pid_t pids[4];

    pids[0] = fork();
    if (pids[0] == 0) {
        process1(total);
        printf("From Process 1: counter = %d.\n", *total);
        sem_post(sem1); // Signal process 2
        exit(0);
    }

    pids[1] = fork();
    if (pids[1] == 0) {
        sem_wait(sem1); // Wait for process 1 to finish
        process2(total);
        printf("From Process 2: counter = %d.\n", *total);
        sem_post(sem2); // Signal process 3
        exit(0);
    }

    pids[2] = fork();
    if (pids[2] == 0) {
        sem_wait(sem2); // Wait for process 2 to finish
        process3(total);
        printf("From Process 3: counter = %d.\n", *total);
        sem_post(sem3); // Signal process 4
        exit(0);
    }

    pids[3] = fork();
    if (pids[3] == 0) {
        sem_wait(sem3); // Wait for process 3 to finish
        process4(total);
        printf("From Process 4: counter = %d.\n", *total);
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

    // Close and unlink semaphores
    sem_close(sem1);
    sem_close(sem2);
    sem_close(sem3);
    sem_unlink("sem1");
    sem_unlink("sem2");
    sem_unlink("sem3");

    printf("End of Simulation.\n");

    return 0;
}

