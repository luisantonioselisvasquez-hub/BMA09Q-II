#include <stdio.h>
#include <stdlib.h>

/* -------------------- 1. ÁRBOL DE SEGMENTOS (SUMA) -------------------- */

typedef struct {
    int n;      /* Tamaño del arreglo original */
    int *tree;  /* Arreglo auxiliar de tamaño 4 * n */
} SegmentTree;

SegmentTree* st_crear(int n) {
    SegmentTree *st = (SegmentTree*)malloc(sizeof(SegmentTree));
    st->n = n;
    /* Hasta 4*n nodos son necesarios para garantizar espacio sin desbordar */
    st->tree = (int*)calloc((size_t)(4 * n + 4), sizeof(int));
    return st;
}

/* Construye el árbol de forma recursiva (Divide y Vencerás) */
void st_construir_rec(SegmentTree *st, const int *arr, int nodo, int izq, int der) {
    if (izq == der) {
        st->tree[nodo] = arr[izq];
        return;
    }
    int mid = izq + (der - izq) / 2;
    int hijo_izq = 2 * nodo;
    int hijo_der = 2 * nodo + 1;

    st_construir_rec(st, arr, hijo_izq, izq, mid);
    st_construir_rec(st, arr, hijo_der, mid + 1, der);
    st->tree[nodo] = st->tree[hijo_izq] + st->tree[hijo_der];
}

void st_construir(SegmentTree *st, const int *arr) {
    if (st->n > 0) {
        st_construir_rec(st, arr, 1, 0, st->n - 1);
    }
}

/* Consulta la suma en el intervalo cerrado [ql, qr] */
int st_consulta_rec(const SegmentTree *st, int nodo, int izq, int der, int ql, int qr) {
    if (ql > der || qr < izq) {
        return 0; /* Elemento neutro de la suma */
    }
    if (ql <= izq && der <= qr) {
        return st->tree[nodo]; /* Segmento completamente contenido */
    }
    int mid = izq + (der - izq) / 2;
    return st_consulta_rec(st, 2 * nodo, izq, mid, ql, qr) +
           st_consulta_rec(st, 2 * nodo + 1, mid + 1, der, ql, qr);
}

int st_consulta(const SegmentTree *st, int ql, int qr) {
    if (ql > qr || ql < 0 || qr >= st->n) return 0;
    return st_consulta_rec(st, 1, 0, st->n - 1, ql, qr);
}

/* Actualización puntual: asigna 'valor' en la posición 'pos' */
void st_actualizar_rec(SegmentTree *st, int nodo, int izq, int der, int pos, int valor) {
    if (izq == der) {
        st->tree[nodo] = valor;
        return;
    }
    int mid = izq + (der - izq) / 2;
    if (pos <= mid) {
        st_actualizar_rec(st, 2 * nodo, izq, mid, pos, valor);
    } else {
        st_actualizar_rec(st, 2 * nodo + 1, mid + 1, der, pos, valor);
    }
    st->tree[nodo] = st->tree[2 * nodo] + st->tree[2 * nodo + 1];
}

void st_actualizar(SegmentTree *st, int pos, int valor) {
    if (pos < 0 || pos >= st->n) return;
    st_actualizar_rec(st, 1, 0, st->n - 1, pos, valor);
}

void st_liberar(SegmentTree *st) {
    if (!st) return;
    free(st->tree);
    free(st);
}

/* -------------------- 2. ÁRBOL DE FENWICK / BIT (SUMA) -------------------- */

typedef struct {
    int n;      /* Tamaño del arreglo original */
    int *bit;   /* Arreglo indexado en base 1 de tamaño n + 1 */
} FenwickTree;

FenwickTree* bit_crear(int n) {
    FenwickTree *ft = (FenwickTree*)malloc(sizeof(FenwickTree));
    ft->n = n;
    /* Solo requiere n + 1 posiciones (mucho menos memoria que 4*n) */
    ft->bit = (int*)calloc((size_t)(n + 1), sizeof(int));
    return ft;
}

/* Suma 'delta' a la posición 'idx' (1 <= idx <= n) usando bit menos significativo */
void bit_sumar(FenwickTree *ft, int idx, int delta) {
    for (; idx <= ft->n; idx += idx & (-idx)) {
        ft->bit[idx] += delta;
    }
}

/* Calcula la suma del prefijo [1..idx] */
int bit_prefijo(const FenwickTree *ft, int idx) {
    int suma = 0;
    for (; idx > 0; idx -= idx & (-idx)) {
        suma += ft->bit[idx];
    }
    return suma;
}

/* Suma en rango [l..r] (base 1): requiere operación invertible (resta) */
int bit_consulta_rango(const FenwickTree *ft, int l, int r) {
    if (l > r || l < 1 || r > ft->n) return 0;
    return bit_prefijo(ft, r) - bit_prefijo(ft, l - 1);
}

void bit_liberar(FenwickTree *ft) {
    if (!ft) return;
    free(ft->bit);
    free(ft);
}

/* -------------------- 3. APLICACIÓN: CONTEO DE INVERSIONES -------------------- */
/*
 * Una inversión es un par (i, j) con i < j tal que arr[i] > arr[j].
 * Recorriendo de derecha a izquierda y usando un BIT de frecuencias, contamos
 * en O(n log M) cuántos elementos menores ya aparecieron a la derecha.
 * Nota: aquí se asumen valores positivos en [1..max_val].
 */
long long contar_inversiones(const int *arr, int n, int max_val) {
    FenwickTree *freq = bit_crear(max_val);
    long long inversiones = 0;

    for (int i = n - 1; i >= 0; i--) {
        if (arr[i] > 1) {
            inversiones += bit_prefijo(freq, arr[i] - 1);
        }
        bit_sumar(freq, arr[i], 1);
    }

    bit_liberar(freq);
    return inversiones;
}

/* -------------------- 4. DEMOSTRACIÓN Y COMPARACIÓN -------------------- */

int main(void) {
    int arr[] = {2, 4, 1, 3, 5};
    int n = (int)(sizeof(arr) / sizeof(arr[0]));

    SegmentTree *st = st_crear(n);
    FenwickTree *ft = bit_crear(n);

    st_construir(st, arr);
    for (int i = 0; i < n; i++) {
        bit_sumar(ft, i + 1, arr[i]); /* Conversión a base 1 para BIT */
    }

    printf("=== COMPARACION: SEGMENT TREE vs. FENWICK TREE (BIT) ===\n");
    printf("Arreglo inicial (base 0): [2, 4, 1, 3, 5]\n\n");

    /* Consulta de suma en rango [1..3] (base 0 -> elementos 4, 1, 3 => suma 8) */
    int q_st = st_consulta(st, 1, 3);
    int q_ft = bit_consulta_rango(ft, 2, 4); /* Mismo rango en base 1 */
    printf("Consulta suma rango [1..3] (base 0) / [2..4] (base 1):\n");
    printf("  Segment Tree : %d (memoria asignada: %d enteros)\n", q_st, 4 * n + 4);
    printf("  Fenwick Tree : %d (memoria asignada: %d enteros)\n\n", q_ft, n + 1);

    /* Actualización puntual: arr[2] pasa de 1 a 10 */
    int nueva_val = 10;
    int delta = nueva_val - arr[2];
    arr[2] = nueva_val;
    st_actualizar(st, 2, nueva_val);
    bit_sumar(ft, 3, delta); /* Posición 3 en base 1 */

    q_st = st_consulta(st, 1, 3); /* Ahora 4 + 10 + 3 = 17 */
    q_ft = bit_consulta_rango(ft, 2, 4);
    printf("Tras actualizar indice 2 -> 10, consulta en el mismo rango:\n");
    printf("  Segment Tree : %d\n", q_st);
    printf("  Fenwick Tree : %d\n\n", q_ft);

    /* Aplicación práctica: conteo de inversiones */
    int seq[] = {8, 4, 2, 1};
    int m = (int)(sizeof(seq) / sizeof(seq[0]));
    long long inv = contar_inversiones(seq, m, 8);
    printf("=== APLICACION: ESTADISTICAS ACUMULADAS (INVERSIONES) ===\n");
    printf("Secuencia: [8, 4, 2, 1] -> Total de inversiones: %lld\n", inv);

    st_liberar(st);
    bit_liberar(ft);
    return 0;
}