#include <stdio.h>
#include <stdlib.h>
#ifdef __MINGW32__
#define __USE_MINGW_ANSI_STDIO 1
#endif

typedef long long ll;
typedef struct {
    int n;
    ll *suma;     /* suma real del segmento (ya incluye su propia etiqueta) */
    ll *lazy;     /* incremento pendiente de enviar a los hijos */
    int visitados;
} ArbolLazy;

static void construir(ArbolLazy *t, const int *a, int k, int l, int r) {
    int m;
    t->lazy[k] = 0;
    if (l == r) {
        t->suma[k] = a[l];
        return;
    }
    m = (l + r) / 2;
    construir(t, a, 2 * k, l, m);
    construir(t, a, 2 * k + 1, m + 1, r);
    t->suma[k] = t->suma[2 * k] + t->suma[2 * k + 1];
}

static ArbolLazy *crear(const int *a, int n) {
    ArbolLazy *t = (ArbolLazy *)malloc(sizeof(ArbolLazy));
    t->n = n;
    t->suma = (ll *)malloc(4 * (size_t)n * sizeof(ll));
    t->lazy = (ll *)malloc(4 * (size_t)n * sizeof(ll));
    t->visitados = 0;
    construir(t, a, 1, 0, n - 1);
    return t;
}

static void liberar(ArbolLazy *t) {
    free(t->suma);
    free(t->lazy);
    free(t);
}

/* Aplica "sumar v a todo el segmento [l,r]" al nodo k sin bajar a los hijos */
static void aplicar(ArbolLazy *t, int k, int l, int r, ll v) {
    t->suma[k] += v * (r - l + 1);
    t->lazy[k] += v;
}

/* Envia la etiqueta pendiente del nodo k a sus dos hijos */
static void empujar(ArbolLazy *t, int k, int l, int r) {
    int m;
    if (t->lazy[k] != 0) {
        m = (l + r) / 2;
        aplicar(t, 2 * k, l, m, t->lazy[k]);
        aplicar(t, 2 * k + 1, m + 1, r, t->lazy[k]);
        t->lazy[k] = 0;
    }
}

static void actualizar_rango(ArbolLazy *t, int k, int l, int r,
                             int ql, int qr, ll v) {
    int m;
    t->visitados++;
    if (qr < l || r < ql) return;
    if (ql <= l && r <= qr) {          /* cubierto por completo: se pospone */
        aplicar(t, k, l, r, v);
        return;
    }
    empujar(t, k, l, r);               /* antes de bajar, se resuelve lo pendiente */
    m = (l + r) / 2;
    actualizar_rango(t, 2 * k, l, m, ql, qr, v);
    actualizar_rango(t, 2 * k + 1, m + 1, r, ql, qr, v);
    t->suma[k] = t->suma[2 * k] + t->suma[2 * k + 1];
}

static ll consulta(ArbolLazy *t, int k, int l, int r, int ql, int qr) {
    int m;
    t->visitados++;
    if (qr < l || r < ql) return 0;
    if (ql <= l && r <= qr) return t->suma[k];
    empujar(t, k, l, r);
    m = (l + r) / 2;
    return consulta(t, 2 * k, l, m, ql, qr) +
           consulta(t, 2 * k + 1, m + 1, r, ql, qr);
}

static void imprimir_arbol(ArbolLazy *t, int k, int l, int r, int prof) {
    int i, m;
    for (i = 0; i < prof; i++) printf("    ");
    printf("nodo %2d [%d,%d]  suma=%lld  lazy=%lld\n",
           k, l, r, t->suma[k], t->lazy[k]);
    if (l == r) return;
    m = (l + r) / 2;
    imprimir_arbol(t, 2 * k, l, m, prof + 1);
    imprimir_arbol(t, 2 * k + 1, m + 1, r, prof + 1);
}

/* ---------- Verificacion contra fuerza bruta ---------- */
static int prueba_aleatoria(int n, int operaciones) {
    ll *a;
    int *b;
    ArbolLazy *t;
    int i, op, l, r, x, ok;
    ll v, s;

    a = (ll *)malloc(n * sizeof(ll));
    b = (int *)malloc(n * sizeof(int));
    for (i = 0; i < n; i++) { b[i] = rand() % 201 - 100; a[i] = b[i]; }
    t = crear(b, n);
    ok = 1;
    for (op = 0; op < operaciones && ok; op++) {
        l = rand() % n;
        r = rand() % n;
        if (l > r) { x = l; l = r; r = x; }
        if (rand() % 2) {
            v = rand() % 201 - 100;
            for (i = l; i <= r; i++) a[i] += v;
            actualizar_rango(t, 1, 0, n - 1, l, r, v);
        } else {
            s = 0;
            for (i = l; i <= r; i++) s += a[i];
            if (consulta(t, 1, 0, n - 1, l, r) != s) ok = 0;
        }
    }
    liberar(t); free(a); free(b);
    return ok;
}

int main(void) {
    int a[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int tamanos[6] = {1, 2, 7, 64, 100, 1000};
    int n, i, ok, r;
    ArbolLazy *t;

    n = sizeof(a) / sizeof(a[0]);
    t = crear(a, n);

    printf("=== ARBOL DE SEGMENTOS CON LAZY PROPAGATION ===\n");
    printf("a = [1, 2, 3, 4, 5, 6, 7, 8]\n");
    printf("suma total inicial = %lld\n", consulta(t, 1, 0, n - 1, 0, n - 1));

    printf("\n-- Operacion: sumar 10 a los elementos [2,5] --\n");
    t->visitados = 0;
    actualizar_rango(t, 1, 0, n - 1, 2, 5, 10);
    printf("nodos visitados en la actualizacion: %d\n", t->visitados);
    printf("(las etiquetas lazy distintas de 0 son actualizaciones pendientes)\n");
    imprimir_arbol(t, 1, 0, n - 1, 0);

    printf("\n-- Consultas posteriores --\n");
    t->visitados = 0;
    printf("suma [0,7] = %lld   (esperado 36 + 40 = 76)\n",
           consulta(t, 1, 0, n - 1, 0, 7));
    printf("suma [3,6] = %lld   (esperado 4+5+6+7 + 30 = 52)\n",
           consulta(t, 1, 0, n - 1, 3, 6));
    printf("suma [0,1] = %lld   (esperado 3, sin cambios)\n",
           consulta(t, 1, 0, n - 1, 0, 1));

    printf("\n-- Estado del arbol tras las consultas --\n");
    printf("(la consulta [3,6] obligo a empujar etiquetas hacia abajo)\n");
    imprimir_arbol(t, 1, 0, n - 1, 0);

    printf("\n-- Segunda actualizacion: sumar -4 a [0,7] (todo el arreglo) --\n");
    t->visitados = 0;
    actualizar_rango(t, 1, 0, n - 1, 0, 7, -4);
    printf("nodos visitados: %d (basta con marcar la raiz)\n", t->visitados);
    printf("suma [0,7] = %lld   (esperado 76 - 32 = 44)\n",
           consulta(t, 1, 0, n - 1, 0, 7));
    liberar(t);

    printf("\n-- Verificacion contra fuerza bruta --\n");
    srand(2024);
    ok = 1;
    for (i = 0; i < 6; i++) {
        r = prueba_aleatoria(tamanos[i], 5000);
        printf("n = %4d, 5000 operaciones aleatorias: %s\n",
               tamanos[i], r ? "OK" : "FALLO");
        ok &= r;
    }
    printf("%s\n", ok ? "Todas las pruebas pasaron." : "HAY ERRORES.");
    return ok ? 0 : 1;
}
