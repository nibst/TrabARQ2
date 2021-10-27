///             Arquitetura e Organização de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler
///                     (00323741)  Níkolas Padão Schuster
///                     (00275960)  Pedro Afonso Tremea Serpa
///                     (00xxxxxx)  Ricardo

#include <stdio.h>

#define INI_INDICE 8 // oitavo byte
#define INI_ROOT 264 // 256(indices) + 4x2(metadados de 2 bytes)
#define NUM_INDICES 256
#define NUM_CLUSTERS 256
#define CLUSTER_SIZE 32768
#define CLUSTER_TYPE_DATA 0x01
#define CLUSTER_TYPE_DIRECTORY_TABLE 0x02
#define TAM_NOME_MAX 121
#define TAM_EXTENSAO 4 //uma a mais pro fim de string
#define NUM_METAFILES 257 //tem a mais do que é possível, mas enfim é a melhor divisao sem deixar muitos bytes sem nada
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
    char nomeDir[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO];
    MetaFiles metafiles[NUM_METAFILES];
}DirectoryFile;

//tem que ter 32kb -> 32768
typedef struct Type_Cluster
{
    BYTE cluster_type;//como vai ser interpretado os dados a seguir
    BYTE conteudo[CLUSTER_SIZE - (3 * sizeof(BYTE))];
    BYTE cluster_pai; //se igual a END_OF_FILE quer dizer q esse cluster n tem pai
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

// retorna o indice do metadado do arquivo/dir com o nome[] dentro de *dir, -1 para erro
int getIndexMeta(DirectoryFile *dir, char nome[], char extensao[]);

// retorna o numero de metadados validos no diretorio, retorna 0 se for um arquivo txt,-1 caso ocorra algum erro
int nrMetaFiles(FILE *arqDados, BYTE index);

//pega o nome do diretorio do cluster numCluster
int getDirName(BYTE numCluster, char dirName[],Cluster *clus,FILE *arqDados);

// pega o caminho do cluster numCluster até root separando por barras
int getPathFromClusToRoot(BYTE numCluster, char *path);
