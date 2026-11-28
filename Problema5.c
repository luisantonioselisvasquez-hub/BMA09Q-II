#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define T 3                    // Grado del Árbol B+
#define MAX_CLAVES (2 * T - 1) // Máximo 5 claves por nodo
#define MAX_HIJOS (2 * T)      // Máximo 6 hijos por nodo

// Datos completos de cada ejemplar (residen solo en las hojas)
typedef struct {
    int isbn;
    char titulo[60];
    char autor[50];
    bool disponible;
} Libro;

// Nodo del Árbol B+
typedef struct NodoBiblioteca {
    bool es_hoja;
    int num_claves;
    int claves[MAX_CLAVES];                 
    Libro libros[MAX_CLAVES];       
    struct NodoBiblioteca* hijos[MAX_HIJOS]; 
    struct NodoBiblioteca* siguiente;      
} NodoBiblioteca;

// Crea un nodo nuevo en memoria dinámica
NodoBiblioteca* crear_nodo(bool es_hoja) {
    NodoBiblioteca* nuevo = (NodoBiblioteca*)malloc(sizeof(NodoBiblioteca));
    nuevo->es_hoja = es_hoja;
    nuevo->num_claves = 0;
    nuevo->siguiente = NULL;
    for (int i = 0; i < MAX_HIJOS; i++) {
        nuevo->hijos[i] = NULL;
    }
    return nuevo;
}

// Divide una hoja llena (5 registros), enlaza el hermano derecho y copia la clave guía al padre
void dividir_hoja(NodoBiblioteca* padre, int pos, NodoBiblioteca* hoja_llena) {
    NodoBiblioteca* nueva_hoja = crear_nodo(true);
    int mitad = T; 

    nueva_hoja->num_claves = MAX_CLAVES - mitad;
    for (int j = 0; j < nueva_hoja->num_claves; j++) {
        nueva_hoja->claves[j] = hoja_llena->claves[mitad + j];
        nueva_hoja->libros[j] = hoja_llena->libros[mitad + j];
    }
    hoja_llena->num_claves = mitad;

    // Enlace horizontal entre hojas contiguas
    nueva_hoja->siguiente = hoja_llena->siguiente;
    hoja_llena->siguiente = nueva_hoja;

    // Desplazar punteros y claves en el nodo padre
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
void dividir_interno(NodoBiblioteca* padre, int pos, NodoBiblioteca* interno_lleno) {
    NodoBiblioteca* nuevo_interno = crear_nodo(false);
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

// Inserta un libro en un nodo que aún tiene espacio
void insertar_en_nodo(NodoBiblioteca* nodo, Libro libro) {
    int i = nodo->num_claves - 1;

    if (nodo->es_hoja) {
        while (i >= 0 && libro.isbn < nodo->claves[i]) {
            nodo->claves[i + 1] = nodo->claves[i];
            nodo->libros[i + 1] = nodo->libros[i];
            i--;
        }
        nodo->claves[i + 1] = libro.isbn;
        nodo->libros[i + 1] = libro;
        nodo->num_claves++;
    } else {
        while (i >= 0 && libro.isbn < nodo->claves[i]) {
            i--;
        }
        i++;

        if (nodo->hijos[i]->num_claves == MAX_CLAVES) {
            if (nodo->hijos[i]->es_hoja) {
                dividir_hoja(nodo, i, nodo->hijos[i]);
            } else {
                dividir_interno(nodo, i, nodo->hijos[i]);
            }
            if (libro.isbn >= nodo->claves[i]) {
                i++;
            }
        }
        insertar_en_nodo(nodo->hijos[i], libro);
    }
}

// Función principal de inserción de ejemplares
void insertar_libro(NodoBiblioteca** raiz, Libro libro) {
    NodoBiblioteca* r = *raiz;
    if (r->num_claves == MAX_CLAVES) {
        NodoBiblioteca* nueva_raiz = crear_nodo(false);
        nueva_raiz->hijos[0] = r;
        *raiz = nueva_raiz;

        if (r->es_hoja) {
            dividir_hoja(nueva_raiz, 0, r);
        } else {
            dividir_interno(nueva_raiz, 0, r);
        }
        insertar_en_nodo(nueva_raiz, libro);
    } else {
        insertar_en_nodo(r, libro);
    }
}

// Búsqueda puntual de disponibilidad por ISBN
Libro* buscar_isbn(NodoBiblioteca* raiz, int isbn_buscado) {
    NodoBiblioteca* actual = raiz;
    while (!actual->es_hoja) {
        int i = 0;
        while (i < actual->num_claves && isbn_buscado >= actual->claves[i]) {
            i++;
        }
        actual = actual->hijos[i];
    }

    for (int i = 0; i < actual->num_claves; i++) {
        if (actual->claves[i] == isbn_buscado) {
            return &actual->libros[i];
        }
    }
    return NULL;
}

// Recorre en línea recta el rango [isbn_inicio, isbn_fin] usando los enlaces entre hojas
void consultar_rango_isbn(NodoBiblioteca* raiz, int isbn_inicio, int isbn_fin) {
    NodoBiblioteca* actual = raiz;

    // 1. Descender desde la raíz hasta la hoja donde inicia el rango
    while (!actual->es_hoja) {
        int i = 0;
        while (i < actual->num_claves && isbn_inicio >= actual->claves[i]) {
            i++;
        }
        actual = actual->hijos[i];
    }

    // 2. Avanzar horizontalmente hoja por hoja sin volver a subir
    printf("\n=== EJEMPLARES EN RANGO ISBN [%d - %d] ===\n", isbn_inicio, isbn_fin);
    while (actual != NULL) {
        for (int i = 0; i < actual->num_claves; i++) {
            int codigo = actual->claves[i];
            if (codigo >= isbn_inicio && codigo <= isbn_fin) {
                Libro l = actual->libros[i];
                printf("ISBN %d | %-28s | %-18s | %s\n",
                       l.isbn, l.titulo, l.autor,
                       l.disponible ? "Disponible" : "Prestado");
            } else if (codigo > isbn_fin) {
                return;
            }
        }
        actual = actual->siguiente;
    }
}

// Serializa todos los libros ordenados en catalogo.dat mediante fwrite
void guardar_catalogo(NodoBiblioteca* raiz) {
    FILE* archivo = fopen("catalogo.dat", "wb");
    if (!archivo) return;

    NodoBiblioteca* hoja = raiz;
    while (!hoja->es_hoja) {
        hoja = hoja->hijos[0];
    }

    while (hoja != NULL) {
        for (int i = 0; i < hoja->num_claves; i++) {
            fwrite(&hoja->libros[i], sizeof(Libro), 1, archivo);
        }
        hoja = hoja->siguiente;
    }

    fclose(archivo);
}

int main() {
    NodoBiblioteca* raiz = crear_nodo(true);

    // 7 libros en desorden para forzar la división de una hoja (límite de 5 claves)
    Libro catalogo[] = {
        {5040, "Estructuras de Datos en C",  "Joyanes Aguilar", true},
        {5010, "Fundamentos de Bases Datos", "Elmasri",         true},
        {5085, "Sistemas Operativos",        "Silberschatz",    false},
        {5002, "Algoritmos Modernos",        "Cormen",          true},
        {5060, "Redes de Computadoras",      "Tanenbaum",       true},
        {5025, "Arquitectura de Software",   "Pressman",        false},
        {5090, "Inteligencia Artificial",    "Russell",         true}
    };

    for (int i = 0; i < 7; i++) {
        insertar_libro(&raiz, catalogo[i]);
    }

    // 1. Consulta puntual de un libro específico
    Libro* encontrado = buscar_isbn(raiz, 5060);
    if (encontrado != NULL) {
        printf("Consulta puntual -> ISBN %d: %s (%s)\n",
               encontrado->isbn, encontrado->titulo,
               encontrado->disponible ? "Disponible" : "Prestado");
    }

    // 2. Consulta secuencial por rango de área temática (ej. 5010 al 5065)
    consultar_rango_isbn(raiz, 5010, 5065);

    // 3. Guardar catálogo ordenado en catalogo.dat y verificar lectura binaria
    guardar_catalogo(raiz);

    FILE* archivo = fopen("catalogo.dat", "rb");
    Libro temp;
    printf("\n=== VERIFICACION EN DISCO (catalogo.dat) ===\n");
    while (fread(&temp, sizeof(Libro), 1, archivo) == 1) {
        printf("ISBN %d - %s | Autor: %s\n", temp.isbn, temp.titulo, temp.autor);
    }
    fclose(archivo);

    return 0;
}
