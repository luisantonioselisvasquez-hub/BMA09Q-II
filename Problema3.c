 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

// ==========================================
// ESTRUCTURA PARA REPRESENTAR UNA ARISTA
// ==========================================

typedef struct {
    int origen;
    int destino;
} Arista;

// ==========================================
// ESTRUCTURA PARA REPRESENTAR EL GRAFO
// ==========================================

typedef struct {
    int numVertices;
    int numAristas;
    int capacidad;

    Arista *aristas;
} Grafo;

// ==========================================
// LEER UN NUMERO ENTERO
// ==========================================

int leerEntero(char mensaje[]) {

    char buffer[100];
    char *fin;
    long numero;

    while (1) {

        printf("%s", mensaje);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            return -1;
        }

        errno = 0;
        numero = strtol(buffer, &fin, 10);

        if (fin == buffer ||
            errno == ERANGE ||
            numero < INT_MIN ||
            numero > INT_MAX) {

            printf("Ingrese un numero entero valido.\n");
            continue;
        }

        while (*fin == ' ' || *fin == '\t' ||
               *fin == '\n' || *fin == '\r') {
            fin++;
        }

        if (*fin != '\0') {
            printf("Entrada incorrecta.\n");
            continue;
        }

        return (int)numero;
    }
}

// ==========================================
// CREAR EL GRAFO DINAMICAMENTE
// ==========================================

Grafo *crearGrafo(int vertices) {

    Grafo *g;

    g = (Grafo *)malloc(sizeof(Grafo));

    if (g == NULL) {
        return NULL;
    }

    g->numVertices = vertices;
    g->numAristas = 0;
    g->capacidad = 4;

    g->aristas = (Arista *)malloc(
        g->capacidad * sizeof(Arista)
    );

    if (g->aristas == NULL) {
        free(g);
        return NULL;
    }

    return g;
}

// ==========================================
// AGREGAR UN SWITCH
// ==========================================

void agregarSwitch(Grafo *g) {

    g->numVertices++;

    printf("\nNuevo switch agregado: SW%d\n",
           g->numVertices);
}

// ==========================================
// VERIFICAR SI UNA ARISTA EXISTE
// ==========================================

int existeArista(Grafo *g, int u, int v) {

    int i;
    int a, b;

    for (i = 0; i < g->numAristas; i++) {

        a = g->aristas[i].origen;
        b = g->aristas[i].destino;

        if ((a == u && b == v) ||
            (a == v && b == u)) {

            return 1;
        }
    }

    return 0;
}

// ==========================================
// AGREGAR UNA CONEXION
// ==========================================

void agregarConexion(Grafo *g) {

    int u, v;
    int nuevaCapacidad;

    Arista *nuevo;

    printf("\n--- NUEVA CONEXION ---\n");

    u = leerEntero("Switch de origen: ");
    v = leerEntero("Switch de destino: ");

    if (u < 1 || u > g->numVertices ||
        v < 1 || v > g->numVertices) {

        printf("Error: switch inexistente.\n");
        return;
    }

    if (existeArista(g, u - 1, v - 1)) {

        printf("Esta conexion ya existe.\n");
        return;
    }

    // Ampliamos el arreglo cuando se llena
    if (g->numAristas == g->capacidad) {

        nuevaCapacidad = g->capacidad * 2;

        nuevo = (Arista *)realloc(
            g->aristas,
            nuevaCapacidad * sizeof(Arista)
        );

        if (nuevo == NULL) {
            printf("Error al ampliar memoria.\n");
            return;
        }

        g->aristas = nuevo;
        g->capacidad = nuevaCapacidad;
    }

    // Guardamos la nueva conexion
    g->aristas[g->numAristas].origen = u - 1;
    g->aristas[g->numAristas].destino = v - 1;

    g->numAristas++;

    printf("Conexion SW%d - SW%d agregada.\n", u, v);
}

// ==========================================
// ELIMINAR UNA CONEXION
// ==========================================

void eliminarConexion(Grafo *g) {

    int u, v;
    int posicion;
    int i;
    int a, b;

    posicion = -1;

    printf("\n--- ELIMINAR CONEXION ---\n");

    u = leerEntero("Primer switch: ");
    v = leerEntero("Segundo switch: ");

    if (u < 1 || u > g->numVertices ||
        v < 1 || v > g->numVertices) {

        printf("Switch inexistente.\n");
        return;
    }

    // Buscamos la conexion
    for (i = 0; i < g->numAristas; i++) {

        a = g->aristas[i].origen;
        b = g->aristas[i].destino;

        if ((a == u - 1 && b == v - 1) ||
            (a == v - 1 && b == u - 1)) {

            posicion = i;
            break;
        }
    }

    if (posicion == -1) {

        printf("La conexion no existe.\n");
        return;
    }

    // Desplazamos las aristas posteriores
    for (i = posicion; i < g->numAristas - 1; i++) {

        g->aristas[i] = g->aristas[i + 1];
    }

    g->numAristas--;

    printf("Conexion eliminada correctamente.\n");
}

// ==========================================
// MOSTRAR EL GRAFO
// ==========================================

void mostrarGrafo(Grafo *g) {

    int i;

    printf("\n========== TOPOLOGIA LAN ==========\n");

    printf("Cantidad de switches: %d\n",
           g->numVertices);

    printf("Cantidad de conexiones: %d\n",
           g->numAristas);

    if (g->numAristas == 0) {

        printf("No existen conexiones registradas.\n");
        return;
    }

    for (i = 0; i < g->numAristas; i++) {

        printf("E%d: SW%d <--> SW%d\n",
               i + 1,
               g->aristas[i].origen + 1,
               g->aristas[i].destino + 1);
    }
}

// ==========================================
// FIND: BUSCAR EL REPRESENTANTE
// ==========================================

int find(int padre[], int x) {

    // Compresion de caminos
    if (padre[x] != x) {

        padre[x] = find(padre, padre[x]);
    }

    return padre[x];
}

// ==========================================
// UNION: UNIR DOS CONJUNTOS
// ==========================================

int unir(int padre[], int rango[], int x, int y) {

    int raizX;
    int raizY;

    raizX = find(padre, x);
    raizY = find(padre, y);

    // Si tienen la misma raiz, existe un ciclo
    if (raizX == raizY) {

        return 0;
    }

    // Union por rango
    if (rango[raizX] < rango[raizY]) {

        padre[raizX] = raizY;

    } else if (rango[raizX] > rango[raizY]) {

        padre[raizY] = raizX;

    } else {

        padre[raizY] = raizX;
        rango[raizX]++;
    }

    return 1;
}

// ==========================================
// DETECCION DE CICLOS EN EL GRAFO
// ==========================================

void detectarCiclos(Grafo *g) {

    int n;
    int i;

    int ciclos;
    int aceptadas;
    int componentes;

    int u, v;

    int *padre;
    int *rango;

    Arista *bosque;

    n = g->numVertices;

    // Reservamos memoria dinamica
    padre = (int *)malloc(n * sizeof(int));

    rango = (int *)calloc(n, sizeof(int));

    bosque = NULL;

    if (g->numAristas > 0) {

        bosque = (Arista *)malloc(
            g->numAristas * sizeof(Arista)
        );
    }

    if (padre == NULL || rango == NULL ||
        (g->numAristas > 0 && bosque == NULL)) {

        printf("Error de memoria.\n");

        free(padre);
        free(rango);
        free(bosque);

        return;
    }

    // Inicialmente cada switch es su propio conjunto
    for (i = 0; i < n; i++) {

        padre[i] = i;
    }

    ciclos = 0;
    aceptadas = 0;

    printf("\n========== DETECCION DE CICLOS ==========\n");

    // Analizamos todas las aristas
    for (i = 0; i < g->numAristas; i++) {

        u = g->aristas[i].origen;
        v = g->aristas[i].destino;

        printf("\nAnalizando E%d: SW%d - SW%d\n",
               i + 1, u + 1, v + 1);

        if (unir(padre, rango, u, v)) {

            printf("Resultado: NO FORMA CICLO.\n");

            // Incorporamos la arista al bosque
            bosque[aceptadas] = g->aristas[i];

            aceptadas++;

        } else {

            printf("RESULTADO: CICLO DETECTADO.\n");

            printf("SW%d y SW%d ya pertenecen ",
                   u + 1, v + 1);

            printf("al mismo conjunto.\n");

            printf("Enlace que cierra el ciclo: E%d\n",
                   i + 1);

            ciclos++;
        }
    }

    // Contamos los componentes conexos
    componentes = 0;

    for (i = 0; i < n; i++) {

        if (find(padre, i) == i) {

            componentes++;
        }
    }

    // Resultados finales
    printf("\n========== RESULTADO FINAL ==========\n");

    printf("Switches: %d\n", n);
    printf("Conexiones: %d\n", g->numAristas);

    printf("Enlaces que cierran ciclos: %d\n",
           ciclos);

    printf("Componentes conexos: %d\n",
           componentes);

    if (ciclos > 0) {

        printf("\nLA RED PRESENTA CICLOS.\n");

    } else {

        printf("\nLA RED NO PRESENTA CICLOS.\n");
    }

    // Mostramos el bosque de expansion
    printf("\n--- BOSQUE DE EXPANSION ---\n");

    for (i = 0; i < aceptadas; i++) {

        printf("SW%d - SW%d\n",
               bosque[i].origen + 1,
               bosque[i].destino + 1);
    }

    printf("Total de enlaces del bosque: %d\n",
           aceptadas);

    // Comprobamos si es un arbol
    if (componentes == 1 && aceptadas == n - 1) {

        printf("\nLa topologia es un arbol de expansion.\n");

    } else if (componentes > 1 && ciclos == 0) {

        printf("\nLa topologia es un bosque sin ciclos.\n");

    } else {

        printf("\nLa topologia no es un arbol de expansion.\n");
    }

    // Liberamos la memoria
    free(padre);
    free(rango);
    free(bosque);
}

// ==========================================
// MENU PRINCIPAL
// ==========================================

void mostrarMenu() {

    printf("\n");
    printf("====================================\n");
    printf("       ANALIZADOR DE REDES LAN\n");
    printf("       DETECCION DE CICLOS\n");
    printf("====================================\n");
printf("Se encuentra en el LIFIEE y necesita crear la red LAN respectiva, se le avisara por protocolo STP si existe un bucle\n\n");
    printf("1. Agregar un switch\n");
    printf("2. Agregar una conexion\n");
    printf("3. Eliminar una conexion\n");
    printf("4. Mostrar topologia\n");
    printf("5. Detectar ciclos (UNION-FIND)\n");
    printf("6. Salir\n");

    printf("====================================\n");
}

// ==========================================
// FUNCION PRINCIPAL
// ==========================================

int main() {

    int cantidad;
    int opcion;

    Grafo *red;

    printf("ANALIZADOR DE GRAFOS PARA REDES LAN\n\n");

    do {

        cantidad = leerEntero(
            "Ingrese la cantidad inicial de switches: "
        );

        if (cantidad < 1) {

            printf("Debe existir al menos un switch.\n");
        }

    } while (cantidad < 1);

    // Creamos el grafo
    red = crearGrafo(cantidad);

    if (red == NULL) {

        printf("No se pudo crear el grafo.\n");
        return 1;
    }

    do {

        mostrarMenu();

        opcion = leerEntero("Seleccione una opcion: ");

        switch (opcion) {

            case 1:

                agregarSwitch(red);
                break;

            case 2:

                agregarConexion(red);
                break;

            case 3:

                eliminarConexion(red);
                break;

            case 4:

                mostrarGrafo(red);
                break;

            case 5:

                detectarCiclos(red);
                break;

            case 6:

                printf("\nFinalizando programa...\n");
                break;

            default:

                printf("Opcion incorrecta.\n");
        }

    } while (opcion != 6);

    // Liberamos toda la memoria dinamica
    free(red->aristas);
    free(red);

    printf("Memoria liberada correctamente.\n");

    return 0;
}

