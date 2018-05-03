#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unif01.h"
#include "ulcg.h"
#include "gdef.h"
#include "swrite.h"
#include "tables.h"
#include "sres.h"

static void ShowUsage(char *progPath);

/**
Nella compilazione di qualsiasi file che utilizza la libreria TestU01 durante la compilazione
si devono aggiungere i seguenti parametri:
	-ltestu01 -lprobdist -lmylib -lm
altrimenti non riesce a trovare i riferimenti ai file header
Devo anche impostare le variabili di ambiente ogni volta che viene riaccesa la macchina
	export LD_LIBRARY_PATH=<install directory>/lib
    export LIBRARY_PATH=<install directory>/lib
    export C_INCLUDE_PATH=<install directory>/include
**/



int main(int argc, char *argv[]){
	

	if(argc < 4){
		ShowUsage(argv[0]);
        return 0;
	}

	if(strcmp(argv[1],"-h") == 0){
        ShowUsage(argv[0]);
        return 0;
    }

    if(strcmp(argv[1],"-p") == 0){	// sono nel caso in cui voglio testare un singolo PNRG
    	swrite_Basic = TRUE;
    	unif01_Gen *gen;
    	if(strcmp(argv[2],"lcg") == 0){		// controllo che voglio analizzare LCG
			gen = ulcg_CreateLCG(2147483647, 16807, 0, 12345);
			if(strcmp(argv[3],"-a") == 0){
				bbattery_FIPS_140_2(gen);
			}
			else{
				if(strcmp(argv[3],"-s") == 0){
					if(strcmp(argv[4],"monobit") == 0){			//eseguo solo il test monobit
						long y = 20000;
						long u = 1;
						sstring_HammingWeight2(gen,NULL,u,y,0,32,y);
					}
				}
				else{
					ShowUsage(argv[0]);
				}
			}
			ulcg_DeleteGen(gen);
    	}
    }

	return 0;
}

static void ShowUsage(char *progPath)
{
    printf("   Usage: %s <options>\n\n", progPath);
    printf("   options:\n");
    printf("   <-p> <lcg> <-a>: Execute all BSI tests on the PNRG LCG.\n");
    printf("   <-p> <lcg> <-s> <monobit>: Execute monobit test on the PNRG LCG.\n");
}