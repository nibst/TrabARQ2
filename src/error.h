
///             Arquitetura e Organização de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler
///                     (00323741)  Níkolas Padão Schuster
///                     (00275960)  Pedro Afonso Tremea Serpa
///                     (00xxxxxx)  Ricardo 

#include "commands.h"
#include "arquivos.h"

// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorOpeningFile(Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorNumArguments(Arguments *arguments, Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorGettingCluster(Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus, dir e path, fclose em arqDados e retorna 1
int errorInvalidPath(Arguments *arguments, Cluster *clus, DirectoryFile *dir, char *path, FILE *arqDados);

// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorGettingIndexValue(Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorAllocatingCluster(Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorWritingData(Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus, dir e arg_copy, fclose em arqDados e retorna 1
int errorFileDoesNotExist(char *arg_copy, Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus, dir e arg_copy, fclose em arqDados e retorna 1
int errorDirectoryNotEmpty(char *arg_copy, Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus, dir e arg_copy, fclose em arqDados e retorna 1
int errorFreeingCluster(char *arg_copy, Cluster *clus, DirectoryFile *dir, FILE *arqDados);