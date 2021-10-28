///             Arquitetura e Organização de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler - TURMA B
///                     (00323741)  Nikolas Padão - TURMA B
///                     (00275960)  Pedro Afonso Tremea Serpa - TURMA B
///                     (00325735)  Ricardo Hermes Dalcin - TURMA A

#include "commands.h"
#include "arquivos.h"

// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorOpeningFile(Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorNumArguments(Arguments *arguments, Cluster *clus, DirectoryFile *dir);

// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorGettingCluster(Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus, dir e path, fclose em arqDados e retorna 1
int errorInvalidPath(Arguments *arguments, Cluster *clus, DirectoryFile *dir, char *path, FILE *arqDados);

// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorGettingIndexValue(Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorAllocatingCluster(Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorInvalidExtension(char *caminho_arquivo, char *arg_cpy, Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorWritingData(Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus, dir e arg_copy, fclose em arqDados e retorna 1
int errorFileDoesNotExist(char *arg_copy, Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus, dir, fclose em arqDados e retorna 1
int errorFileAlreadyExist(Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus, dir, fclose em arqDados e retorna 1
int errorDirAlreadyExist(Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus, dir e arg_copy, fclose em arqDados e retorna 1
int errorDirectoryNotEmpty(char *arg_copy, Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus, dir e arg_copy, fclose em arqDados e retorna 1
int errorEditingIndexTable(char *arg_copy, Cluster *clus, DirectoryFile *dir, FILE *arqDados);

int errorCannotEditDir(char *caminho_arquivo, char *conteudo_arquivo, Cluster *clus, DirectoryFile *dir, FILE *arqDados);

// mensagem de error, da free em clus, dir e arg_copy, fclose em arqDados e retorna 1
int errorCannotAlterRoot(char *arg_copy, Cluster *clus, DirectoryFile *dir);

int errorHigherHierarchyToLower(MetaFiles *meta, char *arg_cpy, char *path_file,Cluster *clus, DirectoryFile *dir, FILE *arqDados);


