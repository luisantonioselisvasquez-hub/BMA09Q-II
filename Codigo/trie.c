#include "trie.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool key_is_valid(const char *key, bool allow_empty) {
    if (key == NULL) {
        return false;
    }
    size_t len = strlen(key);
    if (len >= MAX_WORD_LEN) {
        return false;
    }
    if (len == 0) {
        return allow_empty;
    }
    for (size_t i = 0; i < len; i++) {
        if (key[i] < 'a' || key[i] > 'z') {
            return false;
        }
    }
    return true;
}

static TrieNode *node_create(void) {
    // calloc deja los punteros en NULL e is_end_of_word en false
    return calloc(1, sizeof(TrieNode));
}

static bool has_children(const TrieNode *node) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i] != NULL) {
            return true;
        }
    }
    return false;
}

Trie *trie_create(void) {
    Trie *t = malloc(sizeof(Trie));
    if (t == NULL) {
        return NULL;
    }
    t->root = node_create();
    if (t->root == NULL) {
        free(t);
        return NULL;
    }
    t->node_count = 1;
    t->word_count = 0;
    return t;
}

// Liberacion en post-orden: primero los hijos, luego el nodo
static void destroy_rec(TrieNode *node) {
    if (node == NULL) {
        return;
    }
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        destroy_rec(node->children[i]);
    }
    free(node);
}

void trie_destroy(Trie *t) {
    if (t == NULL) {
        return;
    }
    destroy_rec(t->root);
    free(t);
}

bool trie_insert(Trie *t, const char *key) {
    if (t == NULL || !key_is_valid(key, false)) {
        return false;
    }
    TrieNode *cur = t->root;
    for (size_t i = 0; key[i] != '\0'; i++) {
        int idx = key[i] - 'a';
        if (cur->children[idx] == NULL) {
            cur->children[idx] = node_create();
            if (cur->children[idx] == NULL) {
                return false;
            }
            t->node_count++;
        }
        cur = cur->children[idx];
    }
    if (!cur->is_end_of_word) {
        cur->is_end_of_word = true;
        t->word_count++;
    }
    return true;
}

// Recorre el prefijo y devuelve el nodo donde termina (o NULL si no existe)
static const TrieNode *find_node(const Trie *t, const char *prefix) {
    const TrieNode *cur = t->root;
    for (size_t i = 0; prefix[i] != '\0'; i++) {
        cur = cur->children[prefix[i] - 'a'];
        if (cur == NULL) {
            return NULL;
        }
    }
    return cur;
}

bool trie_search(const Trie *t, const char *key) {
    if (t == NULL || !key_is_valid(key, false)) {
        return false;
    }
    const TrieNode *node = find_node(t, key);
    return node != NULL && node->is_end_of_word;
}

// Devuelve true si el nodo recibido puede ser liberado por su padre.
// removed se pone en true solo si la clave existia y se desmarco.
static bool delete_rec(Trie *t, TrieNode *node, const char *key, size_t depth, bool *removed) {
    if (key[depth] == '\0') {
        if (!node->is_end_of_word) {
            return false;  // la clave no existe como palabra
        }
        node->is_end_of_word = false;
        t->word_count--;
        *removed = true;
        return !has_children(node);
    }

    int idx = key[depth] - 'a';
    TrieNode *child = node->children[idx];
    if (child == NULL) {
        return false;
    }

    if (delete_rec(t, child, key, depth + 1, removed)) {
        free(child);
        node->children[idx] = NULL;
        t->node_count--;
        // Poda hacia arriba solo si este nodo tampoco sirve a otra clave
        return !node->is_end_of_word && !has_children(node);
    }
    return false;
}

bool trie_delete(Trie *t, const char *key) {
    if (t == NULL || !key_is_valid(key, false)) {
        return false;
    }
    bool removed = false;
    delete_rec(t, t->root, key, 0, &removed);  // la raiz nunca se libera
    return removed;
}

static void collect(const TrieNode *node, char *buffer, size_t depth,
                    visit_fn visit, void *ctx, size_t *count) {
    if (node->is_end_of_word) {
        buffer[depth] = '\0';
        visit(buffer, ctx);
        (*count)++;
    }
    // Recorrer 'a'..'z' en orden garantiza salida lexicografica
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i] != NULL) {
            buffer[depth] = (char)('a' + i);
            collect(node->children[i], buffer, depth + 1, visit, ctx, count);
        }
    }
}

size_t trie_autocomplete(const Trie *t, const char *prefix, visit_fn visit, void *ctx) {
    if (t == NULL || visit == NULL || !key_is_valid(prefix, true)) {
        return 0;
    }
    const TrieNode *start = find_node(t, prefix);
    if (start == NULL) {
        return 0;
    }
    char buffer[MAX_WORD_LEN];
    size_t depth = strlen(prefix);
    memcpy(buffer, prefix, depth);
    size_t count = 0;
    collect(start, buffer, depth, visit, ctx, &count);
    return count;
}

size_t trie_memory_bytes(const Trie *t) {
    return t == NULL ? 0 : t->node_count * sizeof(TrieNode);
}

static void print_rec(const TrieNode *node, int depth) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        const TrieNode *child = node->children[i];
        if (child != NULL) {
            printf("%*s%c%s\n", depth * 2, "", 'a' + i, child->is_end_of_word ? " *" : "");
            print_rec(child, depth + 1);
        }
    }
}

void trie_print(const Trie *t) {
    printf("(raiz)\n");
    print_rec(t->root, 1);
}
