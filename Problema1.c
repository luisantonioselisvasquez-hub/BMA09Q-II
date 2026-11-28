#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define T 3
#define MAX_CLAVES (2 * T - 1) // 5 claves por nodo
#define MAX_HIJOS (2 * T)      // 6 hijos por nodo

typedef struct {
    int dni;
    char nombre[30];
} Ciudadano;

typedef struct NodoB {
    Ciudadano datos[MAX_CLAVES];
    struct NodoB *hijos[MAX_HIJOS];
    int num_claves;
    bool es_hoja;
} NodoB;

NodoB* crear_nodo(bool es_hoja) {
    NodoB *nuevo = (NodoB*)malloc(sizeof(NodoB));
    nuevo->es_hoja = es_hoja;
    nuevo->num_claves = 0;
    for (int i = 0; i < MAX_HIJOS; i++) nuevo->hijos[i] = NULL;
    return nuevo;
}

// Búsqueda recursiva: O(log_t n)
Ciudadano* buscar_dni(NodoB *raiz, int dni) {
    if (!raiz) return NULL;
    int i = 0;
    while (i < raiz->num_claves && dni > raiz->datos[i].dni) i++;
    if (i < raiz->num_claves && raiz->datos[i].dni == dni) return &raiz->datos[i];
    return raiz->es_hoja ? NULL : buscar_dni(raiz->hijos[i], dni);
}

// Divide un hijo lleno y sube el dato del medio al padre
void dividir_hijo(NodoB *padre, int idx, NodoB *lleno) {
    NodoB *hermano = crear_nodo(lleno->es_hoja);
    hermano->num_claves = T - 1;
    for (int j = 0; j < T - 1; j++) hermano->datos[j] = lleno->datos[j + T];
    if (!lleno->es_hoja) {
        for (int j = 0; j < T; j++) hermano->hijos[j] = lleno->hijos[j + T];
    }
    lleno->num_claves = T - 1;

    for (int j = padre->num_claves; j >= idx + 1; j--) padre->hijos[j + 1] = padre->hijos[j];
    padre->hijos[idx + 1] = hermano;
    for (int j = padre->num_claves - 1; j >= idx; j--) padre->datos[j + 1] = padre->datos[j];
    padre->datos[idx] = lleno->datos[T - 1];
    padre->num_claves++;
}

void insertar_no_lleno(NodoB *nodo, Ciudadano reg) {
    int i = nodo->num_claves - 1;
    if (nodo->es_hoja) {
        while (i >= 0 && reg.dni < nodo->datos[i].dni) {
            nodo->datos[i + 1] = nodo->datos[i];
            i--;
        }
        nodo->datos[i + 1] = reg;
        nodo->num_claves++;
    } else {
        while (i >= 0 && reg.dni < nodo->datos[i].dni) i--;
        i++;
        if (nodo->hijos[i]->num_claves == MAX_CLAVES) {
            dividir_hijo(nodo, i, nodo->hijos[i]);
            if (reg.dni > nodo->datos[i].dni) i++;
        }
        insertar_no_lleno(nodo->hijos[i], reg);
    }
}

void insertar(NodoB **raiz, Ciudadano reg) {
    if ((*raiz)->num_claves == MAX_CLAVES) {
        NodoB *nueva = crear_nodo(false);
        nueva->hijos[0] = *raiz;
        dividir_hijo(nueva, 0, *raiz);
        *raiz = nueva;
    }
    insertar_no_lleno(*raiz, reg);
}

// Guarda todo el árbol ordenado en archivo binario (recorrido In-Orden)
void guardar_binario(NodoB *raiz, FILE *f) {
    if (!raiz) return;
    int i;
    for (i = 0; i < raiz->num_claves; i++) {
        if (!raiz->es_hoja) guardar_binario(raiz->hijos[i], f);
        fwrite(&raiz->datos[i], sizeof(Ciudadano), 1, f);
    }
    if (!raiz->es_hoja) guardar_binario(raiz->hijos[i], f);
}

int main() {
    NodoB *raiz = crear_nodo(true);
    Ciudadano lista[] = {
        {72451230, "Carlos"}, {45891204, "Ana"}, {80123456, "Luis"},
        {10293847, "Maria"},  {65432198, "Jorge"}, {51239874, "Elena"}
    };

    for (int i = 0; i < 6; i++) insertar(&raiz, lista[i]);

    FILE *f = fopen("padron.dat", "wb");
    guardar_binario(raiz, f);
    fclose(f);

    Ciudadano *res = buscar_dni(raiz, 65432198);
    if (res) printf("Encontrado: %d - %s\n", res->dni, res->nombre);
    return 0;
}
