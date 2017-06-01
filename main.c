#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define ROW_SIZE 8
#define COL_SIZE 8
#define WHITE_PIECE 1
#define BLACK_PIECE 2
#define SHM_SIZE 4096 /* make it a 4K shared memory segment */
struct Point {
    int x;
    int y;
};


int gameBoard[ROW_SIZE][COL_SIZE];

int main() {

    key_t key;
    int shmid;
    char *data;
    int file;
    int fd_read;
    pid_t firstGivenPid;
    pid_t secondGivenPid;
    //set the board
    memset(gameBoard, 0, sizeof(char) * ROW_SIZE * COL_SIZE);
    //initial the game with the black and white cells.
    gameBoard[3][4] = WHITE_PIECE;
    gameBoard[4][3] = WHITE_PIECE;
    gameBoard[4][4] = BLACK_PIECE;
    gameBoard[3][3] = BLACK_PIECE;

    /* make the key: */
    if ((key = ftok("ex31.c", 'k')) == -1) {
        write(STDERR_FILENO, "failed ftok", strlen("failed ftok"));
        exit(1);
    }

    /* connect to (and possibly create) the segment: */
    if ((shmid = shmget(key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }

    /* attach to the segment to get a pointer to it: */
    data = shmat(shmid, NULL, 0);
    if (data == (char *) (-1)) {
        perror("shmat");
        exit(1);
    }
    //create fifo
    file = mkfifo("fifo_clientTOserver", 0666);
    if (file < 0) {
        perror("Unable to create a fifo");
        exit(-1);
    }
    //open fifo
    if ((fd_read = open("fifo_clientTOserver", O_RDONLY)) < 0) {
        perror("Unable to open a fifo");
        exit(-1);
    }
    //get the pid's
    if (read(fd_read, &firstGivenPid, sizeof(pid_t)) < 0) {
        //todo handle
    }
    if (read(fd_read, &secondGivenPid, sizeof(pid_t)) < 0) {
        //todo handle
    }
    //close the fifo
    close(fd_read);
    printf("Hello, World!\n");
    return 0;
}