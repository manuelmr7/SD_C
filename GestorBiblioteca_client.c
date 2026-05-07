/*=============================================================================
 * GestorBiblioteca_cliente.c
 * Implementación del cliente RPC para el Gestor Bibliotecario.
 * Sistemas Distribuidos - Práctica 1 RPC
 *
 * Uso: ./GestorBiblioteca_cliente <hostname_servidor>
 *===========================================================================*/
#define _GNU_SOURCE
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdio_ext.h>          /* __fpurge */
#include "GestorBiblioteca.h"
#define Cls             system("clear")
#define Pause           system("read -p \"Pulsa la tecla return para continuar..... \" a")
#define MostrarAviso(T) { printf(T); Pause; }
int MenuPrincipal(void)
{
    int Salida;
    do
    {
        Cls;
        printf(" GESTOR BIBLIOTECARIO 1.0 (M. PRINCIPAL)\n");
        printf("*****************************************\n");
        printf("\t1.- M. Administración\n");
        printf("\t2.- Consulta de libros\n");
        printf("\t3.- Préstamo de libros\n");
        printf("\t4.- Devolución de libros\n");
        printf("\t0.- Salir\n\n");
        printf(" Elige opción: ");
        scanf("%d", &Salida);
        if (Salida < 0 || Salida > 4)
            MostrarAviso("\n\n *** Error en la entrada de Datos.***\n\n");
    } while (Salida < 0 || Salida > 4);
    return Salida;
}

int MenuAdministracion(void)
{
    int Salida;
    do
    {
        Cls;
        printf(" GESTOR BIBLIOTECARIO 1.0 (M. ADMINISTRACION)\n");
        printf("**********************************************\n");
        printf("\t1.- Cargar datos Biblioteca\n");
        printf("\t2.- Guardar datos Biblioteca\n");
        printf("\t3.- Nuevo libro\n");
        printf("\t4.- Comprar libros\n");
        printf("\t5.- Retirar libros\n");
        printf("\t6.- Ordenar libros\n");
        printf("\t7.- Buscar libros\n");
        printf("\t8.- Listar libros\n");
        printf("\t0.- Salir\n\n");
        printf(" Elige opción: ");
        scanf("%d", &Salida);
        if (Salida < 0 || Salida > 8)
            MostrarAviso("\n\n *** Error en la entrada de Datos.***\n\n");
    } while (Salida < 0 || Salida > 8);
    return Salida;
}
/* Rellena Salida con Texto seguido de (Ancho-len) repeticiones de Caracter */
void Formatea(char *Salida, const char *Texto, int Ancho, char Caracter)
{
    Cadena Vacia;
    int len = Ancho - (int)strlen(Texto);
    int l = 0, c = 0;

    while (Texto[l] != '\0')
    {
        if ((unsigned char)Texto[l] > 128)
            c++;
        l++;
    }
    len += c / 2;

    if (len < 0)
        len = 0;

    for (int i = 0; i < len; i++)
        Vacia[i] = Caracter;
    Vacia[len] = '\0';

    sprintf(Salida, "%s%s", Texto, Vacia);
}
void MostrarLibro(TLibro *L, int Pos, bool_t Cabecera)
{
    Cadena T, A, B, PI;

    if (Cabecera == TRUE)
    {
        printf("%-*s%-*s%-*s%*s%*s%*s\n",
               5, "POS", 58, "TITULO", 18, "ISBN", 4, "DIS", 4, "PRE", 4, "RES");
        printf("     %-*s%-*s%-*s\n",
               30, "AUTOR", 28, "PAIS (IDIOMA)", 12, "AÑO");
        Formatea(B, "*", 93, '*');
        printf("%s\n", B);
    }

    Formatea(T, L->Titulo, 58, ' ');
    Formatea(A, L->Autor,  30, ' ');

    strcpy(B, L->Pais);
    strcat(B, "(");
    strcat(B, L->Idioma);
    strcat(B, ")");
    Formatea(PI, B, 28, ' ');

    printf("%-5d%s%-*s%*d%*d%*d\n",
           Pos + 1, T, 18, L->Isbn,
           4, L->NoLibros, 4, L->NoPrestados, 4, L->NoListaEspera);
    printf("     %s%s%-*d\n", A, PI, 12, L->Anio);
}

/* Devuelve TRUE si el campo Campo del libro L contiene el texto Texto */
bool_t Comprobar(TLibro *L, Cadena Texto, char Campo)
{
    bool_t Encontrado = FALSE;

    switch (tolower(Campo))
    {
        case 'i':
            Encontrado = strcasestr(L->Isbn,   Texto) != NULL ? TRUE : FALSE;
            break;
        case 't':
            Encontrado = strcasestr(L->Titulo, Texto) != NULL ? TRUE : FALSE;
            break;
        case 'a':
            Encontrado = strcasestr(L->Autor,  Texto) != NULL ? TRUE : FALSE;
            break;
        case 'p':
            Encontrado = strcasestr(L->Pais,   Texto) != NULL ? TRUE : FALSE;
            break;
        case 'd':
            Encontrado = strcasestr(L->Idioma, Texto) != NULL ? TRUE : FALSE;
            break;
        case '*':
            Encontrado = (strcasestr(L->Isbn,   Texto) != NULL ||
                          strcasestr(L->Titulo, Texto) != NULL ||
                          strcasestr(L->Autor,  Texto) != NULL ||
                          strcasestr(L->Pais,   Texto) != NULL ||
                          strcasestr(L->Idioma, Texto) != NULL) ? TRUE : FALSE;
            break;
    }
    return Encontrado;
}
static int BuscarYMostrar(CLIENT *clnt, int IdAdmin, Cadena Texto, char Campo)
{
    int     *nlib;
    int      n, i, mostrados = 0;
    bool_t   cabecera = TRUE;
    TPosicion tpos;

    nlib = nlibros_1(&IdAdmin, clnt);
    if (nlib == NULL)
    {
        clnt_perror(clnt, "Error RPC (NLibros)");
        return 0;
    }
    n = *nlib;

    for (i = 0; i < n; i++)
    {
        TLibro *libro;
        tpos.Ida = IdAdmin;
        tpos.Pos = i;

        libro = descargar_1(&tpos, clnt);
        if (libro == NULL)
        {
            clnt_perror(clnt, "Error RPC (Descargar)");
            break;
        }

        if (Comprobar(libro, Texto, Campo))
        {
            MostrarLibro(libro, i, cabecera);
            cabecera = FALSE;
            mostrados++;
        }
    }
    return mostrados;
}
static void MostrarMenuOrdenacion(void)
{
    printf(" Código de Ordenación\n");
    printf("  0.- Por Isbn\n");
    printf("  1.- Por Título\n");
    printf("  2.- Por Autor\n");
    printf("  3.- Por Año\n");
    printf("  4.- Por País\n");
    printf("  5.- Por Idioma\n");
    printf("  6.- Por nº de libros Disponibles\n");
    printf("  7.- Por nº de libros Prestados\n");
    printf("  8.- Por nº de libros en espera\n");
    printf(" Introduce Código: ");
}
static void MostrarMenuBusqueda(void)
{
    printf(" Código de Búsqueda\n");
    printf("  I.- Por Isbn\n");
    printf("  T.- Por Título\n");
    printf("  A.- Por Autor\n");
    printf("  P.- Por País\n");
    printf("  D.- Por Idioma\n");
    printf("  *.- Por todos los campos.\n");
    printf(" Introduce Código: ");
}
static void MostrarMenuConsulta(void)
{
    printf(" Código de Consulta\n");
    printf("  I.- Por Isbn\n");
    printf("  T.- Por Título\n");
    printf("  A.- Por Autor\n");
    printf("  P.- Por País\n");
    printf("  D.- Por Idioma\n");
    printf("  *.- Por todos los campos.\n");
    printf(" Introduce Código: ");
}
static void OpCargarDatos(CLIENT *clnt, int IdAdmin)
{
    TFichero datos;
    int     *res;

    datos.Ida = IdAdmin;
    printf("\n Introduce el nombre del fichero de datos: ");
    __fpurge(stdin);
    scanf("%s", datos.NomFile);

    res = cargardatos_1(&datos, clnt);
    if (res == NULL)
    {
        clnt_perror(clnt, "Error RPC (CargarDatos)");
        Pause;
        return;
    }

    switch (*res)
    {
        case  1: MostrarAviso("\n\n *** La biblioteca ha sido cargada.**\n\n"); break;
        case  0: MostrarAviso("\n\n *** Error: no se pudo abrir el fichero o error de memoria.***\n\n"); break;
        case -1: MostrarAviso("\n\n *** Error: no autorizado.***\n\n"); break;
    }
}

static void OpGuardarDatos(CLIENT *clnt, int IdAdmin)
{
    bool_t *res = guardardatos_1(&IdAdmin, clnt);

    if (res == NULL)
    {
        clnt_perror(clnt, "Error RPC (GuardarDatos)");
        Pause;
        return;
    }

    if (*res)
	{
		MostrarAviso("\n\n *** Se ha guardado el estado actual de la biblioteca.**\n\n");
	}
        
    else
	{
		MostrarAviso("\n\n *** Error al guardar los datos (fichero no cargado o no autorizado).***\n\n");
	}
        
}

static void OpNuevoLibro(CLIENT *clnt, int IdAdmin)
{
    TNuevo datos;
    int   *res;

    datos.Ida = IdAdmin;

    printf("\n Introduce el Isbn: ");
    __fpurge(stdin); scanf("%s", datos.Libro.Isbn);

    printf(" Introduce el Autor: ");
    __fpurge(stdin); scanf("%s", datos.Libro.Autor);

    printf(" Introduce el Titulo: ");
    __fpurge(stdin); scanf("%s", datos.Libro.Titulo);

    printf(" Introduce el Año: ");
    scanf("%d", &datos.Libro.Anio);

    printf(" Introduce el País: ");
    __fpurge(stdin); scanf("%s", datos.Libro.Pais);

    printf(" Introduce el Idioma: ");
    __fpurge(stdin); scanf("%s", datos.Libro.Idioma);

    printf(" Introduce Número de Libros inicial: ");
    scanf("%d", &datos.Libro.NoLibros);

    datos.Libro.NoPrestados   = 0;
    datos.Libro.NoListaEspera = 0;

    res = nuevolibro_1(&datos, clnt);
    if (res == NULL)
    {
        clnt_perror(clnt, "Error RPC (NuevoLibro)");
        Pause;
        return;
    }

    switch (*res)
    {
        case  1: MostrarAviso("\n\n *** El libro ha sido añadido correctamente.**\n\n"); break;
        case  0: MostrarAviso("\n\n *** Error: ya existe un libro con ese Isbn.***\n\n"); break;
        case -1: MostrarAviso("\n\n *** Error: no autorizado.***\n\n"); break;
    }
}

static void OpComprarLibros(CLIENT *clnt, int IdAdmin)
{
    TConsulta consulta;
    int      *pos;
    TPosicion tpos;
    TLibro   *libro;
    TComRet   cr;
    int      *res;
    char      resp;

    consulta.Ida = IdAdmin;
    printf("\n Introduce Isbn a Buscar: ");
    __fpurge(stdin);
    scanf("%s", consulta.Isbn);

    pos = buscar_1(&consulta, clnt);
    if (pos == NULL) { clnt_perror(clnt, "Error RPC (Buscar)"); Pause; return; }

    if (*pos == -2) { MostrarAviso("\n\n *** Error: no autorizado.***\n\n"); return; }
    if (*pos == -1) { MostrarAviso("\n\n *** No se ha encontrado ningún libro con ese Isbn.***\n\n"); return; }

    tpos.Ida = IdAdmin;
    tpos.Pos = *pos;

    libro = descargar_1(&tpos, clnt);
    if (libro == NULL) { clnt_perror(clnt, "Error RPC (Descargar)"); Pause; return; }

    printf("\n");
    MostrarLibro(libro, tpos.Pos, TRUE);

    printf("\n ¿ Es este el libro que deseas comprar más unidades (s/n) ? ");
    __fpurge(stdin);
    scanf("%c", &resp);

    if (tolower(resp) == 's')
    {
        cr.Ida = IdAdmin;
        strcpy(cr.Isbn, consulta.Isbn);
        printf(" Introduce Número de Libros comprados: ");
        scanf("%d", &cr.NoLibros);

        res = comprar_1(&cr, clnt);
        if (res == NULL) { clnt_perror(clnt, "Error RPC (Comprar)"); Pause; return; }

        switch (*res)
        {
            case  1: MostrarAviso("\n\n *** Se han añadido los nuevos libros.**\n\n"); break;
            case  0: MostrarAviso("\n\n *** Error: no se encontró el libro.***\n\n"); break;
            case -1: MostrarAviso("\n\n *** Error: no autorizado.***\n\n"); break;
        }
    }
}

static void OpRetirarLibros(CLIENT *clnt, int IdAdmin)
{
    TConsulta consulta;
    int      *pos;
    TPosicion tpos;
    TLibro   *libro;
    TComRet   cr;
    int      *res;
    char      resp;

    consulta.Ida = IdAdmin;
    printf("\n Introduce Isbn a Buscar: ");
    __fpurge(stdin);
    scanf("%s", consulta.Isbn);

    pos = buscar_1(&consulta, clnt);
    if (pos == NULL) { clnt_perror(clnt, "Error RPC (Buscar)"); Pause; return; }

    if (*pos == -2) { MostrarAviso("\n\n *** Error: no autorizado.***\n\n"); return; }
    if (*pos == -1) { MostrarAviso("\n\n *** No se ha encontrado ningún libro con ese Isbn.***\n\n"); return; }

    tpos.Ida = IdAdmin;
    tpos.Pos = *pos;

    libro = descargar_1(&tpos, clnt);
    if (libro == NULL) { clnt_perror(clnt, "Error RPC (Descargar)"); Pause; return; }

    printf("\n");
    MostrarLibro(libro, tpos.Pos, TRUE);

    printf("\n ¿ Es este el libro que deseas retirar unidades (s/n) ? ");
    __fpurge(stdin);
    scanf("%c", &resp);

    if (tolower(resp) == 's')
    {
        cr.Ida = IdAdmin;
        strcpy(cr.Isbn, consulta.Isbn);
        printf(" Introduce Número de unidades a retirar: ");
        scanf("%d", &cr.NoLibros);

        res = retirar_1(&cr, clnt);
        if (res == NULL) { clnt_perror(clnt, "Error RPC (Retirar)"); Pause; return; }

        switch (*res)
        {
            case  1: MostrarAviso("\n\n *** Se han retirado el número de libros indicados.**\n\n"); break;
            case  2: MostrarAviso("\n\n *** Error: no hay suficientes ejemplares disponibles.***\n\n"); break;
            case  0: MostrarAviso("\n\n *** Error: no se encontró el libro.***\n\n"); break;
            case -1: MostrarAviso("\n\n *** Error: no autorizado.***\n\n"); break;
        }
    }
}

static void OpOrdenarLibros(CLIENT *clnt, int IdAdmin)
{
    TOrdenacion ord;
    bool_t     *res;

    ord.Ida = IdAdmin;
    Cls;
    MostrarMenuOrdenacion();
    scanf("%d", &ord.Campo);

    res = ordenar_1(&ord, clnt);
    if (res == NULL) { clnt_perror(clnt, "Error RPC (Ordenar)"); Pause; return; }

    if (*res)
	{
		MostrarAviso("\n\n *** La biblioteca ha sido ordenada correctamente.**\n\n");
	}
        
    else
	{
		MostrarAviso("\n\n *** Error: no autorizado o biblioteca vacía.***\n\n");
	}
        
}

static void OpBuscarLibros(CLIENT *clnt, int IdAdmin)
{
    Cadena texto;
    char   campo;

    printf("\n Introduce el texto a Buscar: ");
    __fpurge(stdin);
    scanf("%s", texto);

    Cls;
    MostrarMenuBusqueda();
    __fpurge(stdin);
    scanf("%c", &campo);

    printf("\n");
    if (BuscarYMostrar(clnt, IdAdmin, texto, campo) == 0)
        printf("\n *** No se encontraron libros con ese criterio de búsqueda.***\n");

    Pause;
}

static void OpListarLibros(CLIENT *clnt, int IdAdmin)
{
    int      *nlib;
    int       n, i;
    bool_t    cabecera = TRUE;
    TPosicion tpos;

    nlib = nlibros_1(&IdAdmin, clnt);
    if (nlib == NULL) { clnt_perror(clnt, "Error RPC (NLibros)"); Pause; return; }
    n = *nlib;

    if (n == 0)
    {
        MostrarAviso("\n *** La biblioteca está vacía.***\n\n");
        return;
    }

    printf("\n");
    for (i = 0; i < n; i++)
    {
        TLibro *libro;
        tpos.Ida = IdAdmin;
        tpos.Pos = i;

        libro = descargar_1(&tpos, clnt);
        if (libro == NULL) { clnt_perror(clnt, "Error RPC (Descargar)"); break; }

        MostrarLibro(libro, i, cabecera);
        cabecera = FALSE;
    }
    Pause;
}
static void BucleAdministracion(CLIENT *clnt, int IdAdmin)
{
    int op;

    do
    {
        op = MenuAdministracion();
        switch (op)
        {
            case 1: OpCargarDatos   (clnt, IdAdmin); break;
            case 2: OpGuardarDatos  (clnt, IdAdmin); break;
            case 3: OpNuevoLibro    (clnt, IdAdmin); break;
            case 4: OpComprarLibros (clnt, IdAdmin); break;
            case 5: OpRetirarLibros (clnt, IdAdmin); break;
            case 6: OpOrdenarLibros (clnt, IdAdmin); break;
            case 7: OpBuscarLibros  (clnt, IdAdmin); break;
            case 8: OpListarLibros  (clnt, IdAdmin); break;
            case 0: break;
        }
    } while (op != 0);
}

static void OpAdministracion(CLIENT *clnt)
{
    Cadena passwd;
    int   *res;
    int    IdAdmin;
    bool_t *dr;

    printf("\n Por favor inserte la contraseña de Administración: ");
    __fpurge(stdin);
    scanf("%s", passwd);

    res = conexion_1(passwd, clnt);
    if (res == NULL)
    {
        clnt_perror(clnt, "Error RPC (Conexion)");
        Pause;
        return;
    }

    if (*res == -1)
    {
        MostrarAviso("\n\n *** Ya hay un administrador conectado, inténtelo más tarde.***\n\n");
        return;
    }
    if (*res == -2)
    {
        MostrarAviso("\n\n *** Contraseña incorrecta.***\n\n");
        return;
    }

    IdAdmin = *res;
    MostrarAviso("\n\n *** Contraseña correcta, puede acceder al menú de Administración.**\n\n");

    BucleAdministracion(clnt, IdAdmin);

    /* Desconexión al salir del submenú */
    dr = desconexion_1(&IdAdmin, clnt);
    if (dr == NULL)
        clnt_perror(clnt, "Error RPC (Desconexion)");
}

static void OpConsulta(CLIENT *clnt)
{
    Cadena texto;
    char   campo;
    int    ida = -1;

    printf("\n Introduce el texto a Buscar: ");
    __fpurge(stdin);
    scanf("%s", texto);

    Cls;
    MostrarMenuConsulta();
    __fpurge(stdin);
    scanf("%c", &campo);

    printf("\n");
    if (BuscarYMostrar(clnt, ida, texto, campo) == 0)
        printf("\n *** No se encontraron libros con ese criterio de búsqueda.***\n");

    Pause;
}

static void OpPrestamo(CLIENT *clnt)
{
    Cadena    texto;
    char      campo, resp;
    int       ida = -1;
    TPosicion tpos;
    int      *res;
    int       pos;
    int mostrados;

    printf("\n Introduce el texto a Buscar: ");
    __fpurge(stdin);
    scanf("%s", texto);

    Cls;
    MostrarMenuConsulta();
    __fpurge(stdin);
    scanf("%c", &campo);

    printf("\n");
    mostrados = BuscarYMostrar(clnt, ida, texto, campo);
    if(mostrados==0)
    {
        printf("\n ***No se encoontraron libros para prestar.***\n");
        Pause;
        return;
    }
    printf("\n ¿ Quieres sacar algún libro de la biblioteca (s/n) ? ");
    __fpurge(stdin);
    scanf("%c", &resp);

    if (tolower(resp) == 's')
    {
        printf(" Introduce la Posición del libro a solicitar su préstamo: ");
        scanf("%d", &pos);

        tpos.Ida = ida;
        tpos.Pos = pos - 1;

        res = prestar_1(&tpos, clnt);
        if (res == NULL) { clnt_perror(clnt, "Error RPC (Prestar)"); Pause; return; }

        switch (*res)
        {
            case  1: MostrarAviso("\n\n *** El préstamo se ha concedido, recoge el libro en el mostrador.**\n\n"); break;
            case  0: MostrarAviso("\n\n *** No hay ejemplares disponibles. Se le ha apuntado en la lista de espera.**\n\n"); break;
            case -1: MostrarAviso("\n\n *** Error: posición incorrecta.***\n\n"); break;
        }
    }
}

static void OpDevolucion(CLIENT *clnt)
{
    Cadena    texto;
    char      resp;
    int       ida = -1;
    TPosicion tpos;
    int      *res;
    int       pos;
    int mostrados;

    printf("\n Introduce el Isbn a Buscar: ");
    __fpurge(stdin);
    scanf("%s", texto);

    printf("\n");
    mostrados = BuscarYMostrar(clnt, ida, texto, 'i'); //Busco siempre por ISBN
    if(mostrados==0)
    {
        printf("\n *** No se encoontraron libros para devolver.***\n");
        Pause;
        return;
    }
    printf("\n ¿ Quieres devolver algún libro de la biblioteca (s/n) ? ");
    __fpurge(stdin);
    scanf("%c", &resp);

    if (tolower(resp) == 's')
    {
        printf(" Introduce la Posición del libro a devolver: ");
        scanf("%d", &pos);

        tpos.Ida = ida;
        tpos.Pos = pos - 1;

        res = devolver_1(&tpos, clnt);
        if (res == NULL) { clnt_perror(clnt, "Error RPC (Devolver)"); Pause; return; }

        switch (*res)
        {
            case  1: MostrarAviso("\n\n *** Se ha devuelto el libro y se pondrá en la estantería.**\n\n"); break;
            case  0: MostrarAviso("\n\n *** Se ha devuelto el libro, pasará al siguiente usuario en espera.**\n\n"); break;
            case  2: MostrarAviso("\n\n *** Error: el libro no estaba prestado ni reservado.***\n\n"); break;
            case -1: MostrarAviso("\n\n *** Error: posición incorrecta.***\n\n"); break;
        }
    }
}


int main(int argc, char *argv[])
{
    CLIENT *clnt;
    int     op;

    if (argc < 2)
    {
        fprintf(stderr, "Uso: %s <hostname_servidor>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //Conexión RCP
    clnt = clnt_create(argv[1], GESTORBIBLIOTECA, GESTORBIBLIOTECA_VER, "tcp");
    if (clnt == NULL)
    {
        clnt_pcreateerror(argv[1]);
        exit(EXIT_FAILURE);
    }
    do
    {
        op = MenuPrincipal();
        switch (op)
        {
            case 1: OpAdministracion(clnt); break;
            case 2: OpConsulta      (clnt); break;
            case 3: OpPrestamo      (clnt); break;
            case 4: OpDevolucion    (clnt); break;
            case 0: break;
        }
    } while (op != 0);

    clnt_destroy(clnt);
    printf("\n Hasta pronto.\n\n");
    return EXIT_SUCCESS;
}
