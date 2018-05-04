#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>
#include "unif01.h"
#include "ulcg.h"
#include "gdef.h"
#include "swrite.h"
#include "tables.h"
#include "sres.h"
#include "fmass.h"
#include "statcoll.h"
#include "gofw.h"
#include "ugfsr.h"
#include "bitarray.h"
#include "bitfile.h"
#include "ufile.h"

static void ShowUsage(char *progPath);
static int delete_Huffman_header(char* path);
static char* Compress_Name(char* name,int header);
static int all_or_single(char* param);
static off_t FileDimension(int file);

typedef unsigned int count_t;


/**
Nella compilazione di qualsiasi file che utilizza la libreria TestU01 durante la compilazione
si devono aggiungere i seguenti parametri:
	-ltestu01 -lprobdist -lmylib -lm
altrimenti non riesce a trovare i riferimenti ai file header
Devo anche impostare le variabili di ambiente ogni volta che viene riaccesa la macchina
	export LD_LIBRARY_PATH=<install directory>/lib
    export LIBRARY_PATH=<install directory>/lib
    export C_INCLUDE_PATH=<install directory>/include



    gcc tool_analysis.c bitarray.c bitfile.c -o tool_analysis -ltestu01 -lprobdist -lmylib -lm
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

    /**
    Il controllo che segue deve essere fatto per forza in questo punto e non quando mi trovo a trattare gli algoritmi di compressione. Questo perchè quando sono con gli 
    algoritmi di compressione lancio un nuovo processo con fork e se mi accorgo di questa anomalia e faccio show usage e return 0, esco dal processo figlio, che di 
    conseguenza mi sblocca la wait del padre che pensa che sia andato tutto corretto e continua con la chiamata a gen di un file che non esiste perchè non esisteva un 
    algoritmo di compressione con quel nome e quindi il figlio non ha fatto niente  eno ha creato nessun file compresso.
    **/

    if(strcmp(argv[1],"-c") == 0 && (strcmp(argv[2],"lcg") == 0 || strcmp(argv[2],"mersenne_t") == 0)){			
    	ShowUsage(argv[0]);
        return 0;
    }

    if(strcmp(argv[1],"-c") == 0 && (argc < 5 || (strcmp(argv[3],"-s") == 0 && argc < 6))){
    	ShowUsage(argv[0]);
        return 0;	
    }

    unif01_Gen *gen;

    long y = 20000;
	long u = 1;				// Da definire come costanti
	long s = 12345;

    if(strcmp(argv[1],"-p") == 0){	// sono nel caso in cui voglio testare un singolo PNRG
    	swrite_Basic = TRUE;
    	if(strcmp(argv[2],"lcg") == 0){		// controllo che voglio analizzare LCG
			gen = ulcg_CreateLCG(2147483647, 16807, 0, s);
		}
		else{
			if(strcmp(argv[2],"mersenne_t") == 0){
				gen = ugfsr_CreateMT19937_98(s);
			}
			else{
				ShowUsage(argv[0]);				// se in input mi è detto di aspettarmi un PNRG ma nessuno dei due possibili allora faccio vedere i possibili comandi 
				return 0;						// e faccio return, altrimenti anche se faccio show usage poi vado a vedere terzo elemento di argv ma non ho gen
			}									// e quindi mi va in errore i vari test
		}
    }


    if(strcmp(argv[1],"-c") == 0){			// Devo testare un algoritmo di compressione dati
    	pid_t pid = fork();					// Creo un processo figlio perchè poi dovrò lanciare con execv l'algoritmo di Huffman
     	if(pid == -1){
         printf("Errore fork");
         exit(1);
     	}
     	if(pid == 0){
    		char* program_name;
        	char* program_arguments[5];
        	program_arguments[4]=NULL;
        	if(strcmp(argv[2],"Huffman")==0){
        		program_name = "/home/biar/Desktop/PNRG_analisi/HUFFMAN";
            	program_arguments[0]="/home/biar/Desktop/PNRG_analisi/HUFFMAN";
            	program_arguments[1]="-c";
            	program_arguments[2]="-i";
            	program_arguments[3]=argv[all_or_single(argv[3])];
        	}
        	else{
        		ShowUsage(argv[0]);
        		return 0;
        	}

        	int res = execv(program_name,program_arguments);			//lancio l'algoritmo di compressione dati e creo il file compresso nella stella cartella dove si trova tool_analysis
        	if(res == -1){
            	printf("errore execv\n");
            	printf("%s\n",strerror(errno));
            	exit(1);
        	}
    	}
    	pid = wait(NULL);						// ora sono nel padre e quindi nel mio tool analysis
    	if(pid == -1){
        	printf("errore wait");
        	exit(1);
    	}
    	if(strcmp(argv[2],"Huffman")==0){
    		char* temp = Compress_Name(argv[all_or_single(argv[3])],0);	// mi sono creato il path al file codificato appena creato da passare alla funzione successiva
    		delete_Huffman_header(temp);
    		temp = Compress_Name(argv[all_or_single(argv[3])],1);
    		printf("%s\n", temp);
    		gen = ufile_CreateReadBin(temp,1);
    	}
    
    }

    if(strcmp(argv[2],"lcg") == 0 || strcmp(argv[2],"mersenne_t") == 0) y = 20000;
    else{
    	char* temp = Compress_Name(argv[all_or_single(argv[3])],1);
		y = FileDimension(open(temp,O_RDONLY));
    }

    if(strcmp(argv[3],"-a") == 0){
		bbattery_FIPS_140_2(gen);
	}
	else{
		if(strcmp(argv[3],"-s") == 0){
			if(strcmp(argv[4],"monobit") == 0){			//eseguo solo il test monobit
				sstring_HammingWeight2(gen,NULL,u,y,0,32,y);
			}
			else{
				if(strcmp(argv[4],"poker") == 0){		// eseguo solo il poker test
					smultin_MultinomialBits(gen,NULL,NULL,1,5000,0,32,4,FALSE);
				}
				else{
					if(strcmp(argv[4],"run") == 0){		// eseguo solo il run test
						sstring_Run(gen,NULL,1,y,0,32);
					}
					else{
						if(strcmp(argv[4],"longRun") == 0){		// eseguo solo il longrun test
							long f = 1500;
							sstring_LongestHeadRun(gen,NULL,u,y,0,32,f);
						}
						else{
							ShowUsage(argv[0]);
						}
					}
				}
			}
		}
	}
	if(strcmp(argv[2],"lcg") == 0){
		ulcg_DeleteGen(gen);
	}
	else{
		if(strcmp(argv[2],"mersenne_t") == 0){
			ugfsr_DeleteGen(gen);
		}
		else{
			ufile_DeleteReadBin(gen);
		}
	}

	return 0;
}

static void ShowUsage(char *progPath)
{
    printf("   Usage: %s <options>\n\n", progPath);
    printf("   options:\n");
    printf("   <-p> <alg> <-a>: Execute all BSI tests on a PNRG .\n");
    printf("   <-p> <alg> <-s> <monobit>: Execute monobit test on a PNRG.\n");
    printf("   <-p> <alg> <-s> <poker>: Execute poker test on a PNRG.\n");
    printf("   <-p> <alg> <-s> <run>: Execute run test on a PNRG.\n");
    printf("   <-p> <alg> <-s> <longRun>: Execute longrun test on a PNRG.\n\n");
    printf("   acceptable PNRG:\n\n    lcg.\n    mersenne_t.\n\n");
    printf("   <-c> <alg> <-a> <path_file>: Execute all BSI tests on a compression algorithm.\n");
    printf("   <-c> <alg> <-s> <monobit> <path_file>: Execute monobit test on a compression algorithm.\n");
    printf("   <-c> <alg> <-s> <poker> <path_file>: Execute poker test on a compression algorithm.\n");
    printf("   <-c> <alg> <-s> <run> <path_file>: Execute run test on a compression algorithm.\n");
    printf("   <-c> <alg> <-s> <longRun> <path_file>: Execute longrun test on a compression algorithm.\n\n");
    printf("   acceptable algorithms:\n\n    Huffman.\n");
}

static int delete_Huffman_header(char* path){				// questa funzione serve per eliminare le informazione di header dell'Huffman Tree così da avere solo dati compressi da poter testare
	FILE *inFile, *outFile;
	bit_file_t *bfp;
	count_t count;
    int c;
    int status = -1;        /* in case of premature EOF */
    if ((inFile = fopen(path, "rb")) == NULL){
        perror("Opening Input File");
        if (outFile != NULL){
            fclose(outFile);
        }
        return errno;
    }

    bfp = MakeBitFile(inFile, BF_READ);


    while ((c = BitFileGetChar(bfp)) != EOF){
        BitFileGetBits(bfp, (void *)(&count), 8 * sizeof(count_t));

        if ((count == 0) && (c == 0)){							// le informazioni di header sono terminate con il valore 0 contato 0 volte
        	char* temp = (char*)malloc(sizeof(char)*strlen(path)+50);
        	int u=0;
        	char m = path[u];
        	while(m!='.'){
            	temp[u]=m;
            	u++;
            	m = path[u];
        	}
        	temp[u]='\0';
        	strcat(temp,"headerLess.HUFF");
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
            fclose(outFile);
            fclose(inFile);
            status = 0;
            break;
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

static char* Compress_Name(char* name,int header){
	char* temp = (char*)malloc(sizeof(char)*strlen(name)+50);
    int u=0;
    char m = name[u];
    while(m!='.'){
        temp[u]=m;
        u++;
        m = name[u];
    }
    temp[u]='\0';
    if(header == 0){
    	strcat(temp,".HUFF");
    }
    else{
    	strcat(temp,"headerLess.HUFF");
    }
    return temp;
}

static int all_or_single(char* param){
	if(strcmp(param,"-a") == 0) return 4;
	return 5;
}

static off_t FileDimension(int file){
    struct stat buf;
    fstat(file,&buf);
    return buf.st_size;
}
