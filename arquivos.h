typedef struct MetaDados
{                                 //usei char para guardar pois eh o unico tipo de 1 byte
    unsigned char TamanhoIndice;  //Tamanho do índice (quantas entradas o índice possui, utilizar o valor 2^8)
    unsigned char TamanhoCluster; //Tamanho do cluster (utilizar o valor 32KB)
    unsigned char InicioIndices;  //Byte onde o índice inicia (metadados iniciam no byte zero e vão até byte 3) (recebe 4 então)
    unsigned char PrimCluster;    //Byte onde inicia o primeiro cluster (TamanhoIndice + InicioIndices)
} metadados;

typedef struct Index
{
    unsigned char Indice;
} index;

typedef struct Cluster
{
    //esse aqui a gente precisa combinar como que vai ser estruturado
} cluster;

typedef struct Arquivo
{
    metadados meta;
    index indice[128];
    cluster Clusters[128];
} arquivo;

void inicializaMetadados(metadados *meta);

void inicializaIndex(index *ind);

void inicializaCluster(cluster *clus);

void inicializaarquivo(arquivo *arq);
