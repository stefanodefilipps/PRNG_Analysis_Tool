#include "unif01.h"
#include "ulcg.h"
#include <stdio.h>

/**
Nella compilazione di qualsiasi file che utilizza la libreria TestU01 durante la compilazione
si devono aggiungere i seguenti parametri:
	-ltestu01 -lprobdist -lmylib -lm
altrimenti non riesce a trovare i riferimenti ai file header
**/

int main(void){
	int i;
	double x;
	unif01_Gen *gen;

	gen = ulcg_CreateLCG(2147483647, 16807, 0, 12345);
	x = 0.0;
	for(i=0;i<50;i++){
		x = gen->GetU01(gen->param,gen->state);
		printf("%lf\n",x);
	}
	ulcg_DeleteGen(gen);
	return 0;
}