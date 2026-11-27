#ifndef RADIX_H
#define RADIX_H

#include "trie.h"  // reutiliza ALPHABET_SIZE, MAX_WORD_LEN, visit_fn y key_is_valid

typedef struct RadixNode {
    char *label;                                // fragmento de la arista que llega a este nodo
    struct RadixNode *children[ALPHABET_SIZE];  // se indexa por la primera letra de la etiqueta hija
    bool is_end_of_word;
} RadixNode;

typedef struct {
    RadixNode *root;    // la raiz tiene etiqueta vacia
    size_t node_count;  // incluye la raiz
    size_t word_count;
} Radix;

Radix *radix_create(void);
void radix_destroy(Radix *r);

// O(L): inserta la clave dividiendo aristas cuando hay coincidencia parcial.
bool radix_insert(Radix *r, const char *key);

// O(L): true solo si la clave completa existe como palabra.
bool radix_search(const Radix *r, const char *key);

// O(L): elimina la clave y vuelve a fusionar nodos con un solo hijo.
bool radix_delete(Radix *r, const char *key);

// O(L_p + K): palabras que empiezan con prefix, en orden lexicografico.
size_t radix_autocomplete(const Radix *r, const char *prefix, visit_fn visit, void *ctx);

// Memoria de nodos mas memoria de las etiquetas, en bytes.
size_t radix_memory_bytes(const Radix *r);

// Imprime el arbol indentado mostrando las etiquetas; '*' marca fin de palabra.
void radix_print(const Radix *r);

#endif
