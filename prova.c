#include "unif01.h"
#include "ulcg.h"
#include <stdio.h>
#include "gdef.h"
#include "swrite.h"
#include "ugfsr.h"
#include <sys/types.h>
#include <unistd.h>
#include "huflocal.h"
#include "huffman.h"
#include "bitarray.h"
#include "bitfile.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>


/**
Nella compilazione di qualsiasi file che utilizza la libreria TestU01 durante la compilazione
si devono aggiungere i seguenti parametri:
	-ltestu01 -lprobdist -lmylib -lm
altrimenti non riesce a trovare i riferimenti ai file header

gcc prova.c bitarray.c bitfile.c -o prova
**/

int main(int argc, char *argv[]){
	FILE *inFile, *outFile;
	bit_file_t *bfp;
	count_t count;
    int c;
    int status = -1;        /* in case of premature EOF */
    if ((inFile = fopen(argv[1], "rb")) == NULL){
        perror("Opening Input File");
        if (outFile != NULL){
            fclose(outFile);
        }
        return errno;
    }

    bfp = MakeBitFile(inFile, BF_READ);


    while ((c = BitFileGetChar(bfp)) != EOF){
        BitFileGetBits(bfp, (void *)(&count), 8 * sizeof(count_t));

        if ((count == 0) && (c == 0)){
        	printf("%s\n", argv[1]);
        	char* temp = (char*)malloc(sizeof(char)*strlen(argv[1])+50);
        	int u=0;
        	char m = argv[1][u];
        	while(m!='.'){
            	temp[u]=m;
            	u++;
            	m = argv[1][u];
        	}
        	temp[u]='\0';
        	strcat(temp,"headerLess.HUFF");
        	printf("%s\n",temp );
        	if(access(temp,F_OK) == 0){
            	int result = remove(temp);
            	if(result == -1){
                	perror("errore nella delete");
                	exit(1);
            	}
        	}
        	if ((outFile = fopen(temp, "wb")) == NULL){
            	perror("Opening Output File");
            	if (inFile != NULL){
                	fclose(inFile);
            	}
            	return errno;
        	}
        	while ((c = BitFileGetChar(bfp)) != EOF){
        		fputc(c,outFile);
        	}
            /* we just read end of table marker */
            status = 0;
            break;
            fclose(outFile);
        }
    }
    if (0 != status)
    {
        /* we hit EOF before we read a full header */
        fprintf(stderr, "error: malformed file header.\n");
        errno = EILSEQ;     /* Illegal byte sequence seems reasonable */
    }

    return status;
}
