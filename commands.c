///             Arquitetura e Organiza��o de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler
///                     (00323741)  N�kolas Pad�o
///                     (00275960)  Pedro Afonso Tremea Serpa
///                     (00xxxxxx)  Ricardo

#include "commands.h"
#include "arquivos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BYTE *makeByteBuffer(int size)
{
    BYTE *byte_buffer = (BYTE *)malloc(size);
    return byte_buffer;
}

int validMetafile(MetaFiles meta)
{
    return meta.valida == VALIDO;
}
int isDirectory(MetaFiles meta)
{
    return (!(strcmp(meta.extensao, "dir")));
}
int matchesDirName(MetaFiles meta, char *dirName)
{
    return (!(strcmp(meta.nome_file, dirName)));
}

/*
retorna 0 caso de certo
retorna 1 caso de errado*/
int CD_function(Arguments *arguments)
{

    FileSystem *arq = (FileSystem *)malloc(sizeof(FileSystem));
    char *dirName;
    int i = 0;
    int j;
    int match = 1;
    char *path = (char *)malloc(sizeof(char) * (strlen(arguments->args)) + 1);
    // fazer copia da linha pq strtok modifica ela
    strcpy(path, arguments->args);

    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    readFileSystem(arq);

    if (arguments->num_args != arguments->owner->expected_args)
    {
        printf("[ERROR] Expected %u arguments but got %u: '%s'\n", arguments->owner->expected_args, arguments->num_args, arguments->args);
        return 1;
    }
    dirName = strtok(path, "/");
    // se o primeiro argumento for diferente de root
    if ((strcmp("root", dirName)))
    {
        printf("[ERROR] invalid path '%s'\n", arguments->args);
        return 1;
    }
    while (((dirName = strtok(NULL, "/")) != NULL) && (match == 1))
    {
        memcpy(dir, arq->clusters[i].conteudo, sizeof(DirectoryFile));
        j = 0;
        match = 0; // match � variavel para dizer se achou o dir procurado
        while (j < NUM_METAFILES && !match)
        {
            /*  1-se a metafile for invalida nem olha, se for valida checar se � extensao dir
                2-checar se � extensao dir, strcmp retorna 0 se forem iguais
                3-checar se � o mesmo nome de diretorio*/
            if ((validMetafile(dir->metafiles[j])) && (isDirectory(dir->metafiles[j])) && (matchesDirName(dir->metafiles[j], dirName)))
                match = 1;
            j++;
        }
        // se achou dir
        if (match == 1)
            // apontar o i pro proximo cluster que tem a proxima directory table
            i = dir->metafiles[j].cluster_inicial;
        else
        {
            printf("[ERROR] invalid path '%s'\n", arguments->args);
            return 1;
        }
    }
    arguments->cluster_atual = i;
    free(arq);
    free(dir);
    free(path);
    return 0;
}

/*int DIR_function(Arguments *arguments)
{
    FileSystem *arq = (FileSystem *)malloc(sizeof(FileSystem));

    Cluster clus;
    int     indice, pos, tam=16; //Tamanho maximo de nome a ser decidido
    char    nome[tam];

    indice = arguments.cluster_atual;
    pos    = 256 + (indice * 32000)    //tabela + (indice * tam cluster)


    clus = fseek(arq, pos, SEEK_SET)        //Ta certo isso? seria pra salvar um cluster na memória pra consultas.

    if (clus.cluster_type == 1 ) //se não for um cluster de pasta
    {
        printf("VocÊ não está em uma pasta"); //Isso é mais pra marcar se vai dar algum bug, pq é pra sempre ta dentro de alguma pasta.

        return 1
    }

    else
    {
        fseek(clus, ____, SEEK_CUR);    //Tamanho dos metadados do cluster em bytes
        fgets(nome, tam, arq);

        while (nome != _____)   //Enquanto não chegar no fim da pasta, talvez usar o mesmo EOF pra arquivos. Tem que ver como vai ser setado na função de criar itens na pasta
        {
            printf("\n %s", nome);
            fseek(clus, 1, SEEK_CUR);   //1 byte pro ponteiro da tabela FAT e parte pro próximo arquivo.
            fgets(nome, tam, arq);
        }

        return 0
    }




    // pega o arquivo e anda 256(tabela fat) + 32k*indice posições pra chegar na pasta
    // Verifica se é pasta nos metadados
    // anda mais metadados de pasta posições
    // enquanto não chegar no fim da pasta...
             // print nome do arquivo, anda mais meta do arquivo. repete


}*/

int RM_function(Arguments *arguments)
{
    // TODO:
    //   TEM >1 CLUSTERS
    FILE *arqDados;
    char *dirName;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    int j, match = 1, aux = 0, offset;
    BYTE index, i = 0, dirPai = 0, estado;
    char *path = (char *)malloc(sizeof(char) * (strlen(arguments->args)) + 1);
    // fazer copia da linha pq strtok modifica ela
    strcpy(path, arguments->args);
    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        printf("\n*** ERRO AO ABRIR ARQUIVO***\n");
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    if (arguments->num_args != arguments->owner->expected_args)
    {
        printf("[ERROR] Expected %u arguments but got %u: '%s'\n", arguments->owner->expected_args, arguments->num_args, arguments->args);
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    // caminhamento ate o arq a ser apagado
    dirName = strtok(path, "/");
    // se o primeiro argumento for diferente de root
    if ((strcmp("root", dirName)))
    {
        printf("[ERROR] invalid path '%s'\n", arguments->args);
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }

    if (buscarCluster(0, clus, arqDados) != 0)
    {
        printf("[ERROR] Error in getting the cluster\n");
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    while (((dirName = strtok(NULL, "/")) != NULL) && (match == 1))
    {
        match = 0;  // match � variavel para dizer se achou o dir procurado e eh valido
        dirPai = i; // guarda o indice do diretorio pai
        memcpy(dir, clus->conteudo, sizeof(DirectoryFile));
        index = getIndexMeta(dir, dirName);
        if (index == END_OF_FILE)
        {
            printf("[ERROR] invalid path '%s'\n", arguments->args);
            free(clus);
            free(dir);
            fclose(arqDados);
            return 1;
        }

        if ((validMetafile(dir->metafiles[index])))
        {
            match = 1;
        }
        // se achou dir
        if (match == 1)
            // apontar o i pro proximo cluster que tem a proxima directory table
            i = dir->metafiles[index].cluster_inicial;
        else
        {
            printf("[ERROR] invalid path '%s'\n", arguments->args);
            free(clus);
            free(dir);
            fclose(arqDados);
            return 1;
        }
        if (buscarCluster(i, clus, arqDados) != 0)
        {
            printf("[ERROR] Error in getting the cluster\n");
            free(clus);
            free(dir);
            fclose(arqDados);
            return 1;
        }
    }

    if (clus->cluster_type == CLUSTER_TYPE_DIRECTORY_TABLE) // testa se eh diretorio e se esta vazio
    {
        aux = nrMetaFiles(arqDados, i);
        if (aux != 0)
        {
            printf("[ERROR] Directory not empty\n");
            free(clus);
            free(dir);
            fclose(arqDados);
            return 1;
        }
    }

    // apaga os dados em si
    if (mudaEstadoIndex(i, VAZIO, arqDados))
    {
        printf("[ERROR] Nao foi possivel apagar\n");
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    // dirPai->metafiles[index].valida = INVALIDO;
    estado = INVALIDO;
    offset = 1 + TAM_NOME_MAX + TAM_EXTENSAO + (sizeof(MetaFiles) * index);
    if (writeBlockOfData(dirPai, offset, 1, &estado, arqDados))
    {
        printf("[ERROR] Nao foi possivel apagar\n");
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }

    free(clus);
    free(dir);
    fclose(arqDados);
    printf("Directory %s removed\n", arguments->args);
    return 0;
}

int MKDIR_function(Arguments *arguments)
{
    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    char nome[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO] = "dir";
    int i, metafile_n, offset;
    BYTE *buffer;

    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        printf("\n*** ERRO AO ABRIR ARQUIVO***\n");
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    if (arguments->num_args != arguments->owner->expected_args)
    {
        printf("[ERROR] Expected %u arguments but got %u: '%s'\n", arguments->owner->expected_args, arguments->num_args, arguments->args);
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
    {
        printf("[ERROR] Error in getting the cluster\n");
        free(clus);
        free(dir);
        return 1;
    }

    // copia o conteudo para dir, assim da para interpretar ele como um directory table
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    i = 0;
    while (i < NUM_METAFILES && (dir->metafiles[i].valida == VALIDO))
        i++;
    metafile_n = i;
    // TODO, fazer algo caso diretorio esteja cheio
    if (metafile_n == NUM_METAFILES)
        ;

    else
    {
        // separa nome da extensao
        strcpy(nome, arguments->args);

        // vai ser o cluster onde o arquivo criado vai estar
        if (criaCluster(nome, extensao, &(dir->metafiles[i]), arqDados) != 0)
        {
            printf("[ERROR] Make file error\n");
            free(clus);
            free(dir);
            fclose(arqDados);
            return 1;
        }
    }
    buffer = makeByteBuffer(sizeof(MetaFiles));
    memcpy(buffer, &(dir->metafiles[i]), sizeof(MetaFiles));
    offset = 1 + TAM_NOME_MAX + TAM_EXTENSAO + (metafile_n * (sizeof(MetaFiles)));
    writeBlockOfData(arguments->cluster_atual, offset, sizeof(MetaFiles), buffer, arqDados);
    // free(buffer);

    // escreve dentro do cluster da pasta as infos
    offset = 1;
    writeBlockOfData(dir->metafiles[i].cluster_inicial, offset, TAM_NOME_MAX, (BYTE *)nome, arqDados);

    offset += TAM_NOME_MAX;
    writeBlockOfData(dir->metafiles[i].cluster_inicial, offset, TAM_EXTENSAO, (BYTE *)extensao, arqDados);

    printf("Directory '%s' created\n", nome);
    free(clus);
    free(dir);
    fclose(arqDados);
    return 0;
}

int MKFILE_function(Arguments *arguments)
{
    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    char nome[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO];
    int i, metafile_n, offset;
    BYTE *buffer;
    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        printf("\n*** ERRO AO ABRIR ARQUIVO***\n");
        free(buffer);
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    if (arguments->num_args != arguments->owner->expected_args)
    {
        printf("[ERROR] Expected %u arguments but got %u: '%s'\n", arguments->owner->expected_args, arguments->num_args, arguments->args);
        free(buffer);
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
    {
        printf("[ERROR] Error in getting the cluster\n");
        free(buffer);
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    // copia o conteudo para dir, assim da para interpretar ele como um directory table
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    i = 0;
    while (i < NUM_METAFILES && (dir->metafiles[i].valida == VALIDO))
        i++;
    metafile_n = i;
    // TODO, fazer algo caso diretorio esteja cheio
    if (metafile_n == NUM_METAFILES)
        ;

    else
    {
        // separa nome da extensao
        strcpy(nome, strtok(arguments->args, "."));
        strcpy(extensao, strtok(NULL, "."));

        // vai ser o cluster onde o arquivo criado vai estar
        if (criaCluster(nome, extensao, &(dir->metafiles[i]), arqDados) != 0)
        {
            printf("[ERROR] Make file error\n");
            free(buffer);
            free(clus);
            free(dir);
            fclose(arqDados);
            return 1;
        }
    }
    buffer = makeByteBuffer(sizeof(MetaFiles));
    memcpy(buffer, &(dir->metafiles[i]), sizeof(MetaFiles));
    offset = 1 + TAM_NOME_MAX + TAM_EXTENSAO + (metafile_n * (sizeof(MetaFiles)));
    writeBlockOfData(arguments->cluster_atual, offset, sizeof(MetaFiles), buffer, arqDados);

    printf("File '%s.%s' created\n", nome, extensao);
    free(buffer);
    free(clus);
    free(dir);
    fclose(arqDados);
    return 0;
}

Command commands[NCOMMANDS] =
    {
        {.name = "CD",
         .expected_args = 1u,
         .func = &CD_function},
        {
            .name = "DIR",
            .expected_args = 0u,
            //.func = &DIR_function
        },
        {.name = "RM",
         .expected_args = 1u,
         .func = &RM_function},
        {.name = "MKDIR",
         .expected_args = 1u,
         .func = &MKDIR_function},
        {.name = "MKFILE",
         .expected_args = 1u,
         .func = &MKFILE_function},
        {
            .name = "EDIT",
            .expected_args = 2u,
            //.func = &EDIT_function
        },
        {
            .name = "MOVE",
            .expected_args = 2u,
            //.func = &MOVE_function
        },
        {
            .name = "RENAME",
            .expected_args = 2u,
            //.func = &RENAME_function
        }};
