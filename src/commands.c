<<<<<<< HEAD
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
// FUNCOES AUXILIARES*******************************************************************
// se o diretorio estiver cheio, criar um novo diretorio de mesmo nome em outro cluster
// esse diretorio extende o anterior que estava cheio
int dir_full(char nome[], char extensao[], BYTE index, FILE *arqDados, Cluster *clus, DirectoryFile *dir)
{
    char nomeDir[TAM_NOME_MAX];
    strcpy(nomeDir, dir->nomeDir);
    if (buscarCluster(index, clus, arqDados))
    {
        return 1;
    }
    // copia o conteudo para dir, assim da para interpretar ele como um directory table
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));
    // deixar nome do diretorio igual
    strcpy(dir->nomeDir, nomeDir);
    return 0;
}
// escreve os metadados a serem inseridos no diretorio pai
void modifyMetaFiles(MetaFiles *meta, BYTE index, char nome[], char extensao[])
{
    meta->cluster_inicial = index;
    strcpy(meta->extensao, extensao);
    strcpy(meta->nome_file, nome);
    meta->valida = VALIDO;
}

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

//*************************************************************************************
//---------------------------------------------------------------------------------------
/*
retorna 0 caso de certo
retorna 1 caso de errado*/
int CD_function(Arguments *arguments)
{
    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    char *dirName;
    int i = 0;
    int j;
    int match = 1;
    char *path = (char *)malloc(sizeof(char) * (strlen(arguments->args)) + 1);
    BYTE value;
    // fazer copia da linha pq strtok modifica ela
    strcpy(path, arguments->args);

    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        printf("\n*** ERRO AO ABRIR ARQUIVO***\n");
        free(clus);
        free(dir);
        free(path);
        fclose(arqDados);
        return 1;
    }
    // checa se o numero de argumentos está de acordo
    if (arguments->num_args != arguments->owner->expected_args)
    {
        printf("[ERROR] Expected %u arguments but got %u: '%s'\n", arguments->owner->expected_args, arguments->num_args, arguments->args);
        free(clus);
        free(dir);
        free(path);
        fclose(arqDados);
        return 1;
    }
    // pega o cluster do arqDados e coloca no clus
    if (getFirstCluster(clus, arqDados) != 0)
    {
        printf("[ERROR] Error in getting the cluster\n");
        free(clus);
        free(dir);
        free(path);
        fclose(arqDados);
        return 1;
    }
    // copia o conteudo para dir, assim da para interpretar ele como um directory table
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));
    dirName = strtok(path, "/");
    // se o primeiro argumento for diferente de root
    if ((strcmp("root", dirName)))
    {
        printf("[ERROR] invalid path '%s'\n", arguments->args);
        free(clus);
        free(dir);
        free(path);
        fclose(arqDados);
        return 1;
    }
    while (((dirName = strtok(NULL, "/")) != NULL) && (match == 1))
    {
        j = 0;
        match = 0; // match é variavel para dizer se achou o dir procurado
        while (j < NUM_METAFILES && !match)
        {
            /*  1-se a metafile for invalida nem olha, se for valida checar se é  extensao dir
                2-checar se é extensao dir, strcmp retorna 0 se forem iguais
                3-checar se é o mesmo nome de diretorio*/
            if ((validMetafile(dir->metafiles[j])) && (isDirectory(dir->metafiles[j])) && (matchesDirName(dir->metafiles[j], dirName)))
                match = 1;

            j++;
        }
        // se achou dir
        if (match == 1)
        {
            //tirar um j pq ele conta um a mais quando da match
            j--;
            // apontar o i pro proximo cluster que tem a proxima directory table
            i = dir->metafiles[j].cluster_inicial;
        }

        else
        {
            getValorIndex(clus->cluster_number, arqDados, &value);
            // se acabar o cluster e não der match ve se está no cluster que este atual aponta
            if (value != END_OF_FILE && value != VAZIO)
            {
                // pega o cluster do arqDados e coloca no clus
                if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
                {
                    printf("[ERROR] Error in getting the cluster\n");
                    free(clus);
                    free(dir);
                    free(path);
                    fclose(arqDados);
                    return 1;
                }
                // copia o conteudo para dir, assim da para interpretar ele como um directory table
                memcpy(dir, clus->conteudo, sizeof(DirectoryFile));
            }
            else
            {
                printf("[ERROR] invalid path '%s'\n", arguments->args);
                free(clus);
                free(dir);
                free(path);
                fclose(arqDados);
                return 1;
            }
        }
    }
    arguments->cluster_atual = i;
    free(clus);
    free(dir);
    free(path);
    fclose(arqDados);
    return 0;
}
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
int DIR_function(Arguments *arguments)
{

    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    int i;
    BYTE value = END_OF_FILE;

    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        printf("\n*** ERRO AO ABRIR ARQUIVO***\n");
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    if (clus->cluster_type == 1) // se não for um cluster de pasta
    {
        printf("[ERROR] Not in a directory"); // Isso é mais pra marcar se vai dar algum bug, pq é pra sempre ta dentro de alguma pasta.
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    // checa se o numero de argumentos está de acordo
    if (arguments->num_args != arguments->owner->expected_args)
    {
        printf("[ERROR] Expected %u arguments but got %u: '%s'\n", arguments->owner->expected_args, arguments->num_args, arguments->args);
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    // pega o cluster do arqDados e coloca no clus
    if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
    {
        printf("[ERROR] Error in getting the cluster\n");
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    // copia o conteudo para dir, assim da para interpretar ele como um directory table
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    getValorIndex(clus->cluster_number, arqDados, &value);
    // se for vazio -> cluster aponta pra cluster vazio(0)
    if (value == VAZIO)
        printf("<vazio>\n");
    else
    {
        i = 0;
        while (i < NUM_METAFILES) // Enquanto não chegar no fim da pasta, talvez usar o mesmo EOF pra arquivos. Tem que ver como vai ser setado na função de criar itens na pasta
        {
            if (dir->metafiles[i].valida == VALIDO)
            {
                // se as strings são iguais
                if (strcmp(dir->metafiles[i].extensao, "dir") == 0)
                    printf("<DIR>\t%s\n", dir->metafiles[i].nome_file);
                else
                    printf("     \t%s.%s\n", dir->metafiles[i].nome_file, dir->metafiles[i].extensao);
            }
            i++;
            if (i == NUM_METAFILES && (value != END_OF_FILE))
            {
                // pega o cluster do arqDados e coloca no clus
                if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
                {
                    printf("[ERROR] Error in getting the cluster\n");
                    free(clus);
                    free(dir);
                    fclose(arqDados);
                    return 1;
                }
                // copia o conteudo para dir, assim da para interpretar ele como um directory table
                memcpy(dir, clus->conteudo, sizeof(DirectoryFile));
                // reseta a contagem no proximo cluster
                i = 0;
            }
        }
    }
    free(clus);
    free(dir);
    fclose(arqDados);
    return 0;
}

//---------------------------------------------------------------------------------------
// IDEIA =INVES DE COLOCAR MENSAGENS DE ERROR AQUI TALVEZ PASSAR ADIANTE PARA QUEM CHAMOU A FUNC
int MKFILE_function(Arguments *arguments)
{
    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    char nome[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO] = "bin";//caso usuario nao de extensao
    char *teste;
    int i, metafile_n, offset;
    BYTE value = END_OF_FILE, index;
    BYTE *buffer;

    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        printf("[ERROR] Error opening file\n");
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
    // pega o cluster do arqDados e coloca no clus
    if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
    {
        printf("[ERROR] Error in getting the cluster\n");
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

    // pega o valor da tabela[arguments->cluster_atual] e coloca em value
    if (getValorIndex(arguments->cluster_atual, arqDados, &value))
    {
        printf("[ERROR] Error in getting index value\n");
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    // separa nome da extensao
    strcpy(nome, strtok(arguments->args, ". \n"));
    //testa se o usuario colocou sequer uma extensao
    if((teste = strtok(NULL,". \n")) !=NULL)
        strcpy(extensao, teste);

    // se não for um arquivo de texto(nao sei se precisa disso pq qualquer file pode ser de texto, extensao n serve pra nada, é so o jeito de interpretar)
    if (strcmp(extensao, "txt") != 0)
    {
        printf("[ERROR] Only text files can be created\n");
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    // vai ser o cluster onde o arquivo criado vai estar
    if (criaCluster(extensao, arqDados, &index) != 0)
    {
        printf("[ERROR] Make file error\n");
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    else
    {
        // tratar se diretorio estiver cheio
        if (metafile_n == NUM_METAFILES && value == END_OF_FILE)
        {
            // função que faz o tratamento do diretorio estar cheio->aloca um novo cluster para extender o atual
            if (dir_full(nome, extensao, index, arqDados, clus, dir))
            {
                printf("[ERROR] Error creating new cluster\n");
                free(clus);
                free(dir);
                fclose(arqDados);
                return 1;
            }
            // reseta i para 0 para escrever na memória corretamente mais abaixo
            //"memcpy(buffer, &(dir->metafiles[i]), sizeof(MetaFiles));"
            i = 0;
        }
        // modifica metafile->escreve o nome,extensao e deixe como file valida
        modifyMetaFiles(&(dir->metafiles[i]), index, nome, extensao);
    }
    buffer = makeByteBuffer(sizeof(MetaFiles));
    memcpy(buffer, &(dir->metafiles[i]), sizeof(MetaFiles));
    offset = 1 + TAM_NOME_MAX + TAM_EXTENSAO + (metafile_n * (sizeof(MetaFiles)));
    // escreve mudanças
    writeBlockOfData(arguments->cluster_atual, offset, sizeof(MetaFiles), buffer, arqDados);
    free(buffer);
    // se antes o cluster era dado como vazio-> mudar
    if (value == VAZIO)
    {
        if (mudaEstadoIndex(clus->cluster_number, END_OF_FILE, arqDados))
        {
            printf("[ERROR] Error in overwriting old cluster index pointer"); // sla
            free(clus);
            free(dir);
            fclose(arqDados);
            return 1;
        }
    }

    printf("File '%s.%s' created\n", nome, extensao);
    free(clus);
    free(dir);
    fclose(arqDados);
    return 0;
}
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
/*int RM_function(Arguments *arguments)
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
}*/
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
int MKDIR_function(Arguments *arguments)
{
    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    char nome[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO] = "dir";
    int i, metafile_n, offset;
    BYTE *buffer;
    BYTE index, value;

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
    // separa nome da extensao
    strcpy(nome, arguments->args);
    // vai ser o cluster onde o diretorio criado vai estar
    if (criaCluster(extensao, arqDados, &index) != 0)
    {
        printf("[ERROR] Make directory error\n");
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    else
    {
        // tratar se diretorio estiver cheio
        if (metafile_n == NUM_METAFILES && value == END_OF_FILE)
        {
            // função que faz o tratamento do diretorio estar cheio->aloca um novo cluster para extender o atual
            if (dir_full(nome, extensao, index, arqDados, clus, dir))
            {
                printf("[ERROR] Error creating new cluster\n");
                free(clus);
                free(dir);
                fclose(arqDados);
                return 1;
            }
            // reseta i para 0 para escrever na memória corretamente mais abaixo
            //"memcpy(buffer, &(dir->metafiles[i]), sizeof(MetaFiles));"
            i = 0;
        }
        modifyMetaFiles(&(dir->metafiles[i]), index, nome, extensao);
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
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

Command commands[NCOMMANDS] =
{
    {
        .name = "CD",
        .expected_args = 1u,
        .func = &CD_function
    },
    {
        .name = "DIR",
        .expected_args = 0u,
        .func = &DIR_function
    },
    {
        .name = "RM",
        .expected_args = 1u,
        //.func = &RM_function
    },
    {
        .name = "MKDIR",
        .expected_args = 1u,
        .func = &MKDIR_function
    },
    {
        .name = "MKFILE",
        .expected_args = 1u,
        .func = &MKFILE_function
    },
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
    }
};
=======
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

int DIR_function(Arguments *arguments)
{

    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    int i;
    BYTE value = EOF;

    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        printf("\n*** ERRO AO ABRIR ARQUIVO***\n");
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    if (clus->cluster_type == 1) // se não for um cluster de pasta
    {
        printf("[ERROR] Not in a directory"); // Isso é mais pra marcar se vai dar algum bug, pq é pra sempre ta dentro de alguma pasta.

        return 1;
    }
    // checa se o numero de argumentos está de acordo
    if (arguments->num_args != arguments->owner->expected_args)
    {
        printf("[ERROR] Expected %u arguments but got %u: '%s'\n", arguments->owner->expected_args, arguments->num_args, arguments->args);
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    // pega o cluster do arqDados e coloca no clus
    if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
    {
        printf("[ERROR] Error in getting the cluster\n");
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    // copia o conteudo para dir, assim da para interpretar ele como um directory table
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    getValorIndex(clus->cluster_number, arqDados, &value);
    // se for vazio -> cluster aponta pra cluster vazio(0)
    if (value == 0)
        printf("<vazio>\n");
    else
    {
        i = 0;
        while (i < NUM_METAFILES) // Enquanto não chegar no fim da pasta, talvez usar o mesmo EOF pra arquivos. Tem que ver como vai ser setado na função de criar itens na pasta
        {
            if (dir->metafiles[i].valida == VALIDO)
            {
                // se as strings são iguais
                if (strcmp(dir->metafiles[i].extensao, "dir") == 0)
                    printf("<DIR>\t%s\n", dir->metafiles[i].nome_file);
                else
                    printf("     \t%s.%s\n", dir->metafiles[i].nome_file, dir->metafiles[i].extensao);
            }
            i++;
            if (i == NUM_METAFILES && (value != END_OF_FILE))
            {
                // pega o cluster do arqDados e coloca no clus
                if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
                {
                    printf("[ERROR] Error in getting the cluster\n");
                    free(clus);
                    free(dir);
                    fclose(arqDados);
                    return 1;
                }
                // copia o conteudo para dir, assim da para interpretar ele como um directory table
                memcpy(dir, clus->conteudo, sizeof(DirectoryFile));
                //reseta a contagem no proximo cluster
                i = 0;
            }
        }
    }
    free(clus);
    free(dir);
    fclose(arqDados);
    return 0;
}
//IDEIA =INVES DE COLOCAR MENSAGENS DE ERROR AQUI TALVEZ PASSAR ADIANTE PARA QUEM CHAMOU A FUNC
int MKFILE_function(Arguments *arguments)
{
    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    char nome[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO];
    int i, metafile_n, offset,value = END_OF_FILE;
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
    // pega o cluster do arqDados e coloca no clus
    if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
    {
        printf("[ERROR] Error in getting the cluster\n");
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
    free(buffer);
    getValorIndex(arguments->cluster_atual,arqDados,&value);
    /*if())
    {
        printf("[ERROR] Error in getting index value\n");
        return 1;
    }*/
    //se antes o cluster era dado como vazio-> mudar
    if(value == 0)
    {
        if(mudaEstadoIndex(clus->cluster_number, END_OF_FILE, arqDados))
        {
            printf("[ERROR] Error in overwriting old cluster index pointer");//sla
            free(clus);
            free(dir);
            fclose(arqDados);
            return 1;
        }

    }

    printf("File '%s.%s' created\n", nome, extensao);
    free(clus);
    free(dir);
    fclose(arqDados);
    return 0;
}

Command commands[NCOMMANDS] =
{
    {
        .name = "CD",
        .expected_args = 1u,
        .func = &CD_function
    },
    {
        .name = "DIR",
        .expected_args = 0u,
        .func = &DIR_function
    },
    {
        .name = "RM",
        .expected_args = 1u,
        //.func = &RM_function
    },
    {
        .name = "MKDIR",
        .expected_args = 1u,
        //.func = &MKDIR_function
    },
    {
        .name = "MKFILE",
        .expected_args = 1u,
        .func = &MKFILE_function
    },
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
    }
};
>>>>>>> e3e7dbf78fb73b4da9f35f5ef58c8d6395b1ffab
