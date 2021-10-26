///             Arquitetura e Organiza��o de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler
///                     (00323741)  N�kolas Pad�o
///                     (00275960)  Pedro Afonso Tremea Serpa
///                     (00xxxxxx)  Ricardo

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arquivos.h"

#define INI_INDICE 8 // oitavo byte
#define INI_ROOT 264 // 256(indices) + 4x2(metadados de 2 bytes)

void InicializaMetadados(MetaDados *meta)
{
    meta->tamanho_indice = NUM_INDICES;
    meta->tamanho_cluster = CLUSTER_SIZE;
    meta->inicio_indices = INI_INDICE;
    meta->prim_cluster = INI_ROOT;
}
void InicializaIndex(BYTE *ind)
{
    int i;
    ind[0] = END_OF_FILE; // inicia o root como eof
    for (i = 1; i < NUM_INDICES; i++)
    {
        ind[i] = VAZIO; // inicia o resto como vazio
    }
}

void rInicializaIndex(BYTE *ind, int tab[256])
{
    int i;

    for (i = 0; i < NUM_INDICES; i++)
    {
        ind[i] = tab[i];
    }
}


void InicializaClusters(Cluster *clus)
{
    int i;
    int j;
    // cluster 0 simboliza cluster vazio e tamb�m cont�m o root
    // primeiro cluster tem o root directory (padrao)
    DirectoryFile *root = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    memset(root, 'x', sizeof(DirectoryFile));
    strcpy(root->nomeDir, "root");
    strcpy(root->extensao, "dir");
    for (i = 0; i < NUM_METAFILES; i++)
        root->metafiles[i].valida = INVALIDO; // todas metafiles inv�lidas

    clus[0].cluster_type = CLUSTER_TYPE_DIRECTORY_TABLE;
    clus[0].cluster_number = 0;

    memcpy(clus[0].conteudo, root, sizeof(DirectoryFile));

    for (i = 1; i < NUM_CLUSTERS; i++)
    {
        clus[i].cluster_number = i;
        for (j = 1; j < CLUSTER_SIZE - 3; j++)
            clus[i].conteudo[j] = (i % 26) + 65; //!!so para vizualizar lugares "sem nada", depois tirar isso!!
    }

    free(root);
}
void rInicializaClusters(Cluster *clus)
{

}

int writeFileSystem(FileSystem *arq)
{
    FILE *arqDados;

    if ((arqDados = fopen("arqDados", "wb")) == NULL)
    {
        printf("\n*** ERRO AO CRIAR ARQUIVO***\n");
        return 1;
    }

    if (fwrite(arq, sizeof(FileSystem), 1, arqDados) != 1)
    {
        printf("\n*** ERRO AO TENTAR ESCREVER NO ARQUIVO***\n");
        fclose(arqDados);
        return 1;
    }
    fclose(arqDados);
    return 0;
}

int rWriteFileSystem(FileSystem *arq)
{

    FILE *arqDados;


    if ((arqDados = fopen("arqDados", "wb")) == NULL)
    {
        printf("\n*** ERRO AO CRIAR ARQUIVO***\n");
        return 1;
    }

    if (fwrite(arq, sizeof(FileSystem), 1, arqDados) != 1)
    {
        printf("\n*** ERRO AO TENTAR ESCREVER NO ARQUIVO***\n");
        fclose(arqDados);
        return 1;
    }
    fclose(arqDados);
    return 0;
}


void InicializaArquivo(FileSystem *arq)
{
    InicializaMetadados(&(arq->meta));
    InicializaIndex(arq->indice);
    InicializaClusters((arq->clusters));
    writeFileSystem(arq);
}

void rInicializaArquivo(FileSystem *arq)
{
    int i, LEN=255;
    unsigned char tab[LEN];

    InicializaMetadados(&(arq->meta));

    FILE *arquivo = fopen("arqDados","rb+");
    fseek ( arquivo , 8 , SEEK_SET );

    for(i=0; i<LEN; i++)
    {
        tab[i] = fgetc(arquivo);

    }

    rInicializaIndex(arq->indice, tab);
    rInicializaClusters((arq->clusters));

    printf("\n");
}


int readFileSystem(FileSystem *arq)
{
    FILE *arqDados;

    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        printf("\n*** ERRO AO ABRIR ARQUIVO***\n");
        return 1;
    }
    if (fread(arq, sizeof(FileSystem), 1, arqDados) != 1)
    {
        printf("\n*** ERRO AO TENTAR LER DO ARQUIVO***\n");
        fclose(arqDados);
        return 1;
    }
    fclose(arqDados);
    return 0;
}

// escreve um dado de tamanho escolhido no parametro em um cluster escolhido no parametro e em certo offset dentro do cluster
int writeBlockOfData(BYTE cluster, int offset, int sizeBlock, BYTE *data, FILE *arqDados)
{
    int end_cluster = calcEndCluster(cluster);

    if ((fseek(arqDados, offset + end_cluster, SEEK_SET)))
    {
        return 1;
    }
    if (fwrite(data, sizeBlock, 1, arqDados) != 1)
    {
        return 1;
    }

    return 0;
}

// pega o primeiro cluster(root)
int getFirstCluster(Cluster *clus, FILE *arqDados)
{

    if ((arqDados = fopen("arqDados", "rb")) == NULL)
    {
        return 1;
    }
    // se nao der certo fseek
    if ((fseek(arqDados, INI_ROOT, SEEK_SET)))
    {
        return 1;
    }
    // coloca o primeiro cluster em clus
    if (fread(clus, sizeof(Cluster), 1, arqDados) != 1)
    {
        return 1;
    }
    return 0;
}

int getEmptyCluster(FILE *arqDados)
{
    BYTE aux;
    int i;

    if ((fseek(arqDados, (INI_INDICE + 1), SEEK_SET))) // comeca a procurar pelo 2 indice, pois o 1 eh sempre a root
    {
        return END_OF_FILE;
    }
    for (i = 1; i < NUM_INDICES; i++)
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

    if ((fseek(arqDados, calcEndCluster(numCluster), SEEK_SET)))
    {
        return 1;
    }

    if (fread(clus, CLUSTER_SIZE, 1, arqDados) != 1)
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
int calcEndCluster(BYTE x)
{
    return (INI_ROOT + (CLUSTER_SIZE * x));
}

// aloca um cluster na memoria
// nao altera o conteudo do cluster
// retorna 1 caso ocorra algum erro
int criaCluster(char extensao[], FILE *arqDados, BYTE *index)
{
    BYTE tipocluster;

    *index = getEmptyCluster(arqDados);

    if (*index == END_OF_FILE)
    {
        return 1;
    }

    if (mudaEstadoIndex(*index, END_OF_FILE, arqDados))
    {
        return 1;
    }

    if ((fseek(arqDados, calcEndCluster(*index), SEEK_SET)))
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
//escreve os metadados a serem inseridos no diretorio pai

// muda o valor do index para novoEstado(VAZIO,EOF,PONTEIRO), retorna 1 caso ocorra algum erro, senao retorna 0
int mudaEstadoIndex(BYTE index, BYTE novoEstado, FILE *arqDados)
{
    if ((fseek(arqDados, (INI_INDICE + index), SEEK_SET)))
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
    if ((fseek(arqDados, (INI_INDICE + index), SEEK_SET)))
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
int getIndexMeta(DirectoryFile *dir, char nome[])
{
    int i;

    for (i = 0; i < NUM_METAFILES; i++)
    {
        if (!strcmp(dir->metafiles[i].nome_file, nome))
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

/*int getDirName(BYTE numCluster, char dirName[])
{
    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));

    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        free(clus);
        free(dir);
        printf("\n*** ERRO AO ABRIR ARQUIVO***\n");
        return 1;
    }

    if (buscarCluster(numCluster, clus, arqDados))
    {
        fclose(arqDados);
        free(clus);
        free(dir);
        return 1;
    }
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    strcpy(dirName,dir->nomeDir);

    fclose(arqDados);
    free(clus);
    free(dir);
    return 0;

}*/
