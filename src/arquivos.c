#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arquivos.h"

#define INI_INDICE 8 //oitavo byte
#define INI_ROOT 264 //256(indices) + 4x2(metadados de 2 bytes)

void inicializaMetadados(metaDados *meta)
{
    meta->tamanho_indice = NUM_INDICES;
    meta->tamanho_cluster = CLUSTER_SIZE;
    meta->inicio_indices = INI_INDICE;
    meta->prim_cluster = INI_ROOT;
}
void inicializaIndex(BYTE *ind)
{
    int i;
    ind[0] = END_OF_FILE; //inicia o root como eof
    for (i = 1; i < NUM_INDICES; i++)
    {
        ind[i] = VAZIO; //inicia o resto como vazio
    }
}

void inicializaClusters(cluster *clus)
{
    int i;
    int j;
    //cluster 0 simboliza cluster vazio e tamb�m cont�m o root
    //primeiro cluster tem o root directory (padrao)
    directoryFile *root = (directoryFile *)malloc(sizeof(directoryFile));
    strcpy(root->nomeDir, "root");
    strcpy(root->extensao, "dir");
    for (i = 0; i < NUM_METAFILES; i++)
        root->metafiles[i].valida = INVALIDO; //todas metafiles inv�lidas

    clus[0].cluster_type = CLUSTER_TYPE_DIRECTORY_TABLE;
    clus[0].cluster_number = VALIDO;
    memcpy(clus[0].conteudo, root, sizeof(directoryFile));

    /*for (i = 0; i < NUM_CLUSTERS; i++)
    {
        clus[i].cluster_number = i;
        for (j = 9; j < CLUSTER_SIZE - 2; j++)
            clus[i].conteudo[j] = 'x'; //!!so para vizualizar lugares "sem nada", depois tirar isso!!
    }
    free(root);*/
}

void inicializaArquivo(fileSystem *arq)
{

    inicializaMetadados(&(arq->meta));
    inicializaIndex(arq->indice);
    inicializaClusters((arq->clusters));
}
int writeFileSystem(fileSystem *arq)
{
    FILE *arqDados;

    if ((arqDados = fopen("arqDados", "wb")) == NULL)
    {
        printf("\n*** ERRO AO CRIAR ARQUIVO***\n");
        return 1;
    }

    if (fwrite(arq, sizeof(fileSystem), 1, arqDados) != 1)
    {
        printf("\n*** ERRO AO TENTAR ESCREVER NO ARQUIVO***\n");
        fclose(arqDados);
        return 1;
    }
    fclose(arqDados);
    return 0;
}

int readFileSystem(fileSystem *arq)
{
    FILE *arqDados;

    if ((arqDados = fopen("arqDados", "rb")) == NULL)
    {
        printf("\n*** ERRO AO ABRIR ARQUIVO***\n");
        return 1;
    }
    if (fread(arq, sizeof(fileSystem), 1, arqDados) != 1)
    {
        printf("\n*** ERRO AO TENTAR LER DO ARQUIVO***\n");
        fclose(arqDados);
        return 1;
    }
    fclose(arqDados);
    return 0;
}

//nao sei se essa func vai ser necessaria
int getFirstCluster(cluster *clus)
{
    FILE *arqDados;

    if ((arqDados = fopen("arqDados", "rb")) == NULL)
    {
        printf("\n*** ERRO AO ABRIR ARQUIVO***\n");
        return 1;
    }
    //se nao der certo fseek
    if (!(fseek(arqDados, INI_ROOT, SEEK_SET)))
    {
        printf("\n*** ERRO AO PERCORRER ARQUIVO***\n");
        return 1;
    }
    else
    {
        //coloca o primeiro cluster em clus
        if (fread(clus, sizeof(cluster), 1, arqDados) != 1)
        {
            printf("\n*** ERRO AO TENTAR LER DO ARQUIVO***\n");
            fclose(arqDados);
            return 1;
        }
    }
    fclose(arqDados);
    return 0;
}

int getEmptyCluster()
{
    FILE *arqDados;
    BYTE aux;
    metaDados meta;
    int i;

    if ((arqDados = fopen("arqDados", "rb")) == NULL)
    {
        printf("\n*** ERRO AO ABRIR ARQUIVO***\n");
        return END_OF_FILE;
    }

    if (fread(&meta, sizeof(metaDados), 1, arqDados) != 1) //faz a  leitura dos metadados presentes no arquivo
    {
        printf("\n*** ERRO AO TENTAR LER DO ARQUIVO***\n");
        fclose(arqDados);
        return 1;
    }
    if ((fseek(arqDados, (meta.inicio_indices + 1), SEEK_SET))) //comeca a procurar pelo 2 indice, pois o 1 eh sempre a root
    {
        printf("\n*** ERRO AO PERCORRER ARQUIVO***\n");
        return END_OF_FILE;
    }
    for (i = 1; i < NUM_INDICES; i++)
    {
        aux = fgetc(arqDados);
        if (aux == VAZIO)
        {
            fclose(arqDados);
            return i;
        }
    }
    fclose(arqDados);
    return END_OF_FILE;
}