//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover

#include "verificacion.h"

#define DEBUGN13 1

int main (int argc, const char *argv[]){

    struct STAT stat;
    char buffer[1024]; //buffer para escribir en fichero

    if (argc != 3){
        fprintf(stderr, "Error de sintaxis: ./verificacion <nombre_dispositivo> <directorio_simulación>\n");
        return EXIT_FAILURE_1;
    }


    if(bmount(argv[1])==EXIT_FAILURE_1){
        fprintf(stderr,"Error al montar el dispositivo virtual.");
        return EXIT_FAILURE_1;
    }

    mi_stat(argv[2], &stat);

//  Calcular el nº de entries del directorio de simulación a partir del stat de su inodo
    int numentradas = (stat.tamEnBytesLog / sizeof(struct entrada));
    if (numentradas != NUMPROCESOS){
        fprintf(stderr,"NUM ENTRADAS != NUMPROCESOS");
        return EXIT_FAILURE_1;
    }  

    char nombreFich[50];
    sprintf(nombreFich, "%s%s", argv[2], "informe.txt");
    if (mi_creat(nombreFich, 6) < 0){ 
        bumount(argv[1]);
        exit(0);
    }
    int bufferbytes = 0;

    //Cargamos las entries
    struct entrada entries[numentradas];
    int error = mi_read(argv[2], entries, 0, NUMPROCESOS * sizeof(struct entrada));
    if (error <= 1)
    {
        mostrar_error_buscar_entrada(error);
        return EXIT_FAILURE_1;
    }

    for (int n = 0; n < numentradas; n++){
        
        pid_t pid = atoi(strchr(entries[n].nombre, '_') + 1);//Extraer el PID a partir del nombre de la entrada y guardarlo en el registro info 
        struct INFORMACION info;
        info.pid = pid;
        info.nEscrituras = 0;

        char prueba[110]; //con 110 caracteres basta para almacenar la ruta (50+60)
        sprintf(prueba, "%s%s/%s", argv[2], entries[n].nombre, "prueba.dat");

        //Buffer de N registros de escrituras
        int cant_registros_buffer_escrituras = 256; //N=256, 256*24=6*1024
        struct REGISTRO buffer_escrituras [cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
        int offset = 0;
        //Mientras haya escrituras en prueba.dat
        while (mi_read(prueba, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0) {
            for (int numR=0;numR<cant_registros_buffer_escrituras;numR++){
//          Si la escritura es válida entonces
                if (buffer_escrituras[numR].pid == info.pid){
                    if(info.nEscrituras==0){
                        info.PrimeraEscritura = buffer_escrituras[numR];
                        info.UltimaEscritura = buffer_escrituras[numR];
                        info.MenorPosicion = buffer_escrituras[numR];
                        info.MayorPosicion = buffer_escrituras[numR];
                        info.nEscrituras++;
                    }else{
                        //Actualizamos los datos de las fechas la primera y la última escritura si se necesita
                        if ((difftime(buffer_escrituras[numR].fecha, info.PrimeraEscritura.fecha)) <= 0 &&
                            buffer_escrituras[numR].nEscritura < info.PrimeraEscritura.nEscritura){
                            info.PrimeraEscritura = buffer_escrituras[numR];
                        }
                        if ((difftime(buffer_escrituras[numR].fecha, info.UltimaEscritura.fecha)) >= 0 &&
                            buffer_escrituras[numR].nEscritura > info.UltimaEscritura.nEscritura){
                            info.UltimaEscritura = buffer_escrituras[numR];
                        }
                        if (buffer_escrituras[numR].nRegistro < info.MenorPosicion.nRegistro){
                            info.MenorPosicion = buffer_escrituras[numR];
                        }
                        if (buffer_escrituras[numR].nRegistro > info.MayorPosicion.nRegistro){
                            info.MayorPosicion = buffer_escrituras[numR];
                        }
                        info.nEscrituras++;
                    }
                }
            }
            memset(&buffer_escrituras, 0, sizeof(buffer_escrituras));
            offset += sizeof(buffer_escrituras);
        }
#if DEBUGN13
        fprintf(stderr, "[%i) %i escrituras validadas en %s]\n", n + 1, info.nEscrituras, prueba);
#endif

        struct tm *tm;
        char tPrimeraE[80];
        char tUltimaE[80];
        char tMenorP[80];
        char tMayorP[80];


        tm = localtime(&info.PrimeraEscritura.fecha);
        strftime(tPrimeraE, sizeof(tPrimeraE), "%a %d-%m-%Y %H:%M:%S", tm);
        tm = localtime(&info.UltimaEscritura.fecha);
        strftime(tUltimaE, sizeof(tUltimaE), "%a %d-%m-%Y %H:%M:%S", tm);
        tm = localtime(&info.MenorPosicion.fecha);
        strftime(tMenorP, sizeof(tMenorP), "%a %d-%m-%Y %H:%M:%S", tm);
        tm = localtime(&info.MayorPosicion.fecha);
        strftime(tMayorP, sizeof(tMayorP), "%a %d-%m-%Y %H:%M:%S", tm);

        memset(buffer, 0, 1024);
        sprintf(buffer,"PID: %d\nNumero de escrituras:\t%d\nPrimera escritura:\t%d\t%d\t%s\n"
                "Ultima escritura:\t%d\t%d\t%s\nMenor posición:\t\t%d\t%d\t%s\nMayor posición:\t\t%d\t%d\t%s\n\n\n",
                info.pid, info.nEscrituras,info.PrimeraEscritura.nEscritura,info.PrimeraEscritura.nRegistro,
                tPrimeraE,info.UltimaEscritura.nEscritura,info.UltimaEscritura.nRegistro,
                tUltimaE,info.MenorPosicion.nEscritura,info.MenorPosicion.nRegistro,
                tMenorP,info.MayorPosicion.nEscritura,info.MayorPosicion.nRegistro,
                tMayorP);

        //Añadir info al fichero informe.txt por el final
        if ((bufferbytes += mi_write(nombreFich, &buffer,bufferbytes, strlen(buffer))) < 0){
            printf("verificacion.c -> Error al escribir el fichero: '%s'\n", nombreFich);
            bumount();
            return EXIT_FAILURE_1;
        }
    }
    if (bumount() < 0){
        fprintf(stderr, "Error desmontando el dispositivo\n");
        exit(0);
    }
}