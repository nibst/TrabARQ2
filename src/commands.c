///             Arquitetura e Organização de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler
///                     (00323741)  Níkolas Padão Schuster
///                     (00275960)  Pedro Afonso Tremea Serpa
///                     (00xxxxxx)  Ricardo

// error.h tem commands.h e arquivos.h
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// FUNCOES AUXILIARES*******************************************************************
// server para funcao RM, digamos que quer remover root/algo/texto.txt
// essa funcao vai separar o caminho "root/algo" do que eu quero remover "texto.txt"
void getPathToFile(char *caminho_inteiro, char *arquivo)
{
    int i;
    i = strlen(caminho_inteiro);
    while (i >= 0 && caminho_inteiro[i] != '/')
        i--;
    if (i != -1)
        caminho_inteiro[i] = '\0';

    i++; // soma o i para pegar a segunda string
    strcpy(arquivo, (caminho_inteiro + i));
}
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
    int i;
    int j;
    int match = 1;
    char *path = (char *)malloc(sizeof(char) * (strlen(arguments->args)) + 1);
    BYTE value;
    // fazer copia da linha pq strtok modifica ela
    strcpy(path, arguments->args);

    // checa se o numero de argumentos está de acordo
    if (arguments->num_args != arguments->owner->expected_args)
    {
        free(path);
        return errorNumArguments(arguments, clus, dir);
    }
    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        free(path);
        return errorOpeningFile(clus, dir, arqDados);
    }


    dirName = strtok(path, "/");
    // se o primeiro argumento for diferente de root
    if ((strcmp("root", dirName)))
    {
        return errorInvalidPath(arguments, clus, dir, path, arqDados);
    }
    i = 0;
    while (((dirName = strtok(NULL, "/")) != NULL) && (match == 1))
    {
        // pega o cluster do arqDados e coloca no clus
        if (buscarCluster(i, clus, arqDados) != 0)
        {
            free(path);
            return errorGettingCluster(clus, dir, arqDados);
        }
        // copia o conteudo para dir, assim da para interpretar ele como um directory table
        memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

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
            // tirar um j pq ele conta um a mais quando da match
            j--;
            // apontar o i pro proximo cluster que tem a proxima directory table
            i = dir->metafiles[j].cluster_inicial;
        }

        else
        {
            if (getValorIndex(clus->cluster_number, arqDados, &value))
            {
                return errorGettingIndexValue(clus, dir, arqDados);
            }
            // se acabar o cluster e não der match ve se está no cluster que este atual aponta
            if (value != END_OF_FILE && value != VAZIO)
            {
                // pega o cluster do arqDados e coloca no clus
                if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
                {
                    free(path);
                    return errorGettingCluster(clus, dir, arqDados);
                }
                // copia o conteudo para dir, assim da para interpretar ele como um directory table
                memcpy(dir, clus->conteudo, sizeof(DirectoryFile));
            }
            else
            {
                return errorInvalidPath(arguments, clus, dir, path, arqDados);
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
    int i, aux;
    BYTE value = END_OF_FILE;

    if (arguments->num_args != arguments->owner->expected_args)
    {
        return errorNumArguments(arguments, clus, dir);
    }
    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        return errorOpeningFile(clus, dir, arqDados);
    }
    // checa se o numero de argumentos está de acordo

    // pega o cluster do arqDados e coloca no clus
    if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
    {
        return errorGettingCluster(clus, dir, arqDados);
    }
    // copia o conteudo para dir, assim da para interpretar ele como um directory table
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    // pega o valor da tabela[arguments->cluster_atual] e coloca em value
    if (getValorIndex(clus->cluster_number, arqDados, &value))
    {
        return errorGettingIndexValue(clus, dir, arqDados);
    }

    if ((aux = nrMetaFiles(arqDados, arguments->cluster_atual)) == -1)
    {
        // o nrMetaFiles tem a funcao buscaCluster dentro, aí eh esse error
        return errorGettingCluster(clus, dir, arqDados);
    }
    // se for vazio printa <vazio>
    if (aux == 0)
        printf("<vazio>\n\n");
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
                    return errorGettingCluster(clus, dir, arqDados);
                }
                // copia o conteudo para dir, assim da para interpretar ele como um directory table
                memcpy(dir, clus->conteudo, sizeof(DirectoryFile));
                // reseta a contagem no proximo cluster
                i = 0;
            }
        }
        printf("\n");
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
    char extensao[TAM_EXTENSAO] = "bin"; // caso usuario nao de extensao
    char *teste;
    int i, metafile_n, offset;
    BYTE value = END_OF_FILE, index, dirPai;
    BYTE *buffer;

    if (arguments->num_args != arguments->owner->expected_args)
    {
        return errorNumArguments(arguments, clus, dir);
    }
    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        return errorOpeningFile(clus, dir, arqDados);
    }

    // pega o cluster do arqDados e coloca no clus
    if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
    {
        return errorGettingCluster(clus, dir, arqDados);
    }
    // copia o conteudo para dir, assim da para interpretar ele como um directory table
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    i = 0;
    while (i < NUM_METAFILES && (dir->metafiles[i].valida == VALIDO))
        i++;
    metafile_n = i;

    // separa nome da extensao
    strcpy(nome, strtok(arguments->args, ". \n"));
    // testa se o usuario colocou sequer uma extensao
    if ((teste = strtok(NULL, ". \n")) != NULL)
        strcpy(extensao, teste);

    // se não for um arquivo de texto(nao sei se precisa disso pq qualquer file pode ser de texto, extensao n serve pra nada, é so o jeito de interpretar)
    if (strcmp(extensao, "txt") != 0)
    {
        printf("[ERROR] Only text files can be created\n\n");
        free(clus);
        free(dir);
        fclose(arqDados);
        return 1;
    }
    // vai ser o cluster onde o arquivo criado vai estar
    if (criaCluster(extensao, arqDados, &index) != 0)
    {
        return errorAllocatingCluster(clus, dir, arqDados);
    }
    else
    {
        // pega o valor da tabela[clus->cluster_number] e coloca em value
        if ((getValorIndex(clus->cluster_number, arqDados, &value)))
        {
            return errorGettingIndexValue(clus, dir, arqDados);
        }
        // tratar se diretorio estiver cheio
        if (metafile_n == NUM_METAFILES && value == END_OF_FILE)
        {
            // função que faz o tratamento do diretorio estar cheio->aloca um novo cluster para extender o atual
            if (dir_full(nome, extensao, index, arqDados, clus, dir))
            {
                return errorAllocatingCluster(clus, dir, arqDados);
            }
            // reseta i para 0 para escrever na memória corretamente mais abaixo
            //"memcpy(buffer, &(dir->metafiles[i]), sizeof(MetaFiles));"
            i = 0;
        }
        // modifica metafile->escreve o nome,extensao e deixe como file valida
        modifyMetaFiles(&(dir->metafiles[metafile_n]), index, nome, extensao);
    }

    buffer = makeByteBuffer(sizeof(MetaFiles));
    memcpy(buffer, &(dir->metafiles[metafile_n]), sizeof(MetaFiles));
    offset = 1 + TAM_NOME_MAX + TAM_EXTENSAO + (metafile_n * (sizeof(MetaFiles)));
    // escreve mudanças
    if (writeBlockOfData(arguments->cluster_atual, offset, sizeof(MetaFiles), buffer, arqDados))
    {
        free(buffer);
        return errorWritingData(clus, dir, arqDados);
    }
    free(buffer);
    dirPai = arguments->cluster_atual;
    offset = sizeof(clus->cluster_type) + sizeof(clus->conteudo);
    // escreve mudanças do cluster_pai
    if (writeBlockOfData(dir->metafiles[metafile_n].cluster_inicial, offset, sizeof(BYTE), &dirPai, arqDados))
        return errorWritingData(clus, dir, arqDados);

    printf("File '%s.%s' created\n\n", nome, extensao);
    free(clus);
    free(dir);
    fclose(arqDados);
    return 0;
}
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
int RM_function(Arguments *arguments)
{
    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    char nome[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO] = "dir"; // se nao tiver extensao ent eh dir
    char arquivo[TAM_NOME_MAX + TAM_EXTENSAO + 1];
    char *teste;
    char *arg_copy = (char *)malloc(sizeof(char) * strlen(arguments->args) + 1);
    int offset;
    BYTE index, i = 0, estado, j;

    // copia para poder dar mensagem com o caminho completo depois
    strcpy(arg_copy, arguments->args);
    // separa o argumento em dois, o caminho até oq queremos remover e o arquivo que queremos remover
    getPathToFile(arguments->args, arquivo);
    // vai pra pasta onde está oq queremos remover
    if (CD_function(arguments))
    {
        free(arg_copy);
        free(clus);
        free(dir);
        return 1;
    }

    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        free(arg_copy);
        return errorOpeningFile(clus, dir, arqDados);
    }
    if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
    {
        free(arg_copy);
        return errorGettingCluster(clus, dir, arqDados);
    }
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    // separa nome da extensao
    strcpy(nome, strtok(arquivo, ". \n"));
    // testa se o usuario colocou sequer uma extensao
    if ((teste = strtok(NULL, ". \n")) != NULL)
        strcpy(extensao, teste);

    index = getIndexMeta(dir, nome, extensao);

    if ((validMetafile(dir->metafiles[index])))
    {
        // apontar o i pro proximo cluster que tem a proxima directory table
        i = dir->metafiles[index].cluster_inicial;
    }
    else
    {
        return errorFileDoesNotExist(arg_copy, clus, dir, arqDados);
    }

    if (buscarCluster(i, clus, arqDados) != 0)
    {
        free(arg_copy);
        return errorGettingCluster(clus, dir, arqDados);
    }

    if (clus->cluster_type == CLUSTER_TYPE_DIRECTORY_TABLE) // testa se eh diretorio
    {
        // se esta nao vazio
        if (nrMetaFiles(arqDados, i) != 0)
            return errorDirectoryNotEmpty(arg_copy, clus, dir, arqDados);

    }

    // loop para apagar todos os cluster do arquivo
    do
    {
        getValorIndex(i, arqDados, &j);
        // clusters que continham o arquivo a ser removido apontam para vazio na tabela para indicar que o programa pode usar eles
        if (mudaEstadoIndex(i, VAZIO, arqDados))
        {
            return errorFreeingCluster(arg_copy, clus, dir, arqDados);
        }
        i = j;
    }
    while (j != END_OF_FILE);

    // invalida os metadados do arq/dir no diretório pai
    estado = INVALIDO;
    offset = 1 + TAM_NOME_MAX + TAM_EXTENSAO + (sizeof(MetaFiles) * index);
    if (writeBlockOfData(arguments->cluster_atual, offset, 1, &estado, arqDados))
    {
        free(arg_copy);
        return errorWritingData(clus, dir, arqDados);
    }

    free(clus);
    free(dir);
    fclose(arqDados);
    printf("File/Directory %s removed\n\n", arg_copy);
    free(arg_copy);
    return 0;
}
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
    BYTE index, value,dirPai;

    if (arguments->num_args != arguments->owner->expected_args)
    {
        return errorNumArguments(arguments, clus, dir);
    }
    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        return errorOpeningFile(clus, dir, arqDados);
    }

    if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
    {
        return errorGettingCluster(clus, dir, arqDados);
    }

    // copia o conteudo para dir, assim da para interpretar ele como um directory table
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    i = 0;
    while (i < NUM_METAFILES && (dir->metafiles[i].valida == VALIDO))
        i++;
    metafile_n = i;

    // pega o nome do dir
    strcpy(nome, arguments->args);
    // vai ser o cluster onde o diretorio criado vai estar
    if (criaCluster(extensao, arqDados, &index) != 0)
    {
        return errorAllocatingCluster(clus, dir, arqDados);
    }
    else
    {
        // pega o valor da tabela[clus->cluster_number] e coloca em value
        if ((getValorIndex(clus->cluster_number, arqDados, &value)))
        {
            return errorGettingIndexValue(clus, dir, arqDados);
        }
        // tratar se diretorio estiver cheio
        if (metafile_n == NUM_METAFILES && value == END_OF_FILE)
        {
            // função que faz o tratamento do diretorio estar cheio->aloca um novo cluster para extender o atual
            if (dir_full(nome, extensao, index, arqDados, clus, dir))
            {
                return errorAllocatingCluster(clus, dir, arqDados);
            }
            // reseta i para 0 para escrever na memória corretamente mais abaixo
            //"memcpy(buffer, &(dir->metafiles[i]), sizeof(MetaFiles));"
            i = 0;
        }
        modifyMetaFiles(&(dir->metafiles[metafile_n]), index, nome, extensao);
    }
    buffer = makeByteBuffer(sizeof(MetaFiles));
    memcpy(buffer, &(dir->metafiles[metafile_n]), sizeof(MetaFiles));
    offset = 1 + TAM_NOME_MAX + TAM_EXTENSAO + (metafile_n * (sizeof(MetaFiles)));
    if (writeBlockOfData(arguments->cluster_atual, offset, sizeof(MetaFiles), buffer, arqDados))
    {
        free(buffer);
        return errorWritingData(clus, dir, arqDados);
    }
    free(buffer);
    // escreve dentro do cluster da pasta as infos
    offset = 1;
    if (writeBlockOfData(dir->metafiles[metafile_n].cluster_inicial, offset, TAM_NOME_MAX, (BYTE *)nome, arqDados))
        return errorWritingData(clus, dir, arqDados);

    offset += TAM_NOME_MAX;
    if (writeBlockOfData(dir->metafiles[metafile_n].cluster_inicial, offset, TAM_EXTENSAO, (BYTE *)extensao, arqDados))
        return errorWritingData(clus, dir, arqDados);

    dirPai = arguments->cluster_atual;
    offset = sizeof(clus->cluster_type) + sizeof(clus->conteudo);
    // escreve mudanças do cluster_pai
    if (writeBlockOfData(dir->metafiles[metafile_n].cluster_inicial, offset, sizeof(BYTE), &dirPai, arqDados))
        return errorWritingData(clus, dir, arqDados);


    printf("Directory '%s' created\n\n", nome);
    free(clus);
    free(dir);
    fclose(arqDados);
    return 0;
}
int EDIT_function(Arguments *arguments)
{
    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    char nome[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO] = "bin"; // caso usuario nao de extensao
    char arquivo[TAM_NOME_MAX + TAM_EXTENSAO + 1];
    char *teste;

    char *caminho_arquivo = (char *)malloc(sizeof(char) * strlen(arguments->args) + 1);
    char *conteudo_arquivo = (char *)malloc(sizeof(char) * strlen(arguments->args) + 1);

    char *token = strtok(arguments->args, " \"");

    int offset;

    BYTE index, i = 0, estado;

    if (arguments->num_args != arguments->owner->expected_args)
    {
        printf("[ERROR] Expected %u arguments but got %u: '%s'\n\n", arguments->owner->expected_args, arguments->num_args, arguments->args);
        free(clus);
        free(dir);
        free(caminho_arquivo);
        free(conteudo_arquivo);
        return 1;
    }

    strcpy(caminho_arquivo, token);

    // pega o segundo token depois de aspas (conteudo do arquivo)
    token = strtok(NULL, "\"");

    if (token == NULL)
    {
        printf("[ERROR] Argument provided for file content is malformed\n\n");
        free(clus);
        free(dir);
        free(caminho_arquivo);
        free(conteudo_arquivo);
        return 1;
    }

    // copia token para conteudo do arquivo
    strcpy(conteudo_arquivo, token);

    // altera os argumentos para poder chamar a CD
    strcpy(arguments->args, caminho_arquivo);

    // separa o caminho em path e arquivo
    getPathToFile(arguments->args, arquivo);

    // faz CD na pasta onde está o arquivo para editar
    if (CD_function(arguments))
    {
        free(clus);
        free(dir);
        free(caminho_arquivo);
        free(conteudo_arquivo);
        return 1;
    }

    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        printf("[ERROR] Error opening file\n\n");
        free(clus);
        free(dir);
        free(caminho_arquivo);
        free(conteudo_arquivo);
        fclose(arqDados);
        return 1;
    }

    // pega o cluster do arqDados e coloca no clus
    if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
    {
        printf("[ERROR] Error in getting the cluster\n\n");
        free(clus);
        free(dir);
        free(caminho_arquivo);
        free(conteudo_arquivo);
        fclose(arqDados);
        return 1;
    }

    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    // separa nome da extensao
    strcpy(nome, strtok(arquivo, ". \n"));

    // testa se o usuario colocou sequer uma extensao
    if ((teste = strtok(NULL, ". \n")) != NULL)
        strcpy(extensao, teste);

    index = getIndexMeta(dir, nome,extensao);

    if ((validMetafile(dir->metafiles[index])))
    {
        // apontar o i pro proximo cluster que tem a proxima directory table
        i = dir->metafiles[index].cluster_inicial;
    }
    else
    {
        printf("[ERROR] File '%s' does not exist\n\n", caminho_arquivo);
        free(clus);
        free(dir);
        free(caminho_arquivo);
        free(conteudo_arquivo);
        fclose(arqDados);
        return 1;
    }

    if (buscarCluster(i, clus, arqDados) != 0)
    {
        printf("[ERROR] Error in getting the cluster\n\n");
        free(clus);
        free(dir);
        free(caminho_arquivo);
        free(conteudo_arquivo);
        fclose(arqDados);
        return 1;
    }

    if (clus->cluster_type == CLUSTER_TYPE_DIRECTORY_TABLE) // testa se eh diretorio
    {
        printf("[ERROR] Cannot edit a directory\n\n");
        free(clus);
        free(dir);
        free(caminho_arquivo);
        free(conteudo_arquivo);
        fclose(arqDados);
        return 1;
    }

    // memcpy(clus->conteudo, conteudo_arquivo, sizeof(char) * (strlen(conteudo_arquivo)) + 1);

    offset = 1;
    if (writeBlockOfData(i, offset, sizeof(char) * (strlen(conteudo_arquivo)) + 1, (BYTE *)conteudo_arquivo, arqDados))
    {
        printf("[ERROR] There was an error editing file\n\n");
        free(clus);
        free(dir);
        free(caminho_arquivo);
        free(conteudo_arquivo);
        fclose(arqDados);
        return 1;
    }
    printf("File %s edited\n\n", caminho_arquivo);
    free(clus);
    free(dir);
    free(caminho_arquivo);
    free(conteudo_arquivo);
    fclose(arqDados);
    return 0;
}
int RENAME_function(Arguments *arguments)
{
    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    char nome[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO] = "dir"; // caso usuario nao de extensao
    char arquivo[TAM_NOME_MAX + TAM_EXTENSAO + 1];
    char *teste;
    BYTE index;
    int offset, i = 0;
    char *arg_copy = (char *)malloc(sizeof(char) * strlen(arguments->args) + 1);
    char *caminho_arquivo = (char *)malloc(sizeof(char) * strlen(arguments->args) + 1);
    char *novo_nome_arquivo;


    // checa se o numero de argumentos está de acordo
    if (arguments->num_args != arguments->owner->expected_args)
    {
        free(caminho_arquivo);
        free(arg_copy);
        return errorNumArguments(arguments, clus, dir);
    }

    strcpy(arg_copy, arguments->args);
    caminho_arquivo = strtok(arguments->args, " ");
    novo_nome_arquivo = strtok(NULL, "\0");
    // faz o trim do novo nome
    while (novo_nome_arquivo[i] == ' ')
    {
        novo_nome_arquivo++;
    }
    i = strlen(novo_nome_arquivo) - 1;
    while (novo_nome_arquivo[i] == ' ')
    {
        novo_nome_arquivo[i] = '\0';
        i--;
    }

    // separa o argumento em dois, o caminho até oq queremos remover e o arquivo que queremos mudar
    getPathToFile(caminho_arquivo, arquivo);
    strcpy(arguments->args, caminho_arquivo);
    if (CD_function(arguments))
    {
        free(clus);
        free(dir);
        free(arg_copy);
        free(caminho_arquivo);

        return 1;
    }
    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        free(caminho_arquivo);
        free(arg_copy);
        return errorOpeningFile(clus, dir, arqDados);
    }
    if (buscarCluster(arguments->cluster_atual, clus, arqDados) != 0)
    {
        free(caminho_arquivo);
        free(arg_copy);
        return errorGettingCluster(clus, dir, arqDados);
    }
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    // separa nome da extensao
    strcpy(nome, strtok(arquivo, ". \n"));
    // testa se o usuario colocou sequer uma extensao
    if ((teste = strtok(NULL, ". \n")) != NULL)
        strcpy(extensao, teste);

    index = getIndexMeta(dir, nome, extensao);

    offset = 1 + TAM_NOME_MAX + TAM_EXTENSAO + (index * (sizeof(MetaFiles)) + 1); // coloca o offset no inicio do nome

    if (strcmp(extensao, "dir") == 0) // caso seja alterado o nome de um diretorio
    {
        if (buscarCluster(dir->metafiles[index].cluster_inicial, clus, arqDados) != 0) // carrega o cluster do diretorio
        {
            free(caminho_arquivo);
            free(arg_copy);
            return errorGettingCluster(clus, dir, arqDados);
        }
        if (clus->cluster_type != CLUSTER_TYPE_DIRECTORY_TABLE) // caso o cluster lido nao seja um diretorio, foi lido o arq errado
        {
            free(caminho_arquivo);
            free(arg_copy);
            return errorGettingCluster(clus, dir, arqDados);
        }
        if (writeBlockOfData(arguments->cluster_atual, offset, strlen(novo_nome_arquivo) + 1, (BYTE *) novo_nome_arquivo, arqDados)) // escreve o novo nome nos metafiles do dir pai
        {
            free(caminho_arquivo);
            free(arg_copy);
            return errorWritingData(clus, dir, arqDados);
        }

        offset = 1; // coloca o offset no inicio do nome

        if (writeBlockOfData(clus->cluster_number, offset, strlen(novo_nome_arquivo) + 1, (BYTE *) novo_nome_arquivo, arqDados)) // altera o nome do diretorio dentro do seu cluster
        {
            free(caminho_arquivo);
            free(arg_copy);
            return errorWritingData(clus, dir, arqDados);
        }
    }
    else // caso seja alterado um arq de texto
    {
        novo_nome_arquivo = strtok(novo_nome_arquivo, "."); // coloca um \0 no final do nome
        teste = strtok(NULL, "\0");
        if (strcmp(teste, "txt")) // garante que a extensao digitada eh suportada
        {
            free(caminho_arquivo);
            return errorFileDoesNotExist(arg_copy, clus, dir, arqDados);
        }
        if (writeBlockOfData(arguments->cluster_atual, offset, strlen(novo_nome_arquivo) + 1, (BYTE *) novo_nome_arquivo, arqDados)) // escreve o novo nome nos metafiles do dir pai
        {
            free(arg_copy);
            free(caminho_arquivo);
            return errorWritingData(clus, dir, arqDados);
        }
    }

    free(clus);
    free(dir);
    fclose(arqDados);
    free(arg_copy);
    free(caminho_arquivo);
    printf("File/Directory %s/%s renamed to %s\n\n", caminho_arquivo, arquivo, novo_nome_arquivo);
    return 0;
}
int EXIT_function(Arguments *arguments)
{
    printf("\n Desligando... \n");
    exit(0);
    // return 1;
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
        .func = &RM_function
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
        .func = &EDIT_function
    },
    {
        .name = "MOVE",
        .expected_args = 2u,
        //.func = &MOVE_function
    },
    {
        .name = "RENAME",
        .expected_args = 2u,
        .func = &RENAME_function
    },
    {
        .name = "EXIT",
        .expected_args = 0u,
        .func = &EXIT_function
    }
};
