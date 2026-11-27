#include <stdio.h>

#define N 5

int parent[N] = {0, 0, 1, 2, 3};

int find(int x) {
    if (parent[x] == x) {
        return x;
    }

    return find(parent[x]);
}

int main() {
    int elemento = 4;

    printf("Representante: %d\n",
           find(elemento));

    return 0;
}