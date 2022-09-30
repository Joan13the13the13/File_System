//Autores: Eduardo Sánchez, Joan Balaguer, Jaume Adrover
#include "simulacion.h"
 
struct INFORMACION {
  int pid;
  unsigned int nEscrituras; 
  struct REGISTRO PrimeraEscritura;
  struct REGISTRO UltimaEscritura;
  struct REGISTRO MenorPosicion;
  struct REGISTRO MayorPosicion;
};
