#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define ROW_SIZE 8
#define COL_SIZE 8

struct Point {
    int x;
    int y;
};


int gameBoard[ROW_SIZE][COL_SIZE];

void StartPlaying(int myRepresentaionNumber);

void CheckMove(struct Point *p, int *moveFlag);

void CheckLeft(struct Point *p, int *moveFlag);

void CheckRight(struct Point *p, int *moveFlag);

void CheckUp(struct Point *p, int *moveFlag);

void CheckDown(struct Point *p, int *moveFlag);

void CheckLeftAndUp(struct Point *p, int *moveFlag);

void CheckLeftAndDown(struct Point *p, int *moveFlag);

void CheckRightAndUp(struct Point *p, int *moveFlag);

void CheckRightAndDown(struct Point *p, int *moveFlag);

void PrintBoard();

struct Point *ParseStruct(char *move);

void initializeStruct(struct Play *p);

int main() {

    int myRepresentaionNumber = 1;
    memset(gameBoard, 0, ROW_SIZE * COL_SIZE * sizeof(int));

    StartPlaying(myRepresentaionNumber);
    return 0;
}

struct Point *ParseStruct(char *move) {
    int x;
    int y;

    char *temp = strtok(move, ",");
    if (strlen(temp) > 2) {
        //todo to big number
        printf("err");
    }
    x = temp[1] - 48;
    temp = strtok(NULL, ",");
    if (strlen(temp) > 2) {
        //todo to big number
        printf("err");
    }
    y = temp[0] - 48;

    struct Point *point = malloc(sizeof(struct Point));
    if (point == NULL) {
        //todo handle
    }
    return point;
}

void CheckMove(struct Point *p, int *moveFlag) {

    if ((p->x >= ROW_SIZE) || (p->x < 0) || (p->y < 0) || (p->y >= COL_SIZE)) {
        //todo wrong move
    }

    if (gameBoard[p->x][p->y] != 0) {
        //todo wrong move
    }


}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

void StartPlaying(int myNumber) {

    char move[6];
    int moved;
    struct Point *moveCoordinats;
    while (1) {

        //get move
        moved = 0;
        scanf("%s", move);
        moveCoordinats = ParseStruct(move);
        CheckMove(moveCoordinats, &moved);

        //if not legal move free struct and get move again
        while (moved == 0) {
            free(moveCoordinats);

            scanf("%s", move);
            moveCoordinats = ParseStruct(move);
            CheckMove(moveCoordinats, &moved);
        }

        //move was legal and we executed the move
        free(moveCoordinats);
        PrintBoard();
    }
}

#pragma clang diagnostic pop

//left means that i pressed on location left from me
void CheckLeft(struct Point *p, int *moveFlag) {
    //check if right from the given move there is an empty space or my piece
    if()
}

void CheckRight(struct Point *p, int *moveFlag) {}

void CheckUp(struct Point *p, int *moveFlag) {}

void CheckDown(struct Point *p, int *moveFlag) {}

void CheckLeftAndUp(struct Point *p, int *moveFlag) {}

void CheckLeftAndDown(struct Point *p, int *moveFlag) {}

void CheckRightAndUp(struct Point *p, int *moveFlag) {}

void CheckRightAndDown(struct Point *p, int *moveFlag) {}