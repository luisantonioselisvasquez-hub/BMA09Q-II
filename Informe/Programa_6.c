#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#define K 2

typedef struct Nodo {
    double coords[K];
    struct Nodo *izq;
    struct Nodo *der;
} Nodo;

/* Crea un nuevo nodo 2D */
Nodo* crearNodo(double x, double y) {
    Nodo *nuevo = (Nodo*)malloc(sizeof(Nodo));
    if (nuevo == NULL) {
        printf("Error de asignacion de memoria.\n");
        exit(1);
    }
    nuevo->coords[0] = x;
    nuevo->coords[1] = y;
    nuevo->izq = NULL;
    nuevo->der = NULL;
    return nuevo;
}

/* Inserta un punto alternando eje = profundidad % K */
Nodo* insertar(Nodo *raiz, double x, double y, int profundidad) {
    int eje;
    double punto[K] = {x, y};

    if (raiz == NULL) {
        return crearNodo(x, y);
    }

    eje = profundidad % K;
    if (punto[eje] < raiz->coords[eje]) {
        raiz->izq = insertar(raiz->izq, x, y, profundidad + 1);
    } else {
        raiz->der = insertar(raiz->der, x, y, profundidad + 1);
    }
    return raiz;
}

/* Calcula la distancia euclidiana entre dos puntos 2D */
double distanciaEuclidiana(double a[K], double b[K]) {
    double dx = a[0] - b[0];
    double dy = a[1] - b[1];
    return sqrt(dx * dx + dy * dy);
}

/* Muestra el arbol con su profundidad y eje activo */
void mostrarArbol(Nodo *raiz, int profundidad) {
    int eje;
    if (raiz == NULL) return;

    eje = profundidad % K;
    printf("Nivel %d (eje %c): (%.2f, %.2f)\n",
           profundidad, (eje == 0) ? 'X' : 'Y',
           raiz->coords[0], raiz->coords[1]);

    mostrarArbol(raiz->izq, profundidad + 1);
    mostrarArbol(raiz->der, profundidad + 1);
}

/* Busqueda del vecino mas cercano con traza paso a paso y poda */
void buscarNNS(Nodo *actual, double consulta[K], int profundidad,
               Nodo **mejorNodo, double *mejorDist,
               int *visitados, int *podadas) {
    int eje;
    double distActual, distEje;
    Nodo *ramaCercana, *ramaOpuesta;
    const char *nombreOpuesta;

    if (actual == NULL) return;

    (*visitados)++;
    eje = profundidad % K;
    distActual = distanciaEuclidiana(actual->coords, consulta);

    printf("  [VISITA] Nivel %d (eje %c) -> Nodo (%.2f, %.2f) | dist = %.4f\n",
           profundidad, (eje == 0) ? 'X' : 'Y',
           actual->coords[0], actual->coords[1], distActual);

    if (distActual < *mejorDist) {
        *mejorDist = distActual;
        *mejorNodo = actual;
        printf("  [MEJOR]  Nuevo vecino mas cercano: (%.2f, %.2f) con d_min = %.4f\n",
               actual->coords[0], actual->coords[1], *mejorDist);
    }

    /* Elegir rama cercana y rama opuesta */
    if (consulta[eje] < actual->coords[eje]) {
        ramaCercana = actual->izq;
        ramaOpuesta = actual->der;
        nombreOpuesta = "derecha";
    } else {
        ramaCercana = actual->der;
        ramaOpuesta = actual->izq;
        nombreOpuesta = "izquierda";
    }

    /* 1. Descenso guiado por la rama cercana */
    buscarNNS(ramaCercana, consulta, profundidad + 1,
              mejorNodo, mejorDist, visitados, podadas);

    /* 2. Retroceso: verificar si la hiperesfera cruza el plano de corte */
    distEje = fabs(consulta[eje] - actual->coords[eje]);
    if (distEje < *mejorDist) {
        if (ramaOpuesta != NULL) {
            printf("  [CRUCE]  d_eje (%.4f) < d_min (%.4f) en (%.2f, %.2f) -> Explorando rama %s\n",
                   distEje, *mejorDist, actual->coords[0], actual->coords[1], nombreOpuesta);
        }
        buscarNNS(ramaOpuesta, consulta, profundidad + 1,
                  mejorNodo, mejorDist, visitados, podadas);
    } else {
        if (ramaOpuesta != NULL) {
            (*podadas)++;
            printf("  [PODA]   d_eje (%.4f) >= d_min (%.4f) en (%.2f, %.2f) -> Rama %s podada\n",
                   distEje, *mejorDist, actual->coords[0], actual->coords[1], nombreOpuesta);
        }
    }
}

/* Libera la memoria dinamica del arbol */
void liberarArbol(Nodo *raiz) {
    if (raiz == NULL) return;
    liberarArbol(raiz->izq);
    liberarArbol(raiz->der);
    free(raiz);
}

int main() {
    Nodo *raiz = NULL;
    int opcion, totalNodos = 0;
    double x, y;

    do {
        printf("\n=== MENU ARBOL K-D (2D) - VECINO MAS CERCANO ===\n");
        printf("1. Insertar punto (x, y)\n");
        printf("2. Cargar conjunto de prueba (7 puntos)\n");
        printf("3. Mostrar arbol y ejes de particion\n");
        printf("4. Buscar vecino mas cercano (NNS)\n");
        printf("5. Reiniciar arbol\n");
        printf("0. Salir\n");
        printf("Elige una opcion: ");
        if (scanf("%d", &opcion) != 1) break;

        switch (opcion) {
            case 1:
                printf("Ingrese coordenada x: ");
                scanf("%lf", &x);
                printf("Ingrese coordenada y: ");
                scanf("%lf", &y);
                raiz = insertar(raiz, x, y, 0);
                totalNodos++;
                printf("Punto (%.2f, %.2f) insertado.\n", x, y);
                break;

            case 2:
                liberarArbol(raiz);
                raiz = NULL;
                totalNodos = 0;
                /* Conjunto balanceado de prueba */
                raiz = insertar(raiz, 5.0, 5.0, 0);
                raiz = insertar(raiz, 2.0, 3.0, 0);
                raiz = insertar(raiz, 8.0, 7.0, 0);
                raiz = insertar(raiz, 1.0, 8.0, 0);
                raiz = insertar(raiz, 4.0, 2.0, 0);
                raiz = insertar(raiz, 7.0, 3.0, 0);
                raiz = insertar(raiz, 9.0, 9.0, 0);
                totalNodos = 7;
                printf("Se cargaron 7 puntos de prueba en el arbol.\n");
                break;

            case 3:
                if (raiz == NULL) {
                    printf("El arbol esta vacio.\n");
                } else {
                    printf("\n--- Estructura del Arbol K-D ---\n");
                    mostrarArbol(raiz, 0);
                }
                break;

            case 4:
                if (raiz == NULL) {
                    printf("El arbol esta vacio. Inserte puntos primero.\n");
                } else {
                    double consulta[K];
                    Nodo *mejorNodo = NULL;
                    double mejorDist = DBL_MAX;
                    int visitados = 0, podadas = 0;

                    printf("Ingrese coordenada qx de consulta: ");
                    scanf("%lf", &consulta[0]);
                    printf("Ingrese coordenada qy de consulta: ");
                    scanf("%lf", &consulta[1]);

                    printf("\n--- Traza de Busqueda del Vecino Mas Cercano para Q(%.2f, %.2f) ---\n",
                           consulta[0], consulta[1]);
                    buscarNNS(raiz, consulta, 0, &mejorNodo, &mejorDist, &visitados, &podadas);

                    printf("\n--- Resultado NNS ---\n");
                    printf("Punto de consulta Q : (%.2f, %.2f)\n", consulta[0], consulta[1]);
                    printf("Vecino mas cercano  : (%.2f, %.2f)\n", mejorNodo->coords[0], mejorNodo->coords[1]);
                    printf("Distancia minima    : %.4f\n", mejorDist);
                    printf("Nodos visitados     : %d de %d\n", visitados, totalNodos);
                    printf("Ramas podadas       : %d\n", podadas);
                }
                break;

            case 5:
                liberarArbol(raiz);
                raiz = NULL;
                totalNodos = 0;
                printf("Arbol reiniciado.\n");
                break;

            case 0:
                liberarArbol(raiz);
                printf("Saliendo del programa...\n");
                break;

            default:
                printf("Opcion no valida.\n");
        }
    } while (opcion != 0);

    return 0;
}