#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

#define T 3
#define MAX_CLAVES 5  // 2*T - 1
#define MAX_HIJOS  6  // 2*T

typedef struct Nodo {
    int asientos[MAX_CLAVES];
    struct Nodo *hijos[MAX_HIJOS];
    int n;
    bool hoja;
} Nodo;

typedef struct {
    int asiento;
    char cliente[20];
} Pedido;

// Variables globales para compartir entre los hilos
Nodo *raiz = NULL;
HANDLE mutex_arbol;   // Candado que protege el árbol
int ventas_ok = 0;
int rechazados = 0;

Nodo* crear_nodo(bool hoja) {
    Nodo *nuevo = (Nodo*)malloc(sizeof(Nodo));
    nuevo->hoja = hoja;
    nuevo->n = 0;
    for (int i = 0; i < MAX_HIJOS; i++) nuevo->hijos[i] = NULL;
    return nuevo;
}

bool existe_asiento(Nodo *actual, int asiento) {
    int i = 0;
    while (i < actual->n && asiento > actual->asientos[i]) i++;
    if (i < actual->n && actual->asientos[i] == asiento) return true;
    if (actual->hoja) return false;
    return existe_asiento(actual->hijos[i], asiento);
}

void dividir_hijo(Nodo *padre, int pos) {
    Nodo *lleno = padre->hijos[pos];
    Nodo *nuevo = crear_nodo(lleno->hoja);
    nuevo->n = T - 1;

    for (int j = 0; j < T - 1; j++) nuevo->asientos[j] = lleno->asientos[j + T];
    if (!lleno->hoja) {
        for (int j = 0; j < T; j++) nuevo->hijos[j] = lleno->hijos[j + T];
    }
    lleno->n = T - 1;

    for (int j = padre->n; j > pos; j--) padre->hijos[j + 1] = padre->hijos[j];
    padre->hijos[pos + 1] = nuevo;

    for (int j = padre->n - 1; j >= pos; j--) padre->asientos[j + 1] = padre->asientos[j];
    padre->asientos[pos] = lleno->asientos[T - 1];
    padre->n++;
}

void insertar_no_lleno(Nodo *actual, int asiento) {
    int i = actual->n - 1;
    if (actual->hoja) {
        while (i >= 0 && asiento < actual->asientos[i]) {
            actual->asientos[i + 1] = actual->asientos[i];
            i--;
        }
        actual->asientos[i + 1] = asiento;
        actual->n++;
    } else {
        while (i >= 0 && asiento < actual->asientos[i]) i--;
        i++;
        if (actual->hijos[i]->n == MAX_CLAVES) {
            dividir_hijo(actual, i);
            if (asiento > actual->asientos[i]) i++;
        }
        insertar_no_lleno(actual->hijos[i], asiento);
    }
}

void insertar(int asiento) {
    if (raiz->n == MAX_CLAVES) {
        Nodo *nueva = crear_nodo(false);
        nueva->hijos[0] = raiz;
        dividir_hijo(nueva, 0);
        insertar_no_lleno(nueva, asiento);
        raiz = nueva;
    } else {
        insertar_no_lleno(raiz, asiento);
    }
}

// Función que ejecuta cada cliente en paralelo
DWORD WINAPI hilo_cliente(LPVOID arg) {
    Pedido *p = (Pedido*)arg;

    // 1. Cierra el candado (mutex) para que solo entre un cliente a la vez
    WaitForSingleObject(mutex_arbol, INFINITE);

    if (existe_asiento(raiz, p->asiento)) {
        printf("[RECHAZADO] %s: asiento #%d ya ocupado.\n", p->cliente, p->asiento);
        rechazados++;
    } else {
        insertar(p->asiento);
        printf("[EXITO] %s reservo asiento #%d.\n", p->cliente, p->asiento);
        ventas_ok++;
    }

    // 2. Abre el candado para que pase el siguiente cliente
    ReleaseMutex(mutex_arbol);
    return 0;
}

int main() {
    raiz = crear_nodo(true);
    mutex_arbol = CreateMutex(NULL, FALSE, NULL);

    HANDLE hilos[4];
    Pedido pedidos[4] = {
        {25, "Cliente_1"},
        {10, "Cliente_2"},
        {25, "Cliente_3"}, // Duplicado: será rechazado
        {40, "Cliente_4"}
    };

    // Lanza los 4 clientes al mismo tiempo
    for (int i = 0; i < 4; i++) {
        hilos[i] = CreateThread(NULL, 0, hilo_cliente, &pedidos[i], 0, NULL);
    }

    // Espera a que los 4 terminen
    WaitForMultipleObjects(4, hilos, TRUE, INFINITE);

    printf("\nVentas confirmadas: %d | Rechazados: %d\n", ventas_ok, rechazados);

    CloseHandle(mutex_arbol);
    return 0;
}
