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
#include "sstring.h"
#include "smultin.h"
#include "gofs.h"
#include "sspectral.h"
#include <math.h>

static void ShowUsage(char *progPath);
static int delete_Huffman_header(char* path);
static char* Compress_Name(char* name,int header,char* alg,int vel);
static int all_or_single(char* param,int arg);
static off_t FileDimension(int file);
static char* ParseFileName(char *path);
static void fprintfAlg(FILE* report, char** argv, int argc);
static int my_log2(double x);

typedef unsigned int count_t;


/**
Nella compilazione di qualsiasi file che utilizza la libreria TestU01 durante la compilazione
si devono aggiungere i seguenti parametri:
	-ltestu01 -lprobdist -lmylib -lm
altrimenti non riesce a trovare i riferimenti ai file header
Devo anche impostare le variabili di ambiente ogni volta che viene riaccesa la macchina

export LD_LIBRARY_PATH=/home/biar/Desktop/PNRG_analisi/lib
export LIBRARY_PATH=/home/biar/Desktop/PNRG_analisi/lib
export C_INCLUDE_PATH=/home/biar/Desktop/PNRG_analisi/include



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
	int vel = 0;
    if(argc == 6 && strcmp(argv[3],"-a") == 0 ) vel = atoi(argv[4]+1);
    if(argc == 7) vel = atoi(argv[5]+1);
	

    FILE* report=fopen("/home/biar/Desktop/PNRG_analisi/Report.txt","a");
    if(report == NULL){
        perror("errore nell'apertura del file di report");
        exit(1);
    }
    if(argc == 6){
        fprintf(report,"\n%s;",ParseFileName(argv[5]));
    } 
    else{
        fprintf(report,"\n%s;",ParseFileName(argv[6]));
    }

    if(strcmp(argv[1],"-c") == 0){                       			// Devo testare un algoritmo di compressione dati
    	pid_t pid = fork();					// Creo un processo figlio perchè poi dovrò lanciare con execv l'algoritmo di Huffman
     	if(pid == -1){
         printf("Errore fork");
         exit(1);
     	}
     	if(pid == 0){
    		char* program_name;
        	char* program_arguments[7];
        	program_arguments[4] = program_arguments[5] = NULL;
        	if(strcmp(argv[2],"Huffman")==0){
        		program_name = "/home/biar/Desktop/PNRG_analisi/HUFFMAN";
            	program_arguments[0]="/home/biar/Desktop/PNRG_analisi/HUFFMAN";
            	program_arguments[1]="-c";
            	program_arguments[2]="-i";
            	program_arguments[3]=argv[all_or_single(argv[3],argc)];
        	}
        	else{
        		if(strcmp(argv[2],"LZW") == 0){
        			if(argc == 7){
        				if(strcmp(argv[5],"-9") == 0){
        					program_name = "/home/biar/Desktop/PNRG_analisi/LZW_4_9_bit/LZW";
        				}
        				else {
                            program_name = "/home/biar/Desktop/PNRG_analisi/LZW_15_bit/LZW";
                        }

        			}
        			else{
        				if(argc == 6 && strcmp(argv[3],"-a") == 0){
        				if(strcmp(argv[4],"-9") == 0){
        					program_name = "/home/biar/Desktop/PNRG_analisi/LZW_4_9_bit/LZW";
        				}
        				else {
                            program_name = "/home/biar/Desktop/PNRG_analisi/LZW_15_bit/LZW";
                        }
        				}
        				else {
                            program_name = "/home/biar/Desktop/PNRG_analisi/LZW";
                            }	
        			}
            		program_arguments[0]="/home/biar/Desktop/PNRG_analisi/LZW";
            		program_arguments[1]="-c";
            		program_arguments[2]="-i";
            		program_arguments[3]=argv[all_or_single(argv[3],argc)];
        		}
        		else{
        			if(strcmp(argv[2],"bzip2") == 0){
        				program_name = "/bin/bzip2";
             			program_arguments[0]="bzip2";
             			program_arguments[1]="-k";
             			program_arguments[2]="-f";
             			if(argc == 6){
             				program_arguments[3]=argv[all_or_single(argv[3],argc)];
             			}
             			else{
             				if(argc == 6 && strcmp(argv[3],"-a") == 0 ){
             					int vel = atoi(argv[4]+1);				// +1 perchè da linea di comando ho -number e quindi io devo prendere solo number
             					if(vel == 0 || vel < 1 || vel > 9){
             					ShowUsage(argv[0]);
             					return 0;
             					}
             				}
             				else{
             					int vel = atoi(argv[5]+1);				// +1 perchè da linea di comando ho -number e quindi io devo prendere solo number
             					if(vel == 0 || vel < 1 || vel > 9){
             					ShowUsage(argv[0]);
             					return 0;
             					}
             				}
             				program_arguments[3]=argv[5];
             				program_arguments[4]=argv[all_or_single(argv[3],argc)];
             			}
        			}
        			else{
        				if(strcmp(argv[2],"gzip") == 0){
        					program_name = "/bin/gzip";
             				program_arguments[0]="gzip";
             				program_arguments[1]="-k";
             				program_arguments[2]="-f";
             				if(argc == 6){
             					program_arguments[3]=argv[all_or_single(argv[3],argc)];
             				}
             				else{
             					if(argc == 6 && strcmp(argv[3],"-a") == 0 ){
             						int vel = atoi(argv[4]+1);				// +1 perchè da linea di comando ho -number e quindi io devo prendere solo number
             						if(vel == 0 || vel < 1 || vel > 9){
             						ShowUsage(argv[0]);
             						return 0;
             						}
             					}
             					else{
             						int vel = atoi(argv[5]+1);				// +1 perchè da linea di comando ho -number e quindi io devo prendere solo number
             						if(vel == 0 || vel < 1 || vel > 9){
             						ShowUsage(argv[0]);
             						return 0;
             						}
             					}
             					program_arguments[3]=argv[5];
             					program_arguments[4]=argv[all_or_single(argv[3],argc)];
             				}	
             			}
             			else{
             				if(strcmp(argv[2],"lzma") == 0){
        						program_name = "/usr/bin/lzma";
        						printf("%s\n", program_name);
             					program_arguments[0]="lzma";
             					program_arguments[1]="-k";
             					program_arguments[2]="-f";
             					if(argc == 6){
             						program_arguments[3]=argv[all_or_single(argv[3],argc)];
             					}
             					else{
             						if(argc == 6 && strcmp(argv[3],"-a") == 0 ){
             							int vel = atoi(argv[4]+1);				// +1 perchè da linea di comando ho -number e quindi io devo prendere solo number
             							if(vel == 0 || vel < 1 || vel > 9){
             							ShowUsage(argv[0]);
             							return 0;
             							}
             						}
             						else{
             							int vel = atoi(argv[5]+1);				// +1 perchè da linea di comando ho -number e quindi io devo prendere solo number
             							if(vel == 0 || vel < 1 || vel > 9){
             							ShowUsage(argv[0]);
             							return 0;
             							}
             						}
             						program_arguments[3]=argv[5];
             						program_arguments[4]=argv[all_or_single(argv[3],argc)];
             					}	
             				}
             				else{
             					if(strcmp(argv[2],"Aritmetic")==0){
        							program_name = "/home/biar/Desktop/PNRG_analisi/ARITMETIC";
            						program_arguments[0]="/home/biar/Desktop/PNRG_analisi/ARITMETIC";
            						program_arguments[1]="-a";
            						program_arguments[2]="-i";
            						program_arguments[3]=argv[all_or_single(argv[3],argc)];
            						program_arguments[4]="-o";
            						program_arguments[5]= Compress_Name(argv[all_or_single(argv[3],argc)],0,argv[2],vel);
        						}
             				}
             			}
        			}
        		}
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
    		char* temp = Compress_Name(argv[all_or_single(argv[3],argc)],0,argv[2],vel);	// mi sono creato il path al file codificato appena creato da passare alla funzione successiva
    		if(strcmp(argv[2],"Huffman")==0){
    			delete_Huffman_header(temp);
    			temp = Compress_Name(argv[all_or_single(argv[3],argc)],1,argv[2],vel);
    		}
    		printf("%s\n", temp);
    		gen = ufile_CreateReadBin(temp,1);
    
    }

    if(strcmp(argv[1],"-f") == 0){								// sto testando dei file che derivano da lossy algorithm quali jpeg o mp3 ecc
    	gen = ufile_CreateReadBin(argv[2],1);	
    	printf("%s\n", argv[2]);
    }

    if(strcmp(argv[2],"lcg") == 0 || strcmp(argv[2],"mersenne_t") == 0) y = 20000;
    else{
    	if(strcmp(argv[1],"-f") == 0){
    		y = FileDimension(open(argv[2],O_RDONLY));	
    	}
    	else{
    		char* temp = Compress_Name(argv[all_or_single(argv[3],argc)],1,argv[2],vel);
			y = FileDimension(open(temp,O_RDONLY));
    	}
    }

    if(strcmp(argv[3],"-a") == 0){
		bbattery_FIPS_140_2(gen);
	}
	else{
        fprintfAlg(report,argv,argc);
		if(strcmp(argv[3],"-s") == 0){
			if(strcmp(argv[4],"monobit") == 0){			//eseguo solo il test monobit
				sres_Basic * res = sres_CreateBasic();
				sstring_HammingWeight2(gen,res,u,y,0,32,y);
                fflush(report);
                fprintf(report,"monobit;");
				printf("STATISTIC VALUE %lf \n", res->sVal2[gofw_Mean] );
				printf("P VALUE %lf \n", res->pVal2[gofw_Mean] );
                fflush(report);
                fprintf(report,"%lf;%lf\n\n",res->sVal2[gofw_Mean],res->pVal2[gofw_Mean]);
			}
			else{
				if(strcmp(argv[4],"poker") == 0){		// eseguo solo il poker test
                    fflush(report);
                    fprintf(report,"poker;");
					smultin_Res* res = smultin_CreateRes(NULL);
					smultin_MultinomialBits(gen,NULL,res,1,5000,0,32,4,FALSE);
					printf("STATISTIC VALUE %lf \n", res->sVal2[1][gofw_Mean]);
					printf("P VALUE %lf \n", res->pVal2[1][gofw_Mean]);
                    fflush(report);
                    fprintf(report,"%lf;%lf\n\n",res->sVal2[1][gofw_Mean],res->pVal2[1][gofw_Mean]);
				}
				else{
					if(strcmp(argv[4],"run") == 0){		// eseguo solo il run test
                        fflush(report);
                        fprintf(report,"run;");
						sstring_Res3* res = sstring_CreateRes3();
						sstring_Run(gen,res,1,y,0,32);
						printf("STATISTIC VALUE %lf \n", res->NRuns->sVal2[gofw_Mean] );
						printf("P VALUE %lf \n", res->NRuns->pVal2[gofw_Mean] );
                        fflush(report);
                        fprintf(report,"%lf;%lf\n\n",res->NRuns->sVal2[gofw_Mean],res->NRuns->pVal2[gofw_Mean]);
					}
					else{
						if(strcmp(argv[4],"longRun") == 0){		// eseguo solo il longrun test
                            fflush(report);
                            fprintf(report,"longRun;");
							long f = 1500;
							sstring_Res2* res  = sstring_CreateRes2();
							sstring_LongestHeadRun(gen,res,u,y,0,32,f);
							printf("STATISTIC VALUE %lf \n", res->Chi->sVal2[gofw_Mean] );
							printf("P VALUE %lf \n", res->Chi->pVal2[gofw_Mean] );
                            fflush(report);
                            fprintf(report,"%lf;%lf\n\n",res->Chi->sVal2[gofw_Mean],res->Chi->pVal2[gofw_Mean]);
						}
						else{
							if(strcmp(argv[4],"autoC") == 0){
                                fflush(report);
                                fprintf(report,"autoC;");
								sres_Basic * res = sres_CreateBasic();
								sstring_AutoCor(gen,res,1,y,0,32,1000);
								printf("STATISTIC VALUE %lf \n", res->sVal2[gofw_Mean] );
								printf("P VALUE %lf \n", res->pVal2[gofw_Mean] );
                                fflush(report);
                                fprintf(report,"%lf;%lf\n\n",res->sVal2[gofw_Mean],res->pVal2[gofw_Mean]);
							}
							else{
                                if(strcmp(argv[4],"Matrix") == 0){
                                    fflush(report);
                                    fprintf(report, "MatrixRank;");
                                    sres_Chi2 * res = sres_CreateChi2();
                                    int vero = 0;
                                    if ((y/2) > gofs_MinExpected)
                                    {
                                        vero = 1;
                                    }
                                    printf("MINORE %d\n", vero);
                                    printf("MINIMO:   %lf    %ld\n", gofs_MinExpected, y/2);
                                    smarsa_MatrixRank(gen,res,1,y,0,32,32,32);
                                    printf("STATISTIC VALUE %lf \n", res->sVal2[gofw_Mean] );
                                    printf("P VALUE %lf \n", res->pVal2[gofw_Mean] );
                                    fflush(report);
                                    fprintf(report,"%lf;%lf\n\n",res->sVal2[gofw_Mean],res->pVal2[gofw_Mean]);
                                }
                                else{
                                    if(strcmp(argv[4],"Fourier") == 0){
                                        fflush(report);
                                        fprintf(report, "Fourier;");
                                        sspectral_Res * res = sspectral_CreateRes();
                                        sspectral_Fourier1(gen,res,1,log2(((double)y)),0,32);
                                        printf("STATISTIC VALUE %lf \n", res->Bas->sVal2[gofw_Mean] );
                                        printf("P VALUE %lf \n", res->Bas->pVal2[gofw_Mean] );
                                        fflush(report);
                                        fprintf(report,"%lf;%lf\n\n",res->Bas->sVal2[gofw_Mean],res->Bas->pVal2[gofw_Mean]);
                                    }
                                    else{
                                       if(strcmp(argv[4],"NOTemplateMatching") == 0){
                                           fflush(report);
                                           fprintf(report, "NOTemplateMatching;");
                                           sres_Poisson * res = sres_CreatePoisson(); 
                                           unsigned long key = 1;
                                           smarsa_CATBits(gen,res,1,y,0,32,5,key);
                                           printf("STATISTIC VALUE %lf \n", res->sVal2);
                                           printf("P VALUE %lf \n", res->pVal2);
                                           fflush(report);
                                           fprintf(report,"%lf;%lf\n\n",res->sVal2,res->pVal2);
                                       }
                                       else ShowUsage(argv[0]); 
                                    }
                                } 
                            }
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
    printf("   <-c> <alg> <-a> <-vel*> <path_file>: Execute all BSI tests on a compression algorithm.\n");
    printf("   <-c> <alg> <-s> <monobit> <-vel*> <path_file>: Execute monobit test on a compression algorithm.\n");
    printf("   <-c> <alg> <-s> <poker> <-vel*> <path_file>: Execute poker test on a compression algorithm.\n");
    printf("   <-c> <alg> <-s> <run> <-vel*> <path_file>: Execute run test on a compression algorithm.\n");
    printf("   <-c> <alg> <-s> <longRun> <-vel*> <path_file>: Execute longrun test on a compression algorithm.\n");
    printf("   <-c> <alg> <-s> <autoC> <-vel*> <path_file>: Execute auto correlation test on a compression algorithm.\n");
    printf("   <-c> <alg> <-s> <Matrix> <-vel*> <path_file>: Execute auto correlation test on a compression algorithm.\n");
    printf("   <-c> <alg> <-s> <Fourier> <-vel*> <path_file>: Execute auto correlation test on a compression algorithm.\n");
    printf("   <-c> <alg> <-s> <NOTemplateMatching> <-vel*> <path_file>: Execute auto correlation test on a compression algorithm.\n");
    printf("   acceptable algorithms:\n\n    Huffman\n\n    LZW\n\n    Aritmetic\n\n    bzip2\n\n    gzip\n\n    lzma\n\n");
    printf("   <-vel> parameter is optional and it can assume values:\n\n in range [1,9] if the compressione algorithm is bzip2,gzip,lzma.\n 9 or 15 if the compression algorithm is LZW\n\n");
    printf("   <-f> <pathfile> and same parameters above to test directly a file");
}


static int my_log2(double x) { 
    return (int)(floor((log(x) / log(2)))); 
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

static char* Compress_Name(char* name,int header,char* alg,int vel){
	char* temp = (char*)malloc(sizeof(char)*strlen(name)+50);
    int u=0;
    char m = name[u];
    while(m!='.'){
        temp[u]=m;
        u++;
        m = name[u];
    }
    temp[u]='\0';
    if(header == 0 && strcmp(alg,"Huffman") == 0){
    	strcat(temp,".HUFF");
    }
    if(header == 1 && strcmp(alg,"Huffman") == 0){
    	strcat(temp,"headerLess.HUFF");
    }
    if(strcmp(alg,"LZW") == 0){
    	if(vel == 9)strcat(temp,".LZW9");
    	if(vel == 15)strcat(temp,".LZW5");
    	if(vel == 0) strcat(temp,".LZW");
    }
    if(strcmp(alg,"bzip2") == 0){
    	strcpy(temp,name);
    	strcat(temp,".bz2");
    }
    if(strcmp(alg,"gzip") == 0){
    	strcpy(temp,name);
    	strcat(temp,".gz");
    }
    if(strcmp(alg,"lzma") == 0){
    	strcpy(temp,name);
    	strcat(temp,".lzma");
    }
    if(strcmp(alg,"Aritmetic") == 0){
    	strcat(temp,".ARIT");
    }
    return temp;
}

static int all_or_single(char* param,int arg){
	if(arg == 7){
		if(strcmp(param,"-a") == 0) return 5;
		return 6;
	}
	if(arg == 6 && strcmp(param,"-a") == 0 ) return 5;
	if(strcmp(param,"-a") == 0) return 4;
	return 5;
}

static off_t FileDimension(int file){
    struct stat buf;
    fstat(file,&buf);
    return buf.st_size;
}

static char* ParseFileName(char *path){
    char* slash = strrchr(path,'/');
    char* name = (char*)malloc(sizeof(char)*strlen(slash));
    strcpy(name,slash+1);
    return name;

}

static void fprintfAlg(FILE* report, char** argv, int argc){
    if(strcmp(argv[2],"Huffman")==0){
        fprintf(report,"Huffman;");
    }
    else{
        if(strcmp(argv[2],"LZW") == 0){
            if(argc == 7){
                if(strcmp(argv[5],"-9") == 0){
                    fprintf(report,"LZW -9;");
                }
                else {
                    fprintf(report,"LZW -15;");
                }

            }
            else{
                if(argc == 6 && strcmp(argv[3],"-a") == 0){
                if(strcmp(argv[4],"-9") == 0){
                    fprintf(report,"LZW -9;");
                }
                else {
                    fprintf(report,"LZW -15;");
                }
                }
                else {
                    fprintf(report,"LZW;");
                    }   
            }
        }
        else{
            if(strcmp(argv[2],"bzip2") == 0){
                if(argc == 6){
                    fprintf(report,"bzip2;");
                }
                else{
                    if(argc == 6 && strcmp(argv[3],"-a") == 0 ){
                        int vel = atoi(argv[4]+1);              // +1 perchè da linea di comando ho -number e quindi io devo prendere solo number
                        if(vel == 0 || vel < 1 || vel > 9){
                        ShowUsage(argv[0]);
                        }
                        fprintf(report,"bzip2 %d;", vel);
                    }
                    else{
                        int vel = atoi(argv[5]+1);              // +1 perchè da linea di comando ho -number e quindi io devo prendere solo number
                        if(vel == 0 || vel < 1 || vel > 9){
                        ShowUsage(argv[0]);
                        }
                        fprintf(report,"bzip2 %d;", vel);
                    }
                }
            }
            else{
                if(strcmp(argv[2],"gzip") == 0){
                    if(argc == 6){
                        fprintf(report,"gzip;");
                    }
                    else{
                        if(argc == 6 && strcmp(argv[3],"-a") == 0 ){
                            int vel = atoi(argv[4]+1);              // +1 perchè da linea di comando ho -number e quindi io devo prendere solo number
                            if(vel == 0 || vel < 1 || vel > 9){
                            ShowUsage(argv[0]);
                            }
                            fprintf(report,"gzip %d;", vel);
                        }
                        else{
                            int vel = atoi(argv[5]+1);              // +1 perchè da linea di comando ho -number e quindi io devo prendere solo number
                            if(vel == 0 || vel < 1 || vel > 9){
                            ShowUsage(argv[0]);
                            }
                            fprintf(report,"gzip %d;", vel);
                        }
                    }   
                }
                else{
                    if(strcmp(argv[2],"lzma") == 0){
                        if(argc == 6){
                            fprintf(report,"lzma;");
                        }
                        else{
                            if(argc == 6 && strcmp(argv[3],"-a") == 0 ){
                                int vel = atoi(argv[4]+1);              // +1 perchè da linea di comando ho -number e quindi io devo prendere solo number
                                if(vel == 0 || vel < 1 || vel > 9){
                                ShowUsage(argv[0]);
                                }
                                fprintf(report,"lzma %d;", vel);
                            }
                            else{
                                int vel = atoi(argv[5]+1);              // +1 perchè da linea di comando ho -number e quindi io devo prendere solo number
                                if(vel == 0 || vel < 1 || vel > 9){
                                ShowUsage(argv[0]);
                                }
                                fprintf(report,"lzma %d;", vel);
                            }
                        }   
                    }
                    else{
                        if(strcmp(argv[2],"Aritmetic")==0){
                            fprintf(report,"Aritmetic;");
                        }
                    }
                }
            }
        }
    }
}
