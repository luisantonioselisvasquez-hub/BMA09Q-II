#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CAPACIDAD 4
#define MAX_PROF 12
#define MAX_REGISTROS 500

typedef struct {
    char codigo[20];
    char nombre[40];
    char tipo[30];
    double x;
    double y;
} Establecimiento;

/* ---------------- QUADTREE ---------------- */

typedef struct NodoQ {
    double xmin, xmax, ymin, ymax;
    Establecimiento *datos;
    int cantidad;
    int capacidad;
    struct NodoQ *hijo[4];
} NodoQ;

/* ---------------- ARBOL K-D ---------------- */

typedef struct NodoKD {
    Establecimiento dato;
    int eje;                 /* 0 = X, 1 = Y */
    struct NodoKD *izq;
    struct NodoKD *der;
} NodoKD;

/* ---------------- VARIABLES GLOBALES ---------------- */

NodoQ *mapa = NULL;
NodoKD *raizKD = NULL;

double xmin, xmax, ymin, ymax;
int limitesDefinidos = 0;

Establecimiento registros[MAX_REGISTROS];
int totalRegistros = 0;

/* ---------------- UTILIDADES ---------------- */

void limpiarEntrada(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* descartar caracteres sobrantes */
    }
}

int codigoExiste(const char *codigo)
{
    int i;
    for (i = 0; i < totalRegistros; i++) {
        if (strcmp(registros[i].codigo, codigo) == 0)
            return 1;
    }
    return 0;
}

/* ---------------- QUADTREE: CREACION ---------------- */

NodoQ *crearNodoQ(double a, double b, double c, double d)
{
    NodoQ *n;
    int i;

    n = (NodoQ *)malloc(sizeof(NodoQ));
    if (n == NULL) {
        printf("Error de memoria al crear Quadtree.\n");
        exit(1);
    }

    n->xmin = a;
    n->xmax = b;
    n->ymin = c;
    n->ymax = d;
    n->cantidad = 0;
    n->capacidad = CAPACIDAD;
    n->datos = (Establecimiento *)malloc(
        CAPACIDAD * sizeof(Establecimiento));

    if (n->datos == NULL) {
        free(n);
        printf("Error de memoria.\n");
        exit(1);
    }

    for (i = 0; i < 4; i++)
        n->hijo[i] = NULL;

    return n;
}

int esHojaQ(NodoQ *n)
{
    return n->hijo[0] == NULL;
}

int cuadranteQ(NodoQ *n, double x, double y)
{
    double xm, ym;

    xm = (n->xmin + n->xmax) / 2.0;
    ym = (n->ymin + n->ymax) / 2.0;

    if (y >= ym)
        return (x < xm) ? 0 : 1;

    return (x < xm) ? 2 : 3;
}

void guardarQ(NodoQ *n, Establecimiento e)
{
    Establecimiento *temp;
    int nueva;

    if (n->cantidad >= n->capacidad) {
        nueva = n->capacidad * 2;
        temp = (Establecimiento *)realloc(
            n->datos, nueva * sizeof(Establecimiento));

        if (temp == NULL) {
            printf("Error de memoria.\n");
            exit(1);
        }

        n->datos = temp;
        n->capacidad = nueva;
    }

    n->datos[n->cantidad] = e;
    n->cantidad++;
}

void subdividirQ(NodoQ *n)
{
    double xm, ym;

    xm = (n->xmin + n->xmax) / 2.0;
    ym = (n->ymin + n->ymax) / 2.0;

    n->hijo[0] = crearNodoQ(n->xmin, xm, ym, n->ymax);
    n->hijo[1] = crearNodoQ(xm, n->xmax, ym, n->ymax);
    n->hijo[2] = crearNodoQ(n->xmin, xm, n->ymin, ym);
    n->hijo[3] = crearNodoQ(xm, n->xmax, n->ymin, ym);
}

int insertarQ(NodoQ *n, Establecimiento e, int prof)
{
    int i, q, cantidadAnterior;
    Establecimiento *anteriores;

    if (e.x < n->xmin || e.x > n->xmax ||
        e.y < n->ymin || e.y > n->ymax)
        return 0;

    if (!esHojaQ(n)) {
        q = cuadranteQ(n, e.x, e.y);
        return insertarQ(n->hijo[q], e, prof + 1);
    }

    if (n->cantidad < CAPACIDAD || prof >= MAX_PROF) {
        guardarQ(n, e);
        return 1;
    }

    cantidadAnterior = n->cantidad;
    anteriores = (Establecimiento *)malloc(
        cantidadAnterior * sizeof(Establecimiento));

    if (anteriores == NULL) {
        printf("Error de memoria.\n");
        exit(1);
    }

    for (i = 0; i < cantidadAnterior; i++)
        anteriores[i] = n->datos[i];

    subdividirQ(n);
    n->cantidad = 0;

    for (i = 0; i < cantidadAnterior; i++) {
        q = cuadranteQ(n, anteriores[i].x, anteriores[i].y);
        insertarQ(n->hijo[q], anteriores[i], prof + 1);
    }

    free(anteriores);

    q = cuadranteQ(n, e.x, e.y);
    return insertarQ(n->hijo[q], e, prof + 1);
}

void mostrarEstablecimientosQ(NodoQ *n)
{
    int i;

    if (n == NULL)
        return;

    if (esHojaQ(n)) {
        for (i = 0; i < n->cantidad; i++) {
            printf("\nCodigo: %s\n", n->datos[i].codigo);
            printf("Nombre: %s\n", n->datos[i].nombre);
            printf("Tipo: %s\n", n->datos[i].tipo);
            printf("Coordenadas: (%.2f, %.2f)\n",
                   n->datos[i].x, n->datos[i].y);
        }
    } else {
        for (i = 0; i < 4; i++)
            mostrarEstablecimientosQ(n->hijo[i]);
    }
}

void mostrarArbolQ(NodoQ *n, int nivel)
{
    int i, j;
    char *nombres[4];

    nombres[0] = "NOROESTE";
    nombres[1] = "NORESTE";
    nombres[2] = "SUROESTE";
    nombres[3] = "SURESTE";

    if (n == NULL)
        return;

    for (i = 0; i < nivel; i++)
        printf("  ");

    printf("REGION [%.2f, %.2f] x [%.2f, %.2f]\n",
           n->xmin, n->xmax, n->ymin, n->ymax);

    if (esHojaQ(n)) {
        for (i = 0; i < n->cantidad; i++) {
            for (j = 0; j <= nivel; j++)
                printf("  ");
            printf("%s: %s (%.2f, %.2f)\n",
                   n->datos[i].codigo, n->datos[i].nombre,
                   n->datos[i].x, n->datos[i].y);
        }
    } else {
        for (i = 0; i < 4; i++) {
            for (j = 0; j <= nivel; j++)
                printf("  ");
            printf("[%s]\n", nombres[i]);
            mostrarArbolQ(n->hijo[i], nivel + 2);
        }
    }
}

int buscarCodigoQ(NodoQ *n, const char *codigo)
{
    int i;

    if (n == NULL)
        return 0;

    if (esHojaQ(n)) {
        for (i = 0; i < n->cantidad; i++) {
            if (strcmp(n->datos[i].codigo, codigo) == 0) {
                printf("\nESTABLECIMIENTO ENCONTRADO (QUADTREE)\n");
                printf("Codigo: %s\n", n->datos[i].codigo);
                printf("Nombre: %s\n", n->datos[i].nombre);
                printf("Tipo: %s\n", n->datos[i].tipo);
                printf("Ubicacion: (%.2f, %.2f)\n",
                       n->datos[i].x, n->datos[i].y);
                return 1;
            }
        }
        return 0;
    }

    for (i = 0; i < 4; i++) {
        if (buscarCodigoQ(n->hijo[i], codigo))
            return 1;
    }
    return 0;
}

int intersectaQ(NodoQ *n, double a, double b, double c, double d)
{
    return !(n->xmax < a || n->xmin > b ||
             n->ymax < c || n->ymin > d);
}

void consultarZonaQ(NodoQ *n, double a, double b,
                    double c, double d, int *total)
{
    int i;
    Establecimiento e;

    if (n == NULL || !intersectaQ(n, a, b, c, d))
        return;

    if (esHojaQ(n)) {
        for (i = 0; i < n->cantidad; i++) {
            e = n->datos[i];
            if (e.x >= a && e.x <= b && e.y >= c && e.y <= d) {
                printf("\n%s - %s\n", e.codigo, e.nombre);
                printf("Tipo: %s\n", e.tipo);
                printf("Ubicacion: (%.2f, %.2f)\n", e.x, e.y);
                (*total)++;
            }
        }
    } else {
        for (i = 0; i < 4; i++)
            consultarZonaQ(n->hijo[i], a, b, c, d, total);
    }
}

void buscarCercanoQ(NodoQ *n, double x, double y,
                    Establecimiento *mejor, double *distMin)
{
    int i;
    double dx, dy, distancia;

    if (n == NULL)
        return;

    if (esHojaQ(n)) {
        for (i = 0; i < n->cantidad; i++) {
            dx = n->datos[i].x - x;
            dy = n->datos[i].y - y;
            distancia = sqrt(dx * dx + dy * dy);

            if (distancia < *distMin) {
                *distMin = distancia;
                *mejor = n->datos[i];
            }
        }
    } else {
        for (i = 0; i < 4; i++)
            buscarCercanoQ(n->hijo[i], x, y, mejor, distMin);
    }
}

void liberarQ(NodoQ *n)
{
    int i;

    if (n == NULL)
        return;

    for (i = 0; i < 4; i++)
        liberarQ(n->hijo[i]);

    free(n->datos);
    free(n);
}

/* ---------------- ARBOL K-D ---------------- */

NodoKD *crearNodoKD(Establecimiento e, int eje)
{
    NodoKD *n;

    n = (NodoKD *)malloc(sizeof(NodoKD));
    if (n == NULL) {
        printf("Error de memoria al crear arbol k-d.\n");
        exit(1);
    }

    n->dato = e;
    n->eje = eje;
    n->izq = NULL;
    n->der = NULL;
    return n;
}

NodoKD *insertarKD(NodoKD *raiz, Establecimiento e, int eje)
{
    double valorNuevo, valorRaiz;

    if (raiz == NULL)
        return crearNodoKD(e, eje);

    if (eje == 0) {
        valorNuevo = e.x;
        valorRaiz = raiz->dato.x;
    } else {
        valorNuevo = e.y;
        valorRaiz = raiz->dato.y;
    }

    if (valorNuevo < valorRaiz)
        raiz->izq = insertarKD(raiz->izq, e, 1 - eje);
    else
        raiz->der = insertarKD(raiz->der, e, 1 - eje);

    return raiz;
}

void mostrarArbolKD(NodoKD *n, int nivel)
{
    int i;

    if (n == NULL)
        return;

    mostrarArbolKD(n->der, nivel + 1);

    for (i = 0; i < nivel; i++)
        printf("    ");

    printf("%s - %s (%.2f, %.2f) [eje %c]\n",
           n->dato.codigo, n->dato.nombre,
           n->dato.x, n->dato.y,
           (n->eje == 0) ? 'X' : 'Y');

    mostrarArbolKD(n->izq, nivel + 1);
}

void liberarKD(NodoKD *n)
{
    if (n == NULL)
        return;

    liberarKD(n->izq);
    liberarKD(n->der);
    free(n);
}

void reconstruirKD(void)
{
    int i;

    liberarKD(raizKD);
    raizKD = NULL;

    for (i = 0; i < totalRegistros; i++)
        raizKD = insertarKD(raizKD, registros[i], 0);
}

int buscarCodigoKD(NodoKD *n, const char *codigo)
{
    if (n == NULL)
        return 0;

    if (strcmp(n->dato.codigo, codigo) == 0) {
        printf("\nESTABLECIMIENTO ENCONTRADO (ARBOL K-D)\n");
        printf("Codigo: %s\n", n->dato.codigo);
        printf("Nombre: %s\n", n->dato.nombre);
        printf("Tipo: %s\n", n->dato.tipo);
        printf("Ubicacion: (%.2f, %.2f)\n",
               n->dato.x, n->dato.y);
        return 1;
    }

    if (buscarCodigoKD(n->izq, codigo))
        return 1;

    return buscarCodigoKD(n->der, codigo);
}

void buscarCercanoKD(NodoKD *n, double x, double y,
                     Establecimiento *mejor, double *distMin)
{
    double dx, dy, distancia, diferencia;
    NodoKD *cerca;
    NodoKD *lejos;

    if (n == NULL)
        return;

    dx = n->dato.x - x;
    dy = n->dato.y - y;
    distancia = sqrt(dx * dx + dy * dy);

    if (distancia < *distMin) {
        *distMin = distancia;
        *mejor = n->dato;
    }

    if (n->eje == 0) {
        diferencia = x - n->dato.x;
    } else {
        diferencia = y - n->dato.y;
    }

    if (diferencia < 0) {
        cerca = n->izq;
        lejos = n->der;
    } else {
        cerca = n->der;
        lejos = n->izq;
    }

    buscarCercanoKD(cerca, x, y, mejor, distMin);

    /* Explora el otro lado solo si podria contener un punto mas cercano. */
    if (fabs(diferencia) < *distMin)
        buscarCercanoKD(lejos, x, y, mejor, distMin);
}

void consultarRangoKD(NodoKD *n, double a, double b,
                      double c, double d, int *total)
{
    double valorCorte, minimo, maximo;

    if (n == NULL)
        return;

    if (n->dato.x >= a && n->dato.x <= b &&
        n->dato.y >= c && n->dato.y <= d) {
        printf("\n%s - %s\n", n->dato.codigo, n->dato.nombre);
        printf("Tipo: %s\n", n->dato.tipo);
        printf("Ubicacion: (%.2f, %.2f)\n",
               n->dato.x, n->dato.y);
        (*total)++;
    }

    if (n->eje == 0) {
        valorCorte = n->dato.x;
        minimo = a;
        maximo = b;
    } else {
        valorCorte = n->dato.y;
        minimo = c;
        maximo = d;
    }

    if (minimo <= valorCorte)
        consultarRangoKD(n->izq, a, b, c, d, total);

    if (maximo >= valorCorte)
        consultarRangoKD(n->der, a, b, c, d, total);
}

/* ---------------- GESTION DE REGISTROS ---------------- */

int agregarEstablecimiento(Establecimiento e)
{
    if (!limitesDefinidos) {
        printf("Primero defina los limites del mapa.\n");
        return 0;
    }

    if (totalRegistros >= MAX_REGISTROS) {
        printf("Se alcanzo el maximo de %d registros.\n", MAX_REGISTROS);
        return 0;
    }

    if (codigoExiste(e.codigo)) {
        printf("Ya existe un establecimiento con ese codigo.\n");
        return 0;
    }

    if (e.x < xmin || e.x > xmax || e.y < ymin || e.y > ymax) {
        printf("Las coordenadas estan fuera de los limites del mapa.\n");
        return 0;
    }

    if (!insertarQ(mapa, e, 0)) {
        printf("No fue posible insertar el punto en el Quadtree.\n");
        return 0;
    }

    registros[totalRegistros] = e;
    totalRegistros++;

    /* Mantiene el arbol k-d sincronizado con los mismos registros. */
    reconstruirKD();

    return 1;
}

void cargarEjemplo(void)
{
    Establecimiento datos[7];
    int i, insertados;

    strcpy(datos[0].codigo, "H01");
    strcpy(datos[0].nombre, "Hospital Central");
    strcpy(datos[0].tipo, "Hospital");
    datos[0].x = 20; datos[0].y = 30;

    strcpy(datos[1].codigo, "C01");
    strcpy(datos[1].nombre, "Colegio Nacional");
    strcpy(datos[1].tipo, "Colegio");
    datos[1].x = 40; datos[1].y = 60;

    strcpy(datos[2].codigo, "F01");
    strcpy(datos[2].nombre, "Farmacia Salud");
    strcpy(datos[2].tipo, "Farmacia");
    datos[2].x = 70; datos[2].y = 80;

    strcpy(datos[3].codigo, "P01");
    strcpy(datos[3].nombre, "Parque Principal");
    strcpy(datos[3].tipo, "Parque");
    datos[3].x = 15; datos[3].y = 25;

    strcpy(datos[4].codigo, "H02");
    strcpy(datos[4].nombre, "Hospital Norte");
    strcpy(datos[4].tipo, "Hospital");
    datos[4].x = 85; datos[4].y = 50;

    strcpy(datos[5].codigo, "F02");
    strcpy(datos[5].nombre, "Farmacia Sur");
    strcpy(datos[5].tipo, "Farmacia");
    datos[5].x = 60; datos[5].y = 25;

    strcpy(datos[6].codigo, "C02");
    strcpy(datos[6].nombre, "Colegio Sur");
    strcpy(datos[6].tipo, "Colegio");
    datos[6].x = 30; datos[6].y = 75;

    insertados = 0;
    for (i = 0; i < 7; i++) {
        if (agregarEstablecimiento(datos[i]))
            insertados++;
    }

    printf("\nEstablecimientos nuevos insertados: %d de 7\n", insertados);
    printf("Total actual de establecimientos: %d\n", totalRegistros);
}

void mostrarRegistros(void)
{
    int i;

    if (totalRegistros == 0) {
        printf("\nNo hay establecimientos registrados.\n");
        return;
    }

    printf("\n========== REGISTROS DEL SIG ==========\n");
    for (i = 0; i < totalRegistros; i++) {
        printf("\nCodigo: %s\n", registros[i].codigo);
        printf("Nombre: %s\n", registros[i].nombre);
        printf("Tipo: %s\n", registros[i].tipo);
        printf("Coordenadas: (%.2f, %.2f)\n",
               registros[i].x, registros[i].y);
    }
    printf("\nTotal: %d establecimientos\n", totalRegistros);
}

void reiniciarSistema(void)
{
    liberarQ(mapa);
    liberarKD(raizKD);

    mapa = crearNodoQ(xmin, xmax, ymin, ymax);
    raizKD = NULL;
    totalRegistros = 0;

    printf("\nSistema reiniciado correctamente.\n");
}

/* ---------------- MENU PRINCIPAL ---------------- */

int main(void)
{
    int opcion;
    char codigo[20];
    Establecimiento e, mejor;
    double x, y, a, b, c, d, distancia;
    int total;

    opcion = -1;

    do {
        printf("\n");
        printf("====================================================\n");
        printf("       SISTEMA DE INFORMACION GEOGRAFICA (SIG)\n");
        printf("             QUADTREE Y ARBOL K-D\n");
        printf("====================================================\n");
        printf(" 1. Definir limites del mapa\n");
        printf(" 2. Registrar establecimiento (actualiza ambos arboles)\n");
        printf(" 3. Cargar establecimientos de prueba\n");
        printf(" 4. Mostrar establecimientos registrados\n");
        printf(" 5. Mostrar estructura QUADTREE\n");
        printf(" 6. Buscar establecimiento por codigo (Quadtree)\n");
        printf(" 7. Consultar establecimientos por zona (Quadtree)\n");
        printf(" 8. Buscar mas cercano (recorrido Quadtree)\n");
        printf(" 9. Reiniciar sistema\n");
        printf("10. Mostrar estructura ARBOL K-D\n");
        printf("11. Buscar mas cercano (ARBOL K-D)\n");
        printf("12. Consultar establecimientos por zona (ARBOL K-D)\n");
        printf("13. Buscar establecimiento por codigo (ARBOL K-D)\n");
        printf(" 0. Salir\n");
        printf("----------------------------------------------------\n");
        printf("Seleccione una opcion: ");

        if (scanf("%d", &opcion) != 1) {
            printf("Entrada incorrecta. Ingrese un numero.\n");
            limpiarEntrada();
            opcion = -1;
            continue;
        }

        switch (opcion) {
        case 1:
            printf("\n===== DEFINIR LIMITES DEL MAPA =====\n");
            printf("X minimo: ");
            if (scanf("%lf", &xmin) != 1) { limpiarEntrada(); break; }
            printf("X maximo: ");
            if (scanf("%lf", &xmax) != 1) { limpiarEntrada(); break; }
            printf("Y minimo: ");
            if (scanf("%lf", &ymin) != 1) { limpiarEntrada(); break; }
            printf("Y maximo: ");
            if (scanf("%lf", &ymax) != 1) { limpiarEntrada(); break; }

            if (xmin >= xmax || ymin >= ymax) {
                printf("Limites incorrectos: minimo debe ser menor que maximo.\n");
                break;
            }

            if (mapa != NULL)
                liberarQ(mapa);
            if (raizKD != NULL)
                liberarKD(raizKD);

            mapa = crearNodoQ(xmin, xmax, ymin, ymax);
            raizKD = NULL;
            totalRegistros = 0;
            limitesDefinidos = 1;

            printf("Mapa definido correctamente. Ambos arboles estan listos.\n");
            break;

        case 2:
            if (!limitesDefinidos) {
                printf("Primero defina los limites del mapa.\n");
                break;
            }

            printf("\n===== REGISTRAR ESTABLECIMIENTO =====\n");
            printf("Codigo (sin espacios): ");
            scanf("%19s", e.codigo);
            printf("Nombre: ");
            scanf(" %39[^\n]", e.nombre);
            printf("Tipo: ");
            scanf(" %29[^\n]", e.tipo);
            printf("Coordenada X: ");
            if (scanf("%lf", &e.x) != 1) { limpiarEntrada(); break; }
            printf("Coordenada Y: ");
            if (scanf("%lf", &e.y) != 1) { limpiarEntrada(); break; }

            if (agregarEstablecimiento(e))
                printf("Registro agregado al Quadtree y al arbol k-d.\n");
            break;

        case 3:
            if (!limitesDefinidos) {
                printf("Primero defina los limites del mapa.\n");
                break;
            }
            cargarEjemplo();
            break;

        case 4:
            mostrarRegistros();
            break;

        case 5:
            if (!limitesDefinidos) {
                printf("Primero defina los limites del mapa.\n");
                break;
            }
            if (totalRegistros == 0) {
                printf("No hay datos. Registre o cargue establecimientos.\n");
                break;
            }
            printf("\n========== ESTRUCTURA QUADTREE ==========\n");
            mostrarArbolQ(mapa, 0);
            break;

        case 6:
            if (totalRegistros == 0) {
                printf("No hay establecimientos registrados.\n");
                break;
            }
            printf("Codigo a buscar: ");
            scanf("%19s", codigo);
            if (!buscarCodigoQ(mapa, codigo))
                printf("Establecimiento no encontrado en Quadtree.\n");
            break;

        case 7:
            if (totalRegistros == 0) {
                printf("No hay establecimientos registrados.\n");
                break;
            }
            printf("X minimo de consulta: ");
            if (scanf("%lf", &a) != 1) { limpiarEntrada(); break; }
            printf("X maximo de consulta: ");
            if (scanf("%lf", &b) != 1) { limpiarEntrada(); break; }
            printf("Y minimo de consulta: ");
            if (scanf("%lf", &c) != 1) { limpiarEntrada(); break; }
            printf("Y maximo de consulta: ");
            if (scanf("%lf", &d) != 1) { limpiarEntrada(); break; }

            if (a > b || c > d) {
                printf("Rango incorrecto.\n");
                break;
            }

            total = 0;
            printf("\n===== CONSULTA POR ZONA: QUADTREE =====\n");
            consultarZonaQ(mapa, a, b, c, d, &total);
            printf("\nTotal encontrados: %d\n", total);
            break;

        case 8:
            if (totalRegistros == 0) {
                printf("No hay establecimientos registrados.\n");
                break;
            }
            printf("Coordenada X de consulta: ");
            if (scanf("%lf", &x) != 1) { limpiarEntrada(); break; }
            printf("Coordenada Y de consulta: ");
            if (scanf("%lf", &y) != 1) { limpiarEntrada(); break; }

            distancia = 1.0e100;
            buscarCercanoQ(mapa, x, y, &mejor, &distancia);

            printf("\n===== MAS CERCANO POR RECORRIDO QUADTREE =====\n");
            printf("Codigo: %s\n", mejor.codigo);
            printf("Nombre: %s\n", mejor.nombre);
            printf("Distancia: %.4f\n", distancia);
            break;

        case 9:
            if (!limitesDefinidos) {
                printf("Primero defina los limites del mapa.\n");
                break;
            }
            reiniciarSistema();
            break;

        case 10:
            if (totalRegistros == 0) {
                printf("No hay establecimientos registrados.\n");
                break;
            }
            printf("\n========== ESTRUCTURA ARBOL K-D ==========\n");
            printf("Cada nodo indica el eje de division X o Y.\n\n");
            mostrarArbolKD(raizKD, 0);
            break;

        case 11:
            if (totalRegistros == 0) {
                printf("No hay establecimientos registrados.\n");
                break;
            }
            printf("Coordenada X de consulta: ");
            if (scanf("%lf", &x) != 1) { limpiarEntrada(); break; }
            printf("Coordenada Y de consulta: ");
            if (scanf("%lf", &y) != 1) { limpiarEntrada(); break; }

            distancia = 1.0e100;
            buscarCercanoKD(raizKD, x, y, &mejor, &distancia);

            printf("\n===== MAS CERCANO POR ARBOL K-D =====\n");
            printf("Codigo: %s\n", mejor.codigo);
            printf("Nombre: %s\n", mejor.nombre);
            printf("Tipo: %s\n", mejor.tipo);
            printf("Ubicacion: (%.2f, %.2f)\n", mejor.x, mejor.y);
            printf("Distancia euclidiana: %.4f\n", distancia);
            printf("Metodo: busqueda con poda por el eje de division.\n");
            break;

        case 12:
            if (totalRegistros == 0) {
                printf("No hay establecimientos registrados.\n");
                break;
            }
            printf("X minimo de consulta: ");
            if (scanf("%lf", &a) != 1) { limpiarEntrada(); break; }
            printf("X maximo de consulta: ");
            if (scanf("%lf", &b) != 1) { limpiarEntrada(); break; }
            printf("Y minimo de consulta: ");
            if (scanf("%lf", &c) != 1) { limpiarEntrada(); break; }
            printf("Y maximo de consulta: ");
            if (scanf("%lf", &d) != 1) { limpiarEntrada(); break; }

            if (a > b || c > d) {
                printf("Rango incorrecto.\n");
                break;
            }

            total = 0;
            printf("\n===== CONSULTA POR ZONA: ARBOL K-D =====\n");
            consultarRangoKD(raizKD, a, b, c, d, &total);
            printf("\nTotal encontrados: %d\n", total);
            break;

        case 13:
            if (totalRegistros == 0) {
                printf("No hay establecimientos registrados.\n");
                break;
            }
            printf("Codigo a buscar: ");
            scanf("%19s", codigo);
            if (!buscarCodigoKD(raizKD, codigo))
                printf("Establecimiento no encontrado en arbol k-d.\n");
            break;

        case 0:
            printf("\nCerrando sistema SIG...\n");
            break;

        default:
            printf("Opcion incorrecta.\n");
        }

    } while (opcion != 0);

    liberarQ(mapa);
    liberarKD(raizKD);

    return 0;
}
