#include "radix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Copia una cadena en el heap (strdup no es C estandar)
static char *dup_string(const char *s) {
    size_t len = strlen(s);
    char *copy = malloc(len + 1);
    if (copy != NULL) {
        memcpy(copy, s, len + 1);
    }
    return copy;
}

static RadixNode *node_create(const char *label) {
    RadixNode *node = calloc(1, sizeof(RadixNode));
    if (node == NULL) {
        return NULL;
    }
    node->label = dup_string(label);
    if (node->label == NULL) {
        free(node);
        return NULL;
    }
    return node;
}

static void node_free(RadixNode *node) {
    free(node->label);
    free(node);
}

static size_t count_children(const RadixNode *node) {
    size_t n = 0;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i] != NULL) {
            n++;
        }
    }
    return n;
}

// Longitud del prefijo comun entre a y b
static size_t common_prefix_len(const char *a, const char *b) {
    size_t i = 0;
    while (a[i] != '\0' && a[i] == b[i]) {
        i++;
    }
    return i;
}

Radix *radix_create(void) {
    Radix *r = malloc(sizeof(Radix));
    if (r == NULL) {
        return NULL;
    }
    r->root = node_create("");
    if (r->root == NULL) {
        free(r);
        return NULL;
    }
    r->node_count = 1;
    r->word_count = 0;
    return r;
}

static void destroy_rec(RadixNode *node) {
    if (node == NULL) {
        return;
    }
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        destroy_rec(node->children[i]);
    }
    node_free(node);
}

void radix_destroy(Radix *r) {
    if (r == NULL) {
        return;
    }
    destroy_rec(r->root);
    free(r);
}

// Divide el nodo en la posicion at de su etiqueta:
//   antes:  [abcd] -> hijos           despues:  [ab] -> [cd] -> hijos
// El nuevo nodo (sufijo) hereda los hijos y la marca de fin de palabra.
static bool split_node(Radix *r, RadixNode *node, size_t at) {
    RadixNode *tail = node_create(node->label + at);
    if (tail == NULL) {
        return false;
    }
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        tail->children[i] = node->children[i];
        node->children[i] = NULL;
    }
    tail->is_end_of_word = node->is_end_of_word;
    node->is_end_of_word = false;
    node->label[at] = '\0';  // se trunca despues de haber copiado el sufijo
    node->children[tail->label[0] - 'a'] = tail;
    r->node_count++;
    return true;
}

bool radix_insert(Radix *r, const char *key) {
    if (r == NULL || !key_is_valid(key, false)) {
        return false;
    }
    RadixNode *cur = r->root;
    const char *rest = key;

    while (*rest != '\0') {
        int idx = *rest - 'a';
        RadixNode *child = cur->children[idx];

        // Caso 1: no hay arista con esa letra, se crea una hoja con todo lo que falta
        if (child == NULL) {
            RadixNode *leaf = node_create(rest);
            if (leaf == NULL) {
                return false;
            }
            leaf->is_end_of_word = true;
            cur->children[idx] = leaf;
            r->node_count++;
            r->word_count++;
            return true;
        }

        // Caso 2: coincidencia parcial con la etiqueta, hay que dividir la arista
        size_t common = common_prefix_len(rest, child->label);
        if (common < strlen(child->label)) {
            if (!split_node(r, child, common)) {
                return false;
            }
        }

        // Caso 3: la etiqueta coincide completa, se desciende
        rest += common;
        cur = child;
    }

    if (!cur->is_end_of_word) {
        cur->is_end_of_word = true;
        r->word_count++;
    }
    return true;
}

bool radix_search(const Radix *r, const char *key) {
    if (r == NULL || !key_is_valid(key, false)) {
        return false;
    }
    const RadixNode *cur = r->root;
    const char *rest = key;

    while (*rest != '\0') {
        const RadixNode *child = cur->children[*rest - 'a'];
        if (child == NULL) {
            return false;
        }
        size_t len = strlen(child->label);
        if (strncmp(rest, child->label, len) != 0) {
            return false;
        }
        rest += len;
        cur = child;
    }
    return cur->is_end_of_word;
}

// Fusiona un nodo (sin fin de palabra y con un unico hijo) con ese hijo:
//   [ab] -> [cd] -> hijos     pasa a     [abcd] -> hijos
static void merge_with_only_child(Radix *r, RadixNode *node) {
    RadixNode *only = NULL;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i] != NULL) {
            only = node->children[i];
            break;
        }
    }
    size_t la = strlen(node->label);
    size_t lb = strlen(only->label);
    char *merged = malloc(la + lb + 1);
    if (merged == NULL) {
        return;  // el arbol sigue siendo valido, solo queda sin comprimir
    }
    memcpy(merged, node->label, la);
    memcpy(merged + la, only->label, lb + 1);

    free(node->label);
    node->label = merged;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        node->children[i] = only->children[i];
    }
    node->is_end_of_word = only->is_end_of_word;
    node_free(only);
    r->node_count--;
}

// Elimina rest bajo parent. Al regresar de la recursion repara el hijo:
// lo libera si quedo inutil o lo fusiona si quedo con un solo hijo.
static void remove_rec(Radix *r, RadixNode *parent, const char *rest, bool *removed) {
    int idx = *rest - 'a';
    RadixNode *child = parent->children[idx];
    if (child == NULL) {
        return;
    }
    size_t len = strlen(child->label);
    if (strncmp(rest, child->label, len) != 0) {
        return;
    }
    rest += len;

    if (*rest == '\0') {
        if (!child->is_end_of_word) {
            return;
        }
        child->is_end_of_word = false;
        r->word_count--;
        *removed = true;
    } else {
        remove_rec(r, child, rest, removed);
        if (!*removed) {
            return;
        }
    }

    size_t n = count_children(child);
    if (!child->is_end_of_word && n == 0) {
        node_free(child);
        parent->children[idx] = NULL;
        r->node_count--;
    } else if (!child->is_end_of_word && n == 1) {
        merge_with_only_child(r, child);
    }
}

bool radix_delete(Radix *r, const char *key) {
    if (r == NULL || !key_is_valid(key, false)) {
        return false;
    }
    bool removed = false;
    remove_rec(r, r->root, key, &removed);
    return removed;
}

static void collect(const RadixNode *node, char *buffer, size_t depth,
                    visit_fn visit, void *ctx, size_t *count) {
    if (node->is_end_of_word) {
        buffer[depth] = '\0';
        visit(buffer, ctx);
        (*count)++;
    }
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        const RadixNode *child = node->children[i];
        if (child != NULL) {
            size_t len = strlen(child->label);
            memcpy(buffer + depth, child->label, len);
            collect(child, buffer, depth + len, visit, ctx, count);
        }
    }
}

size_t radix_autocomplete(const Radix *r, const char *prefix, visit_fn visit, void *ctx) {
    if (r == NULL || visit == NULL || !key_is_valid(prefix, true)) {
        return 0;
    }
    char buffer[MAX_WORD_LEN];
    size_t depth = 0;
    const RadixNode *cur = r->root;
    const char *rest = prefix;

    while (*rest != '\0') {
        const RadixNode *child = cur->children[*rest - 'a'];
        if (child == NULL) {
            return 0;
        }
        size_t label_len = strlen(child->label);
        size_t rest_len = strlen(rest);
        size_t cmp_len = rest_len < label_len ? rest_len : label_len;
        if (strncmp(rest, child->label, cmp_len) != 0) {
            return 0;
        }
        // Si el prefijo termina a mitad de arista, igual cuelgan de ese nodo
        // todas las palabras que lo contienen, asi que se toma la etiqueta completa.
        memcpy(buffer + depth, child->label, label_len);
        depth += label_len;
        rest += cmp_len;
        cur = child;
    }

    size_t count = 0;
    collect(cur, buffer, depth, visit, ctx, &count);
    return count;
}

static size_t memory_rec(const RadixNode *node) {
    size_t total = sizeof(RadixNode) + strlen(node->label) + 1;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i] != NULL) {
            total += memory_rec(node->children[i]);
        }
    }
    return total;
}

size_t radix_memory_bytes(const Radix *r) {
    return r == NULL ? 0 : memory_rec(r->root);
}

static void print_rec(const RadixNode *node, int depth) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        const RadixNode *child = node->children[i];
        if (child != NULL) {
            printf("%*s\"%s\"%s\n", depth * 2, "", child->label, child->is_end_of_word ? " *" : "");
            print_rec(child, depth + 1);
        }
    }
}

void radix_print(const Radix *r) {
    printf("(raiz)\n");
    print_rec(r->root, 1);
}
