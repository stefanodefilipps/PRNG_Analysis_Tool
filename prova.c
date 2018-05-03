#include "unif01.h"
#include "ulcg.h"
#include <stdio.h>
#include "gdef.h"
#include "swrite.h"

/**
Nella compilazione di qualsiasi file che utilizza la libreria TestU01 durante la compilazione
si devono aggiungere i seguenti parametri:
	-ltestu01 -lprobdist -lmylib -lm
altrimenti non riesce a trovare i riferimenti ai file header
**/

int main(int argc, char *argv[]){
	swrite_Basic = TRUE;
	int i;
	double x;
	unif01_Gen *gen;
	long y = 20000;

	gen = ulcg_CreateLCG(2147483647, 16807, 0, 12345);
	sstring_HammingWeight2(gen,NULL,2,y,0,32,y);
	ulcg_DeleteGen(gen);
	return 0;
}