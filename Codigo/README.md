# Grupo 6: Tries y árboles radix

Implementación en C (estándar C99) de un **Trie estándar** y un **Árbol Radix (Patricia Trie)**, con las operaciones de inserción, búsqueda exacta, eliminación con poda, autocompletado por prefijo y estadísticas de memoria. El informe con el análisis de complejidad, la comparación con tablas hash y las aplicaciones está en la carpeta `../Informe/`.

**Curso:** Algoritmos y Estructura de Datos I (BMA09Q) · **Docente:** M. Sc. Luis Antonio Selis Vasquez · **Ciclo:** 2026-2

**Integrantes**

- Ibarra Sanchez, Jean Paul Fernando
- Valencia Casanova, Renzo Leiser
- Romero Cerron, Byron Samir

## Requisitos

- Compilador de C con soporte para C99 (`gcc`)
- Opción A (terminal): Linux, macOS o WSL en Windows, con `make`
- Opción B (Windows): Dev-C++ con un proyecto de consola en C

Las capturas del informe se obtuvieron compilando en Dev-C++ (TDM-GCC 4.9.2, 64 bits) sobre Windows.

## Compilar y ejecutar

### Opción A: con make (Linux, macOS o WSL)

Desde esta carpeta (`codigo/`):

```bash
make          # compila y genera el ejecutable trie_demo
make run      # compila (si hace falta) y ejecuta las pruebas
make clean    # borra los archivos generados
```

La compilación usa `-std=c99 -Wall -Wextra -Wpedantic` y no produce advertencias.

### Opción B: con Dev-C++ (Windows)

1. *Archivo → Nuevo → Proyecto → Console Application*, y elegir **Proyecto C**.
2. Quitar del proyecto el `main.c` de ejemplo y agregar los cinco archivos de esta carpeta: `main.c`, `trie.c`, `radix.c`, `trie.h` y `radix.h` (*Proyecto → Agregar al proyecto*).
3. Compilar y ejecutar con `F11`.

El `Makefile` no se usa en Dev-C++: el propio proyecto se encarga de la compilación.

Si aparecen errores como `invalid conversion from 'void*' to 'char*'`, o el aviso `'-std=c11' is valid for C/ObjC but not for C++`, el proyecto se creó como **C++** y Dev-C++ está usando el compilador de C++. Hay que crear el proyecto de nuevo y marcar la opción **C Project** (abajo a la izquierda de la ventana de nuevo proyecto), no *C++ Project*.

Si aparece el error `'for' loop initial declarations are only allowed in C99 mode`, hay que activar C99: *Herramientas → Opciones del compilador → Configuración → Generación de código → Language standard (-std)* y elegir **ISO C99**.

## Archivos

| Archivo | Contenido |
|---|---|
| `trie.h` / `trie.c` | Trie estándar: nodo con 26 punteros (`a`-`z`) e indicador de fin de palabra. Incluye la validación de claves que usan ambas estructuras. |
| `radix.h` / `radix.c` | Árbol Radix: nodos con etiqueta de arista, división de nodos al insertar y fusión al eliminar. |
| `main.c` | Programa de pruebas que ejecuta las mismas operaciones sobre las dos estructuras y compara resultados. |
| `Makefile` | Reglas de compilación (`all`, `run`, `clean`). |

## Qué muestra la ejecución

`make run` imprime cuatro secciones, que corresponden a las figuras 1 a 4 del informe:

1. **Inserción y búsqueda:** inserta 7 palabras, busca claves existentes y no existentes, y rechaza claves inválidas (mayúsculas y cadena vacía).
2. **Autocompletado:** consulta por prefijo (`pro`, `progr`, `proce`, `xyz`) con resultados en orden lexicográfico en ambas estructuras.
3. **Eliminación y poda:** elimina claves y muestra cómo cambia el número de nodos, y la estructura final del Radix.
4. **Compresión de caminos:** compara el Trie y el Radix con cadenas largas de prefijo común (30 nodos contra 8, 73,3 % menos).

## Restricciones de la implementación

- Solo se aceptan claves de letras minúsculas `a`-`z` y de menos de 64 caracteres.
- Las claves inválidas se rechazan y las operaciones devuelven `false`.
- No se miden tiempos de ejecución; la complejidad temporal se sustenta de forma analítica en el informe.
