#define NUM_INDICES 256
#define NUM_CLUSTERS 256
#define CLUSTER_SIZE 32768
#define CLUSTER_TYPE_DATA 0x01
#define CLUSTER_TYPE_DIRECTORY_TABLE 0x02
#define TAM_NOME_MAX 256
#define TAM_EXTENSAO 4 //uma a mais pro fim de string
#define NUM_METAFILES 100
#define INVALIDO 0x00
#define VALIDO 0x01
#define VAZIO 0x00
#define END_OF_FILE 0xFF
typedef unsigned short WORD; //2bytes
typedef unsigned char BYTE;  //1byte
typedef struct MetaDados
{
    //usei WORD para guardar pois eh o unico tipo de 2 byte
    WORD tamanho_indice;  //Tamanho do índice (quantas entradas o índice possui, utilizar o valor 2^8)
    WORD tamanho_cluster; //Tamanho do cluster (utilizar o valor 32KB)
    WORD inicio_indices;  //Byte onde o índice inicia (metadados iniciam no byte zero e vão até byte 3) (recebe 4 então)
    WORD prim_cluster;    //Byte onde inicia o primeiro cluster (TamanhoIndice + InicioIndices)
} metaDados;

typedef struct MetaFiles
{
    BYTE valida; //se for valida é 1, se não é 0, ser 0 seria basicamente remover,
    char nome_file[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO];
    BYTE cluster_inicial; //que cluster q começa a file
} metaFiles;
typedef struct DirectoryFile
{
    //BYTE valida;//se for valida é 1, se não é 0, ser 0 seria basicamente remover, nao sei se eh necessario aqui ja que tem ja nos metafiles
    char nomeDir[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO];
    metaFiles metafiles[NUM_METAFILES]; //!!sla quantos, pode ter varios dps calculo
} directoryFile;

//tem que ter 32kb
typedef struct Cluster
{
    BYTE cluster_type; //como vai ser interpretado os dados a seguir
    BYTE conteudo[CLUSTER_SIZE - (2 * sizeof(BYTE))];
    BYTE cluster_number; //cluster em que está->vai checar a tabela de indices e apontar pro proximo cluster
} cluster;

typedef struct FileSystem
{
    metaDados meta;
    BYTE indice[NUM_INDICES];       //TODO colocar como constante, o indice i aponta o proximo cluster do cluster de numero i.
    cluster clusters[NUM_CLUSTERS]; //TODO colocar como constante
} fileSystem;

void inicializaMetadados(metaDados *meta);

void inicializaIndex(BYTE *ind);

void inicializaClusters(cluster *clus);

void inicializaArquivo(fileSystem *arq);

//fazer funcao de ler e armazenar arquivo binario
int writeFileSystem(fileSystem *arq);

int readFileSystem(fileSystem *arq);

//seila se sequer precisa disso
int getFirstCluster(cluster *clus);

//retorna o indice do 1 cluster vazio, EOF caso ocorra algum erro
int getEmptyCluster();
