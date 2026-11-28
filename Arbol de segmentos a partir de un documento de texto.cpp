#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
using namespace std;

const int MAXN = 200; // Limite para 120 o más datos
int A[MAXN];
int Tree[4*MAXN] ;

// Función para cargar los datos desde nuestro archivo de texto hacia el arreglo A
// Devuelve la cantidad real de números leídos (o -1 si hubo error al abrir)
int leerDatosDesdeArchivo(const char* nombreArchivo, int limiteMaximo) {
    ifstream archivo(nombreArchivo);

    // Verificar que el archivo exista y se haya podido abrir
    if (!archivo.is_open()) {
        cout << "Error: No se pudo abrir el archivo '" << nombreArchivo << "'.\n";
        return -1; //Devuelve -1 si hubo error al abrir)
    }

    int n = 0;
    // Leer números secuencialmente hasta llegar al fin del archivo o al límite
    while (n < limiteMaximo && archivo >> A[n]) {
        n++;
    }

    archivo.close();
    return n;
}

void build(int v, int L, int R) {
    // Caso base: nodo hoja (intervalo unitario)
    if (L == R) {
        Tree[v] = A[L];
        return;
    }

    // Paso recursivo: dividir intervalo y construir subárboles
    int M = L + (R - L) / 2;
    build(2 * v, L, M);
    build(2 * v + 1, M + 1, R);

    // Combinación (Merge): consolidar sumas parciales
    Tree[v] = Tree[2 * v] + Tree[2 * v + 1];
}

// Función recursiva para exportar el árbol rotado 90° a un archivo de texto
// v: índice del nodo actual en Tree
// [L, R]: subintervalo cubierto por el nodo

void exportarArbolRotado(ofstream &salida, int v, int L, int R, string prefijo, bool esIzquierdo) {
    int M = L + (R - L) / 2;

    // 1. Procesar primero el hijo derecho (rama superior)
    if (L < R) {
        string nuevoPrefijo = prefijo + (esIzquierdo ? "¦      " : "       ");
        exportarArbolRotado(salida, 2 * v + 1, M + 1, R, nuevoPrefijo, false);
    }

    // 2. Imprimir el nodo actual con su conector correspondiente
    salida << prefijo;
    if (v == 1) {
        salida << "--- "; // Conector inicial para la raíz
    } else {
        salida << (esIzquierdo ? "+-- " : "+-- ");
    }
    salida << "[v=" << v << "] " << Tree[v] << " [" << L << ".." << R << "]\n";

    // 3. Procesar el hijo izquierdo (rama inferior)
    if (L < R) {
        string nuevoPrefijo = prefijo + (esIzquierdo ? "       " : "¦      ");
        exportarArbolRotado(salida, 2 * v, L, M, nuevoPrefijo, true);
    }
}

// 3. Resumen en consola de los niveles superiores (para evidenciar la construcción)
void mostrarResumenConstruccion(int n) {
    int h = (n > 1) ? (int)ceil(log2(n)) : 0;

    cout << "=========================================================\n";
    cout << "  RESUMEN DE CONSTRUCCION - ARBOL DE SEGMENTOS\n";
    cout << "=========================================================\n";
    cout << "Elementos cargados (n)   : " << n << "\n";
    cout << "Altura del arbol (h)     : " << h << " niveles\n";
    cout << "Nodos utiles generados   : " << (2 * n - 1) << " nodos\n";
    cout << "Suma total en raiz Tree[1]: " << Tree[1] << "\n\n";

    // Mostrar los primeros 3 niveles (raíz e hijos principales)
    cout << "--- Niveles superiores (Niveles 0 a 2) ---\n";
    cout << "Nivel 0 [0.." << n - 1 << "] : " << Tree[1] << "\n";

    if (n > 1) {
        int M = (n - 1) / 2;
        cout << "Nivel 1 [0.." << M << "] : " << Tree[2]
             << "  |  [" << M + 1 << ".." << n - 1 << "] : " << Tree[3] << "\n";
    }
    if (n > 3) {
        int M = (n - 1) / 2;
        int M1 = M / 2;
        int M2 = M + 1 + (n - 1 - (M + 1)) / 2;
        cout << "Nivel 2 [0.." << M1 << "] : " << Tree[4]
             << "  |  [" << M1 + 1 << ".." << M << "] : " << Tree[5]
             << "  |  [" << M + 1 << ".." << M2 << "] : " << Tree[6]
             << "  |  [" << M2 + 1 << ".." << n - 1 << "] : " << Tree[7] << "\n";
    }
    cout << "=========================================================\n";
}


int main() {
    int n = leerDatosDesdeArchivo("Datos_para_construccion_de_arbol.txt", MAXN);
    if (n <= 0) return 1;

    // Construir el árbol completo desde la raíz (v = 1) cubriendo [0, n - 1]
    build(1, 0, n - 1);
    
    ofstream archivoSalida("arbol_rotado_90.txt");
    if (archivoSalida.is_open()) {
        archivoSalida << "ARBOL DE SEGMENTOS ROTADO 90 GRADOS (Raiz a la izquierda, hojas a la derecha)\n";
        archivoSalida << "=============================================================================\n";
       exportarArbolRotado(archivoSalida, 1, 0, n - 1, "", false);
        archivoSalida.close();
        cout << "Arbol completo exportado exitosamente a 'arbol_rotado_90.txt'.\n\n";
    } else {
        cout << "Error: No se pudo crear el archivo de salida.\n";
    }

    // Mostrar el resumen verificable en consola
    mostrarResumenConstruccion(n);
    

    return 0; }
