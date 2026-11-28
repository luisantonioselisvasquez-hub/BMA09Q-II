#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace std;

const int MAXN = 32;
int A[MAXN];
int Tree[4 * MAXN];
char lienzo[30][200]; // Matriz de caracteres para dibujar la pirámide
void update(int v, int L, int R, int idx, int val) {
    if (L == R) {
        A[idx] = val;
        Tree[v] = val;
        return;
    }

    int M = L + (R - L) / 2;
    if (idx <= M) {
        update(2 * v, L, M, idx, val);
    } else {
        update(2 * v + 1, M + 1, R, idx, val);
    }

    Tree[v] = Tree[2 * v] + Tree[2 * v + 1];
}

// Función auxiliar para buscar el primer elemento con 'valorBuscar' y reemplazarlo
bool actualizarPorValor(int n, int valorBuscar, int nuevoValor) {
    int idx = -1;
    // Buscar la primera coincidencia de izquierda a derecha
    for (int i = 0; i < n; i++) {
        if (A[i] == valorBuscar) {
            idx = i;
            break; // Detenerse al encontrar el primero
        }
    }

    // Si no se encontró el valor en el arreglo, avisar y salir
    if (idx == -1) {
        return false;
    }

    // Actualizar la hoja y propagar el cambio hacia la raíz
    update(1, 0, n - 1, idx, nuevoValor);
    return true;
}
// 1. Construcción del árbol (Build)
void build(int v, int L, int R) {
    if (L == R) {
        Tree[v] = A[L];
        return;
    }
    int M = L + (R - L) / 2;
    build(2 * v, L, M);
    build(2 * v + 1, M + 1, R);
    Tree[v] = Tree[2 * v] + Tree[2 * v + 1];
}

// 2. Función recursiva para ubicar nodos y ramas en el lienzo
void dibujarNodo(int v, int L, int R, int fila, int col, int sep) {
    // Escribir el valor numérico del nodo centrado en (fila, col)
    char texto[16];
    sprintf(texto, "%d", Tree[v]);
    int len = strlen(texto);
    int inicio = col - len / 2;
    for (int i = 0; i < len; i++) {
        if (inicio + i >= 0 && inicio + i < 200) {
            lienzo[fila][inicio + i] = texto[i];
        }
    }

    // Si es hoja, no dibuja ramas inferiores
    if (L == R) return;

    int M = L + (R - L) / 2;
    int siguienteSep = (sep > 2) ? sep / 2 : 2;

    // Dibujar ramas diagonales hacia los hijos
    lienzo[fila + 1][col - sep / 2] = '/';
    lienzo[fila + 1][col + sep / 2] = '\\';

    // Llamadas recursivas para ubicar hijo izquierdo y derecho
    dibujarNodo(2 * v, L, M, fila + 2, col - sep, siguienteSep);
    dibujarNodo(2 * v + 1, M + 1, R, fila + 2, col + sep, siguienteSep);
}

// 3. Controlador de impresión
void imprimirArbol(int n) {
    int h = (n > 1) ? (int)ceil(log2(n)) : 0;
    int totalFilas = 2 * h + 1;
    int sepInicial = 1 << h; // Equivalente a 2^h espacios de separación
    int colRaiz = sepInicial * 2;

    // Limpiar el lienzo con espacios en blanco
    for (int i = 0; i < totalFilas; i++) {
        for (int j = 0; j < 200; j++) {
            lienzo[i][j] = ' ';
        }
        lienzo[i][199] = '\0';
    }

    // Llenar el lienzo desde la raíz (v = 1)
    dibujarNodo(1, 0, n - 1, 0, colRaiz, sepInicial);

    // Imprimir cada fila eliminando espacios sobrantes a la derecha
    for (int i = 0; i < totalFilas; i++) {
        int ultimoChar = 198;
        while (ultimoChar >= 0 && lienzo[i][ultimoChar] == ' ') {
            ultimoChar--;
        }
        lienzo[i][ultimoChar + 1] = '\0';
        cout << lienzo[i] << "\n";
    }
}

int main() {
    int n = 16;
    int datos[] = {3, 2, 5, 10, 12, 45, 33, 14, 8, 21, 7, 19, 4, 11, 25, 6};
    for (int i = 0; i < n; i++) {
        A[i] = datos[i];
    }

    build(1, 0, n - 1);

    int opcion;
    do {
        cout << "\n--- MENU ARBOL DE SEGMENTOS ---\n";
        cout << "1. Mostrar el arbol\n";
        cout << "2. Realizar update (por valor)\n";
        cout << "3. Cerrar\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        switch (opcion) {
            case 1:
                cout << "\nEstado actual del arbol:\n\n";
                imprimirArbol(n);
                break;

            case 2: {
                int buscar, reemplazo;
                cout << "\nIngrese el valor que desea reemplazar: ";
                cin >> buscar;
                cout << "Ingrese el nuevo valor: ";
                cin >> reemplazo;

                if (actualizarPorValor(n, buscar, reemplazo)) {
                    cout << "\nActualizacion exitosa. Nuevo arbol:\n\n";
                    imprimirArbol(n);
                } else {
                    cout << "\nAlerta: El valor " << buscar << " no existe en el arreglo.\n";
                }
                break;
            }

            case 3:
                cout << "\nCerrando programa...\n";
                break;

            default:
                cout << "\nOpcion invalida. Intente nuevamente con 1, 2 o 3.\n";
                break;
        }
    } while (opcion != 3);

    return 0;
}
