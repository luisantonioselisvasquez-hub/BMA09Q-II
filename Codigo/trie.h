#ifndef TRIE_H
#define TRIE_H

#include <stdbool.h>
#include <stddef.h>

// Tamano del alfabeto Sigma: letras minusculas 'a'-'z'
#define ALPHABET_SIZE 26

// Longitud maxima de una clave incluyendo el '\0'. Acota los buffers de recorrido.
#define MAX_WORD_LEN 64

// Funcion que se invoca por cada palabra hallada en un autocompletado
typedef void (*visit_fn)(const char *word, void *ctx);

typedef struct TrieNode {
    struct TrieNode *children[ALPHABET_SIZE];  // arista implicita: indice = c - 'a'
    bool is_end_of_word;                       // true si el camino hasta aqui es una clave
} TrieNode;

typedef struct {
    TrieNode *root;
    size_t node_count;  // cantidad de nodos (incluye la raiz)
    size_t word_count;  // cantidad de claves almacenadas
} Trie;

// Valida que la clave solo tenga letras 'a'-'z' y longitud < MAX_WORD_LEN.
// allow_empty permite la cadena vacia (util para prefijos).
bool key_is_valid(const char *key, bool allow_empty);

Trie *trie_create(void);
void trie_destroy(Trie *t);

// O(L): inserta la clave. Devuelve false si la clave es invalida o falla malloc.
bool trie_insert(Trie *t, const char *key);

// O(L): true solo si la clave completa existe como palabra.
bool trie_search(const Trie *t, const char *key);

// O(L): elimina la clave con poda recursiva. Devuelve true si la clave existia.
bool trie_delete(Trie *t, const char *key);

// O(L_p + K): llama a visit por cada palabra que empieza con prefix, en orden
// lexicografico. Devuelve la cantidad de palabras encontradas.
size_t trie_autocomplete(const Trie *t, const char *prefix, visit_fn visit, void *ctx);

// Memoria ocupada por los nodos, en bytes.
size_t trie_memory_bytes(const Trie *t);

// Imprime el arbol indentado; '*' marca fin de palabra.
void trie_print(const Trie *t);

#endif
