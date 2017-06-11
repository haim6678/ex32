#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <signal.h>

#define ROW_SIZE 8
#define COL_SIZE 8
#define WHITE_PIECE 1
#define BLACK_PIECE 2
#define KEEP_ON 4
#define SHM_SIZE 4096 /* make it a 4K shared memory segment */
struct Point {
    int x;
    int y;
    int player;
};

//declare the functions
int CheckEnd();

int ReadData(char *data);

void ExecuteUp(struct Point *loc, int number);

void ExecuteDown(struct Point *loc, int number);

void ExecuteLeft(struct Point *loc, int number);

void ExecuteRight(struct Point *loc, int number);

void ExecuteRightAndUp(struct Point *loc, int number);

void ExecuteRightAndDown(struct Point *loc, int number);

void ExecuteLeftAndUp(struct Point *loc, int number);

void ExecuteLeftAndDown(struct Point *loc, int number);

int gameBoard[ROW_SIZE][COL_SIZE];

void RunGame(char *data);

void HandleEnd(int winner);

void ExecuteMoveOnBoard(struct Point loc);

char *memory;

void RealeseResoursecAndExit(int shmid);

/**
 * operation- the main function
 */
int main() {

    key_t key;
    int shmid;
    char *data;
    int file;
    int fd_read;
    pid_t firstGivenPid = 0;
    pid_t secondGivenPid = 0;
    int loc;
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
    memory = data;
    //
    *memory = '\0';
    //create fifo
    file = mkfifo("fifo_clientTOserver", 0666);
    if (file < 0) {
        perror("Unable to create a fifo");
        /* remove the memory: */
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            perror("shmctl");
            exit(1);
        }
        exit(-1);
    }
    //open fifo
    if ((fd_read = open("fifo_clientTOserver", O_RDWR)) < 0) {
        perror("Unable to open a fifo");
        /* remove the memory: */
        if (shmctl(shmid, IPC_RMID, NULL) == -1) {
            perror("shmctl");
            exit(1);
        }
        exit(-1);
    }

    //get the pid's
    if (read(fd_read, &firstGivenPid, sizeof(pid_t)) < 0) {
        perror("failed to read from fifo");
        if (close(fd_read) < 0) {
            perror("failed to close fifo");
        }
        RealeseResoursecAndExit(shmid);
    }

    if (read(fd_read, &secondGivenPid, sizeof(pid_t)) < 0) {
        perror("failed to read from fifo");
        if (close(fd_read) < 0) {
            perror("failed to close fifo");
        }
        RealeseResoursecAndExit(shmid);
    }
    //close the fifo
    if (close(fd_read) < 0) {
        perror("failed to close fifo");
        RealeseResoursecAndExit(shmid);
    }

    //sending them the signal
    if (kill(firstGivenPid, SIGUSR1) < 0) {
        perror("failed to send signal");
        RealeseResoursecAndExit(shmid);
    }

    //wait for first player to make a move
    while (*memory != 'b') {
        sleep(1);
    }
    //read the move
    loc = ReadData(data);
    //send to other player a signal to start play
    if (kill(secondGivenPid, SIGUSR1) < 0) {
        perror("failed to send signal");
        RealeseResoursecAndExit(shmid);
    }

    //start following and managing the game
    RunGame(data);
    //finish game
    RealeseResoursecAndExit(shmid);
    return 0;
}

/**
 * input - the memory identifier
 * operation- delete the memory and exit
 */
void RealeseResoursecAndExit(int shmid) {
    if (unlink("fifo_clientTOserver") < 0) {
        perror("failed to close file");
    }
    /* remove it: */
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }
    exit(0);
}

/**
 * input - pointer tobuild structure shared memory
 * output- the second player move
 * operation - reads from memory
 */
int ReadData(char *data) {
    int x;
    int y;
    char temp;
    int player;

    temp = *memory;
    if (temp == 'b') {
        player = 2;
    } else if (temp == 'w') {
        player = 1;
    } else if( temp == '*'){
        return 1;
    } else{
        return 0;
    }
    (*memory++);
    x = (*memory) - 48;
    (*memory++);
    y = (*memory) - 48;
    (*memory++);
    (*memory++);
    return 0;
}


/**
 * input - the pointer to the shared memory
 * operation- keep an aye on the game, if it's end-then finish the process
 */
void RunGame(char *data) {
    int keepOn = KEEP_ON;
    int check = 0;
    while (keepOn == KEEP_ON) {
        check = ReadData(data);
        if (check == 1){
            break;
        }
        sleep(1);
    }
    (*memory++);
    (*memory++);
    while (*memory != '*'){

    }
    (*memory--);
    keepOn =((*memory) - 48);
    HandleEnd(keepOn);
}

/**
 * input- the number that indicate the winner
 * operation- print the message and exit's
 */
void HandleEnd(int winner) {
    //print game over and the winner
    if (write(STDOUT_FILENO, "GAME OVER\n", strlen("GAME OVER\n")) < 0) {
        perror("failed to write to screen");
    }
    if (winner == 1) {
        //white win
        if (write(STDOUT_FILENO, "Winning player: White\n", strlen("Winning player: White\n")) < 0) {
            perror("failed to write to screen");
        }
    } else if (winner == 2) {
        //black win
        if (write(STDOUT_FILENO, "Winning player: Black\n", strlen("Winning player: Black\n")) < 0) {
            perror("failed to write to screen");
        }
    } else if (winner == 3) {
        //they are even
        if (write(STDOUT_FILENO, "No winning player\n", strlen("No winning player\n")) < 0) {
            perror("failed to write to screen");
        }
    }
    return;
}
