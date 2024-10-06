// Your Name: Roberto Cervantes
// Username: cs017

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

#define SHM_KEY 12345 // Shared memory key for identification

// Function for process 1 to increment the total counter
void process1(int *total) {
    for (int i = 0; i < 100000; i++) { // Loop to increment the counter 100,000 times
        (*total)++;
    }
}

// Function for process 2 to increment the total counter
void process2(int *total) {
    for (int i = 0; i < 300000 - 200000; i++) { // Loop to increment the counter 100,000 times
        (*total)++;
    }
}

// Function for process 3 to increment the total counter
void process3(int *total) {
    for (int i = 0; i < 400000 - 300000; i++) { // Loop to increment the counter 100,000 times
        (*total)++;
    }
}

// Function for process 4 to increment the total counter
void process4(int *total) {
    for (int i = 0; i < 500000 - 300000; i++) { // Loop to increment the counter 200,000 times
        (*total)++;
    }
}

int main() {
    int shmid; // Shared memory ID
    int *total; // Pointer to the shared total counter

    // Create shared memory segment with read/write permissions
    shmid = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0) { // Check for errors in creating shared memory
        perror("shmget failed");
        exit(1);
    }

    // Attach the shared memory segment to the parent process
    total = (int *)shmat(shmid, NULL, 0);
    if (total == (int *)-1) { // Check for errors in attaching shared memory
        perror("shmat failed");
        exit(1);
    }

    *total = 0; // Initialize shared memory variable to zero

    // Create semaphores for synchronization between processes
    sem_t *sem1 = sem_open("sem1", O_CREAT, 0644, 0);
    sem_t *sem2 = sem_open("sem2", O_CREAT, 0644, 0);
    sem_t *sem3 = sem_open("sem3", O_CREAT, 0644, 0);

    // Check for errors in opening semaphores
    if (sem1 == SEM_FAILED || sem2 == SEM_FAILED || sem3 == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

    // Create 4 child processes
    pid_t pids[4]; // Array to store process IDs

    // Create first child process
    pids[0] = fork();
    if (pids[0] == 0) { // Child process 1
        process1(total); // Call process1 function to increment counter
        printf("From Process 1: counter = %d.\n", *total); // Print counter value
        sem_post(sem1); // Signal process 2 to start
        exit(0); // Exit child process
    }

    // Create second child process
    pids[1] = fork();
    if (pids[1] == 0) { // Child process 2
        sem_wait(sem1); // Wait for process 1 to finish
        process2(total); // Call process2 function to increment counter
        printf("From Process 2: counter = %d.\n", *total); // Print counter value
        sem_post(sem2); // Signal process 3 to start
        exit(0); // Exit child process
    }

    // Create third child process
    pids[2] = fork();
    if (pids[2] == 0) { // Child process 3
        sem_wait(sem2); // Wait for process 2 to finish
        process3(total); // Call process3 function to increment counter
        printf("From Process 3: counter = %d.\n", *total); // Print counter value
        sem_post(sem3); // Signal process 4 to start
        exit(0); // Exit child process
    }

    // Create fourth child process
    pids[3] = fork();
    if (pids[3] == 0) { // Child process 4
        sem_wait(sem3); // Wait for process 3 to finish
        process4(total); // Call process4 function to increment counter
        printf("From Process 4: counter = %d.\n", *total); // Print counter value
        exit(0); // Exit child process
    }

    // Parent process waits for each child to finish
    for (int i = 0; i < 4; i++) {
        wait(NULL); // Wait for each child process to finish
    }

    // Final output of child IDs after all counters are printed
    for (int i = 0; i < 4; i++) {
        printf("Child with ID: %d has just exited.\n", pids[i]); // Print each child's exit message
    }

    // Detach and remove the shared memory segment
    shmdt(total); // Detach shared memory
    shmctl(shmid, IPC_RMID, NULL); // Remove shared memory segment

    // Close and unlink semaphores
    sem_close(sem1); // Close semaphore 1
    sem_close(sem2); // Close semaphore 2
    sem_close(sem3); // Close semaphore 3
    sem_unlink("sem1"); // Unlink semaphore 1
    sem_unlink("sem2"); // Unlink semaphore 2
    sem_unlink("sem3"); // Unlink semaphore 3

    printf("End of Simulation.\n"); // Print end of simulation message

    return 0; // Exit program
}

