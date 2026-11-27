#include <stdio.h>

#include "radix.h"
#include "trie.h"

static void print_word(const char *word, void *ctx) {
    (void)ctx;
    printf("      - %s\n", word);
}

static void title(const char *text) {
    printf("\n==============================================================\n");
    printf(" %s\n", text);
    printf("==============================================================\n");
}

static const char *ok_text(bool ok) {
    return ok ? "OK" : "ERROR";
}

static const char *found_text(bool found) {
    return found ? "ENCONTRADA" : "NO ENCONTRADA";
}

static void stats(const char *name, size_t nodes, size_t words, size_t bytes) {
    printf("  %-6s -> nodos: %3zu | palabras: %2zu | memoria: %5zu bytes\n",
           name, nodes, words, bytes);
}

static void show_stats(const Trie *t, const Radix *r) {
    stats("Trie", t->node_count, t->word_count, trie_memory_bytes(t));
    stats("Radix", r->node_count, r->word_count, radix_memory_bytes(r));
}

static void search_both(const Trie *t, const Radix *r, const char *key) {
    printf("  buscar(\"%s\")  Trie: %-14s Radix: %s\n",
           key, found_text(trie_search(t, key)), found_text(radix_search(r, key)));
}

static void autocomplete_both(const Trie *t, const Radix *r, const char *prefix) {
    printf("\n  Prefijo \"%s\"\n", prefix);
    printf("    Trie:\n");
    size_t n_trie = trie_autocomplete(t, prefix, print_word, NULL);
    printf("    Radix:\n");
    size_t n_radix = radix_autocomplete(r, prefix, print_word, NULL);
    printf("    Resultados -> Trie: %zu | Radix: %zu\n", n_trie, n_radix);
}

static void delete_both(Trie *t, Radix *r, const char *key) {
    bool dt = trie_delete(t, key);
    bool dr = radix_delete(r, key);
    printf("\n  eliminar(\"%s\")  Trie: %s | Radix: %s\n", key, dt ? "eliminada" : "no existia",
           dr ? "eliminada" : "no existia");
    show_stats(t, r);
}

// Figura 1: insercion y busqueda
static void test_insert_search(Trie *t, Radix *r) {
    title("FIGURA 1: Insercion y busqueda");
    const char *words[] = {"pro", "programa", "programador", "proceso",
                           "procesador", "protocolo", "prueba"};
    size_t n = sizeof(words) / sizeof(words[0]);

    for (size_t i = 0; i < n; i++) {
        bool ok_t = trie_insert(t, words[i]);
        bool ok_r = radix_insert(r, words[i]);
        printf("  insertar(\"%s\")  Trie: %s | Radix: %s\n", words[i], ok_text(ok_t), ok_text(ok_r));
    }

    printf("\n");
    const char *queries[] = {"proceso", "programa", "prog", "protocolo", "pr", "procesamiento"};
    size_t q = sizeof(queries) / sizeof(queries[0]);
    for (size_t i = 0; i < q; i++) {
        search_both(t, r, queries[i]);
    }

    printf("\n  Claves invalidas rechazadas:\n");
    printf("  insertar(\"Hola\")  Trie: %s | Radix: %s\n", ok_text(trie_insert(t, "Hola")),
           ok_text(radix_insert(r, "Hola")));
    printf("  insertar(\"\")      Trie: %s | Radix: %s\n", ok_text(trie_insert(t, "")),
           ok_text(radix_insert(r, "")));

    printf("\n");
    show_stats(t, r);
}

// Figura 2: autocompletado
static void test_autocomplete(const Trie *t, const Radix *r) {
    title("FIGURA 2: Consulta por prefijo (autocompletado)");
    autocomplete_both(t, r, "pro");
    autocomplete_both(t, r, "progr");
    autocomplete_both(t, r, "proce");
    autocomplete_both(t, r, "xyz");
}

// Figura 3: eliminacion y poda
static void test_delete(Trie *t, Radix *r) {
    title("FIGURA 3: Eliminacion y poda recursiva");
    printf("\n  Estado inicial:\n");
    show_stats(t, r);

    // Poda de una rama exclusiva: "protocolo" no comparte nada mas alla de "pro"
    delete_both(t, r, "protocolo");

    // En el Radix, "procesador" deja a "proces" con un solo hijo y se fusiona
    delete_both(t, r, "procesador");

    // "pro" es prefijo de otras claves: solo se desmarca, no se libera ningun nodo
    delete_both(t, r, "pro");

    // Eliminar algo que no existe no debe alterar nada
    delete_both(t, r, "inexistente");

    printf("\n  Verificacion de claves que comparten prefijo:\n");
    const char *queries[] = {"protocolo", "procesador", "pro", "proceso", "programa", "programador"};
    size_t q = sizeof(queries) / sizeof(queries[0]);
    for (size_t i = 0; i < q; i++) {
        search_both(t, r, queries[i]);
    }

    printf("\n  Estructura del Radix tras las eliminaciones:\n");
    radix_print(r);
}

// Figura 4: compresion de caminos
static void test_compression(void) {
    title("FIGURA 4: Compresion de caminos (Trie vs Radix)");
    const char *words[] = {"internet", "interno", "internacional",
                           "internacionalizacion", "interactivo"};
    size_t n = sizeof(words) / sizeof(words[0]);

    Trie *t = trie_create();
    Radix *r = radix_create();
    if (t == NULL || r == NULL) {
        printf("  Error de memoria\n");
        trie_destroy(t);
        radix_destroy(r);
        return;
    }

    printf("\n  Claves:");
    for (size_t i = 0; i < n; i++) {
        trie_insert(t, words[i]);
        radix_insert(r, words[i]);
        printf(" %s%s", words[i], i + 1 < n ? "," : "");
    }
    printf("\n\n  Trie estandar (un nodo por caracter):\n");
    trie_print(t);
    printf("\n  Arbol Radix (un nodo por fragmento):\n");
    radix_print(r);

    printf("\n  Comparacion:\n");
    show_stats(t, r);
    printf("  Reduccion de nodos: %.1f%%\n",
           100.0 * (double)(t->node_count - r->node_count) / (double)t->node_count);

    trie_destroy(t);
    radix_destroy(r);
}

int main(void) {
    Trie *t = trie_create();
    Radix *r = radix_create();
    if (t == NULL || r == NULL) {
        fprintf(stderr, "No se pudo reservar memoria\n");
        trie_destroy(t);
        radix_destroy(r);
        return 1;
    }

    test_insert_search(t, r);
    test_autocomplete(t, r);
    test_delete(t, r);
    test_compression();

    trie_destroy(t);
    radix_destroy(r);
    return 0;
}
