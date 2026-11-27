
#ifndef DSU_H
#define DSU_H

/*
    ============================================================
    1. BIBLIOTECAS
    ============================================================

*/

#include <stdio.h>
#include <stdlib.h>


/*
    ============================================================
    2. DEFINICION DE LA ESTRUCTURA DSU
    ============================================================
*/

typedef struct {

    int *parent;
    /*
        Arreglo de padres.

        parent[i] almacena el padre del elemento i.

        Si parent[i] == i, significa que i es una raíz.
    */


    int *rank;
    /*
        Arreglo de rangos.
    */


    int *size;
    /*
        Arreglo que almacena el tamaño de cada conjunto.
    */


    int *active;
    /*
        Arreglo que indica si una computadora está activa.
    */


    int n;
    /*
        Cantidad de elementos que hemos agregado.
    */


    int capacity;
    /*
        Capacidad máxima de memoria reservada actualmente.
    */


    int components;
    /*
        Número de conjuntos independientes existentes.

    */

} DSU;



/*
    ============================================================
    3. CREAR LA ESTRUCTURA DSU
    ============================================================

    crearDSU() reserva memoria para administrar nuestros conjuntos.

    Recibe:
        capacidadInicial:
        Cantidad de posiciones que queremos reservar inicialmente.

    Devuelve:
        Un puntero a la estructura DSU si todo sale bien.
        NULL si no se pudo reservar memoria.
*/

DSU *crearDSU(int capacidadInicial) {

    DSU *dsu = (DSU *)malloc(sizeof(DSU));
    if (dsu == NULL)
        return NULL;


    if (capacidadInicial < 1)
        capacidadInicial = 4;


    /*
        Inicializamos los contadores.

    */

    dsu->n = 0;
    dsu->capacity = capacidadInicial;
    dsu->components = 0;


    dsu->parent = (int *)malloc(
        capacidadInicial * sizeof(int));

    dsu->rank = (int *)malloc(
        capacidadInicial * sizeof(int));

    dsu->size = (int *)malloc(
        capacidadInicial * sizeof(int));

    dsu->active = (int *)malloc(
        capacidadInicial * sizeof(int));


    /*
        Comprobamos si alguno de los cuatro arreglos
        no pudo reservarse correctamente.

    */

    if (dsu->parent == NULL ||
        dsu->rank == NULL ||
        dsu->size == NULL ||
        dsu->active == NULL) {

        free(dsu->parent);
        free(dsu->rank);
        free(dsu->size);
        free(dsu->active);

        // Liberamos también la estructura principal.
        free(dsu);

        // Informamos que la creación falló.
        return NULL;
    }

    /*
        Si llegamos aquí, todas las reservas de memoria
        fueron exitosas.

        Devolvemos la dirección de nuestra estructura.
    */

    return dsu;
}



/*
    ============================================================
    4. AUMENTAR LA CAPACIDAD DE MEMORIA
    ============================================================

    ampliarDSU() aumenta el espacio disponible.

*/

int ampliarDSU(DSU *dsu) {

    // Duplicamos la capacidad actual.
    int nuevaCapacidad = dsu->capacity * 2;


    /*
        Reservamos cuatro arreglos nuevos con mayor capacidad.

    */

    int *nuevoParent = (int *)malloc(
        nuevaCapacidad * sizeof(int));

    int *nuevoRank = (int *)malloc(
        nuevaCapacidad * sizeof(int));

    int *nuevoSize = (int *)malloc(
        nuevaCapacidad * sizeof(int));

    int *nuevoActive = (int *)malloc(
        nuevaCapacidad * sizeof(int));


    /*
        Comprobamos que todas las reservas hayan funcionado.

        Si alguna falla, liberamos los arreglos nuevos.
        Los arreglos originales todavía permanecen intactos.
    */

    if (nuevoParent == NULL ||
        nuevoRank == NULL ||
        nuevoSize == NULL ||
        nuevoActive == NULL) {

        free(nuevoParent);
        free(nuevoRank);
        free(nuevoSize);
        free(nuevoActive);

        // 0 significa que no se pudo ampliar.
        return 0;
    }


    /*
        Copiamos los elementos antiguos a los arreglos nuevos.

        El ciclo comienza en 0 y termina en n - 1.

        No copiamos toda la capacidad, porque algunas
        posiciones todavía no contienen elementos.
    */

    int i;

    for (i = 0; i < dsu->n; i++) {

        nuevoParent[i] = dsu->parent[i];
        nuevoRank[i] = dsu->rank[i];
        nuevoSize[i] = dsu->size[i];
        nuevoActive[i] = dsu->active[i];

    }


    /*
        Ahora que los datos están copiados,
        podemos liberar los arreglos antiguos.

        Es importante hacer esto después de copiar,
        para no perder la información.
    */

    free(dsu->parent);
    free(dsu->rank);
    free(dsu->size);
    free(dsu->active);


    /*
        Actualizamos los punteros de la estructura.

        Ahora apuntan a los arreglos nuevos.
    */

    dsu->parent = nuevoParent;
    dsu->rank = nuevoRank;
    dsu->size = nuevoSize;
    dsu->active = nuevoActive;


    // Guardamos la nueva capacidad.
    dsu->capacity = nuevaCapacidad;


    // 1 significa que la ampliación fue exitosa.
    return 1;
}



/*
    ============================================================
    5. AGREGAR UNA COMPUTADORA
    ============================================================

    Cada computadora nueva comienza formando su propio conjunto.


    Cada una tiene su propio padre: ella misma.
*/

int agregarComputadora(DSU *dsu) {


    /*
        Comprobamos si ya utilizamos toda la capacidad.

        Si n == capacity, significa que no hay espacio libre.
    */

    if (dsu->n == dsu->capacity) {

        /*
            Intentamos ampliar la memoria.

        */

        if (!ampliarDSU(dsu))
            return -1;
    }


    /*
        El nuevo identificador será igual a n.

    */

    int id = dsu->n;


    /*
        Inicializamos los datos de la nueva computadora.

        parent[id] = id:
        La computadora es su propio padre y su propia raíz.

        rank[id] = 0:
        Su árbol tiene altura inicial cero.

        size[id] = 1:
        El grupo contiene solamente una computadora.

        active[id] = 1:
        La computadora comienza activa.
    */

    dsu->parent[id] = id;
    dsu->rank[id] = 0;
    dsu->size[id] = 1;
    dsu->active[id] = 1;


    /*
        Incrementamos la cantidad de elementos registrados.

        También incrementamos el número de componentes,
        porque acabamos de crear un conjunto independiente.
    */

    dsu->n++;
    dsu->components++;


    /*
        Devolvemos el identificador asignado.

        Por ejemplo, si agregamos la primera computadora,
        devolverá 0.
    */

    return id;
}



/*
    ============================================================
    6. COMPROBAR SI UNA COMPUTADORA ES VALIDA
    ============================================================

    Esta función comprueba tres condiciones:

    1. El ID no puede ser negativo.
    2. El ID debe ser menor que n.
    3. La computadora debe estar activa.
*/

int computadoraValida(DSU *dsu, int id) {

    return id >= 0 &&
           id < dsu->n &&
           dsu->active[id] == 1;

}



/*
    ============================================================
    7. FIND: ENCONTRAR LA RAIZ
    ============================================================.
*/

int find(DSU *dsu, int x) {

    if (dsu->parent[x] != x) {

        dsu->parent[x] = find(dsu, dsu->parent[x]);

    }

    return dsu->parent[x];
}



/*
    ============================================================
    8. UNION: UNIR DOS CONJUNTOS
    ============================================================

    unir() conecta dos conjuntos diferentes.

    Utiliza dos optimizaciones:

    1. Unión por rango:
       Intenta mantener los árboles equilibrados.

    2. Actualización del tamaño:
       Suma los elementos de ambos conjuntos.

    Devuelve:
        1 si consiguió unir dos conjuntos diferentes.
        0 si ya pertenecían al mismo conjunto.
*/

int unir(DSU *dsu, int a, int b) {


    /*
        Primero buscamos las raíces de ambos elementos.

        No debemos unir directamente a y b.

        Debemos unir sus raíces.
    */

    int raizA = find(dsu, a);
    int raizB = find(dsu, b);


    /*
        Si las raíces son iguales, ambos elementos
        ya pertenecen al mismo conjunto.

        No necesitamos realizar ninguna unión.

        Devolvemos 0.
    */

    if (raizA == raizB)
        return 0;


    /*
        ========================================================
        CASO 1: LA RAIZ A TIENE MENOR RANGO
        ========================================================
    */

    if (dsu->rank[raizA] < dsu->rank[raizB]) {

        dsu->parent[raizA] = raizB;

        dsu->size[raizB] += dsu->size[raizA];

    }


    /*
        ========================================================
        CASO 2: LA RAIZ A TIENE MAYOR RANGO
        ========================================================

    */

    else if (dsu->rank[raizA] > dsu->rank[raizB]) {

        dsu->parent[raizB] = raizA;

        dsu->size[raizA] += dsu->size[raizB];

    }


    /*
        ========================================================
        CASO 3: AMBAS RAICES TIENEN EL MISMO RANGO
        ========================================================

    */

    else {

        dsu->parent[raizB] = raizA;

        dsu->size[raizA] += dsu->size[raizB];

        dsu->rank[raizA]++;
    }


    /*
        Como unimos dos conjuntos diferentes,
        ahora tenemos un conjunto menos.

        Ejemplo:

        Antes: 3 conjuntos.
        Después: 2 conjuntos.
    */

    dsu->components--;


    /*
        Indicamos que la unión se realizó correctamente.
    */

    return 1;
}



/*
    ============================================================
    9. CONSULTAR CONECTIVIDAD
    ============================================================

    Esta función responde:

    ¿Los elementos a y b pertenecen al mismo conjunto?
*/

int conectado(DSU *dsu, int a, int b) {

    /*
        Buscamos la raíz de ambos elementos.

        Si las raíces son iguales, pertenecen al mismo grupo.

        Si son diferentes, pertenecen a grupos diferentes.
    */

    return find(dsu, a) == find(dsu, b);
}



/*
    ============================================================
    10. REINICIAR TODOS LOS CONJUNTOS
    ============================================================

*/

void reiniciarDSU(DSU *dsu) {

    int i;


    /*
        Inicializamos el contador de componentes en cero.

        Lo volveremos a calcular recorriendo los elementos.
    */

    dsu->components = 0;


    /*
        Recorremos todos los elementos registrados.

        i representa el identificador de cada computadora.
    */

    for (i = 0; i < dsu->n; i++) {


        /*
            Cada computadora vuelve a ser su propio padre.

            Esto elimina las conexiones anteriores.
        */

        dsu->parent[i] = i;


        /*
            Reiniciamos el rango.

            Todos los árboles vuelven a tener altura cero.
        */

        dsu->rank[i] = 0;


        /*
            Cada computadora vuelve a tener tamaño 1.

            Porque ahora cada una constituye su propio conjunto.
        */

        dsu->size[i] = 1;


        /*
            Solamente contamos las computadoras activas.

            Si está activa, forma un conjunto independiente.
        */

        if (dsu->active[i] == 1) {

            dsu->components++;

        }

    }
}



/*
    ============================================================
    11. DESACTIVAR UNA COMPUTADORA
    ============================================================


   
*/

int desactivarComputadora(DSU *dsu, int id) {


    /*
        Primero verificamos si la computadora existe
        y se encuentra activa.

        Si no cumple las condiciones, devolvemos 0.
    */

    if (!computadoraValida(dsu, id))
        return 0;


    /*
        Cambiamos su estado de activa a inactiva.
    */

    dsu->active[id] = 0;


    /*
        Indicamos que la desactivación se realizó.
    */

    return 1;
}



/*
    

    Cuando terminamos de utilizar DSU, debemos liberar
    la memoria que reservamos con malloc().

    
*/

void liberarDSU(DSU *dsu) {


    /*
        Si el puntero es NULL, no existe una estructura
        que debamos liberar.

        return termina inmediatamente la función.
    */

    if (dsu == NULL)
        return;


    /*
        Liberamos individualmente los cuatro arreglos.
    */

    free(dsu->parent);
    free(dsu->rank);
    free(dsu->size);
    free(dsu->active);


    /*
        Finalmente liberamos la estructura principal.

        Primero liberamos sus arreglos y después
        liberamos la estructura que contiene sus punteros.
    */

    free(dsu);
}


/*
    ============================================================
    FIN DEL ARCHIVO DSU.H
    ============================================================


*/

#endif

