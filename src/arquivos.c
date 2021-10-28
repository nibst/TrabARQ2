///             Arquitetura e Organização de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler
///                     (00323741)  Nikolas Padão
///                     (00275960)  Pedro Afonso Tremea Serpa
///                     (00325735)  Ricardo Hermes Dalcin

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arquivos.h"
#define MAX_INSTRUCTION_SIZE 1000
void inicializaMetadados(MetaDados *meta)
{
    meta->tamanho_indice = NUM_INDICES;
    meta->tamanho_cluster = CLUSTER_SIZE;
    meta->inicio_indices = INI_INDICE;
    meta->prim_cluster = INI_ROOT;
}
void inicializaIndex(BYTE *ind)
{
    int i;
    ind[0] = END_OF_FILE; // inicia o root como eof, pra indicar que tem conteudo dentro
    for (i = 1; i < NUM_INDICES - 1; i++)
    {
        ind[i] = VAZIO; // inicia o resto como vazio
    }
    ind[i] = END_OF_FILE; // ultimo cluster é especifico pra indicar end of file, nao pode ser usado
}

void inicializaClusters(Cluster *clus)
{
    int i;
    int j;
    // cluster 0 simboliza cluster vazio e tamb�m cont�m o root
    // primeiro cluster tem o root directory (padrao)
    DirectoryFile *root = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    strcpy(root->nomeDir, "root");
    strcpy(root->extensao, "dir");
    for (i = 0; i < NUM_METAFILES; i++)
        root->metafiles[i].valida = INVALIDO; // todas metafiles inv�lidas

    clus[0].cluster_type = CLUSTER_TYPE_DIRECTORY_TABLE;
    clus[0].cluster_number = 0;
    clus[0].cluster_pai = END_OF_FILE;

    memcpy(clus[0].conteudo, root, sizeof(DirectoryFile));

    for (i = 1; i < NUM_CLUSTERS; i++)
    {
        clus[i].cluster_number = i;
        clus[i].cluster_pai = END_OF_FILE;
        for (j = 0; j < CLUSTER_SIZE - 3; j++)
            clus[i].conteudo[j] = INVALIDO;
    }

    free(root);
}
void inicializaArquivo(FileSystem *arq)
{
    inicializaMetadados(&(arq->meta));
    inicializaIndex(arq->indice);
    inicializaClusters((arq->clusters));
    writeFileSystem(arq);
}
int writeFileSystem(FileSystem *arq)
{
    FILE *arqDados;

    if ((arqDados = fopen("arqDados", "wb")) == NULL)
    {
        printf("[ERROR] Opening file error\n\n");
        return 1;
    }

    if (fwrite(arq, sizeof(FileSystem), 1, arqDados) != 1)
    {
        printf("[ERROR] Error in writing data\n\n");
        fclose(arqDados);
        return 1;
    }
    fclose(arqDados);
    return 0;
}
int readMetaDados(MetaDados *meta, FILE *arqDados)
{
    if (fseek(arqDados, 0, SEEK_SET))
    {
        return 1;
    }
    if (fread(meta, sizeof(MetaDados), 1, arqDados) != 1)
    {
        return 1;
    }
    return 0;
}
// escreve um dado de tamanho escolhido no parametro em um cluster escolhido no parametro e em certo offset dentro do cluster
int writeBlockOfData(BYTE cluster, int offset, int sizeBlock, BYTE *data, FILE *arqDados)
{
    MetaDados meta;
    if (readMetaDados(&meta, arqDados))
        return 1;
    int end_cluster = calcEndCluster(cluster, meta);
    
    if ((fseek(arqDados, offset + end_cluster, SEEK_SET)))
    {
        return 1;
    }
    if (fwrite(data, sizeBlock, 1, arqDados) != 1)
    {
        return 1;
    }
    fflush(arqDados);
    return 0;
}

int getEmptyCluster(FILE *arqDados, MetaDados meta)
{
    BYTE aux;
    int i;

    if ((fseek(arqDados, (meta.inicio_indices + 1), SEEK_SET))) // comeca a procurar pelo 2 indice, pois o 1 eh sempre a root
    {
        return END_OF_FILE;
    }
    for (i = 1; i < meta.tamanho_indice; i++)
    {
        aux = fgetc(arqDados);
        if (aux == VAZIO)
        {
            return i;
        }
    }
    return END_OF_FILE;
}
// insere o cluster de numero numCluster em clus, retorna 1 caso ocorra algum erro
int buscarCluster(BYTE numCluster, Cluster *clus, FILE *arqDados)
{
    MetaDados meta;
    if (readMetaDados(&meta, arqDados))
        return 1;

    if ((fseek(arqDados, calcEndCluster(numCluster, meta), SEEK_SET)))
    {
        return 1;
    }

    if (fread(clus, meta.tamanho_cluster, 1, arqDados) != 1)
    {
        return 1;
    }

    return 0;
}

// retorna o indice do arquivo/dir com o nome[] dentro de *dir, EOF para erro
BYTE getArq(DirectoryFile *dir, char nome[])
{
    BYTE index;
    int i;

    for (i = 0; i < NUM_METAFILES; i++)
    {
        if (!strcmp(dir->metafiles[i].nome_file, nome))
        {
            index = dir->metafiles[i].cluster_inicial;
            return index;
        }
    }
    return END_OF_FILE;
}

// retorna o endereco do byte do cluster de indice x(ex calcEndCluster(0) = 264)
int calcEndCluster(BYTE x, MetaDados meta)
{
    return (meta.prim_cluster + (meta.tamanho_cluster * x));
}

// aloca um cluster na memoria
// nao altera o conteudo do cluster
// retorna 1 caso ocorra algum erro
int criaCluster(char extensao[], FILE *arqDados, BYTE *index)
{
    MetaDados meta;
    BYTE tipocluster;
    if (readMetaDados(&meta, arqDados))
        return 1;
    *index = getEmptyCluster(arqDados, meta);

    if (*index == END_OF_FILE)
    {
        return 1;
    }

    if (mudaEstadoIndex(*index, END_OF_FILE, arqDados))
    {
        return 1;
    }

    if ((fseek(arqDados, calcEndCluster(*index, meta), SEEK_SET)))
    {
        return 1;
    }

    if (!strcmp(extensao, "dir"))
    {
        tipocluster = CLUSTER_TYPE_DIRECTORY_TABLE;
    }
    else
    {
        tipocluster = CLUSTER_TYPE_DATA;
    }

    if (fwrite(&tipocluster, sizeof(BYTE), 1, arqDados) != 1)
    {
        return 1;
    }

    return 0;
}
// escreve os metadados a serem inseridos no diretorio pai

// muda o valor do index para novoEstado(VAZIO,EOF,PONTEIRO), retorna 1 caso ocorra algum erro, senao retorna 0
int mudaEstadoIndex(BYTE index, BYTE novoEstado, FILE *arqDados)
{
    MetaDados meta;
    if (readMetaDados(&meta, arqDados))
        return 1;
    if ((fseek(arqDados, (meta.inicio_indices + index), SEEK_SET)))
    {
        return 1;
    }

    if (!fwrite(&novoEstado, sizeof(BYTE), 1, arqDados))
    {
        return 1;
    }

    return 0;
}
// pega o valor que está no indice "index" da tabela
int getValorIndex(BYTE index, FILE *arqDados, BYTE *value)
{
    MetaDados meta;
    if (readMetaDados(&meta, arqDados))
        return 1;
    if ((fseek(arqDados, (meta.inicio_indices + index), SEEK_SET)))
    {
        return 1;
    }
    if (fread(value, sizeof(BYTE), 1, arqDados) != 1)
    {
        return 1;
    }
    return 0;
}

// retorna o indice do metadado do arquivo/dir com o nome[] dentro de *dir, -1 para erro
int getIndexMeta(DirectoryFile *dir, char nome[], char extensao[])
{
    int i;

    for (i = 0; i < NUM_METAFILES; i++)
    {
        if ((!strcmp(dir->metafiles[i].nome_file, nome)) && (!strcmp(dir->metafiles[i].extensao, extensao)))
        {
            return i;
        }
    }
    return -1;
}

// retorna o numero de metadados validos no diretorio, retorna 0 se for um arquivo txt,-1 caso ocorra algum erro
int nrMetaFiles(FILE *arqDados, BYTE numCluster)
{
    int i, cont;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));

    if (buscarCluster(numCluster, clus, arqDados))
    {
        free(clus);
        free(dir);
        return -1;
    }
    if (clus->cluster_type != CLUSTER_TYPE_DIRECTORY_TABLE)
    {
        free(clus);
        free(dir);
        return 0; // retorna 0 caso o cluster nao seja um diretorio
    }

    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    cont = 0;
    for (i = 0; i < NUM_METAFILES; i++)
    {
        if (dir->metafiles[i].valida == VALIDO)
            cont++;
    }

    free(dir);
    free(clus);
    return cont;
}

// Monta o caminho do root até certa pasta a partir dessa certa pasta(faz o caminho inverso pasta->root)
int getPathFromClusToRoot(BYTE numCluster, char *path)
{
    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    char *path_aux = (char *)malloc(sizeof(char) * MAX_INSTRUCTION_SIZE);
    int i;
    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        free(dir);
        free(clus);
        free(path_aux);
        return 1;
    }
    // enquanto cluster nao for root
    strcpy(path_aux, "");
    while (numCluster != 0)
    {
        if (buscarCluster(numCluster, clus, arqDados))
        {
            fclose(arqDados);
            free(clus);
            free(dir);
            free(path_aux);
            return 1;
        }
        memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

        strcpy(path, dir->nomeDir);

        strcat(path_aux, path);
        strcat(path_aux, "/");
        numCluster = clus->cluster_pai;
    }
    if (buscarCluster(numCluster, clus, arqDados))
    {
        fclose(arqDados);
        free(clus);
        free(dir);
        free(path_aux);
        return 1;
    }
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    strcpy(path, dir->nomeDir);
    strcat(path_aux, path);

    int tam = strlen(path_aux);
    strcpy(path, "");
    i = tam - 1;
    while (i >= 0)
    {
        while ((path_aux[i] != '/') && i >= 0)
            i--;
        if (i >= 0)
        {
            path_aux[i] = '\0';
            strcat(path, path_aux + i + 1);
            strcat(path, "/");
        }
        else
            strcat(path, path_aux);
    }
    fclose(arqDados);
    free(clus);
    free(dir);
    free(path_aux);
    return 0;
}
int clusIsInsideOfClusN(Cluster *clus, BYTE clusN, FILE *arqDados)
{
    while (clus->cluster_pai != END_OF_FILE)
    {
        if (clus->cluster_number == clusN)
            return 1;
        if (buscarCluster(clus->cluster_pai, clus, arqDados))
            return 1;
    }
    return 0;
}
