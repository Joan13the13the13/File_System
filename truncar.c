//Autores: Joan Balaguer Llagostera, Eduardo Sánchez, Jaume Adrover 
#include "ficheros.h"

int main(int argc, char *argv[]){

    struct STAT p_stat;

    //Validación sintaxis
    if (argc != 4){
        fprintf(stderr, "Error de sintaxis: ./truncar <nombre_dispositivo> <ninodo> <nbytes>\n");
        return EXIT_FAILURE_1;
    }

    int ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);

    //Montamos dispositivo
    if (bmount(argv[1]) == EXIT_FAILURE_1){
        fprintf(stderr, "Error al montar el dispositivo virtual.\n");
        return EXIT_FAILURE_1;
    }

    //Liberamos el inodo si no truncamos el archivo.
    if (nbytes == 0){
        if (liberar_inodo(ninodo) == EXIT_FAILURE_1){
            fprintf(stderr, "Error al liberar el inodo %i.\n", ninodo);
            return EXIT_FAILURE_1;
        }
    }
    else{
        mi_truncar_f(ninodo, nbytes);
    }


    if (mi_stat_f(ninodo, &p_stat)){
        fprintf(stderr, "Error en mi_stat_f()\n");
        return EXIT_FAILURE;
    }

    // Para imprimir fecha y hora
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    ts = localtime(&p_stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    //Información del inodo escrito
    printf("\nDATOS INODO %d:\n", ninodo);
    printf("tipo=%c\n", p_stat.tipo);
    printf("permisos=%d\n", p_stat.permisos);
    printf("atime: %s\n", atime);
    printf("ctime: %s\n", ctime);
    printf("mtime: %s\n", mtime);
    printf("nLinks= %d\n", p_stat.nlinks);
    printf("tamEnBytesLog= %d\n", p_stat.tamEnBytesLog);
    printf("numBloquesOcupados= %d\n", p_stat.numBloquesOcupados);

    //Desmontar
    if (bumount() == EXIT_FAILURE_1){
        fprintf(stderr, "Error desmontando dispositivo.\n");
        return EXIT_FAILURE_1;
    }

    return EXIT_SUCCESS;
}