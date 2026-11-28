#include <stdio.h>
#include <stdlib.h>

#define MAX_PUNTOS 100
#define MAX_DIM 10

typedef struct {
    char nombre;
    double coords[MAX_DIM];
} Punto;

typedef struct NodoKD {
    Punto p;
    struct NodoKD *izq, *der;
} NodoKD;

void limpiarBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

NodoKD *nuevoKD(Punto p) {
    NodoKD *n = (NodoKD *)malloc(sizeof(NodoKD));
    if (!n) {
        printf("Error: memoria insuficiente.\n");
        exit(EXIT_FAILURE);
    }
    n->p = p;
    n->izq = NULL;
    n->der = NULL;
    return n;
}

/* ORDENAMIENTO POR EJE ACTIVO */
void ordenarPorEje(Punto datos[], int inicio, int fin, int eje) {
    int i, j;
    Punto clave;
    for (i = inicio + 1; i <= fin; i++) {
        clave = datos[i];
        j = i - 1;
        while (j >= inicio && datos[j].coords[eje] > clave.coords[eje]) {
            datos[j + 1] = datos[j];
            j--;
        }
        datos[j + 1] = clave;
    }
}

/* CONSTRUCCION ESTATICA BALANCEADA (PARA K DIMENSIONES) */
NodoKD *construirBalanceado(Punto datos[], int inicio, int fin, int prof, int k) {
    int eje, mid;
    NodoKD *raiz;
    if (inicio > fin) return NULL;

    eje = prof % k;
    ordenarPorEje(datos, inicio, fin, eje);
    mid = inicio + (fin - inicio) / 2;

    /* Ajuste para cumplir regla estricta: IZQ (<) y DER (>=) */
    while (mid > inicio && datos[mid - 1].coords[eje] == datos[mid].coords[eje]) {
        mid--;
    }

    raiz = nuevoKD(datos[mid]);
    raiz->izq = construirBalanceado(datos, inicio, mid - 1, prof + 1, k);
    raiz->der = construirBalanceado(datos, mid + 1, fin, prof + 1, k);
    return raiz;
}

/* CONSTRUCCION DINAMICA SECUENCIAL (PARA K DIMENSIONES) */
NodoKD *insertarKD(NodoKD *r, Punto p, int prof, int k) {
    int eje;
    if (!r) return nuevoKD(p);

    eje = prof % k;
    if (p.coords[eje] < r->p.coords[eje])
        r->izq = insertarKD(r->izq, p, prof + 1, k);
    else
        r->der = insertarKD(r->der, p, prof + 1, k);

    return r;
}

/* CALCULO DE ALTURA POR ARISTAS (ARBOL VACIO = -1) */
int alturaKD(NodoKD *r) {
    int hi, hd;
    if (!r) return -1;
    hi = alturaKD(r->izq);
    hd = alturaKD(r->der);
    return 1 + (hi > hd ? hi : hd);
}

/* IMPRESION JERARQUICA PARA CUALQUIER DIMENSION K */
void mostrarKD(NodoKD *r, int prof, int k, const char *rama) {
    int i, eje, d;
    const char *nombresEjes[] = {"X", "Y", "Z", "W"};
    if (!r) return;

    for (i = 0; i < prof; i++) printf(" ");
    eje = prof % k;

    if (eje < 4)
        printf("[%s] Nivel %d (Eje %s) -> %c = (", rama, prof, nombresEjes[eje], r->p.nombre);
    else
        printf("[%s] Nivel %d (Dim %d) -> %c = (", rama, prof, eje, r->p.nombre);

    for (d = 0; d < k; d++) {
        printf("%.1f%s", r->p.coords[d], (d < k - 1) ? ", " : "");
    }
    printf(")\n");

    mostrarKD(r->izq, prof + 1, k, "IZQ");
    mostrarKD(r->der, prof + 1, k, "DER");
}

/* LIBERAR MEMORIA */
void liberarKD(NodoKD *r) {
    if (r) {
        liberarKD(r->izq);
        liberarKD(r->der);
        free(r);
    }
}

int main(void) {
    Punto puntos[MAX_PUNTOS];
    Punto copia[MAX_PUNTOS];
    int n = 0, k = 2, opcion = 0;
    int i, d, errorCoord;
    NodoKD *kdBal = NULL;
    NodoKD *kdSec = NULL;

    printf("Ingrese la dimension k del arbol (ej. 2 para 2D, 3 para 3D, max %d): ", MAX_DIM);
    if (scanf("%d", &k) != 1 || k < 1 || k > MAX_DIM) {
        printf("Dimension invalida. Se usara 2D por defecto.\n");
        k = 2;
    }
    limpiarBuffer();

    do {
        printf("\n=== ARBOL K-D EN %dD ===\n", k);
        printf("1. Ingresar puntos manualmente\n");
        printf("2. Construir arbol estatico balanceado (Mediana)\n");
        printf("3. Construir arbol dinamico (Insercion secuencial)\n");
        printf("4. Salir\n");
        printf("Seleccione una opcion: ");

        if (scanf("%d", &opcion) != 1) {
            printf("Entrada invalida. Ingrese un numero del 1 al 4.\n");
            limpiarBuffer();
            continue;
        }
        limpiarBuffer();

        switch (opcion) {
            case 1:
                printf("\nCantidad de puntos a ingresar (1 - %d): ", MAX_PUNTOS);
                if (scanf("%d", &n) != 1 || n < 1 || n > MAX_PUNTOS) {
                    printf("Cantidad invalida.\n");
                    limpiarBuffer();
                    n = 0;
                    break;
                }
                limpiarBuffer();

                for (i = 0; i < n; i++) {
                    printf("Punto %d -> Nombre (1 letra) y %d coordenadas (ej. A", i + 1, k);
                    for (d = 0; d < k; d++) printf(" %d", (d + 1) * 2);
                    printf("): ");

                    if (scanf(" %c", &puntos[i].nombre) != 1) {
                        printf("Nombre invalido. Intente nuevamente.\n");
                        limpiarBuffer();
                        i--;
                        continue;
                    }

                    errorCoord = 0;
                    for (d = 0; d < k; d++) {
                        if (scanf("%lf", &puntos[i].coords[d]) != 1) {
                            errorCoord = 1;
                            break;
                        }
                    }
                    limpiarBuffer();

                    if (errorCoord) {
                        printf("Coordenadas invalidas. Intente nuevamente el punto %d.\n", i + 1);
                        i--;
                    }
                }
                printf("Puntos registrados correctamente.\n");
                break;

            case 2:
                if (n == 0) {
                    printf("\nPrimero ingrese los puntos (opcion 1).\n");
                    break;
                }
                liberarKD(kdBal);
                for (i = 0; i < n; i++) copia[i] = puntos[i];
                kdBal = construirBalanceado(copia, 0, n - 1, 0, k);
                printf("\n--- ARBOL %dD ESTATICO BALANCEADO (MEDIANA) ---\n", k);
                mostrarKD(kdBal, 0, k, "RAIZ");
                printf("Altura resultante: %d\n", alturaKD(kdBal));
                break;

            case 3:
                if (n == 0) {
                    printf("\nPrimero ingrese los puntos (opcion 1).\n");
                    break;
                }
                liberarKD(kdSec);
                kdSec = NULL;
                for (i = 0; i < n; i++) {
                    kdSec = insertarKD(kdSec, puntos[i], 0, k);
                }
                printf("\n--- ARBOL %dD DINAMICO (INSERCION SECUENCIAL) ---\n", k);
                mostrarKD(kdSec, 0, k, "RAIZ");
                printf("Altura resultante: %d\n", alturaKD(kdSec));
                break;

            case 4:
                printf("\nSaliendo del programa...\n");
                break;

            default:
                printf("\nOpcion no valida.\n");
        }
    } while (opcion != 4);

    liberarKD(kdBal);
    liberarKD(kdSec);
    return 0;
}

