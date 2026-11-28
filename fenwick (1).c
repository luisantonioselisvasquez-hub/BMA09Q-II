#include <stdio.h>
#include <stdlib.h>
#ifdef __MINGW32__
#define __USE_MINGW_ANSI_STDIO 1
#endif

typedef long long ll;
typedef struct {
    int n;
    ll *t;
} Fenwick;
static Fenwick *fw_crear(int n) {
    Fenwick *f = (Fenwick *)malloc(sizeof(Fenwick));
    f->n = n;
    f->t = (ll *)calloc(n + 1, sizeof(ll));
    return f;
}
static void fw_liberar(Fenwick *f) {
    free(f->t);
    free(f);
}
/* cada nodo "se suma" a su padre */
static Fenwick *fw_desde_arreglo(const int *a, int n) {
    Fenwick *f;
    int i, padre;
    f = fw_crear(n);
    for (i = 1; i <= n; i++) {
        f->t[i] += a[i - 1];
        padre = i + (i & -i);
        if (padre <= n) f->t[padre] += f->t[i];
    }
    return f;
}

static void fw_sumar(Fenwick *f, int i, ll delta) {
    for (; i <= f->n; i += i & -i) f->t[i] += delta;
}

static ll fw_prefijo(Fenwick *f, int i) {
    ll s = 0;
    for (; i > 0; i -= i & -i) s += f->t[i];
    return s;
}

static ll fw_rango(Fenwick *f, int l, int r) {
    return fw_prefijo(f, r) - fw_prefijo(f, l - 1);
}

/* ---------- Parte 2: actualizacion y consulta de rango ----------*/

typedef struct {
    int n;
    Fenwick *b1, *b2;
} FenwickRango;

static FenwickRango *fr_crear(int n) {
    FenwickRango *x = (FenwickRango *)malloc(sizeof(FenwickRango));
    x->n = n;
    x->b1 = fw_crear(n);
    x->b2 = fw_crear(n);
    return x;
}

static void fr_liberar(FenwickRango *x) {
    fw_liberar(x->b1);
    fw_liberar(x->b2);
    free(x);
}

static void fr_sumar_rango(FenwickRango *x, int l, int r, ll v) {
    fw_sumar(x->b1, l, v);
    fw_sumar(x->b2, l, v * (l - 1));
    if (r + 1 <= x->n) {
        fw_sumar(x->b1, r + 1, -v);
        fw_sumar(x->b2, r + 1, -v * r);
    }
}

static ll fr_prefijo(FenwickRango *x, int i) {
    return fw_prefijo(x->b1, i) * i - fw_prefijo(x->b2, i);
}

static ll fr_rango(FenwickRango *x, int l, int r) {
    return fr_prefijo(x, r) - fr_prefijo(x, l - 1);
}

static void mostrar_cobertura(Fenwick *f) {
    int i, low, bits, b, x;
    char bin[16];
    printf(" i  binario  i&-i  cubre a[l..r]   t[i]\n");
    bits = 0;
    for (x = f->n; x > 0; x >>= 1) bits++;
    for (i = 1; i <= f->n; i++) {
        low = i & -i;
        for (b = bits - 1; b >= 0; b--) bin[bits - 1 - b] = ((i >> b) & 1) ? '1' : '0';
        bin[bits] = '\0';
        printf("%2d  %-7s  %3d   [%2d..%2d]      %4lld\n",
               i, bin, low, i - low + 1, i, f->t[i]);
    }
}

static void camino_prefijo(int i) {
    int j, primero;
    printf("prefijo(%d): ", i);
    primero = 1;
    for (j = i; j > 0; j -= j & -j) {
        printf("%st[%d]", primero ? "" : " + ", j);
        primero = 0;
    }
    printf("\n");
}

static void camino_actualizacion(int i, int n) {
    int j, primero;
    printf("sumar en %d: se modifican t[", i);
    primero = 1;
    for (j = i; j <= n; j += j & -j) {
        printf("%s%d", primero ? "" : "], t[", j);
        primero = 0;
    }
    printf("]\n");
}

/* ---------- Verificacion contra fuerza bruta ---------- */
static int prueba_aleatoria_puntual(int n, int operaciones) {
    ll *a;
    int *b;
    Fenwick *f;
    int i, op, p, l, r, x, ok;
    ll d, s;

    a = (ll *)calloc(n + 1, sizeof(ll));
    b = (int *)malloc(n * sizeof(int));
    for (i = 0; i < n; i++) { b[i] = rand() % 201 - 100; a[i + 1] = b[i]; }
    f = fw_desde_arreglo(b, n);
    ok = 1;
    for (op = 0; op < operaciones && ok; op++) {
        if (rand() % 2) {
            p = rand() % n + 1;
            d = rand() % 201 - 100;
            a[p] += d;
            fw_sumar(f, p, d);
        } else {
            l = rand() % n + 1;
            r = rand() % n + 1;
            if (l > r) { x = l; l = r; r = x; }
            s = 0;
            for (i = l; i <= r; i++) s += a[i];
            if (fw_rango(f, l, r) != s) ok = 0;
        }
    }
    fw_liberar(f); free(a); free(b);
    return ok;
}

static int prueba_aleatoria_rango(int n, int operaciones) {
    ll *a;
    FenwickRango *x;
    int i, op, l, r, t, ok;
    ll v, s;

    a = (ll *)calloc(n + 1, sizeof(ll));
    x = fr_crear(n);
    ok = 1;
    for (op = 0; op < operaciones && ok; op++) {
        l = rand() % n + 1;
        r = rand() % n + 1;
        if (l > r) { t = l; l = r; r = t; }
        if (rand() % 2) {
            v = rand() % 201 - 100;
            for (i = l; i <= r; i++) a[i] += v;
            fr_sumar_rango(x, l, r, v);
        } else {
            s = 0;
            for (i = l; i <= r; i++) s += a[i];
            if (fr_rango(x, l, r) != s) ok = 0;
        }
    }
    fr_liberar(x); free(a);
    return ok;
}

int main(void) {
    int a[8] = {5, 2, 8, 1, 9, 3, 7, 4};
    int base[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int tamanos[6] = {1, 2, 7, 64, 100, 1000};
    int n, i, ok, r1, r2;
    Fenwick *f;
    FenwickRango *x;

    n = sizeof(a) / sizeof(a[0]);

    printf("=== ARBOL DE FENWICK ===\n");
    printf("a (indices 1..8) = [5, 2, 8, 1, 9, 3, 7, 4]\n\n");
    f = fw_desde_arreglo(a, n);

    printf("-- Arreglo interno t[] y el tramo que cubre cada posicion --\n");
    mostrar_cobertura(f);

    printf("\n-- Consultas de prefijo --\n");
    camino_prefijo(7);
    printf("   = %lld\n", fw_prefijo(f, 7));
    camino_prefijo(6);
    printf("   = %lld\n", fw_prefijo(f, 6));
    printf("suma a[3..6] = prefijo(6) - prefijo(2) = %lld - %lld = %lld\n",
           fw_prefijo(f, 6), fw_prefijo(f, 2), fw_rango(f, 3, 6));

    printf("\n-- Actualizacion puntual: a[3] += 12 (de 8 pasa a 20) --\n");
    camino_actualizacion(3, n);
    fw_sumar(f, 3, 12);
    printf("suma a[1..8] = %lld\n", fw_prefijo(f, 8));
    printf("suma a[3..6] = %lld\n", fw_rango(f, 3, 6));
    fw_liberar(f);

    printf("\n=== FENWICK CON ACTUALIZACION DE RANGO ===\n");
    x = fr_crear(8);
    for (i = 1; i <= 8; i++) fr_sumar_rango(x, i, i, base[i - 1]);
    printf("a = [1, 2, 3, 4, 5, 6, 7, 8]   suma[1..8] = %lld\n", fr_rango(x, 1, 8));
    printf("sumar 10 a [3,6]\n");
    fr_sumar_rango(x, 3, 6, 10);
    printf("suma[1..8] = %lld   (esperado 36 + 40 = 76)\n", fr_rango(x, 1, 8));
    printf("suma[4..7] = %lld   (esperado 4+5+6+7 + 30 = 52)\n", fr_rango(x, 4, 7));
    fr_liberar(x);

    printf("\n-- Verificacion contra fuerza bruta --\n");
    srand(2024);
    ok = 1;
    for (i = 0; i < 6; i++) {
        r1 = prueba_aleatoria_puntual(tamanos[i], 5000);
        r2 = prueba_aleatoria_rango(tamanos[i], 5000);
        printf("n = %4d  puntual: %-5s  rango: %s\n", tamanos[i],
               r1 ? "OK" : "FALLO", r2 ? "OK" : "FALLO");
        ok &= r1 && r2;
    }
    printf("%s\n", ok ? "Todas las pruebas pasaron." : "HAY ERRORES.");
    return ok ? 0 : 1;
}
