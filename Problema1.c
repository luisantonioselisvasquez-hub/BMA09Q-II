
#include <stdio.h>
#include <stdlib.h>

#include "dsu.h"


// Representar una conexion fisica
typedef struct {

    int origen;
    int destino;

} Conexion;


// Lista dinamica de conexiones
typedef struct {

    Conexion *datos;

    int cantidad;
    int capacidad;

} ListaConexiones;


// Inicializar conexiones
int inicializarLista(ListaConexiones *lista) {

    lista->cantidad = 0;
    lista->capacidad = 4;

    lista->datos = (Conexion *)malloc(
        lista->capacidad * sizeof(Conexion));

    return lista->datos != NULL;
}


// Aumentar capacidad de conexiones
int ampliarLista(ListaConexiones *lista) {

    int nuevaCapacidad = lista->capacidad * 2;

    Conexion *nuevo = (Conexion *)realloc(
        lista->datos,
        nuevaCapacidad * sizeof(Conexion));

    if (nuevo == NULL)
        return 0;

    lista->datos = nuevo;
    lista->capacidad = nuevaCapacidad;

    return 1;
}


// Buscar conexion existente
int existeConexion(ListaConexiones *lista,
                   int a, int b) {

    int i;

    for (i = 0; i < lista->cantidad; i++) {

        if ((lista->datos[i].origen == a &&
             lista->datos[i].destino == b) ||

            (lista->datos[i].origen == b &&
             lista->datos[i].destino == a)) {

            return 1;
        }
    }

    return 0;
}


// Agregar conexion fisica
int agregarConexion(ListaConexiones *lista,
                    int a, int b) {

    if (lista->cantidad == lista->capacidad) {

        if (!ampliarLista(lista))
            return 0;
    }

    lista->datos[lista->cantidad].origen = a;

    lista->datos[lista->cantidad].destino = b;

    lista->cantidad++;

    return 1;
}


// Eliminar conexion fisica
int eliminarConexion(ListaConexiones *lista,
                     int a, int b) {

    int i, j;

    for (i = 0; i < lista->cantidad; i++) {

        if ((lista->datos[i].origen == a &&
             lista->datos[i].destino == b) ||

            (lista->datos[i].origen == b &&
             lista->datos[i].destino == a)) {

            for (j = i; j < lista->cantidad - 1; j++) {

                lista->datos[j] = lista->datos[j + 1];
            }

            lista->cantidad--;

            return 1;
        }
    }

    return 0;
}


// Reconstruir la conectividad
void reconstruirRed(DSU *red,
                    ListaConexiones *lista) {

    int i;

    reiniciarDSU(red);

    for (i = 0; i < lista->cantidad; i++) {

        int a = lista->datos[i].origen;
        int b = lista->datos[i].destino;

        if (computadoraValida(red, a) &&
            computadoraValida(red, b)) {

            unir(red, a, b);
        }
    }
}


// Agregar computadora
void menuAgregarComputadora(DSU *red) {

    int id = agregarComputadora(red);

    if (id == -1) {

        printf("\nError al reservar memoria.\n");

    } else {

        printf("\nComputadora %d agregada correctamente.\n", id);
    }
}


// Eliminar computadora
void menuEliminarComputadora(
    DSU *red,
    ListaConexiones *lista) {

    int id, i, j;

    printf("\nIdentificador de la computadora: ");

    if (scanf("%d", &id) != 1) {
        printf("Entrada invalida.\n");
        return;
    }

    if (!computadoraValida(red, id)) {

        printf("La computadora no existe o ya fue eliminada.\n");

        return;
    }

    // Desactivar computadora
    desactivarComputadora(red, id);

    // Eliminar todas sus conexiones
    i = 0;

    while (i < lista->cantidad) {

        if (lista->datos[i].origen == id ||
            lista->datos[i].destino == id) {

            for (j = i; j < lista->cantidad - 1; j++) {

                lista->datos[j] = lista->datos[j + 1];
            }

            lista->cantidad--;

        } else {

            i++;
        }
    }

    // Reconstruir DSU
    reconstruirRed(red, lista);

    printf("Computadora %d eliminada correctamente.\n", id);
}


// Conectar computadoras
void menuConectar(
    DSU *red,
    ListaConexiones *lista) {

    int a, b;

    printf("\nPrimera computadora: ");

    if (scanf("%d", &a) != 1) {
        printf("Entrada invalida.\n");
        return;
    }

    printf("Segunda computadora: ");

    if (scanf("%d", &b) != 1) {
        printf("Entrada invalida.\n");
        return;
    }

    if (!computadoraValida(red, a) ||
        !computadoraValida(red, b)) {

        printf("Alguna computadora no existe.\n");

        return;
    }

    if (a == b) {

        printf("No puedes conectar una computadora consigo misma.\n");

        return;
    }

    if (existeConexion(lista, a, b)) {

        printf("Esta conexion ya existe.\n");

        return;
    }

    if (!agregarConexion(lista, a, b)) {

        printf("Error de memoria.\n");

        return;
    }

    unir(red, a, b);

    printf("Conexion %d - %d establecida.\n", a, b);
}


// Desconectar computadoras
void menuDesconectar(
    DSU *red,
    ListaConexiones *lista) {

    int a, b;

    printf("\nPrimera computadora: ");

    if (scanf("%d", &a) != 1) {
        printf("Entrada invalida.\n");
        return;
    }

    printf("Segunda computadora: ");

    if (scanf("%d", &b) != 1) {
        printf("Entrada invalida.\n");
        return;
    }

    if (!computadoraValida(red, a) ||
        !computadoraValida(red, b)) {

        printf("Alguna computadora no existe.\n");

        return;
    }

    if (!eliminarConexion(lista, a, b)) {

        printf("No existe esa conexion fisica.\n");

        return;
    }

    reconstruirRed(red, lista);

    printf("Conexion %d - %d eliminada.\n", a, b);
}


// Consultar conectividad
void menuConsultar(DSU *red) {

    int a, b;

    printf("\nPrimera computadora: ");

    if (scanf("%d", &a) != 1) {
        printf("Entrada invalida.\n");
        return;
    }

    printf("Segunda computadora: ");

    if (scanf("%d", &b) != 1) {
        printf("Entrada invalida.\n");
        return;
    }

    if (!computadoraValida(red, a) ||
        !computadoraValida(red, b)) {

        printf("Alguna computadora no existe.\n");

        return;
    }

    if (conectado(red, a, b)) {

        printf("Las computadoras estan conectadas.\n");

    } else {

        printf("Las computadoras NO estan conectadas.\n");
    }
}


// Mostrar computadoras
void mostrarComputadoras(DSU *red) {

    int i;

    printf("\n========== COMPUTADORAS ==========\n");

    for (i = 0; i < red->n; i++) {

        if (red->active[i] == 1) {

            printf("Computadora %d [ACTIVA]\n", i);

        } else {

            printf("Computadora %d [ELIMINADA]\n", i);
        }
    }
}


// Mostrar conexiones
void mostrarConexiones(ListaConexiones *lista) {

    int i;

    printf("\n========== CONEXIONES ==========\n");

    if (lista->cantidad == 0) {

        printf("No existen conexiones.\n");

        return;
    }

    for (i = 0; i < lista->cantidad; i++) {

        printf("%d <--> %d\n",
               lista->datos[i].origen,
               lista->datos[i].destino);
    }
}


// Mostrar grupos
void mostrarGrupos(DSU *red) {

    int i, j;

    printf("\n========== COMPONENTES CONEXOS ==========\n");

    for (i = 0; i < red->n; i++) {

        if (computadoraValida(red, i) &&
            find(red, i) == i) {

            printf("Grupo (raiz %d): ", i);

            for (j = 0; j < red->n; j++) {

                if (computadoraValida(red, j) &&
                    find(red, j) == i) {

                    printf("%d ", j);
                }
            }

            printf("\n");
        }
    }

    printf("\nTotal de componentes: %d\n",
           red->components);
}


// Inicializar red universitaria
int inicializarUniversidad(
    DSU *red,
    ListaConexiones *lista) {

    int i;

    // Agregar ocho computadoras
    for (i = 0; i < 8; i++) {

        if (agregarComputadora(red) == -1)
            return 0;
    }

    // Conexiones iniciales
    if (!agregarConexion(lista, 0, 1))
        return 0;

    if (!agregarConexion(lista, 1, 2))
        return 0;

    if (!agregarConexion(lista, 3, 4))
        return 0;

    if (!agregarConexion(lista, 5, 6))
        return 0;

    // Construir los conjuntos iniciales
    reconstruirRed(red, lista);

    return 1;
}


// Leer enteros evitando errores de entrada
int leerEntero(const char *mensaje, int *valor) {

    int c;

    printf("%s", mensaje);

    if (scanf("%d", valor) != 1) {

        while ((c = getchar()) != '\n' && c != EOF);

        return 0;
    }

    return 1;
}


// Programa principal
int main() {

    DSU *red = crearDSU(8);

    if (red == NULL) {

        printf("Error al crear DSU.\n");

        return 1;
    }

    ListaConexiones lista;

    if (!inicializarLista(&lista)) {

        printf("Error al inicializar conexiones.\n");

        free(lista.datos);
        liberarDSU(red);

        return 1;
    }

    if (!inicializarUniversidad(red, &lista)) {

        printf("Error al construir la red inicial.\n");

        free(lista.datos);
        liberarDSU(red);

        return 1;
    }

    int opcion;

    do {

        printf("\n");
        printf("==================================================================\n");
        printf("     UNIVERSIDAD NACIONAL DE INGENIERIA-FIEE- ADMINISTRADOR DSU\n");
        printf("==================================================================\n");
        printf("Estas en el laboratorio de ciberseguridad de la FIEE, tienes varias redes LAN y 8 computadoras en total identificadas del 0 al 7.\n");
        printf("Puede manejar las redes desde este administrador e incluso agregar computadoras o desconectarlas. Se recomienda ver primero la opcion 8 para saber las conexiones presentes.\n");
        printf("1. Agregar computadora\n");
        printf("2. Eliminar computadora\n");
        printf("3. Conectar computadoras\n");
        printf("4. Desconectar computadoras\n");
        printf("5. Consultar conectividad\n");
        printf("6. Mostrar computadoras\n");
        printf("7. Mostrar conexiones\n");
        printf("8. Mostrar grupos independientes\n");
        printf("0. Salir\n");

        printf("========================================\n");

        if (!leerEntero("Seleccione una opcion: ", &opcion)) {

            printf("Debe ingresar un numero.\n");

            continue;
        }

        switch (opcion) {

            case 1:

                menuAgregarComputadora(red);

                break;

            case 2:

                menuEliminarComputadora(red, &lista);

                break;

            case 3:

                menuConectar(red, &lista);

                break;

            case 4:

                menuDesconectar(red, &lista);

                break;

            case 5:

                menuConsultar(red);

                break;

            case 6:

                mostrarComputadoras(red);

                break;

            case 7:

                mostrarConexiones(&lista);

                break;

            case 8:

                mostrarGrupos(red);

                break;

            case 0:

                printf("\nCerrando administrador...\n");

                break;

            default:

                printf("\nOpcion incorrecta.\n");
        }

    } while (opcion != 0);


    // Liberar memoria
    free(lista.datos);

    liberarDSU(red);

    return 0;
}

