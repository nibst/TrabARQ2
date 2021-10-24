///             Arquitetura e Organização de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler
///                     (00323741)  Níkolas Padão Schuster
///                     (00275960)  Pedro Afonso Tremea Serpa
///                     (00xxxxxx)  Ricardo 

#include <stdio.h>
#include <stdlib.h>
#include "error.h"


// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorOpeningFile(Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("\n*** ERRO AO ABRIR ARQUIVO***\n\n");
    free(clus);
    free(dir);
    fclose(arqDados);
    return 1;
}
// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorNumArguments(Arguments *arguments, Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] Expected %u arguments but got %u: '%s'\n\n", arguments->owner->expected_args, arguments->num_args, arguments->args);
    free(clus);
    free(dir);
    fclose(arqDados);
    return 1;
}
// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorGettingCluster(Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] Error in getting the cluster\n\n");
    free(clus);
    free(dir);
    fclose(arqDados);
    return 1;
}
// mensagem de error, da free em clus, dir e path, fclose em arqDados e retorna 1
int errorInvalidPath(Arguments *arguments, Cluster *clus, DirectoryFile *dir, char *path, FILE *arqDados)
{
    printf("[ERROR] invalid path '%s'\n\n", arguments->args);
    free(clus);
    free(dir);
    free(path);
    fclose(arqDados);
    return 1;
}
// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorGettingIndexValue(Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] Error in getting index value\n\n");
    free(clus);
    free(dir);
    fclose(arqDados);
    return 1;
}
// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorAllocatingCluster(Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] Error in allocating new cluster\n\n");
    free(clus);
    free(dir);
    fclose(arqDados);
    return 1;
}
// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorWritingData(Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] Error in writing data\n\n");
    free(clus);
    free(dir);
    fclose(arqDados);
    return 1;
}
// mensagem de error, da free em clus, dir e arg_copy, fclose em arqDados e retorna 1
int errorFileDoesNotExist(char *arg_copy, Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] File/Directory '%s' does not exist\n\n", arg_copy);
    free(arg_copy);
    free(clus);
    free(dir);
    fclose(arqDados);
    return 1;
}
int errorDirectoryNotEmpty(char *arg_copy, Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] Directory not empty\n\n");
    free(arg_copy);
    free(clus);
    free(dir);
    fclose(arqDados);
    return 1;
}

int errorFreeingCluster(char *arg_copy, Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] Error in freeing cluster\n\n");
    free(arg_copy);
    free(clus);
    free(dir);
    fclose(arqDados);
    return 1;
}