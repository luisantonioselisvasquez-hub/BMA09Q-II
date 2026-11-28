#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define T 3 // Grado mínimo del Árbol B (Máximo 5 claves y 6 hijos por nodo)

// Estructura que representa los datos de una cuenta bancaria
typedef struct {
    int num_cuenta;
    char titular[50];
    double saldo;
} CuentaBancaria;

// Estructura del Nodo del Árbol B
typedef struct NodoB {
    CuentaBancaria claves[2 * T - 1]; 
    struct NodoB *hijos[2 * T];       
    int num_claves;                 
    bool es_hoja;               
} NodoB;

// Crea un nuevo nodo en memoria dinámica
NodoB* crear_nodo(bool es_hoja) {
    NodoB* nuevo = (NodoB*)malloc(sizeof(NodoB));
    if (!nuevo) {
        perror("Error al asignar memoria");
        exit(EXIT_FAILURE);
    }
    nuevo->es_hoja = es_hoja;
    nuevo->num_claves = 0;
    for (int i = 0; i < 2 * T; i++) {
        nuevo->hijos[i] = NULL;
    }
    return nuevo;
}

// Divide un hijo lleno (con 5 claves) y promueve la clave central al padre
void dividir_hijo(NodoB* padre, int indice, NodoB* hijo_lleno) {
    NodoB* nuevo_hijo = crear_nodo(hijo_lleno->es_hoja);
    nuevo_hijo->num_claves = T - 1;

    for (int j = 0; j < T - 1; j++) {
        nuevo_hijo->claves[j] = hijo_lleno->claves[j + T];
    }

    if (!hijo_lleno->es_hoja) {
        for (int j = 0; j < T; j++) {
            nuevo_hijo->hijos[j] = hijo_lleno->hijos[j + T];
        }
    }

    hijo_lleno->num_claves = T - 1;

    for (int j = padre->num_claves; j >= indice + 1; j--) {
        padre->hijos[j + 1] = padre->hijos[j];
    }
    padre->hijos[indice + 1] = nuevo_hijo;

    for (int j = padre->num_claves - 1; j >= indice; j--) {
        padre->claves[j + 1] = padre->claves[j];
    }
    padre->claves[indice] = hijo_lleno->claves[T - 1];
    padre->num_claves++;
}

// Inserta una cuenta en un nodo que aún tiene espacio disponible
void insertar_no_lleno(NodoB* nodo, CuentaBancaria cuenta) {
    int i = nodo->num_claves - 1;

    if (nodo->es_hoja) {
        while (i >= 0 && cuenta.num_cuenta < nodo->claves[i].num_cuenta) {
            nodo->claves[i + 1] = nodo->claves[i];
            i--;
        }
        nodo->claves[i + 1] = cuenta;
        nodo->num_claves++;
    } else {
        while (i >= 0 && cuenta.num_cuenta < nodo->claves[i].num_cuenta) {
            i--;
        }
        i++;

        if (nodo->hijos[i]->num_claves == 2 * T - 1) {
            dividir_hijo(nodo, i, nodo->hijos[i]);
            if (cuenta.num_cuenta > nodo->claves[i].num_cuenta) {
                i++;
            }
        }
        insertar_no_lleno(nodo->hijos[i], cuenta);
    }
}

// Función principal para insertar una cuenta en el Árbol B
void insertar(NodoB** raiz, CuentaBancaria cuenta) {
    NodoB* r = *raiz;

    if (r->num_claves == 2 * T - 1) {
        NodoB* nueva_raiz = crear_nodo(false);
        nueva_raiz->hijos[0] = r;
        dividir_hijo(nueva_raiz, 0, r);

        int i = 0;
        if (nueva_raiz->claves[0].num_cuenta < cuenta.num_cuenta) {
            i++;
        }
        insertar_no_lleno(nueva_raiz->hijos[i], cuenta);
        *raiz = nueva_raiz;
    } else {
        insertar_no_lleno(r, cuenta);
    }
}

// Búsqueda recursiva por número de cuenta
CuentaBancaria* buscar_cuenta(NodoB* nodo, int num_cuenta) {
    int i = 0;
    while (i < nodo->num_claves && num_cuenta > nodo->claves[i].num_cuenta) {
        i++;
    }

    if (i < nodo->num_claves && num_cuenta == nodo->claves[i].num_cuenta) {
        return &nodo->claves[i];
    }

    if (nodo->es_hoja) {
        return NULL;
    }

    return buscar_cuenta(nodo->hijos[i], num_cuenta);
}

// Recorrido In-Orden para guardar los registros ordenados en disco
void guardar_inorden(NodoB* nodo, FILE* archivo) {
    if (nodo != NULL) {
        int i;
        for (i = 0; i < nodo->num_claves; i++) {
            if (!nodo->es_hoja) {
                guardar_inorden(nodo->hijos[i], archivo);
            }
            fwrite(&nodo->claves[i], sizeof(CuentaBancaria), 1, archivo);
        }
        if (!nodo->es_hoja) {
            guardar_inorden(nodo->hijos[i], archivo);
        }
    }
}

// Recorrido In-Orden para mostrar todas las cuentas en consola
void mostrar_inorden(NodoB* nodo) {
    if (nodo != NULL) {
        int i;
        for (i = 0; i < nodo->num_claves; i++) {
            if (!nodo->es_hoja) {
                mostrar_inorden(nodo->hijos[i]);
            }
            printf("Cuenta: %-8d | Titular: %-25s | Saldo: $%.2f\n",
                   nodo->claves[i].num_cuenta,
                   nodo->claves[i].titular,
                   nodo->claves[i].saldo);
        }
        if (!nodo->es_hoja) {
            mostrar_inorden(nodo->hijos[i]);
        }
    }
}

// Limpia el buffer de entrada para evitar saltos al leer cadenas con espacios
void limpiar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    NodoB* raiz = crear_nodo(true);
    int opcion;

    do {
        printf("\n=== SISTEMA BANCARIO ===\n");
        printf("1. Registrar nueva cuenta bancaria\n");
        printf("2. Consultar cuenta por numero\n");
        printf("3. Mostrar todas las cuentas\n");
        printf("4. Salir\n");
        printf("Seleccione una opcion: ");
        scanf("%d", &opcion);
        limpiar_buffer();

        switch (opcion) {
            case 1: {
                CuentaBancaria nueva;
                printf("\n--- Registro de Nueva Cuenta ---\n");
                printf("Numero de cuenta: ");
                scanf("%d", &nueva.num_cuenta);
                limpiar_buffer();

                if (buscar_cuenta(raiz, nueva.num_cuenta) != NULL) {
                    printf("Error: El numero de cuenta %d ya esta registrado.\n", nueva.num_cuenta);
                    break;
                }

                printf("Nombre del titular: ");
                fgets(nueva.titular, sizeof(nueva.titular), stdin);
                nueva.titular[strcspn(nueva.titular, "\n")] = '\0';

                printf("Saldo inicial ($): ");
                scanf("%lf", &nueva.saldo);
                limpiar_buffer();

                insertar(&raiz, nueva);
                printf("Cuenta registrada exitosamente.\n");
                break;
            }

            case 2: {
                int num_buscar;
                printf("\nIngrese el numero de cuenta a consultar: ");
                scanf("%d", &num_buscar);
                limpiar_buffer();

                CuentaBancaria* res = buscar_cuenta(raiz, num_buscar);
                if (res != NULL) {
                    printf("\n--- Resultado de la Busqueda ---\n");
                    printf("Cuenta Nro : %d\n", res->num_cuenta);
                    printf("Titular    : %s\n", res->titular);
                    printf("Saldo Disp.: $%.2f\n", res->saldo);
                } else {
                    printf("La cuenta %d no existe en el sistema.\n", num_buscar);
                }
                break;
            }

            case 3: {
                printf("\n--- Listado General de Cuentas ---\n");
                if (raiz->num_claves == 0) {
                    printf("No hay cuentas registradas aun.\n");
                } else {
                    mostrar_inorden(raiz);
                }
                break;
            }

            case 4: {
                FILE* archivo = fopen("cuentas.dat", "wb");
                if (archivo) {
                    guardar_inorden(raiz, archivo);
                    fclose(archivo);
                }
                printf("\nSaliendo del sistema...\n");
                break;
            }

            default:
                printf("Opcion no valida. Intente nuevamente.\n");
        }
    } while (opcion != 4);

    return 0;
}
