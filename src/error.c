///             Arquitetura e Organização de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler - TURMA B
///                     (00323741)  Nikolas Padão - TURMA B
///                     (00275960)  Pedro Afonso Tremea Serpa - TURMA B
///                     (00325735)  Ricardo Hermes Dalcin - TURMA A

#include <stdio.h>
#include <stdlib.h>
#include "error.h"

// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorOpeningFile(Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] Opening file error\n\n");
    free(clus);
    free(dir);
    fclose(arqDados);
    return 1;
}
// mensagem de error, da free em clus e dir, fclose em arqDados e retorna 1
int errorNumArguments(Arguments *arguments, Cluster *clus, DirectoryFile *dir)
{
    printf("[ERROR] Expected %u arguments but got %u: '%s'\n\n", arguments->owner->expected_args, arguments->num_args, arguments->args);
    free(clus);
    free(dir);
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
int errorInvalidExtension(char *caminho_arquivo, char *arg_cpy, Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] Invalid name extension for new name\n\n");
    free(clus);
    free(dir);
    free(caminho_arquivo);
    free(arg_cpy);
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
int errorFileAlreadyExist(Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] File already exists\n\n");
    free(clus);
    free(dir);
    fclose(arqDados);
    return 1;
}
int errorDirAlreadyExist(Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] Directory already exists\n\n");
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

int errorEditingIndexTable(char *arg_copy, Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] Editing index table error\n\n");
    free(arg_copy);
    free(clus);
    free(dir);
    fclose(arqDados);
    return 1;
}
int errorCannotEditDir(char *caminho_arquivo, char *conteudo_arquivo, Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] Cannot edit a directory\n\n");
    free(clus);
    free(dir);
    free(caminho_arquivo);
    free(conteudo_arquivo);
    fclose(arqDados);
    return 1;
}
int errorCannotAlterRoot(char *arg_copy, Cluster *clus, DirectoryFile *dir)
{
    printf("[ERROR] Cannot alter root\n\n");
    free(arg_copy);
    free(clus);
    free(dir);
    return 1;
}
int errorHigherHierarchyToLower(MetaFiles *meta, char *arg_cpy, char *path_file,Cluster *clus, DirectoryFile *dir, FILE *arqDados)
{
    printf("[ERROR] Cannot move a directory that contains a directory N to this directory N\n\n");
    free(meta);
    free(arg_cpy);
    free(path_file);
    free(clus);
    free(dir);
    fclose(arqDados);
    return 1;

}
