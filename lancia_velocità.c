#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

/**
devo mettere percorso cartella
devo mettere algoritmo
**/

static void ShowUsage(char *progPath);


int main(int arg,char* argv[]){
    if(arg != 4 || (strcmp(argv[2],"bzip2") != 0 && strcmp(argv[2], "lzma") != 0 && strcmp(argv[2],"gzip") != 0)){
        ShowUsage(argv[0]);
        return 0;
    }

    DIR *dp;
    struct dirent *ep;

    dp = opendir(argv[1]);
    if(dp!=NULL){
        char* args[8];
        args[7]=NULL;
        args[0]="/home/biar/Desktop/PNRG_analisi/tool_analysis";
        args[1]="-c";
        args[3]="-s";
        while(ep=readdir(dp)){
            char* name = ep->d_name;
            printf("NOME: %s\n",name );
            if(strstr(name,".txt")==NULL && strstr(name,".epub")==NULL && strstr(name,".mobi")==NULL && strstr(name,".htm")==NULL && strstr(name,".html")==NULL && strstr(name,".js")==NULL && strstr(name,".c")==NULL && strstr(name,".java")==NULL)continue;
            args[2]=argv[2];
            args[4]="monobit";
            char* path=(char*)malloc(sizeof(char)*strlen(argv[1])+1+strlen(name));
            strcpy(path,argv[1]);
            strcat(path,name);
            printf("%s\n",args[0]);
            printf("%s\n",args[1]);
            printf("%s\n",args[2]);
            printf("File: %s\n",path);
            args[5]=argv[3];
            args[6]=path;
            int i = fork();
            if(i == 0){
                int res = execv(args[0],args);
                if(res == -1){
                    printf("errore execv");
                    exit(1);
                }
                return 0;
            }
            wait(NULL);

            args[4] = "poker";
            i = fork();
            if(i == 0){
                int res = execv(args[0],args);
                if(res == -1){
                    printf("errore execv");
                    exit(1);
                }
                return 0;
            }
            wait(NULL);

            args[4] = "run";
            i = fork();
            if(i == 0){
                int res = execv(args[0],args);
                if(res == -1){
                    printf("errore execv");
                    exit(1);
                }
                return 0;
            }
            wait(NULL);

            args[4] = "autoC";
            i = fork();
            if(i == 0){
                int res = execv(args[0],args);
                if(res == -1){
                    printf("errore execv");
                    exit(1);
                }
                return 0;
            }
            wait(NULL);
        }
        closedir(dp);
        printf("LANCIA_VELOCITÀ ha terminato\n");
        return 0;
    }
}

static void ShowUsage(char *progPath){
    printf("<path folder> <algorithm> <-vel>\n");
    printf("<-vel> parameter can assume values:\n\n in range [1,9] if the compressione algorithm is bzip2,gzip,lzma.\n");
}
