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
    for(i = 0; i<NUM_INDICES; i++)
    {
        ind[i] = 0xFF;//aponta pra end of file
    }
}

void inicializaClusters(cluster *clus)
{
    int i;
    //cluster 0 é reservado para simbolizar cluster vazio
    //clus[0] = {0x00,NULL,0x00};//nao sei se tem que fazer isso sla
    //primeiro cluster tem o root directory (padrao)
    directoryFile *root = (directoryFile*)malloc(sizeof(directoryFile));
    root->valida = 0x01;
    strcpy(root->nomeDir,"root");
    strcpy(root->extensao,"dir");
    for(i=0;i<NUM_METAFILES;i++)
        root->metafiles[i].valida = 0x00;//todas metafiles inválidas


    clus[1].cluster_type = CLUSTER_TYPE_DIRECTORY_TABLE;
    clus[1].cluster_number = 0x01;
    memcpy(clus[1].conteudo,root, sizeof(directoryFile));

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

    if((arqDados = fopen("arqDados", "wb")) == NULL)
    {
        printf("\n*** ERRO AO CRIAR ARQUIVO***\n");
        return 1;
    }

    if(fwrite(arq, sizeof(fileSystem), 1, arqDados) != 1)
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

    if((arqDados = fopen("arqDados", "rb")) == NULL)
    {
        printf("\n*** ERRO AO ABRIR ARQUIVO***\n");
        return 1;
    }
    if(fread(arq, sizeof(fileSystem), 1, arqDados) != 1)
    {
        printf("\n*** ERRO AO TENTAR LER DO ARQUIVO***\n");
        fclose(arqDados);
        return 1;
    }
    fclose(arqDados);
    return 0;
}
