<<<<<<< HEAD
///             Arquitetura e Organização de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler
///                     (00323741)  Níkolas Padão
///                     (00275960)  Pedro Afonso Tremea Serpa
///                     (00xxxxxx)  Ricardo

#include <stdio.h>

#define NUM_INDICES 256
#define NUM_CLUSTERS 256
#define CLUSTER_SIZE 32768
#define CLUSTER_TYPE_DATA 0x01
#define CLUSTER_TYPE_DIRECTORY_TABLE 0x02
#define TAM_NOME_MAX 16
#define TAM_EXTENSAO 4 //uma a mais pro fim de string
#define NUM_METAFILES 1488
#define INVALIDO 0x00
#define VALIDO 0x01
#define VAZIO 0x00
#define END_OF_FILE 0xFF
typedef unsigned short WORD;//2bytes
typedef unsigned char BYTE;//1byte
typedef struct Type_MetaDados
{
    //usei WORD para guardar pois eh o unico tipo de 2 byte
    WORD tamanho_indice;  //Tamanho do índice (quantas entradas o índice possui, utilizar o valor 2^8)
    WORD tamanho_cluster; //Tamanho do cluster (utilizar o valor 32KB)
    WORD inicio_indices;  //Byte onde o índice inicia (metadados iniciam no byte zero e vão até byte 3) (recebe 4 então)
    WORD prim_cluster;    //Byte onde inicia o primeiro cluster (TamanhoIndice + InicioIndices)

}MetaDados;

typedef struct Type_MetaFiles
{
    BYTE valida;//se for valida é 1, se não é 0, ser 0 seria basicamente remover,
    char nome_file[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO];
    BYTE cluster_inicial;//que cluster q começa a file

}MetaFiles;

typedef struct Type_DirectoryFile
{
    //BYTE valida;//se for valida é 1, se não é 0, ser 0 seria basicamente remover, nao sei se eh necessario aqui ja que tem ja nos metafiles
    char nomeDir[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO];
    MetaFiles metafiles[NUM_METAFILES];//!!sla quantos, pode ter varios dps calculo

}DirectoryFile;

//tem que ter 32kb
typedef struct Type_Cluster
{
    BYTE cluster_type;//como vai ser interpretado os dados a seguir
    BYTE conteudo[CLUSTER_SIZE - (2 * sizeof(BYTE))];
    BYTE cluster_number;//cluster em que está->vai checar a tabela de indices e apontar pro proximo cluster

}Cluster;

typedef struct Type_FileSystem
{
    MetaDados meta;
    BYTE indice[NUM_INDICES];//TODO colocar como constante, o indice i aponta o proximo cluster do cluster de numero i.
    Cluster clusters[NUM_CLUSTERS];//TODO colocar como constante

} FileSystem;

void inicializaMetadados(MetaDados *meta);

void inicializaIndex(BYTE *ind);

void inicializaClusters(Cluster *clus);

//fazer funcao de ler e armazenar arquivo binario
int writeFileSystem(FileSystem *arq);

int readFileSystem(FileSystem *arq);

void inicializaArquivo(FileSystem *arq);

int writeBlockOfData(BYTE cluster,int offset,int sizeBlock, BYTE *data,FILE *arqDados);

//pega o cluster root
int getFirstCluster(Cluster *clus,FILE *arqDados);

//retorna o indice do 1 cluster vazio, EOF caso ocorra algum erro
int getEmptyCluster(FILE *arqDados);

//insere o cluster correspondente ao indice x em clus, retorna 1 caso ocorra algum erro
int buscarCluster(BYTE x, Cluster *clus, FILE *arqDados);

//retorna o indice do arquivo/dir com o nome[] dentro de *dir, EOF para erro
BYTE getArq(DirectoryFile *dir, char nome[]);

//retorna o endereco do byte do cluster de indice x(ex calcEndCluster(0) = 264)
int calcEndCluster(BYTE x);

//aloca um cluster na memoria
//nao altera o conteudo do cluster
//retorna 1 caso ocorra algum erro
int criaCluster(char extensao[], FILE *arqDados, BYTE *index);

//muda o valor do index para novoEstado(VAZIO,EOF,PONTEIRO), retorna 0 caso ocorra algum erro, senao retorna 1
int mudaEstadoIndex(BYTE index, BYTE novoEstado, FILE *arqDados);

//pega o valor que está no indice index da tabela
int getValorIndex(BYTE index, FILE *arqDados, BYTE *value);
=======
///             Arquitetura e Organização de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler
///                     (00323741)  Níkolas Padão
///                     (00275960)  Pedro Afonso Tremea Serpa
///                     (00xxxxxx)  Ricardo

#include <stdio.h>

#define NUM_INDICES 256
#define NUM_CLUSTERS 256
#define CLUSTER_SIZE 32768
#define CLUSTER_TYPE_DATA 0x01
#define CLUSTER_TYPE_DIRECTORY_TABLE 0x02
#define TAM_NOME_MAX 16
#define TAM_EXTENSAO 4 //uma a mais pro fim de string
#define NUM_METAFILES 1488
#define INVALIDO 0x00
#define VALIDO 0x01
#define VAZIO 0x00
#define END_OF_FILE 0xFF
typedef unsigned short WORD;//2bytes
typedef unsigned char BYTE;//1byte
typedef struct Type_MetaDados
{
    //usei WORD para guardar pois eh o unico tipo de 2 byte
    WORD tamanho_indice;  //Tamanho do índice (quantas entradas o índice possui, utilizar o valor 2^8)
    WORD tamanho_cluster; //Tamanho do cluster (utilizar o valor 32KB)
    WORD inicio_indices;  //Byte onde o índice inicia (metadados iniciam no byte zero e vão até byte 3) (recebe 4 então)
    WORD prim_cluster;    //Byte onde inicia o primeiro cluster (TamanhoIndice + InicioIndices)

}MetaDados;

typedef struct Type_MetaFiles
{
    BYTE valida;//se for valida é 1, se não é 0, ser 0 seria basicamente remover,
    char nome_file[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO];
    BYTE cluster_inicial;//que cluster q começa a file

}MetaFiles;

typedef struct Type_DirectoryFile
{
    //BYTE valida;//se for valida é 1, se não é 0, ser 0 seria basicamente remover, nao sei se eh necessario aqui ja que tem ja nos metafiles
    char nomeDir[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO];
    MetaFiles metafiles[NUM_METAFILES];//!!sla quantos, pode ter varios dps calculo

}DirectoryFile;

//tem que ter 32kb
typedef struct Type_Cluster
{
    BYTE cluster_type;//como vai ser interpretado os dados a seguir
    BYTE conteudo[CLUSTER_SIZE - (2 * sizeof(BYTE))];
    BYTE cluster_number;//cluster em que está->vai checar a tabela de indices e apontar pro proximo cluster

}Cluster;

typedef struct Type_FileSystem
{
    MetaDados meta;
    BYTE indice[NUM_INDICES];//TODO colocar como constante, o indice i aponta o proximo cluster do cluster de numero i.
    Cluster clusters[NUM_CLUSTERS];//TODO colocar como constante

} FileSystem;

void inicializaMetadados(MetaDados *meta);

void inicializaIndex(BYTE *ind);

void inicializaClusters(Cluster *clus);

//fazer funcao de ler e armazenar arquivo binario
int writeFileSystem(FileSystem *arq);

int readFileSystem(FileSystem *arq);

void inicializaArquivo(FileSystem *arq);

int writeBlockOfData(BYTE cluster,int offset,int sizeBlock, BYTE *data,FILE *arqDados);

//seila se sequer precisa disso
int getFirstCluster(Cluster *clus);

//retorna o indice do 1 cluster vazio, EOF caso ocorra algum erro
int getEmptyCluster(FILE *arqDados);

//insere o cluster correspondente ao indice x em clus, retorna 1 caso ocorra algum erro
int buscarCluster(BYTE x, Cluster *clus, FILE *arqDados);

//retorna o indice do arquivo/dir com o nome[] dentro de *dir, EOF para erro
BYTE getArq(DirectoryFile *dir, char nome[]);

//retorna o endereco do byte do cluster de indice x(ex calcEndCluster(0) = 264)
int calcEndCluster(BYTE x);

//aloca um cluster na memoria e escreve os metadados a serem inseridos no diretorio pai
//nao altera o conteudo do cluster
//retorna 1 caso ocorra algum erro
int criaCluster(char nome[], char extensao[], MetaFiles *meta, FILE *arqDados);

//muda o valor do index para novoEstado(VAZIO,EOF,PONTEIRO), retorna 0 caso ocorra algum erro, senao retorna 1
int mudaEstadoIndex(BYTE index, BYTE novoEstado, FILE *arqDados);

//pega o valor que está no indice index da tabela
int getValorIndex(BYTE index, FILE *arqDados, BYTE *value);
>>>>>>> e3e7dbf78fb73b4da9f35f5ef58c8d6395b1ffab
