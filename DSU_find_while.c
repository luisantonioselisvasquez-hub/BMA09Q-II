#include <stdio.h>

#define N 8

int parent[N] = {0, 0, 0, 2, 2, 3, 1, 6};

int find(int x) {
    while (parent[x] != x) {
        x = parent[x];
    }

    return x;
}

int main() {
    int elemento = 3;

    printf("Representante: %d\n",
           find(elemento));

    return 0;
}