#include "commands.h"
#include "arquivos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int validMetafile(metaFiles meta)
{
    return meta.valida == VALIDO;
}
int isDirectory(metaFiles meta)
{
    return (!(strcmp(meta.extensao,"dir")));
}
int matchesDirName(metaFiles meta, char *dirName)
{
    return (!(strcmp(meta.nome_file,dirName)));
}
int CD_function(Arguments *arguments)
{

    fileSystem *arq = (fileSystem *)malloc(sizeof(fileSystem));
    char *dirName;
    int i = 0;
    int j;
    int match = 1;
    char *path = (char *)malloc(sizeof(char) * strlen(arguments->args));
    //fazer copia da linha pq strtok modifica ela
    strcpy(path, arguments->args);

    directoryFile *dir = (directoryFile *)malloc(sizeof(directoryFile));
    readFileSystem(arq);

    /*TODO
     fazer func para testa se o numero de argumentos est� de acordo
    */
    //nao vai precisa desse teste de null quando tiver a funcao de testa numero de arg
    if((dirName = strtok(path,"/"))!=NULL)
    {
        //se o primeiro argumento for diferente de root
        if((strcmp("root",dirName)))
        {
            printf("[ERROR] invalid path '%s'\n",arguments->args);
            return 1;
        }
    }

    while(((dirName = strtok(NULL,"/")) != NULL) && (match == 1))
    {
        memcpy(dir,arq->clusters[i].conteudo,sizeof(directoryFile));
        j=0;
        match = 0;//match � variavel para dizer se achou o dir procurado
        while(j<NUM_METAFILES && !match)
        {
            /*  1-se a metafile for invalida nem olha, se for valida checar se � extensao dir
                2-checar se � extensao dir, strcmp retorna 0 se forem iguais
                3-checar se � o mesmo nome de diretorio*/
            if((validMetafile(dir->metafiles[j])) && (isDirectory(dir->metafiles[j])) && (matchesDirName(dir->metafiles[j],dirName)))
                match = 1;
            j++;
        }
        //se achou dir
        if(match == 1)
            //apontar o i pro proximo cluster que tem a proxima directory table
            i = dir->metafiles[j].cluster_inicial;
        else
        {
            printf("[ERROR] invalid path '%s'\n",arguments->args);
            return 1;
        }
    }

    free(arq);
    free(dir);
    free(path);
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
        //.func = &DIR_function
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
        //.func = &MKFILE_function
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