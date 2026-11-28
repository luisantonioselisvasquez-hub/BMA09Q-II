#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#define K 2 /* 0: eje x, 1: eje y */

typedef struct NodoKD {
    double coord[K];
    double xmin, xmax;
    double ymin, ymax;
    int profundidad;
    struct NodoKD *izq;
    struct NodoKD *der;
} NodoKD;

NodoKD* crear_nodo(double x, double y, int profundidad,
                   double xmin, double xmax, double ymin, double ymax) {
    NodoKD *nuevo = (NodoKD*)malloc(sizeof(NodoKD));
    if (!nuevo) {
        fprintf(stderr, "Error al asignar memoria.\n");
        exit(EXIT_FAILURE);
    }
    nuevo->coord[0] = x;
    nuevo->coord[1] = y;
    nuevo->profundidad = profundidad;
    nuevo->xmin = xmin;
    nuevo->xmax = xmax;
    nuevo->ymin = ymin;
    nuevo->ymax = ymax;
    nuevo->izq = NULL;
    nuevo->der = NULL;
    return nuevo;
}

/* Inserta respetando el orden elegido por el usuario (el primer punto es la raiz)
   y calcula la region rectangular asignada a cada nodo. */
NodoKD* insertar_con_region(NodoKD *raiz, double x, double y, int profundidad,
                            double xmin, double xmax, double ymin, double ymax) {
    if (raiz == NULL) {
        return crear_nodo(x, y, profundidad, xmin, xmax, ymin, ymax);
    }

    int eje = profundidad % K;
    if (eje == 0) {
        /* Corte en x = raiz->coord[0] */
        if (x < raiz->coord[0]) {
            raiz->izq = insertar_con_region(raiz->izq, x, y, profundidad + 1,
                                            xmin, raiz->coord[0], ymin, ymax);
        } else {
            raiz->der = insertar_con_region(raiz->der, x, y, profundidad + 1,
                                            raiz->coord[0], xmax, ymin, ymax);
        }
    } else {
        /* Corte en y = raiz->coord[1] */
        if (y < raiz->coord[1]) {
            raiz->izq = insertar_con_region(raiz->izq, x, y, profundidad + 1,
                                            xmin, xmax, ymin, raiz->coord[1]);
        } else {
            raiz->der = insertar_con_region(raiz->der, x, y, profundidad + 1,
                                            xmin, xmax, raiz->coord[1], ymax);
        }
    }

    return raiz;
}

void imprimir_cota_x(double val, int es_min) {
    if (es_min && val == -DBL_MAX) {
        printf("-inf");
    } else if (!es_min && val == DBL_MAX) {
        printf("+inf");
    } else {
        printf("%.2f", val);
    }
}

void imprimir_cota_y(double val, int es_min) {
    if (es_min && val == -DBL_MAX) {
        printf("-inf");
    } else if (!es_min && val == DBL_MAX) {
        printf("+inf");
    } else {
        printf("%.2f", val);
    }
}

void imprimir_region(const NodoKD *n) {
    printf("x en [");
    imprimir_cota_x(n->xmin, 1);
    printf(", ");
    imprimir_cota_x(n->xmax, 0);
    printf("], y en [");
    imprimir_cota_y(n->ymin, 1);
    printf(", ");
    imprimir_cota_y(n->ymax, 0);
    printf("]");
}

/* Muestra el arbol y la region asociada a cada nodo */
void mostrar_arbol_y_regiones(const NodoKD *raiz, const char *rol) {
    if (raiz == NULL) {
        return;
    }

    int eje = raiz->profundidad % K;
    for (int i = 0; i < raiz->profundidad; i++) {
        printf("    ");
    }
    printf("[%s | Prof %d | Eje %c] Punto (%.2f, %.2f) -> Region: ",
           rol, raiz->profundidad, (eje == 0 ? 'X' : 'Y'),
           raiz->coord[0], raiz->coord[1]);
    imprimir_region(raiz);
    printf("\n");

    mostrar_arbol_y_regiones(raiz->izq, "Hijo Izq");
    mostrar_arbol_y_regiones(raiz->der, "Hijo Der");
}

int son_iguales(double a, double b) {
    return fabs(a - b) < 1e-9;
}

/* Busca un punto exacto y muestra el recorrido y la region de cada paso */
int buscar_punto_exacto(const NodoKD *raiz, double x, double y, int *visitados) {
    if (raiz == NULL) {
        printf("  -> Llego a NULL: el punto (%.2f, %.2f) NO esta en el arbol.\n", x, y);
        return 0;
    }

    (*visitados)++;
    int eje = raiz->profundidad % K;
    printf("  Paso %d | Prof %d (Eje %c) en nodo (%.2f, %.2f) | Region: ",
           *visitados, raiz->profundidad, (eje == 0 ? 'X' : 'Y'),
           raiz->coord[0], raiz->coord[1]);
    imprimir_region(raiz);
    printf("\n");

    if (son_iguales(raiz->coord[0], x) && son_iguales(raiz->coord[1], y)) {
        printf("  -> COINCIDENCIA EXACTA en (%.2f, %.2f)!\n", x, y);
        return 1;
    }

    if (eje == 0) {
        if (x < raiz->coord[0]) {
            printf("     Como x (%.2f) < %.2f, baja a Hijo Izquierdo.\n", x, raiz->coord[0]);
            return buscar_punto_exacto(raiz->izq, x, y, visitados);
        } else {
            printf("     Como x (%.2f) >= %.2f, baja a Hijo Derecho.\n", x, raiz->coord[0]);
            return buscar_punto_exacto(raiz->der, x, y, visitados);
        }
    } else {
        if (y < raiz->coord[1]) {
            printf("     Como y (%.2f) < %.2f, baja a Hijo Izquierdo.\n", y, raiz->coord[1]);
            return buscar_punto_exacto(raiz->izq, x, y, visitados);
        } else {
            printf("     Como y (%.2f) >= %.2f, baja a Hijo Derecho.\n", y, raiz->coord[1]);
            return buscar_punto_exacto(raiz->der, x, y, visitados);
        }
    }
}

int dentro_rango(const NodoKD *n, double xmin, double xmax, double ymin, double ymax) {
    return (n->coord[0] >= xmin && n->coord[0] <= xmax) &&
           (n->coord[1] >= ymin && n->coord[1] <= ymax);
}

void consulta_rango(const NodoKD *raiz,
                    double xmin, double xmax, double ymin, double ymax,
                    int *visitados, int *podados, int *reportados) {
    if (raiz == NULL) {
        return;
    }

    (*visitados)++;
    if (dentro_rango(raiz, xmin, xmax, ymin, ymax)) {
        printf("  - En rango: (%.2f, %.2f) [Prof %d] | Region del nodo: ",
               raiz->coord[0], raiz->coord[1], raiz->profundidad);
        imprimir_region(raiz);
        printf("\n");
        (*reportados)++;
    }

    int eje = raiz->profundidad % K;
    double rmin = (eje == 0) ? xmin : ymin;
    double rmax = (eje == 0) ? xmax : ymax;
    double pivote = raiz->coord[eje];

    if (rmin < pivote) {
        consulta_rango(raiz->izq, xmin, xmax, ymin, ymax, visitados, podados, reportados);
    } else if (raiz->izq != NULL) {
        (*podados)++;
    }

    if (rmax >= pivote) {
        consulta_rango(raiz->der, xmin, xmax, ymin, ymax, visitados, podados, reportados);
    } else if (raiz->der != NULL) {
        (*podados)++;
    }
}

void liberar(NodoKD *raiz) {
    if (raiz == NULL) {
        return;
    }
    liberar(raiz->izq);
    liberar(raiz->der);
    free(raiz);
}

void limpiar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

int main(void) {
    NodoKD *raiz = NULL;
    int opcion = 0;

    printf("=== K-D TREE 2D INTERACTIVO (REGIONES Y BUSQUEDAS) ===\n");
    printf("Nota: El primer punto que insertes sera la RAIZ (Prof 0, Eje X).\n");
    printf("Los siguientes se ubican como hijos/descendientes segun el eje del nivel.\n\n");

    do {
        printf("\nMenu:\n");
        printf("1. Insertar punto (elegir raiz/hijos en orden)\n");
        printf("2. Mostrar arbol y region de cada punto\n");
        printf("3. Buscar un punto exacto\n");
        printf("4. Consultar puntos en un rango [xmin, xmax] x [ymin, ymax]\n");
        printf("5. Reiniciar arbol\n");
        printf("0. Salir\n");
        printf("Elige una opcion: ");
        if (scanf("%d", &opcion) != 1) {
            limpiar_buffer();
            opcion = -1;
        }

        if (opcion == 1) {
            double x, y;
            printf("Ingresa coordenada x: ");
            scanf("%lf", &x);
            printf("Ingresa coordenada y: ");
            scanf("%lf", &y);
            raiz = insertar_con_region(raiz, x, y, 0,
                                       -DBL_MAX, DBL_MAX, -DBL_MAX, DBL_MAX);
            printf("Punto (%.2f, %.2f) insertado.\n", x, y);
        } else if (opcion == 2) {
            if (raiz == NULL) {
                printf("El arbol esta vacio.\n");
            } else {
                printf("\n--- Estructura y regiones del arbol ---\n");
                mostrar_arbol_y_regiones(raiz, "Raiz");
            }
        } else if (opcion == 3) {
            if (raiz == NULL) {
                printf("El arbol esta vacio.\n");
            } else {
                double bx, by;
                int visitados = 0;
                printf("Punto exacto a buscar (x): ");
                scanf("%lf", &bx);
                printf("Punto exacto a buscar (y): ");
                scanf("%lf", &by);
                printf("\n--- Recorrido de busqueda exacta ---\n");
                buscar_punto_exacto(raiz, bx, by, &visitados);
                printf("Total de nodos visitados: %d\n", visitados);
            }
        } else if (opcion == 4) {
            if (raiz == NULL) {
                printf("El arbol esta vacio.\n");
            } else {
                double xmin, xmax, ymin, ymax;
                int visitados = 0, podados = 0, reportados = 0;
                printf("xmin: "); scanf("%lf", &xmin);
                printf("xmax: "); scanf("%lf", &xmax);
                printf("ymin: "); scanf("%lf", &ymin);
                printf("ymax: "); scanf("%lf", &ymax);
                if (xmin > xmax || ymin > ymax) {
                    printf("Rango invalido: asegurate de que xmin <= xmax y ymin <= ymax.\n");
                } else {
                    printf("\n--- Resultados de consulta de rango ---\n");
                    consulta_rango(raiz, xmin, xmax, ymin, ymax,
                                   &visitados, &podados, &reportados);
                    printf("Encontrados: %d | Visitados: %d | Ramas podadas: %d\n",
                           reportados, visitados, podados);
                }
            }
        } else if (opcion == 5) {
            liberar(raiz);
            raiz = NULL;
            printf("Arbol reiniciado.\n");
        }

    } while (opcion != 0);

    liberar(raiz);
    return 0;
}