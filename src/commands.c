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

int EXIT_function(Arguments *arguments)
{
    printf("\n Desligando... \n");
    exit(0);
    //return 1;
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
        .name = "EXIT",
        .expected_args = 0u,
        .func = &EXIT_function
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
