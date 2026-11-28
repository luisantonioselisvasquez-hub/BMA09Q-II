
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CAPACIDAD 4
#define MAX_PROF 12

/* =====================================
       ESTRUCTURA DE LOS PUNTOS
===================================== */

typedef struct {
    char nombre[30];
    double x, y;
} Punto;

/* =====================================
          ESTRUCTURA QUADTREE
===================================== */

typedef struct Nodo {
    double xmin, xmax, ymin, ymax;

    Punto *puntos;
    int cantidad;
    int capacidad;

    struct Nodo *hijo[4];

} Nodo;

/* VARIABLES GLOBALES */

Nodo *raiz = NULL;

double XMIN, XMAX, YMIN, YMAX;

int limitesDefinidos = 0;

/* =====================================
          CREAR NUEVO NODO
===================================== */

Nodo *crearNodo(double a, double b,
                double c, double d) {

    Nodo *n = (Nodo *)malloc(sizeof(Nodo));

    if (n == NULL) {
        printf("Error al reservar memoria.\n");
        exit(1);
    }

    n->xmin = a;
    n->xmax = b;
    n->ymin = c;
    n->ymax = d;

    n->cantidad = 0;
    n->capacidad = CAPACIDAD;

    n->puntos = (Punto *)malloc(
        CAPACIDAD * sizeof(Punto)
    );

    if (n->puntos == NULL) {
        printf("Error de memoria.\n");
        free(n);
        exit(1);
    }

    for (int i = 0; i < 4; i++) {
        n->hijo[i] = NULL;
    }

    return n;
}

/* =====================================
             IDENTIFICAR HOJA
===================================== */

int esHoja(Nodo *n) {
    return n->hijo[0] == NULL;
}

/* =====================================
          IDENTIFICAR CUADRANTE
===================================== */

int cuadrante(Nodo *n, double x, double y) {

    double xm = (n->xmin + n->xmax) / 2.0;
    double ym = (n->ymin + n->ymax) / 2.0;

    if (y >= ym) {
        return (x < xm) ? 0 : 1;
    }

    return (x < xm) ? 2 : 3;
}

/* =====================================
           INSERTAR EN UNA HOJA
===================================== */

void guardarPunto(Nodo *n, Punto p) {

    if (n->cantidad >= n->capacidad) {

        int nuevaCapacidad = n->capacidad * 2;

        Punto *temporal = (Punto *)realloc(
            n->puntos,
            nuevaCapacidad * sizeof(Punto)
        );

        if (temporal == NULL) {
            printf("Error al ampliar memoria.\n");
            exit(1);
        }

        n->puntos = temporal;
        n->capacidad = nuevaCapacidad;
    }

    n->puntos[n->cantidad] = p;
    n->cantidad++;
}

/* =====================================
           SUBDIVIDIR EN 4 REGIONES
===================================== */

void subdividir(Nodo *n) {

    double xm = (n->xmin + n->xmax) / 2.0;
    double ym = (n->ymin + n->ymax) / 2.0;

    n->hijo[0] = crearNodo(
        n->xmin, xm, ym, n->ymax
    );

    n->hijo[1] = crearNodo(
        xm, n->xmax, ym, n->ymax
    );

    n->hijo[2] = crearNodo(
        n->xmin, xm, n->ymin, ym
    );

    n->hijo[3] = crearNodo(
        xm, n->xmax, n->ymin, ym
    );
}

/* =====================================
          INSERTAR PUNTO
===================================== */

int insertar(Nodo *n, Punto p, int profundidad) {

    if (p.x < n->xmin || p.x > n->xmax ||
        p.y < n->ymin || p.y > n->ymax) {

        return 0;
    }

    /* Si no es hoja, buscar su cuadrante */

    if (!esHoja(n)) {

        int q = cuadrante(n, p.x, p.y);

        return insertar(
            n->hijo[q], p, profundidad + 1
        );
    }

    /* Guardar si hay espacio o se alcanzo
       la profundidad maxima */

    if (n->cantidad < CAPACIDAD ||
        profundidad >= MAX_PROF) {

        guardarPunto(n, p);

        return 1;
    }

    /* Guardar temporalmente los puntos */

    int cantidadAnterior = n->cantidad;

    Punto *anteriores = (Punto *)malloc(
        cantidadAnterior * sizeof(Punto)
    );

    if (anteriores == NULL) {
        printf("Error de memoria.\n");
        exit(1);
    }

    for (int i = 0; i < cantidadAnterior; i++) {
        anteriores[i] = n->puntos[i];
    }

    /* Crear los cuatro cuadrantes */

    subdividir(n);

    n->cantidad = 0;

    /* Redistribuir los puntos anteriores */

    for (int i = 0; i < cantidadAnterior; i++) {

        int q = cuadrante(
            n,
            anteriores[i].x,
            anteriores[i].y
        );

        insertar(
            n->hijo[q],
            anteriores[i],
            profundidad + 1
        );
    }

    free(anteriores);

    /* Insertar el nuevo punto */

    int q = cuadrante(n, p.x, p.y);

    return insertar(
        n->hijo[q], p, profundidad + 1
    );
}

/* =====================================
         MOSTRAR ARBOL QUADTREE
===================================== */

void mostrarArbol(Nodo *n, int nivel) {

    for (int i = 0; i < nivel; i++) {
        printf("  ");
    }

    printf("REGION [%.2f, %.2f] x [%.2f, %.2f]\n",
           n->xmin, n->xmax,
           n->ymin, n->ymax);

    if (esHoja(n)) {

        for (int i = 0; i < n->cantidad; i++) {

            for (int j = 0; j <= nivel; j++) {
                printf("  ");
            }

            printf("Punto: %s (%.2f, %.2f)\n",
                   n->puntos[i].nombre,
                   n->puntos[i].x,
                   n->puntos[i].y);
        }

        if (n->cantidad == 0) {

            for (int j = 0; j <= nivel; j++) {
                printf("  ");
            }

            printf("(REGION VACIA)\n");
        }

    } else {

        char *nombres[] = {
            "NOROESTE",
            "NORESTE",
            "SUROESTE",
            "SURESTE"
        };

        for (int i = 0; i < 4; i++) {

            for (int j = 0; j <= nivel; j++) {
                printf("  ");
            }

            printf("[%s]\n", nombres[i]);

            mostrarArbol(n->hijo[i], nivel + 2);
        }
    }
}

/* =====================================
       MOSTRAR SUBDIVISIONES
===================================== */

void mostrarSubdivisiones(Nodo *n, int nivel) {

    if (esHoja(n)) {
        return;
    }

    for (int i = 0; i < nivel; i++) {
        printf("  ");
    }

    printf("ESPACIO SUBDIVIDIDO:\n");

    char *nombres[] = {
        "NOROESTE",
        "NORESTE",
        "SUROESTE",
        "SURESTE"
    };

    for (int i = 0; i < 4; i++) {

        for (int j = 0; j < nivel; j++) {
            printf("  ");
        }

        printf("-> %s\n", nombres[i]);

        mostrarSubdivisiones(
            n->hijo[i], nivel + 2
        );
    }
}

/* =====================================
            BUSCAR PUNTO
===================================== */

int buscar(Nodo *n, double x, double y) {

    if (x < n->xmin || x > n->xmax ||
        y < n->ymin || y > n->ymax) {

        return 0;
    }

    if (!esHoja(n)) {

        int q = cuadrante(n, x, y);

        return buscar(n->hijo[q], x, y);
    }

    for (int i = 0; i < n->cantidad; i++) {

        if (fabs(n->puntos[i].x - x) < 0.000001 &&
            fabs(n->puntos[i].y - y) < 0.000001) {

            printf("\nPUNTO ENCONTRADO\n");

            printf("Nombre: %s\n",
                   n->puntos[i].nombre);

            printf("Coordenadas: (%.2f, %.2f)\n",
                   n->puntos[i].x,
                   n->puntos[i].y);

            return 1;
        }
    }

    return 0;
}

/* =====================================
        COMPROBAR INTERSECCION
===================================== */

int intersecta(Nodo *n, double a, double b,
                double c, double d) {

    return !(n->xmax < a ||
             n->xmin > b ||
             n->ymax < c ||
             n->ymin > d);
}

/* =====================================
         CONSULTA ESPACIAL
===================================== */

void consultaRango(Nodo *n,
                   double a, double b,
                   double c, double d,
                   int *total) {

    if (!intersecta(n, a, b, c, d)) {
        return;
    }

    if (esHoja(n)) {

        for (int i = 0; i < n->cantidad; i++) {

            Punto p = n->puntos[i];

            if (p.x >= a && p.x <= b &&
                p.y >= c && p.y <= d) {

                printf("%s -> (%.2f, %.2f)\n",
                       p.nombre, p.x, p.y);

                (*total)++;
            }
        }

    } else {

        for (int i = 0; i < 4; i++) {

            consultaRango(
                n->hijo[i],
                a, b, c, d,
                total
            );
        }
    }
}

/* =====================================
           LIBERAR MEMORIA
===================================== */

void liberar(Nodo *n) {

    if (n == NULL) {
        return;
    }

    for (int i = 0; i < 4; i++) {
        liberar(n->hijo[i]);
    }

    free(n->puntos);
    free(n);
}

/* =====================================
          REINICIAR ARBOL
===================================== */

void reiniciar() {

    liberar(raiz);

    raiz = crearNodo(
        XMIN, XMAX, YMIN, YMAX
    );

    printf("\nArbol reiniciado correctamente.\n");
    printf("Se conservaron los limites del espacio.\n");
}

/* =====================================
       CARGAR PUNTOS DE PRUEBA
===================================== */

void cargarPrueba() {

    Punto datos[] = {
        {"A", 20, 30},
        {"B", 70, 80},
        {"C", 40, 60},
        {"D", 90, 10},
        {"E", 15, 85},
        {"F", 60, 25},
        {"G", 85, 50}
    };

    int cantidad = sizeof(datos) / sizeof(datos[0]);

    int insertados = 0;

    for (int i = 0; i < cantidad; i++) {

        if (insertar(raiz, datos[i], 0)) {

            insertados++;

        } else {

            printf("Punto %s fuera del espacio.\n",
                   datos[i].nombre);
        }
    }

    printf("\nSe insertaron %d de %d puntos.\n",
           insertados, cantidad);
}

/* =====================================
       DEFINIR LIMITES DEL ESPACIO
===================================== */

void definirLimites() {

    double a, b, c, d;

    printf("\n===== DEFINIR ESPACIO 2D =====\n");

    printf("Limite minimo de X: ");
    scanf("%lf", &a);

    printf("Limite maximo de X: ");
    scanf("%lf", &b);

    printf("Limite minimo de Y: ");
    scanf("%lf", &c);

    printf("Limite maximo de Y: ");
    scanf("%lf", &d);

    if (a >= b || c >= d) {

        printf("Error: los limites no son validos.\n");

        return;
    }

    XMIN = a;
    XMAX = b;
    YMIN = c;
    YMAX = d;

    liberar(raiz);

    raiz = crearNodo(
        XMIN, XMAX, YMIN, YMAX
    );

    limitesDefinidos = 1;

    printf("\nEspacio bidimensional definido correctamente.\n");

    printf("X: %.2f hasta %.2f\n", XMIN, XMAX);
    printf("Y: %.2f hasta %.2f\n", YMIN, YMAX);

    printf("El arbol esta listo para recibir puntos.\n");
}

/* =====================================
          INSERTAR MANUALMENTE
===================================== */

void insertarManual() {

    Punto p;

    printf("\n===== INSERTAR PUNTO =====\n");

    printf("Identificador del punto: ");
    scanf("%29s", p.nombre);

    printf("Coordenada X: ");
    scanf("%lf", &p.x);

    printf("Coordenada Y: ");
    scanf("%lf", &p.y);

    if (insertar(raiz, p, 0)) {

        printf("\nPunto insertado correctamente.\n");

    } else {

        printf("\nNo se pudo insertar el punto.\n");
        printf("Verifique que este dentro del espacio definido.\n");
    }
}

/* =====================================
             BUSQUEDA MANUAL
===================================== */

void buscarManual() {

    double x, y;

    printf("\n===== BUSCAR PUNTO =====\n");

    printf("Ingrese coordenada X: ");
    scanf("%lf", &x);

    printf("Ingrese coordenada Y: ");
    scanf("%lf", &y);

    if (!buscar(raiz, x, y)) {

        printf("\nNo se encontro ningun punto en esas coordenadas.\n");
    }
}

/* =====================================
          CONSULTA POR RANGO
===================================== */

void consultaManual() {

    double a, b, c, d;

    int total = 0;

    printf("\n===== CONSULTA ESPACIAL =====\n");

    printf("X minimo: ");
    scanf("%lf", &a);

    printf("X maximo: ");
    scanf("%lf", &b);

    printf("Y minimo: ");
    scanf("%lf", &c);

    printf("Y maximo: ");
    scanf("%lf", &d);

    if (a > b || c > d) {

        printf("Error: rango incorrecto.\n");

        return;
    }

    printf("\nPUNTOS ENCONTRADOS EN LA REGION:\n");

    consultaRango(
        raiz, a, b, c, d, &total
    );

    printf("\nTotal de puntos encontrados: %d\n",
           total);
}

/* =====================================
             MENU PRINCIPAL
===================================== */

int main() {

    int opcion;

    raiz = NULL;

    do {

        printf("\n");
        printf("=====================================\n");
        printf("       SISTEMA DE ARBOL QUADTREE\n");
        printf("=====================================\n");

        printf("1. Definir limites del espacio 2D\n");
        printf("2. Insertar punto (x, y)\n");
        printf("3. Cargar conjunto de prueba (7 puntos)\n");
        printf("4. Mostrar arbol QUADTREE\n");
        printf("5. Mostrar subdivisiones en cuatro regiones\n");
        printf("6. Buscar punto por coordenadas\n");
        printf("7. Consulta espacial por rango\n");
        printf("8. Reiniciar arbol\n");
        printf("0. Salir\n");

        printf("\nSeleccione una opcion: ");

        if (scanf("%d", &opcion) != 1) {

            printf("Entrada no valida.\n");

            while (getchar() != '\n');

            continue;
        }

        switch (opcion) {

        case 1:

            definirLimites();

            break;

        case 2:

            if (!limitesDefinidos) {

                printf("\nPrimero debe definir los limites (opcion 1).\n");

                break;
            }

            insertarManual();

            break;

        case 3:

            if (!limitesDefinidos) {

                printf("\nPrimero debe definir los limites (opcion 1).\n");

                break;
            }

            cargarPrueba();

            break;

        case 4:

            if (!limitesDefinidos) {

                printf("\nPrimero debe definir los limites (opcion 1).\n");

                break;
            }

            printf("\n===== ESTRUCTURA DEL QUADTREE =====\n");

            mostrarArbol(raiz, 0);

            break;

        case 5:

            if (!limitesDefinidos) {

                printf("\nPrimero debe definir los limites (opcion 1).\n");

                break;
            }

            printf("\n===== SUBDIVISIONES DEL QUADTREE =====\n");

            mostrarSubdivisiones(raiz, 0);

            break;

        case 6:

            if (!limitesDefinidos) {

                printf("\nPrimero debe definir los limites (opcion 1).\n");

                break;
            }

            buscarManual();

            break;

        case 7:

            if (!limitesDefinidos) {

                printf("\nPrimero debe definir los limites (opcion 1).\n");

                break;
            }

            consultaManual();

            break;

        case 8:

            if (!limitesDefinidos) {

                printf("\nPrimero debe definir los limites (opcion 1).\n");

                break;
            }

            reiniciar();

            break;

        case 0:

            printf("\nFinalizando programa...\n");

            break;

        default:

            printf("\nOpcion incorrecta. Intente nuevamente.\n");
        }

    } while (opcion != 0);

    liberar(raiz);

    return 0;
}

