#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define T 3                    // Grado del Árbol B+
#define MAX_CLAVES (2 * T - 1) // Máximo 5 claves por nodo
#define MAX_HIJOS (2 * T)      // Máximo 6 hijos por nodo

// Datos médicos completos (se guardan solo en las hojas)
typedef struct {
    int codigo_historia;
    char nombre[50];
    char diagnostico[50];
    char grupo_sanguineo[5];
    char alergias[50];
} Paciente;

// Nodo del Árbol B+
typedef struct NodoClinico {
    bool es_hoja;
    int num_claves;
    int claves[MAX_CLAVES];            
    Paciente datos[MAX_CLAVES];         
    struct NodoClinico* hijos[MAX_HIJOS]; 
    struct NodoClinico* siguiente;        
} NodoClinico;

// Crea un nodo nuevo en memoria dinámica
NodoClinico* crear_nodo(bool es_hoja) {
    NodoClinico* nuevo = (NodoClinico*)malloc(sizeof(NodoClinico));
    nuevo->es_hoja = es_hoja;
    nuevo->num_claves = 0;
    nuevo->siguiente = NULL;
    for (int i = 0; i < MAX_HIJOS; i++) {
        nuevo->hijos[i] = NULL;
    }
    return nuevo;
}

// Parte una hoja llena (5 claves), enlaza la nueva hoja y copia la clave al padre
void dividir_hoja(NodoClinico* padre, int pos, NodoClinico* hoja_llena) {
    NodoClinico* nueva_hoja = crear_nodo(true);
    int mitad = T; 

    nueva_hoja->num_claves = MAX_CLAVES - mitad;
    for (int j = 0; j < nueva_hoja->num_claves; j++) {
        nueva_hoja->claves[j] = hoja_llena->claves[mitad + j];
        nueva_hoja->datos[j] = hoja_llena->datos[mitad + j];
    }
    hoja_llena->num_claves = mitad;

    // Enlace horizontal entre hojas (característica clave del B+)
    nueva_hoja->siguiente = hoja_llena->siguiente;
    hoja_llena->siguiente = nueva_hoja;

    // Hacer espacio en el padre y copiar la clave guía
    for (int j = padre->num_claves; j >= pos + 1; j--) {
        padre->hijos[j + 1] = padre->hijos[j];
    }
    padre->hijos[pos + 1] = nueva_hoja;

    for (int j = padre->num_claves - 1; j >= pos; j--) {
        padre->claves[j + 1] = padre->claves[j];
    }
    padre->claves[pos] = nueva_hoja->claves[0];
    padre->num_claves++;
}

// Divide un nodo interno cuando se llena
void dividir_interno(NodoClinico* padre, int pos, NodoClinico* interno_lleno) {
    NodoClinico* nuevo_interno = crear_nodo(false);
    nuevo_interno->num_claves = T - 1;

    for (int j = 0; j < T - 1; j++) {
        nuevo_interno->claves[j] = interno_lleno->claves[j + T];
    }
    for (int j = 0; j < T; j++) {
        nuevo_interno->hijos[j] = interno_lleno->hijos[j + T];
    }

    int clave_sube = interno_lleno->claves[T - 1];
    interno_lleno->num_claves = T - 1;

    for (int j = padre->num_claves; j >= pos + 1; j--) {
        padre->hijos[j + 1] = padre->hijos[j];
    }
    padre->hijos[pos + 1] = nuevo_interno;

    for (int j = padre->num_claves - 1; j >= pos; j--) {
        padre->claves[j + 1] = padre->claves[j];
    }
    padre->claves[pos] = clave_sube;
    padre->num_claves++;
}

// Inserta un paciente en un nodo con espacio disponible
void insertar_en_nodo(NodoClinico* nodo, Paciente p) {
    int i = nodo->num_claves - 1;

    if (nodo->es_hoja) {
        while (i >= 0 && p.codigo_historia < nodo->claves[i]) {
            nodo->claves[i + 1] = nodo->claves[i];
            nodo->datos[i + 1] = nodo->datos[i];
            i--;
        }
        nodo->claves[i + 1] = p.codigo_historia;
        nodo->datos[i + 1] = p;
        nodo->num_claves++;
    } else {
        while (i >= 0 && p.codigo_historia < nodo->claves[i]) {
            i--;
        }
        i++;

        if (nodo->hijos[i]->num_claves == MAX_CLAVES) {
            if (nodo->hijos[i]->es_hoja) {
                dividir_hoja(nodo, i, nodo->hijos[i]);
            } else {
                dividir_interno(nodo, i, nodo->hijos[i]);
            }
            if (p.codigo_historia >= nodo->claves[i]) {
                i++;
            }
        }
        insertar_en_nodo(nodo->hijos[i], p);
    }
}

// Inserción general en el árbol
void insertar_paciente(NodoClinico** raiz, Paciente p) {
    NodoClinico* r = *raiz;
    if (r->num_claves == MAX_CLAVES) {
        NodoClinico* nueva_raiz = crear_nodo(false);
        nueva_raiz->hijos[0] = r;
        *raiz = nueva_raiz;

        if (r->es_hoja) {
            dividir_hoja(nueva_raiz, 0, r);
        } else {
            dividir_interno(nueva_raiz, 0, r);
        }
        insertar_en_nodo(nueva_raiz, p);
    } else {
        insertar_en_nodo(r, p);
    }
}

// Busca un paciente por su código descendiendo hasta la hoja
Paciente* buscar_paciente(NodoClinico* raiz, int codigo) {
    NodoClinico* actual = raiz;
    while (!actual->es_hoja) {
        int i = 0;
        while (i < actual->num_claves && codigo >= actual->claves[i]) {
            i++;
        }
        actual = actual->hijos[i];
    }

    for (int i = 0; i < actual->num_claves; i++) {
        if (actual->claves[i] == codigo) {
            return &actual->datos[i];
        }
    }
    return NULL;
}

// Recorre las hojas en línea recta usando 'siguiente' y guarda en historias.dat
void guardar_en_binario(NodoClinico* raiz) {
    FILE* archivo = fopen("historias.dat", "wb");
    if (!archivo) return;

    // Bajamos hasta la primera hoja de la izquierda
    NodoClinico* hoja = raiz;
    while (!hoja->es_hoja) {
        hoja = hoja->hijos[0];
    }

    // Avanzamos hoja por hoja sin volver a subir a la raíz
    while (hoja != NULL) {
        for (int i = 0; i < hoja->num_claves; i++) {
            fwrite(&hoja->datos[i], sizeof(Paciente), 1, archivo);
        }
        hoja = hoja->siguiente;
    }

    fclose(archivo);
}

int main() {
    NodoClinico* raiz = crear_nodo(true);

    // 7 pacientes para forzar al menos una división de hoja (máximo 5 claves)
    Paciente lista[] = {
        {1045, "Elena Morales",   "Apendicitis",   "O+",  "Penicilina"},
        {1012, "Carlos Mendoza",  "Hipertension",  "A+",  "Ninguna"},
        {1089, "Lucia Fernandez", "Fractura",      "B-",  "Ibuprofeno"},
        {1003, "Mateo Quispe",    "Asma",          "O-",  "Polvo"},
        {1067, "Valeria Rojas",   "Gastritis",     "AB+", "Sulfas"},
        {1025, "Jorge Paredes",   "Diabetes",      "A-",  "Ninguna"},
        {1095, "Sofia Castillo",  "Migrana",       "O+",  "Aspirina"}
    };

    for (int i = 0; i < 7; i++) {
        insertar_paciente(&raiz, lista[i]);
    }

    // 1. Búsqueda puntual de un paciente
    Paciente* p = buscar_paciente(raiz, 1067);
    if (p != NULL) {
        printf("Paciente encontrado: %s | Sangre: %s | Alergias: %s\n",
               p->nombre, p->grupo_sanguineo, p->alergias);
    }

    // 2. Guardar registros ordenados en archivo binario
    guardar_en_binario(raiz);

    // 3. Leer historias.dat para verificar el recorrido secuencial
    FILE* archivo = fopen("historias.dat", "rb");
    Paciente temp;
    printf("\nRegistros guardados en historias.dat:\n");
    while (fread(&temp, sizeof(Paciente), 1, archivo) == 1) {
        printf("HC #%d - %s (%s)\n", temp.codigo_historia, temp.nombre, temp.diagnostico);
    }
    fclose(archivo);

    return 0;
}
