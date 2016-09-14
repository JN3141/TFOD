#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int players[] = {6,6,6,6,6};
    int turns = 30;
    int p;
    for (p = 0; p <= 4; p++) {
        // for every turn beyond the end of the last round
        if (p < (turns % 5)) {
            players[p]++;
        }
    }
    printf("{%d,%d,%d,%d,%d}\n",players[0],players[1],players[2],players[3],players[4]);

    /*
    if (pastPlays[0] == 'G') {
        int i;
        int turns = 0;
        int rounds = 0;
        for (i = 1; pastPlays[i-1] != '\0'; i++) {
            if (i % 8 == 0) {
                turns++;
                if (turns % 5 == 0) {
                    rounds++;
                    printf("num rounds is %d\n",rounds);
                }
            }
        }
        printf("num turns is %d\n",turns);
    }
    */

    return 0;
}
