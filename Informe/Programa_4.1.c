#include <stdio.h>
#include <stdlib.h>

#define MAX_PUNTOS 100

typedef struct {
    char nombre;
    double x, y;
} Punto;

typedef struct NodoKD {
    Punto p;
    struct NodoKD *izq, *der;
} NodoKD;

/* CREACION DE NODO */
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

int compararX(const void *a, const void *b) {
    const Punto *p1 = (const Punto *)a;
    const Punto *p2 = (const Punto *)b;
    if (p1->x < p2->x) return -1;
    if (p1->x > p2->x) return 1;
    return 0;
}

int compararY(const void *a, const void *b) {
    const Punto *p1 = (const Punto *)a;
    const Punto *p2 = (const Punto *)b;
    if (p1->y < p2->y) return -1;
    if (p1->y > p2->y) return 1;
    return 0;
}

/* CONSTRUCCION ESTATICA BALANCEADA (MEDIANA) */
NodoKD *construirBalanceado(Punto datos[], int inicio, int fin, int prof) {
    if (inicio > fin) return NULL;

    int eje = prof % 2;
    qsort(&datos[inicio], fin - inicio + 1, sizeof(Punto),
          (eje == 0) ? compararX : compararY);

    int mid = inicio + (fin - inicio) / 2;
    NodoKD *raiz = nuevoKD(datos[mid]);

    raiz->izq = construirBalanceado(datos, inicio, mid - 1, prof + 1);
    raiz->der = construirBalanceado(datos, mid + 1, fin, prof + 1);
    return raiz;
}

/* CONSTRUCCION DINAMICA (INSERCION SECUENCIAL) */
NodoKD *insertarKD(NodoKD *r, Punto p, int prof) {
    if (!r) return nuevoKD(p);

    int eje = prof % 2;
    double a = (eje == 0) ? p.x : p.y;
    double b = (eje == 0) ? r->p.x : r->p.y;

    if (a < b)
        r->izq = insertarKD(r->izq, p, prof + 1);
    else
        r->der = insertarKD(r->der, p, prof + 1);

    return r;
}

int alturaKD(NodoKD *r) {
    if (!r) return -1;
    int hi = alturaKD(r->izq);
    int hd = alturaKD(r->der);
    return 1 + (hi > hd ? hi : hd);
}

/* JERARQUIA QUE INDICA RAMA RESPECTIVA (RAIZ, IZQ, DER) */
void mostrarKD(NodoKD *r, int prof, const char *rama) {
    int i; /* Declarada fuera del for para compatibilidad C89 */
    char eje;
    if (!r) return;
    
    for (i = 0; i < prof; i++) printf("   ");
    eje = (prof % 2 == 0) ? 'X' : 'Y';
    printf("[%s] Nivel %d (Eje %c) -> %c = (%.1f, %.1f)\n",
           rama, prof, eje, r->p.nombre, r->p.x, r->p.y);
    mostrarKD(r->izq, prof + 1, "IZQ");
    mostrarKD(r->der, prof + 1, "DER");
}

/* LIBERAR MEMORIA */
void liberarKD(NodoKD *r) {
    if (r) {
        liberarKD(r->izq);
        liberarKD(r->der);
        free(r);
    }
}

void limpiarBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main(void) {
    Punto puntos[MAX_PUNTOS];
    Punto copia[MAX_PUNTOS];
    int n = 0, opcion = 0, i; /* 'i' declarada aqui */
    NodoKD *kdBal = NULL;
    NodoKD *kdSec = NULL;

    do {
        printf("\n=== CONSTRUCCION DE ARBOL INTERACTIVO K-D (2D) ===\n");
        printf("1. Ingresar puntos manualmente\n");
        printf("2. Cargar puntos del caso de estudio (A hasta G)\n");
        printf("3. Construir arbol estatico balanceado (Mediana)\n");
        printf("4. Construir arbol dinamico (Insercion secuencial)\n");
        printf("5. Salir\n");
        printf("Seleccione una opcion: ");

        if (scanf("%d", &opcion) != 1) {
            printf("Entrada invalida. Ingrese un numero del 1 al 5.\n");
            limpiarBuffer();
            continue;
        }

        switch (opcion) {
            case 1:
                printf("\nCuantos puntos desea ingresar (1 - %d)? ", MAX_PUNTOS);
                if (scanf("%d", &n) != 1 || n < 1 || n > MAX_PUNTOS) {
                    printf("Cantidad invalida.\n");
                    limpiarBuffer();
                    n = 0;
                    break;
                }
                for (i = 0; i < n; i++) {
                    printf("Punto %d -> Nombre (1 letra), X, Y (ej. A 5 5): ", i + 1);
                    if (scanf(" %c %lf %lf", &puntos[i].nombre, &puntos[i].x, &puntos[i].y) != 3) {
                        printf("Formato invalido. Intente nuevamente.\n");
                        limpiarBuffer();
                        i--;
                    }
                }
                printf("Puntos registrados correctamente.\n");
                break;

            case 2: {
                Punto ejemplo[] = {
                    {'A',5,5}, {'B',2,2}, {'C',7,7},
                    {'D',1,9}, {'E',4,8}, {'F',7,3},
                    {'G',9,1}
                };
                n = 7;
                for (i = 0; i < n; i++) puntos[i] = ejemplo[i];
                printf("\nPuntos cargados en orden secuencial: A(5,5), B(2,2), C(7,7), D(1,9), E(4,8), F(7,3), G(9,1).\n");
                break;
            }

            case 3:
                if (n == 0) {
                    printf("\nPrimero ingrese o cargue puntos (opcion 1 o 2).\n");
                    break;
                }
                liberarKD(kdBal);
                kdBal = NULL;
                for (i = 0; i < n; i++) copia[i] = puntos[i];
                kdBal = construirBalanceado(copia, 0, n - 1, 0);
                printf("\n--- ARBOL K-D BALANCEADO (POR MEDIANA) ---\n");
                mostrarKD(kdBal, 0, "RAIZ");
                printf("Altura resultante: %d\n", alturaKD(kdBal));
                break;

            case 4:
                if (n == 0) {
                    printf("\nPrimero ingrese o cargue puntos (opcion 1 o 2).\n");
                    break;
                }
                liberarKD(kdSec);
                kdSec = NULL;
                for (i = 0; i < n; i++) {
                    kdSec = insertarKD(kdSec, puntos[i], 0);
                }
                printf("\n--- ARBOL K-D DINAMICO (INSERCION SECUENCIAL) ---\n");
                mostrarKD(kdSec, 0, "RAIZ");
                printf("Altura resultante: %d\n", alturaKD(kdSec));
                break;

            case 5:
                printf("\nSaliendo del programa...\n");
                break;

            default:
                printf("\nOpcion no valida.\n");
        }
    } while (opcion != 5);

    liberarKD(kdBal);
    liberarKD(kdSec);
    return 0;
}

