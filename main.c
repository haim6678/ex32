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

int CheckEnd();

struct Point ReadData(char *data);

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

/**
 * operation- the main function
 */
int main() {

    key_t key;
    int shmid;
    char *data;
    int file;
    int fd_read;
    pid_t firstGivenPid;
    pid_t secondGivenPid;
    struct Point loc;
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
    if (close(fd_read) < 0) {
        perror("failed to close fifo");
        exit(-1);
    }

    *data = '$';

    //sending them the signal
    if (kill(SIGUSR1, firstGivenPid) < 0) {
        perror("failed to send sognal");
        exit(-1);
    }

    //wait for first player to make a move
    while (*data == '$') {
        sleep(1); //todo need sleep here
    }
    //read the move
    loc = ReadData(data);
    //execute it
    ExecuteMoveOnBoard(loc);
    //send to other player a signal to start play
    if (kill(SIGUSR1, secondGivenPid) < 0) {
        perror("failed to send signal");
        exit(-1);
    }

    //start following and managing the game
    RunGame(data);

    //todo close the fifo
    //todo delete the memory

    return 0;
}

/**
 * input - pointer to shared memory
 * output- the second player move
 * operation - reads from memory
 */
struct Point ReadData(char *data) {
    int x;
    int y;
    char temp;
    int player;

    x = (*data) + 48;
    (*data++);
    y = (*data) + 48;
    (*data++);
    temp = *data;
    if (temp == 'b') {
        player = 2;
    } else if (temp == 'w') {
        player = 1;
    }
    (*data++);
    (*data++);
    struct Point p;
    p.y = y;
    p.x = x;
    p.player = player;
    return p;
}

/**
 * input - the location which the player put is piece
 * operation- perform is move and update the board
 */
void ExecuteMoveOnBoard(struct Point loc) {
    ExecuteUp(&loc, loc.player);
    ExecuteDown(&loc, loc.player);
    ExecuteLeft(&loc, loc.player);
    ExecuteRight(&loc, loc.player);
    ExecuteRightAndUp(&loc, loc.player);
    ExecuteRightAndDown(&loc, loc.player);
    ExecuteLeftAndUp(&loc, loc.player);
    ExecuteLeftAndDown(&loc, loc.player);
}

/**
 * input - the pointer to the shared memory
 * operation- runs the game,every time gets a move,update the board and checking
 *            if the game need to end
 */
void RunGame(char *data) {

    int keepOn = KEEP_ON;
    struct Point loc;
    while (keepOn == KEEP_ON) {

        while (*data == '$') {
            sleep(1); //todo need this?
        }
        //read the data
        loc = ReadData(data);
        //execute it
        ExecuteMoveOnBoard(loc);
        //check board status
        keepOn = CheckEnd();
    }

    HandleEnd(keepOn);
}

/*
/**
 * input- get the point where thw player locate is new piece,a flag to update if we change
 *        something,and a flag to say what's my number on board
 * operation- checking if from the piece position to the right there is a legal move
 *            (that in the end there a second piece of my kind)
 *
 */

void ExecuteRight(struct Point *p, int myNumber) {
    int endX = -1;
    int endY = -1;
    int startX = p->x;
    int startY = p->y;

    //check if right from the given move there is an empty space or my piece
    if ((startY + 1 >= COL_SIZE) || (gameBoard[startX][startY + 1] == 0) ||
        (gameBoard[startX][startY + 1] == myNumber)) {
        return;

        /*check if the move is legal and there is a sequence of the other player pieces
        /with no whitespace and in the and again my piece*/
    } else {
        startY++;
        while ((endX == -1) && (endY == -1) && (startY < COL_SIZE)) {

            //if there is'nt my piece in the other side from the right
            if (gameBoard[startX][startY] == 0) {
                break;
            }
            if ((gameBoard[startX][startY] == myNumber)) {
                endX = startX;
                endY = startY;
            }
            startY++;
        }
    }

    //if we found the move legal the change the board and
    if ((endX != -1) && (endY != -1)) {
        startX = p->x;
        startY = p->y;
        while (startY < endY) {
            gameBoard[startX][startY] = myNumber;
            startY++;
        }
    }
}

/**
 * input- get the point where thw player locate is new piece,a flag to update if we change
 *        something,and a flag to say what's my number on board
 * operation- checking if from the piece position to the left there is a legal move
 *            (that in the end there a second piece of my kind)
 */
void ExecuteLeft(struct Point *p, int myNumber) {
    int endX = -1;
    int endY = -1;
    int startX = p->x;
    int startY = p->y;
    //check if left from the given move there is an empty space or my piece
    if ((startY - 1 < 0) || (gameBoard[startX][startY - 1] == 0) ||
        (gameBoard[startX][startY - 1] == myNumber)) {
        return;

        /*check if the move is legal and there is a sequence of the other player pieces
        /with no whitespace and in the and again my piece*/
    } else {
        startY--;
        while ((endX == -1) && (endY == -1) && (startY >= 0)) {

            //if there is'nt my piece in the other side from the left
            if (gameBoard[startX][startY] == 0) {
                break;
            }
            if ((gameBoard[startX][startY] == myNumber)) {
                endX = startX;
                endY = startY;
            }
            startY--;
        }
    }
    //if we found the move legal the change the board
    if ((endX != -1) && (endY != -1)) {
        startX = p->x;
        startY = p->y;
        while (startY > endY) {
            gameBoard[startX][startY] = myNumber;
            startY--;
        }
    }
}

/**
 * input- get the point where thw player locate is new piece,a flag to update if we change
 *        something,and a flag to say what's my number on board
 * operation- checking if from the piece position to upward there is a legal move
 *            (that in the end there a second piece of my kind)
 */
void ExecuteUp(struct Point *p, int myNumber) {
    int endX = -1;
    int endY = -1;
    int startX = p->x;
    int startY = p->y;
    //check if above from the given move there is an empty space or my piece
    if ((startX - 1 < 0) || (gameBoard[startX - 1][startY] == 0) ||
        (gameBoard[startX - 1][startY] == myNumber)) {
        return;

        /*check if the move is legal and there is a sequence of the other player pieces
        /with no whitespace and in the and again my piece*/
    } else {
        startX--;
        while ((endX == -1) && (endY == -1) && (startX >= 0)) {

            //if there is'nt my piece in the other side from the above
            if (gameBoard[startX][startY] == 0) {
                break;
            }
            if ((gameBoard[startX][startY] == myNumber)) {
                endX = startX;
                endY = startY;
            }
            startX--;
        }
    }
    //if we found the move legal the change the board
    if ((endX != -1) && (endY != -1)) {
        startX = p->x;
        startY = p->y;
        while (startX > endX) {
            gameBoard[startX][startY] = myNumber;
            startX--;
        }
    }
}

/**
 * input- get the point where thw player locate is new piece,a flag to update if we change
 *        something,and a flag to say what's my number on board
 * operation- checking if from the piece position to down there is a legal move
 *            (that in the end there a second piece of my kind)
 */
void ExecuteDown(struct Point *p, int myNumber) {
    int endX = -1;
    int endY = -1;
    int startX = p->x;
    int startY = p->y;
    //check if below from the given move there is an empty space or my piece
    if ((startX + 1 >= ROW_SIZE) || (gameBoard[startX + 1][startY] == 0) ||
        (gameBoard[startX + 1][startY] == myNumber)) {
        return;

        /*check if the move is legal and there is a sequence of the other player pieces
        /with no whitespace and in the and again my piece*/
    } else {
        startX++;
        while ((endX == -1) && (endY == -1) && (startX < ROW_SIZE)) {

            //if there is'nt my piece in the other side from the bottom
            if (gameBoard[startX][startY] == 0) {
                break;
            }
            if ((gameBoard[startX][startY] == myNumber)) {
                endX = startX;
                endY = startY;
            }
            startX++;
        }
    }
    //if we found the move legal the change the board
    if ((endX != -1) && (endY != -1)) {
        startX = p->x;
        startY = p->y;
        while (startX < endX) {
            gameBoard[startX][startY] = myNumber;
            startX++;
        }
    }
}

/**
 * input- get the point where thw player locate is new piece,a flag to update if we change
 *        something,and a flag to say what's my number on board
 * operation- checking if from the piece position to the left and up there is a legal move
 *            (that in the end there a second piece of my kind)
 */
void ExecuteLeftAndUp(struct Point *p, int myNumber) {
    int endX = -1;
    int endY = -1;
    int startX = p->x;
    int startY = p->y;
    //check if left and up from the given move there is an empty space or my piece
    if ((startY - 1 < 0) || (startX - 1 < 0) || (gameBoard[startX - 1][startY - 1] == 0) ||
        (gameBoard[startX - 1][startY - 1] == myNumber)) {
        return;

        /*check if the move is legal and there is a sequence of the other player pieces
        /with no whitespace and in the and again my piece*/
    } else {
        startX--;
        startY--;
        while ((endX == -1) && (endY == -1) && (startX >= 0) && (startY >= 0)) {

            //if there is'nt my piece in the other side from the left and up
            if (gameBoard[startX][startY] == 0) {
                break;
            }
            if ((gameBoard[startX][startY] == myNumber)) {
                endX = startX;
                endY = startY;
            }
            startX--;
            startY--;
        }
    }
    //if we found the move legal the change the board and update the flag to 1
    if ((endX != -1) && (endY != -1)) {
        startX = p->x;
        startY = p->y;
        while ((startX > endX) && (startY > endY)) {
            gameBoard[startX][startY] = myNumber;
            startX--;
            startY--;
        }
    }
}

/**
 * input- get the point where thw player locate is new piece,a flag to update if we change
 *        something,and a flag to say what's my number on board
 * operation- checking if from the piece position to the left and down there is a legal move
 *            (that in the end there a second piece of my kind)
 */
void ExecuteLeftAndDown(struct Point *p, int myNumber) {
    int endX = -1;
    int endY = -1;
    int startX = p->x;
    int startY = p->y;
    //check if left and down from the given move there is an empty space or my piece
    if ((startY - 1 < 0) || (startX + 1 >= ROW_SIZE) || (gameBoard[startX + 1][startY - 1] == 0) ||
        (gameBoard[startX + 1][startY - 1] == myNumber)) {
        return;

        /*check if the move is legal and there is a sequence of the other player pieces
        /with no whitespace and in the and again my piece*/
    } else {
        startX++;
        startY--;
        while ((endX == -1) && (endY == -1) && (startX < ROW_SIZE) && (startY >= 0)) {

            //if there is'nt my piece in the other side from the left and down
            if (gameBoard[startX][startY] == 0) {
                break;
            }
            if ((gameBoard[startX][startY] == myNumber)) {
                endX = startX;
                endY = startY;
            }
            startX++;
            startY--;
        }
    }
    //if we found the move legal the change the board
    if ((endX != -1) && (endY != -1)) {
        startX = p->x;
        startY = p->y;
        while ((startX < endX) && (startY > endY)) {
            gameBoard[startX][startY] = myNumber;
            startX++;
            startY--;
        }
    }
}

/**
 * input- get the point where thw player locate is new piece,a flag to update if we change
 *        something,and a flag to say what's my number on board
 * operation- checking if from the piece position to the right and up there is a legal move
 *            (that in the end there a second piece of my kind)
 */
void ExecuteRightAndUp(struct Point *p, int myNumber) {
    int endX = -1;
    int endY = -1;
    int startX = p->x;
    int startY = p->y;
    //check if right and up from the given move there is an empty space or my piece
    if ((startY + 1 >= COL_SIZE) || (startX - 1 < 0) || (gameBoard[startX - 1][startY + 1] == 0) ||
        (gameBoard[startX - 1][startY + 1] == myNumber)) {
        return;

        /*check if the move is legal and there is a sequence of the other player pieces
        /with no whitespace and in the and again my piece*/
    } else {
        startX--;
        startY++;
        while ((endX == -1) && (endY == -1) && (startX >= 0) && (startY < COL_SIZE)) {

            //if there is'nt my piece in the other side from the right and up
            if (gameBoard[startX][startY] == 0) {
                break;
            }
            if ((gameBoard[startX][startY] == myNumber)) {
                endX = startX;
                endY = startY;
            }
            startX--;
            startY++;
        }
    }
    //if we found the move legal the change the board
    if ((endX != -1) && (endY != -1)) {
        startX = p->x;
        startY = p->y;
        while ((startX > endX) && (startY < endY)) {
            gameBoard[startX][startY] = myNumber;
            startX--;
            startY++;
        }
    }
}

/**
 * input- get the point where thw player locate is new piece,a flag to update if we change
 *        something,and a flag to say what's my number on board
 * operation- checking if from the piece position to the right and down there is a legal move
 *            (that in the end there a second piece of my kind)
 */
void ExecuteRightAndDown(struct Point *p, int myNumber) {
    int endX = -1;
    int endY = -1;
    int startX = p->x;
    int startY = p->y;
    //check if right and down from the given move there is an empty space or my piece
    if ((startY + 1 >= COL_SIZE) || (startX + 1 >= ROW_SIZE) | (gameBoard[startX + 1][startY + 1] == 0) ||
        (gameBoard[startX + 1][startY + 1] == myNumber)) {
        return;

        /*check if the move is legal and there is a sequence of the other player pieces
        /with no whitespace and in the and again my piece*/
    } else {
        startX++;
        startY++;
        while ((endX == -1) && (endY == -1) && (startX < ROW_SIZE) && (startY < COL_SIZE)) {

            //if there is'nt my piece in the other side from the right and down
            if (gameBoard[startX][startY] == 0) {
                break;
            }
            if ((gameBoard[startX][startY] == myNumber)) {
                endX = startX;
                endY = startY;
            }
            startX++;
            startY++;
        }
    }
    //if we found the move legal the change the board and update the flag to 1
    if ((endX != -1) && (endY != -1)) {
        startX = p->x;
        startY = p->y;
        while ((startX < endX) && (startY < endY)) {
            gameBoard[startX][startY] = myNumber;
            startX++;
            startY++;
        }
    }
}

/**
 * input- the number that indicate the winner
 * operation- print the message and exit's
 */
void HandleEnd(int winner) {
    if (winner == 1) {
        //white win
        if (write(STDOUT_FILENO, "Winning player: White", strlen("Winning player: White")) < 0) {
            perror("failed to write to screen");
        }
    } else if (winner == 2) {
        //black win
        if (write(STDOUT_FILENO, "Winning player: Black", strlen("Winning player: Black")) < 0) {
            perror("failed to write to screen");
        }
    } else if (winner == 3) {
        //they are even
        if (write(STDOUT_FILENO, "No winning player", strlen("No winning player")) < 0) {
            perror("failed to write to screen");
        }
    }
    //todo ReleaseMemoryEndExit();
}

/**
 * input - a flag that indicate the sort of check we conduct
 * operation - check if the board is full, or there is only one color
 */
int CheckWinner(int flag) {

    int i = 0;
    int j = 0;
    int white = 0;
    int black = 0;
    for (i = 0; i < ROW_SIZE; i++) {
        for (j = 0; j < COL_SIZE; j++) {

            if (gameBoard[i][j] == 1) {
                white++;
            } else if (gameBoard[i][j] == 2) {
                black++;
            }
        }
    }
    //check if full who won
    if (flag == 1) {
        if (black > white) {
            return 2;
        } else if (white > black) {
            return 1;
        } else {
            return 3;
        }
        //if there is a space check what color is left on board
    } else if (flag == 0) {
        if ((black > 0) && (white == 0)) {
            return 2;
        } else if ((white > 0) && (black == 0)) {
            return 1;
        } else {
            return KEEP_ON;
        }
    }
}

/**
 * operation- check if board is full or only one color left
 */
int CheckEnd() {

    int flag = 0;
    int i = 0;
    int j = 0;
    //check the board
    for (i = 0; i < ROW_SIZE; i++) {
        for (j = 0; j < COL_SIZE; j++) {
            //if we found an empty space
            if (gameBoard[i][j] != 0) {
                flag = KEEP_ON;
                break;
            }
        }
    }

    //we didn't found an empty space
    if (flag == 0) {
        return CheckWinner(1);
        //it's  -1 ->we found an empty space then keep playing and check if left a move
    } else {
        return CheckWinner(0);
    }
}