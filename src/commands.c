///             Arquitetura e Organização de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler
///                     (00323741)  Níkolas Padão
///                     (00275960)  Pedro Afonso Tremea Serpa
///                     (00xxxxxx)  Ricardo

#include "commands.h"
#include "arquivos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int validMetafile(MetaFiles meta)
{
    return meta.valida == VALIDO;
}

int isDirectory(MetaFiles meta)
{
    return (!(strcmp(meta.extensao,"dir")));
}

int matchesDirName(MetaFiles meta, char *dirName)
{
    return (!(strcmp(meta.nome_file,dirName)));
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
    char *path = (char *)malloc(sizeof(char) * strlen(arguments->args));
    
    //fazer copia da linha pq strtok modifica ela
    strcpy(path, arguments->args);

    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    readFileSystem(arq);

    if (arguments->num_args != arguments->owner-> expected_args)
    {
        printf("[ERROR] Expected %u arguments but got %u: '%s'\n",arguments->owner-> expected_args, arguments->num_args, arguments->args);
        return 1;
    }
    dirName = strtok(path,"/");
    //se o primeiro argumento for diferente de root
    if((strcmp("root",dirName)))
    {
        printf("[ERROR] invalid path '%s'\n",arguments->args);
        return -1;
    }
    while(((dirName = strtok(NULL,"/")) != NULL) && (match == 1))
    {
        memcpy(dir,arq->clusters[i].conteudo,sizeof(directoryFile));
        j=0;
        match = 0;//match é variavel para dizer se achou o dir procurado
        
        while(j<NUM_METAFILES && !match)
        {
            /*  1-se a metafile for invalida nem olha, se for valida checar se é extensao dir
                2-checar se é extensao dir, strcmp retorna 0 se forem iguais
                3-checar se é o mesmo nome de diretorio */
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
    arguments->cluster_atual = i;
    free(arq);
    free(dir);
    free(path);
    return 0;
}

DIR_function(Arguments *arguments)
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
