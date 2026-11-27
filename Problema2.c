/*
    ADMINISTRADOR DE CIUDADES Y CARRETERAS
    -------------------------------------
    Lenguaje: C (estilo compatible con compiladores antiguos de Dev-C++)

    Funciones:
      1. Registrar ciudades.
      2. Mostrar ciudades.
      3. Agregar carreteras con distancia.
      4. Mostrar carreteras.
      5. Consultar conectividad mediante DSU.
      6. Mostrar grupos conectados.
      7. Eliminar carreteras.
      8. Eliminar ciudades.
      9. Buscar una ruta posible mediante BFS.
     10. Elegir manualmente la raiz de un grupo DSU.
     11. Guardar el mapa en un archivo.
     12. Cargar el mapa desde un archivo.
      0. Guardar y salir.

    IMPORTANTE:
    - Las carreteras son bidireccionales.
    - DSU administra grupos, no almacena las carreteras.
    - La lista de carreteras es la fuente real de informacion del mapa.
    - BFS encuentra una ruta con el menor numero de carreteras,
      no necesariamente la de menor distancia.
    - La raiz manual es el representante interno del grupo DSU.
      No cambia las carreteras ni la ciudad desde donde comienza BFS.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_NOMBRE 80
#define ARCHIVO_MAPA "mapa.txt"

/*----------------------------------------------------------
  ESTRUCTURAS DE DATOS
----------------------------------------------------------*/

typedef struct
{
    int id;                         /* Identificador de la ciudad */
    char nombre[TAM_NOMBRE];        /* Nombre de la ciudad */
} Ciudad;

typedef struct
{
    int ciudadA;                    /* ID de la primera ciudad */
    int ciudadB;                    /* ID de la segunda ciudad */
    double distancia;               /* Distancia en kilometros */
} Carretera;

typedef struct
{
    int *parent;                    /* Padre de cada elemento DSU */
    int *rank;                      /* Rango aproximado de cada arbol */
    int *size;                      /* Cantidad de ciudades del grupo */
    int n;                          /* Numero de ciudades */
} DSU;

typedef struct
{
    Ciudad *ciudades;               /* Arreglo dinamico de ciudades */
    Carretera *carreteras;          /* Arreglo dinamico de carreteras */
    int nCiudades;                  /* Cantidad actual de ciudades */
    int capCiudades;                /* Capacidad reservada de ciudades */
    int nCarreteras;                /* Cantidad actual de carreteras */
    int capCarreteras;              /* Capacidad reservada de carreteras */

    /*
       preferida[i] guarda la raiz manual elegida para el grupo
       al que pertenece la ciudad i. Si vale -1, no hay eleccion
       manual guardada para ese grupo.

       La preferencia se repite para todos los miembros del grupo.
       Esto permite conservarla cuando reconstruimos DSU.
    */
    int *preferida;

    DSU dsu;                        /* Estructura de conjuntos disjuntos */
} Mapa;

/*----------------------------------------------------------
  FUNCIONES AUXILIARES DE ENTRADA
----------------------------------------------------------*/

void limpiarEntrada(void)
{
    int c;

    while ((c = getchar()) != '\n' && c != EOF)
    {
        /* Consumimos los caracteres sobrantes. */
    }
}

int leerEntero(const char *mensaje)
{
    int valor;
    int resultado;

    printf("%s", mensaje);
    resultado = scanf("%d", &valor);

    while (resultado != 1)
    {
        printf("Entrada no valida. Introduce un numero entero: ");
        limpiarEntrada();
        resultado = scanf("%d", &valor);
    }

    limpiarEntrada();
    return valor;
}

double leerDouble(const char *mensaje)
{
    double valor;
    int resultado;

    printf("%s", mensaje);
    resultado = scanf("%lf", &valor);

    while (resultado != 1 || valor < 0.0)
    {
        printf("Distancia no valida. Introduce un numero >= 0: ");
        limpiarEntrada();
        resultado = scanf("%lf", &valor);
    }

    limpiarEntrada();
    return valor;
}

void leerTexto(const char *mensaje, char *destino, int capacidad)
{
    size_t longitud;

    printf("%s", mensaje);

    if (fgets(destino, capacidad, stdin) == NULL)
    {
        destino[0] = '\0';
        return;
    }

    longitud = strlen(destino);

    if (longitud > 0 && destino[longitud - 1] == '\n')
    {
        destino[longitud - 1] = '\0';
    }
    else
    {
        limpiarEntrada();
    }
}

/*----------------------------------------------------------
  FUNCIONES DE MEMORIA DINAMICA
----------------------------------------------------------*/

int ampliarCiudades(Mapa *mapa)
{
    int nuevaCapacidad;
    Ciudad *nuevoArreglo;
    int *nuevaPreferida;
    int i;

    nuevaCapacidad = mapa->capCiudades * 2;

    if (nuevaCapacidad <= mapa->capCiudades)
    {
        return 0;
    }

    nuevoArreglo = (Ciudad *)realloc(mapa->ciudades,
                                     nuevaCapacidad * sizeof(Ciudad));

    if (nuevoArreglo == NULL)
    {
        return 0;
    }

    mapa->ciudades = nuevoArreglo;

    nuevaPreferida = (int *)realloc(mapa->preferida,
                                    nuevaCapacidad * sizeof(int));

    if (nuevaPreferida == NULL)
    {
        return 0;
    }

    mapa->preferida = nuevaPreferida;

    for (i = mapa->capCiudades; i < nuevaCapacidad; i++)
    {
        mapa->preferida[i] = -1;
    }

    mapa->capCiudades = nuevaCapacidad;
    return 1;
}

int ampliarCarreteras(Mapa *mapa)
{
    int nuevaCapacidad;
    Carretera *nuevoArreglo;

    nuevaCapacidad = mapa->capCarreteras * 2;

    if (nuevaCapacidad <= mapa->capCarreteras)
    {
        return 0;
    }

    nuevoArreglo = (Carretera *)realloc(mapa->carreteras,
                         nuevaCapacidad * sizeof(Carretera));

    if (nuevoArreglo == NULL)
    {
        return 0;
    }

    mapa->carreteras = nuevoArreglo;
    mapa->capCarreteras = nuevaCapacidad;
    return 1;
}

/*----------------------------------------------------------
  CREACION Y LIBERACION DEL MAPA
----------------------------------------------------------*/

int crearMapa(Mapa *mapa)
{
    mapa->nCiudades = 0;
    mapa->capCiudades = 4;
    mapa->nCarreteras = 0;
    mapa->capCarreteras = 4;
    mapa->dsu.n = 0;

    mapa->ciudades = (Ciudad *)malloc(mapa->capCiudades * sizeof(Ciudad));
    mapa->carreteras = (Carretera *)malloc(mapa->capCarreteras * sizeof(Carretera));
    mapa->preferida = (int *)malloc(mapa->capCiudades * sizeof(int));

    mapa->dsu.parent = NULL;
    mapa->dsu.rank = NULL;
    mapa->dsu.size = NULL;

    if (mapa->ciudades == NULL || mapa->carreteras == NULL ||
        mapa->preferida == NULL)
    {
        return 0;
    }

    return 1;
}

void liberarMapa(Mapa *mapa)
{
    free(mapa->ciudades);
    free(mapa->carreteras);
    free(mapa->preferida);
    free(mapa->dsu.parent);
    free(mapa->dsu.rank);
    free(mapa->dsu.size);

    mapa->ciudades = NULL;
    mapa->carreteras = NULL;
    mapa->preferida = NULL;
    mapa->dsu.parent = NULL;
    mapa->dsu.rank = NULL;
    mapa->dsu.size = NULL;
}

/*----------------------------------------------------------
  ALGORITMO DSU: FIND Y UNION
----------------------------------------------------------*/

int find(DSU *dsu, int x)
{
    /*
       Si x es su propio padre, x es la raiz.
       Si no, seguimos subiendo por el arbol.
       La asignacion realiza compresion de caminos.
    */
    if (dsu->parent[x] != x)
    {
        dsu->parent[x] = find(dsu, dsu->parent[x]);
    }

    return dsu->parent[x];
}

void unionDSU(DSU *dsu, int a, int b)
{
    int raizA;
    int raizB;
    int temporal;

    raizA = find(dsu, a);
    raizB = find(dsu, b);

    /* Si las raices son iguales, ya pertenecen al mismo grupo. */
    if (raizA == raizB)
    {
        return;
    }

    /* Union por rango: el arbol de menor rango cuelga del mayor. */
    if (dsu->rank[raizA] < dsu->rank[raizB])
    {
        temporal = raizA;
        raizA = raizB;
        raizB = temporal;
    }

    dsu->parent[raizB] = raizA;
    dsu->size[raizA] = dsu->size[raizA] + dsu->size[raizB];

    if (dsu->rank[raizA] == dsu->rank[raizB])
    {
        dsu->rank[raizA]++;
    }
}

/*----------------------------------------------------------
  CAMBIAR MANUALMENTE LA RAIZ DE UN GRUPO
----------------------------------------------------------*/

void establecerRaizDSU(DSU *dsu, int ciudadElegida)
{
    int raizAnterior;
    int i;
    int cantidad;

    raizAnterior = find(dsu, ciudadElegida);
    cantidad = 0;

    /*
       PRIMER RECORRIDO:
       Aplanamos los miembros del grupo para que todos apunten
       a la raiz anterior. Todavia no cambiamos esa raiz.
       Esto evita crear ciclos mientras recorremos el grupo.
    */
    for (i = 0; i < dsu->n; i++)
    {
        if (find(dsu, i) == raizAnterior)
        {
            dsu->parent[i] = raizAnterior;
        }
    }

    /*
       SEGUNDO RECORRIDO:
       Ahora todos los miembros del grupo apuntan directamente
       a la raiz anterior. Podemos identificarlos sin llamar a
       find y conectarlos con la ciudad elegida.
    */
    for (i = 0; i < dsu->n; i++)
    {
        if (dsu->parent[i] == raizAnterior)
        {
            dsu->parent[i] = ciudadElegida;
            cantidad++;
        }
    }

    dsu->parent[ciudadElegida] = ciudadElegida;

    /*
       El arbol queda con altura 1 (o 0 si solo tiene un elemento).
       Actualizamos los rangos para que union por rango siga siendo
       coherente con la forma actual del arbol.
    */
    for (i = 0; i < dsu->n; i++)
    {
        if (i == ciudadElegida)
        {
            dsu->rank[i] = (cantidad > 1) ? 1 : 0;
            dsu->size[i] = cantidad;
        }
        else if (find(dsu, i) == ciudadElegida)
        {
            dsu->rank[i] = 0;
            dsu->size[i] = 1;
        }
    }
}

/*----------------------------------------------------------
  RECONSTRUIR DSU DESDE LAS CARRETERAS
----------------------------------------------------------*/

int reconstruirDSU(Mapa *mapa)
{
    int i;
    int j;
    int raiz;
    int raizElegida;
    int *nuevosParent;
    int *nuevosRank;
    int *nuevosSize;

    /* Reservamos los arreglos DSU segun el numero de ciudades. */
    nuevosParent = NULL;
    nuevosRank = NULL;
    nuevosSize = NULL;

    if (mapa->nCiudades > 0)
    {
        nuevosParent = (int *)malloc(mapa->nCiudades * sizeof(int));
        nuevosRank = (int *)malloc(mapa->nCiudades * sizeof(int));
        nuevosSize = (int *)malloc(mapa->nCiudades * sizeof(int));

        if (nuevosParent == NULL || nuevosRank == NULL || nuevosSize == NULL)
        {
            free(nuevosParent);
            free(nuevosRank);
            free(nuevosSize);
            return 0;
        }
    }

    free(mapa->dsu.parent);
    free(mapa->dsu.rank);
    free(mapa->dsu.size);

    mapa->dsu.parent = nuevosParent;
    mapa->dsu.rank = nuevosRank;
    mapa->dsu.size = nuevosSize;
    mapa->dsu.n = mapa->nCiudades;

    /* Inicialmente cada ciudad forma su propio conjunto. */
    for (i = 0; i < mapa->nCiudades; i++)
    {
        mapa->dsu.parent[i] = i;
        mapa->dsu.rank[i] = 0;
        mapa->dsu.size[i] = 1;
    }

    /* Unimos los extremos de todas las carreteras existentes. */
    for (i = 0; i < mapa->nCarreteras; i++)
    {
        unionDSU(&mapa->dsu,
                 mapa->carreteras[i].ciudadA,
                 mapa->carreteras[i].ciudadB);
    }

    /*
       Restauramos las raices manuales guardadas.
       Si dos grupos se unieron, puede haber dos preferencias.
       En ese caso se conserva la primera que encontremos por ID.
    */
    for (i = 0; i < mapa->nCiudades; i++)
    {
        if (mapa->preferida[i] >= 0 && mapa->preferida[i] < mapa->nCiudades)
        {
            raiz = find(&mapa->dsu, i);
            raizElegida = mapa->preferida[i];

            if (find(&mapa->dsu, raizElegida) == raiz)
            {
                establecerRaizDSU(&mapa->dsu, raizElegida);

                /* Guardamos la misma preferencia para todo el grupo. */
                for (j = 0; j < mapa->nCiudades; j++)
                {
                    if (find(&mapa->dsu, j) == raizElegida)
                    {
                        mapa->preferida[j] = raizElegida;
                    }
                }
            }
        }
    }

    /* Normalizamos las preferencias de los grupos sin raiz manual. */
    for (i = 0; i < mapa->nCiudades; i++)
    {
        if (mapa->preferida[i] >= 0 && mapa->preferida[i] < mapa->nCiudades)
        {
            if (find(&mapa->dsu, mapa->preferida[i]) != find(&mapa->dsu, i))
            {
                mapa->preferida[i] = -1;
            }
        }
    }

    return 1;
}

/*----------------------------------------------------------
  BUSQUEDA DE CIUDADES Y CARRETERAS
----------------------------------------------------------*/

int buscarCiudadPorNombre(Mapa *mapa, const char *nombre)
{
    int i;

    for (i = 0; i < mapa->nCiudades; i++)
    {
        if (strcmp(mapa->ciudades[i].nombre, nombre) == 0)
        {
            return i;
        }
    }

    return -1;
}

int buscarCarretera(Mapa *mapa, int a, int b)
{
    int i;

    for (i = 0; i < mapa->nCarreteras; i++)
    {
        if ((mapa->carreteras[i].ciudadA == a && mapa->carreteras[i].ciudadB == b) ||
            (mapa->carreteras[i].ciudadA == b && mapa->carreteras[i].ciudadB == a))
        {
            return i;
        }
    }

    return -1;
}

/*----------------------------------------------------------
  OPERACIONES DEL MAPA: CIUDADES
----------------------------------------------------------*/

void registrarCiudad(Mapa *mapa)
{
    char nombre[TAM_NOMBRE];
    int id;

    leerTexto("Nombre de la nueva ciudad: ", nombre, TAM_NOMBRE);

    if (nombre[0] == '\0')
    {
        printf("El nombre no puede estar vacio.\n");
        return;
    }

    if (buscarCiudadPorNombre(mapa, nombre) != -1)
    {
        printf("Ya existe una ciudad con ese nombre.\n");
        return;
    }

    if (mapa->nCiudades == mapa->capCiudades)
    {
        if (!ampliarCiudades(mapa))
        {
            printf("No se pudo ampliar la memoria para ciudades.\n");
            return;
        }
    }

    id = mapa->nCiudades;
    mapa->ciudades[id].id = id;
    strcpy(mapa->ciudades[id].nombre, nombre);
    mapa->preferida[id] = -1;
    mapa->nCiudades++;

    if (!reconstruirDSU(mapa))
    {
        printf("Ciudad registrada, pero no se pudo reconstruir DSU.\n");
        return;
    }

    printf("Ciudad registrada correctamente. ID asignado: %d\n", id);
}

void mostrarCiudades(Mapa *mapa)
{
    int i;

    if (mapa->nCiudades == 0)
    {
        printf("Todavia no hay ciudades registradas.\n");
        return;
    }

    printf("\n========== CIUDADES =========\n");

    for (i = 0; i < mapa->nCiudades; i++)
    {
        printf("ID: %d | Ciudad: %s\n",
               mapa->ciudades[i].id,
               mapa->ciudades[i].nombre);
    }
}

/*----------------------------------------------------------
  OPERACIONES DEL MAPA: CARRETERAS
----------------------------------------------------------*/

void agregarCarretera(Mapa *mapa)
{
    int a;
    int b;
    double distancia;

    if (mapa->nCiudades < 2)
    {
        printf("Necesitas registrar al menos dos ciudades.\n");
        return;
    }

    mostrarCiudades(mapa);
    a = leerEntero("ID de la primera ciudad: ");
    b = leerEntero("ID de la segunda ciudad: ");

    if (a < 0 || a >= mapa->nCiudades || b < 0 || b >= mapa->nCiudades)
    {
        printf("Uno de los ID no existe.\n");
        return;
    }

    if (a == b)
    {
        printf("No se permite conectar una ciudad consigo misma.\n");
        return;
    }

    if (buscarCarretera(mapa, a, b) != -1)
    {
        printf("Esa carretera ya esta registrada.\n");
        return;
    }

    distancia = leerDouble("Distancia de la carretera en kilometros: ");

    if (mapa->nCarreteras == mapa->capCarreteras)
    {
        if (!ampliarCarreteras(mapa))
        {
            printf("No se pudo ampliar la memoria para carreteras.\n");
            return;
        }
    }

    mapa->carreteras[mapa->nCarreteras].ciudadA = a;
    mapa->carreteras[mapa->nCarreteras].ciudadB = b;
    mapa->carreteras[mapa->nCarreteras].distancia = distancia;
    mapa->nCarreteras++;

    if (!reconstruirDSU(mapa))
    {
        printf("Carretera agregada, pero hubo un problema reconstruyendo DSU.\n");
        return;
    }

    printf("Carretera agregada correctamente.\n");
}

void mostrarCarreteras(Mapa *mapa)
{
    int i;
    int a;
    int b;

    if (mapa->nCarreteras == 0)
    {
        printf("Todavia no hay carreteras registradas.\n");
        return;
    }

    printf("\n========== CARRETERAS =========\n");

    for (i = 0; i < mapa->nCarreteras; i++)
    {
        a = mapa->carreteras[i].ciudadA;
        b = mapa->carreteras[i].ciudadB;

        printf("%s <-> %s | %.2f km\n",
               mapa->ciudades[a].nombre,
               mapa->ciudades[b].nombre,
               mapa->carreteras[i].distancia);
    }
}

void eliminarCarretera(Mapa *mapa)
{
    int a;
    int b;
    int posicion;
    int i;

    if (mapa->nCarreteras == 0)
    {
        printf("No hay carreteras que eliminar.\n");
        return;
    }

    mostrarCiudades(mapa);
    a = leerEntero("ID de la primera ciudad: ");
    b = leerEntero("ID de la segunda ciudad: ");

    if (a < 0 || a >= mapa->nCiudades || b < 0 || b >= mapa->nCiudades)
    {
        printf("ID no valido.\n");
        return;
    }

    posicion = buscarCarretera(mapa, a, b);

    if (posicion == -1)
    {
        printf("No existe una carretera directa entre esas ciudades.\n");
        return;
    }

    /* Desplazamos los elementos posteriores una posicion a la izquierda. */
    for (i = posicion; i < mapa->nCarreteras - 1; i++)
    {
        mapa->carreteras[i] = mapa->carreteras[i + 1];
    }

    mapa->nCarreteras--;
    reconstruirDSU(mapa);
    printf("Carretera eliminada. Los grupos fueron recalculados.\n");
}

void eliminarCiudad(Mapa *mapa)
{
    int id;
    int i;
    int destino;

    if (mapa->nCiudades == 0)
    {
        printf("No hay ciudades que eliminar.\n");
        return;
    }

    mostrarCiudades(mapa);
    id = leerEntero("ID de la ciudad que deseas eliminar: ");

    if (id < 0 || id >= mapa->nCiudades)
    {
        printf("ID no valido.\n");
        return;
    }

    /* Quitamos las carreteras conectadas con la ciudad eliminada. */
    destino = 0;

    for (i = 0; i < mapa->nCarreteras; i++)
    {
        if (mapa->carreteras[i].ciudadA != id && mapa->carreteras[i].ciudadB != id)
        {
            mapa->carreteras[destino] = mapa->carreteras[i];
            destino++;
        }
    }

    mapa->nCarreteras = destino;

    /* Movemos las ciudades posteriores para no dejar huecos en los ID. */
    for (i = id; i < mapa->nCiudades - 1; i++)
    {
        mapa->ciudades[i] = mapa->ciudades[i + 1];
        mapa->preferida[i] = mapa->preferida[i + 1];
        mapa->ciudades[i].id = i;
    }

    mapa->nCiudades--;

    /* Actualizamos los ID de los extremos de todas las carreteras. */
    for (i = 0; i < mapa->nCarreteras; i++)
    {
        if (mapa->carreteras[i].ciudadA > id)
        {
            mapa->carreteras[i].ciudadA--;
        }

        if (mapa->carreteras[i].ciudadB > id)
        {
            mapa->carreteras[i].ciudadB--;
        }
    }

    /* Las preferencias deben apuntar a los nuevos ID. */
    for (i = 0; i < mapa->nCiudades; i++)
    {
        if (mapa->preferida[i] == id)
        {
            mapa->preferida[i] = -1;
        }
        else if (mapa->preferida[i] > id)
        {
            mapa->preferida[i]--;
        }
    }

    reconstruirDSU(mapa);
    printf("Ciudad eliminada y mapa actualizado.\n");
}

/*----------------------------------------------------------
  CONSULTAS DSU
----------------------------------------------------------*/

void consultarConexion(Mapa *mapa)
{
    int a;
    int b;

    if (mapa->nCiudades < 2)
    {
        printf("Se necesitan al menos dos ciudades.\n");
        return;
    }

    mostrarCiudades(mapa);
    a = leerEntero("ID de la primera ciudad: ");
    b = leerEntero("ID de la segunda ciudad: ");

    if (a < 0 || a >= mapa->nCiudades || b < 0 || b >= mapa->nCiudades)
    {
        printf("ID no valido.\n");
        return;
    }

    if (find(&mapa->dsu, a) == find(&mapa->dsu, b))
    {
        printf("Las ciudades pertenecen al mismo grupo: estan conectadas.\n");
    }
    else
    {
        printf("Las ciudades no estan conectadas.\n");
    }
}

void mostrarGrupos(Mapa *mapa)
{
    int i;
    int j;
    int raiz;
    int cantidadGrupos;
    int yaMostrado;

    cantidadGrupos = 0;

    if (mapa->nCiudades == 0)
    {
        printf("No hay ciudades registradas.\n");
        return;
    }

    printf("\n========== GRUPOS CONECTADOS =========\n");

    for (i = 0; i < mapa->nCiudades; i++)
    {
        raiz = find(&mapa->dsu, i);
        yaMostrado = 0;

        /* Comprobamos si esta raiz ya fue impresa anteriormente. */
        for (j = 0; j < i; j++)
        {
            if (find(&mapa->dsu, j) == raiz)
            {
                yaMostrado = 1;
                break;
            }
        }

        if (!yaMostrado)
        {
            cantidadGrupos++;
            printf("\nGrupo %d | Raiz DSU: %s | Ciudades: ",
                   cantidadGrupos, mapa->ciudades[raiz].nombre);

            for (j = 0; j < mapa->nCiudades; j++)
            {
                if (find(&mapa->dsu, j) == raiz)
                {
                    printf("%s ", mapa->ciudades[j].nombre);
                }
            }

            printf("\nCantidad: %d\n", mapa->dsu.size[raiz]);
        }
    }

    printf("\nTotal de grupos: %d\n", cantidadGrupos);
}

/*----------------------------------------------------------
  ELEGIR RAIZ MANUALMENTE
----------------------------------------------------------*/

void elegirRaizManual(Mapa *mapa)
{
    int id;
    int raizAnterior;
    int i;

    if (mapa->nCiudades == 0)
    {
        printf("No hay ciudades registradas.\n");
        return;
    }

    mostrarCiudades(mapa);
    id = leerEntero("ID de la ciudad que deseas convertir en raiz: ");

    if (id < 0 || id >= mapa->nCiudades)
    {
        printf("ID no valido.\n");
        return;
    }

    raizAnterior = find(&mapa->dsu, id);

    /*
       Guardamos la ciudad elegida como preferencia para todos
       los miembros de su grupo. Esto permite restaurarla cuando
       DSU se reconstruya despues de agregar/eliminar carreteras.
    */
    for (i = 0; i < mapa->nCiudades; i++)
    {
        if (find(&mapa->dsu, i) == raizAnterior)
        {
            mapa->preferida[i] = id;
        }
    }

    establecerRaizDSU(&mapa->dsu, id);

    printf("La ciudad %s ahora es la raiz DSU de su grupo.\n",
           mapa->ciudades[id].nombre);
}

/*----------------------------------------------------------
  BUSQUEDA DE RUTA MEDIANTE BFS
----------------------------------------------------------*/

void buscarRutaBFS(Mapa *mapa)
{
    int inicio;
    int destino;
    int *cola;
    int *visitado;
    int *anterior;
    int frente;
    int final;
    int actual;
    int vecino;
    int i;
    int encontrado;
    int *ruta;
    int longitud;
    int posicion;

    if (mapa->nCiudades < 2)
    {
        printf("Se necesitan al menos dos ciudades.\n");
        return;
    }

    mostrarCiudades(mapa);
    inicio = leerEntero("ID de la ciudad de origen: ");
    destino = leerEntero("ID de la ciudad de destino: ");

    if (inicio < 0 || inicio >= mapa->nCiudades ||
        destino < 0 || destino >= mapa->nCiudades)
    {
        printf("ID no valido.\n");
        return;
    }

    if (inicio == destino)
    {
        printf("Origen y destino son la misma ciudad: %s.\n",
               mapa->ciudades[inicio].nombre);
        return;
    }

    cola = (int *)malloc(mapa->nCiudades * sizeof(int));
    visitado = (int *)calloc(mapa->nCiudades, sizeof(int));
    anterior = (int *)malloc(mapa->nCiudades * sizeof(int));

    if (cola == NULL || visitado == NULL || anterior == NULL)
    {
        printf("No hay memoria suficiente para ejecutar BFS.\n");
        free(cola);
        free(visitado);
        free(anterior);
        return;
    }

    for (i = 0; i < mapa->nCiudades; i++)
    {
        anterior[i] = -1;
    }

    frente = 0;
    final = 0;
    encontrado = 0;

    cola[final] = inicio;
    final++;
    visitado[inicio] = 1;

    /* BFS: sacamos una ciudad de la cola y exploramos sus vecinos. */
    while (frente < final && !encontrado)
    {
        actual = cola[frente];
        frente++;

        for (i = 0; i < mapa->nCarreteras; i++)
        {
            vecino = -1;

            if (mapa->carreteras[i].ciudadA == actual)
            {
                vecino = mapa->carreteras[i].ciudadB;
            }
            else if (mapa->carreteras[i].ciudadB == actual)
            {
                vecino = mapa->carreteras[i].ciudadA;
            }

            if (vecino != -1 && !visitado[vecino])
            {
                visitado[vecino] = 1;
                anterior[vecino] = actual;
                cola[final] = vecino;
                final++;

                if (vecino == destino)
                {
                    encontrado = 1;
                    break;
                }
            }
        }
    }

    if (!encontrado)
    {
        printf("No existe una ruta entre esas ciudades.\n");
    }
    else
    {
        ruta = (int *)malloc(mapa->nCiudades * sizeof(int));

        if (ruta == NULL)
        {
            printf("No hay memoria suficiente para mostrar la ruta.\n");
        }
        else
        {
            longitud = 0;
            actual = destino;

            while (actual != -1)
            {
                ruta[longitud] = actual;
                longitud++;
                actual = anterior[actual];
            }

            printf("Ruta encontrada: ");

            for (posicion = longitud - 1; posicion >= 0; posicion--)
            {
                printf("%s", mapa->ciudades[ruta[posicion]].nombre);

                if (posicion > 0)
                {
                    printf(" -> ");
                }
            }

            printf("\nNumero de carreteras: %d\n", longitud - 1);
            free(ruta);
        }
    }

    free(cola);
    free(visitado);
    free(anterior);
}

/*----------------------------------------------------------
  GUARDAR Y CARGAR EL MAPA
----------------------------------------------------------*/

int guardarMapa(Mapa *mapa)
{
    FILE *archivo;
    int i;

    archivo = fopen(ARCHIVO_MAPA, "w");

    if (archivo == NULL)
    {
        printf("No se pudo abrir %s para guardar.\n", ARCHIVO_MAPA);
        return 0;
    }

    /* Primera linea: numero de ciudades y carreteras. */
    fprintf(archivo, "%d %d\n", mapa->nCiudades, mapa->nCarreteras);

    for (i = 0; i < mapa->nCiudades; i++)
    {
        fprintf(archivo, "%s\n", mapa->ciudades[i].nombre);
    }

    for (i = 0; i < mapa->nCarreteras; i++)
    {
        fprintf(archivo, "%d %d %.10f\n",
                mapa->carreteras[i].ciudadA,
                mapa->carreteras[i].ciudadB,
                mapa->carreteras[i].distancia);
    }

    /* Guardamos una preferencia de raiz por ciudad. */
    for (i = 0; i < mapa->nCiudades; i++)
    {
        fprintf(archivo, "%d\n", mapa->preferida[i]);
    }

    fclose(archivo);
    printf("Mapa guardado en %s.\n", ARCHIVO_MAPA);
    return 1;
}

int cargarMapa(Mapa *mapa)
{
    FILE *archivo;
    int cantidadCiudades;
    int cantidadCarreteras;
    int i;
    int a;
    int b;
    double distancia;
    char nombre[TAM_NOMBRE];

    archivo = fopen(ARCHIVO_MAPA, "r");

    if (archivo == NULL)
    {
        printf("No existe %s. Se iniciara un mapa vacio.\n", ARCHIVO_MAPA);
        return 0;
    }

    if (fscanf(archivo, "%d %d\n", &cantidadCiudades, &cantidadCarreteras) != 2 ||
        cantidadCiudades < 0 || cantidadCarreteras < 0)
    {
        printf("El archivo tiene un encabezado incorrecto.\n");
        fclose(archivo);
        return 0;
    }

    /* Reservamos memoria suficiente para los datos leidos. */
    while (mapa->capCiudades < cantidadCiudades)
    {
        if (!ampliarCiudades(mapa))
        {
            fclose(archivo);
            return 0;
        }
    }

    while (mapa->capCarreteras < cantidadCarreteras)
    {
        if (!ampliarCarreteras(mapa))
        {
            fclose(archivo);
            return 0;
        }
    }

    mapa->nCiudades = 0;
    mapa->nCarreteras = 0;

    for (i = 0; i < cantidadCiudades; i++)
    {
        if (fgets(nombre, TAM_NOMBRE, archivo) == NULL)
        {
            printf("Error leyendo nombres de ciudades.\n");
            fclose(archivo);
            return 0;
        }

        nombre[strcspn(nombre, "\r\n")] = '\0';
        mapa->ciudades[i].id = i;
        strcpy(mapa->ciudades[i].nombre, nombre);
        mapa->preferida[i] = -1;
        mapa->nCiudades++;
    }

    for (i = 0; i < cantidadCarreteras; i++)
    {
        if (fscanf(archivo, "%d %d %lf", &a, &b, &distancia) != 3)
        {
            printf("Error leyendo carreteras.\n");
            fclose(archivo);
            return 0;
        }

        if (a < 0 || a >= cantidadCiudades || b < 0 || b >= cantidadCiudades)
        {
            printf("El archivo contiene una carretera con ID invalido.\n");
            fclose(archivo);
            return 0;
        }

        mapa->carreteras[i].ciudadA = a;
        mapa->carreteras[i].ciudadB = b;
        mapa->carreteras[i].distancia = distancia;
        mapa->nCarreteras++;
    }

    /* Leemos las raices manuales guardadas, si existen. */
    for (i = 0; i < cantidadCiudades; i++)
    {
        if (fscanf(archivo, "%d", &mapa->preferida[i]) != 1)
        {
            mapa->preferida[i] = -1;
        }
    }

    fclose(archivo);
    reconstruirDSU(mapa);
    printf("Mapa cargado correctamente desde %s.\n", ARCHIVO_MAPA);
    return 1;
}

/*----------------------------------------------------------
  MENU PRINCIPAL
----------------------------------------------------------*/

void mostrarMenu(void)
{
    printf("\n===============================================\n");
    printf("       ADMINISTRADOR DE CIUDADES Y CARRETERAS\n");
     printf("===============================================\n");
    printf("Usted es un ingeniero que esta comenzando a hacer un mapa para Google, utilizando la estructura de datos Conjuntos Disjuntos\n");
	printf("puede almacenar el nombre de las ciudades y las conexiones que hay entre ellas, revise el menu para empezar a crear su mapa.\n");
		printf("Si no tiene un mapa guardado, se recomienda empezar con la opcion 1 para crear las ciudades. \n\n");
    printf("1.  Registrar una ciudad\n");
    printf("2.  Mostrar todas las ciudades\n");
    printf("3.  Agregar una carretera\n");
    printf("4.  Mostrar todas las carreteras\n");
    printf("5.  Consultar conexion entre dos ciudades (DSU)\n");
    printf("6.  Mostrar grupos conectados (DSU)\n");
    printf("7.  Eliminar una carretera\n");
    printf("8.  Eliminar una ciudad\n");
    printf("9.  Buscar una ruta posible (BFS)\n");
    printf("10. Elegir manualmente la raiz de un grupo\n");
    printf("11. Guardar mapa\n");
    printf("12. Cargar mapa\n");
    printf("0.  Guardar y salir\n");
    printf("===============================================\n");
}

int main(void)
{
    Mapa mapa;
    int opcion;

    if (!crearMapa(&mapa))
    {
        printf("No se pudo reservar memoria para iniciar el mapa.\n");
        liberarMapa(&mapa);
        return 1;
    }

    /* Al iniciar, intentamos recuperar el archivo guardado. */
    cargarMapa(&mapa);

    do
    {
        mostrarMenu();
        opcion = leerEntero("Selecciona una opcion: ");

        switch (opcion)
        {
            case 1:
                registrarCiudad(&mapa);
                break;

            case 2:
                mostrarCiudades(&mapa);
                break;

            case 3:
                agregarCarretera(&mapa);
                break;

            case 4:
                mostrarCarreteras(&mapa);
                break;

            case 5:
                consultarConexion(&mapa);
                break;

            case 6:
                mostrarGrupos(&mapa);
                break;

            case 7:
                eliminarCarretera(&mapa);
                break;

            case 8:
                eliminarCiudad(&mapa);
                break;

            case 9:
                buscarRutaBFS(&mapa);
                break;

            case 10:
                elegirRaizManual(&mapa);
                break;

            case 11:
                guardarMapa(&mapa);
                break;

            case 12:
                cargarMapa(&mapa);
                break;

            case 0:
                guardarMapa(&mapa);
                printf("Cerrando el administrador de carreteras...\n");
                break;

            default:
                printf("Opcion no valida. Intenta nuevamente.\n");
        }

    } while (opcion != 0);

    liberarMapa(&mapa);
    return 0;
}
