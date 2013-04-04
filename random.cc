#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cstring>



int main(int argc, const char *argv[])
{
    srand(time(NULL));
    int size = 3;
    int num = 9;
    if (argc > 1) {
        sscanf(argv[1], "%d", &size);
    }
    if (argc > 2) {
        sscanf(argv[2], "%d", &num);
    } else {
        num = rand() % (size * size);
    }
    int board_size = size*size*size*size;
    int *board = new int[board_size];
    memset(board, 0, board_size * sizeof(int));


    int i = 0;
    while (i < num) {
        int pos = rand() % board_size;
        if (board[pos] == 0) {
            ++i;
            board[pos] = rand() % (size * size) + 1;
        }
    }

    for (int i = 0; i < board_size; ++i) {
        if (board[i] == 0) {
            printf(". ");
        } else {
            printf("%d ", board[i]);
        }
        if (((i + 1) % (size * size)) == 0) {
            printf("\n");
        }

    }
}
