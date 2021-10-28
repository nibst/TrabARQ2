///             Arquitetura e Organização de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler
///                     (00323741)  Nikolas Padão
///                     (00275960)  Pedro Afonso Tremea Serpa
///                     (00325735)  Ricardo Hermes Dalcin

// error.h tem commands.h e arquivos.h
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// FUNCOES AUXILIARES*******************************************************************
// trunca o nome do arquivo para entrar no arqDados

void truncaNome(char *nome)
{
    if (strlen(nome) >= TAM_NOME_MAX - 1)
    {
        nome[TAM_NOME_MAX - 1] = '\0';
    }
}

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
    if(TAM_NOME_MAX + TAM_EXTENSAO >= strlen(caminho_inteiro))
        strcpy(arquivo,(caminho_inteiro + i));
    else
        strncpy(arquivo, (caminho_inteiro + i),TAM_NOME_MAX + TAM_EXTENSAO);
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
    BYTE *byte_buffer = (BYTE *)malloc(sizeof(BYTE) * size);
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
            return errorInvalidPath(arguments, clus, dir, path, arqDados);
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
    BYTE index, dirPai;
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

    // separa nome da extensao
    teste = strtok(arguments->args, ". \n");
    if (strlen(teste) >= TAM_NOME_MAX) // garante q o nome tem um tam aceito
    {
        truncaNome(teste);
    }
    strcpy(nome, teste);
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
    // checa se ja existe file com esse nome
    if ((metafile_n = getIndexMeta(dir, nome, extensao)) != -1)
    {
        if (validMetafile(dir->metafiles[metafile_n]))
            return errorFileAlreadyExist(clus, dir, arqDados);
    }

    i = 0;
    // acha um espaço pra criar a file na directory file
    while (i < NUM_METAFILES && (dir->metafiles[i].valida == VALIDO))
        i++;
    metafile_n = i;

    // vai ser o cluster onde o arquivo criado vai estar retorna em index o numero do cluster criado/indice da tabela que o cluster criado aponta
    if (criaCluster(extensao, arqDados, &index) != 0)
    {
        return errorAllocatingCluster(clus, dir, arqDados);
    }
    else
    {
        // modifica metafile->escreve o nome,extensao e deixe como file valida
        modifyMetaFiles(&(dir->metafiles[metafile_n]), index, nome, extensao);
    }

    buffer = makeByteBuffer(sizeof(MetaFiles));
    memcpy(buffer, &(dir->metafiles[metafile_n]), sizeof(MetaFiles));
    offset = 1 + TAM_NOME_MAX + TAM_EXTENSAO + (metafile_n * (sizeof(MetaFiles)));
    // escreve mudanças dos metafiles
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
    char arquivo[TAM_NOME_MAX + TAM_EXTENSAO];
    char *teste;
    char *arg_copy = (char *)malloc(sizeof(char) * strlen(arguments->args) + 1);
    int offset, index;
    BYTE i = 0, estado, j, cluster_atual, aux;

    // copia para poder dar mensagem com o caminho completo depois
    strcpy(arg_copy, arguments->args);
    // garante q nao esta tentando remover a root
    if (strcmp(arg_copy, "root") == 0)
    {
        return errorCannotAlterRoot(arg_copy, clus, dir);
    }
    // separa o argumento em dois, o caminho até oq queremos remover e o arquivo que queremos remover
    getPathToFile(arguments->args, arquivo);

    cluster_atual = arguments->cluster_atual;
    // vai pra pasta onde está oq queremos remover
    if (CD_function(arguments))
    {
        free(arg_copy);
        free(clus);
        free(dir);
        return 1;
    }
    // essa funcao não deve alterar o cluster(geralmente) que o programa se encontra (arguments->cluster_atual)
    aux = arguments->cluster_atual;
    arguments->cluster_atual = cluster_atual;
    cluster_atual = aux;

    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        free(arg_copy);
        return errorOpeningFile(clus, dir, arqDados);
    }
    if (buscarCluster(cluster_atual, clus, arqDados) != 0)
    {
        free(arg_copy);
        return errorGettingCluster(clus, dir, arqDados);
    }
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    // separa nome da extensao
    teste = strtok(arquivo, ". \n");
    if (strlen(teste) >= TAM_NOME_MAX) // garante q o nome tem um tam aceito
    {
        truncaNome(teste);
    }
    strcpy(nome, teste);

    // testa se o usuario colocou sequer uma extensao
    if ((teste = strtok(NULL, ". \n")) != NULL)
        strcpy(extensao, teste);

    if ((index = getIndexMeta(dir, nome, extensao)) == -1)
    {
        return errorFileDoesNotExist(arg_copy, clus, dir, arqDados);
    }

    if ((validMetafile(dir->metafiles[index])))
    {
        // apontar o i pro proximo cluster que tem a proxima directory table
        i = dir->metafiles[index].cluster_inicial;
    }
    else
    {
        return errorFileDoesNotExist(arg_copy, clus, dir, arqDados);
    }
    // vai para o cluster a ser removido
    if (buscarCluster(i, clus, arqDados) != 0)
    {
        free(arg_copy);
        return errorGettingCluster(clus, dir, arqDados);
    }

    if (clus->cluster_type == CLUSTER_TYPE_DIRECTORY_TABLE) // testa se eh diretorio
    {
        // se esta nao vazio
        if (nrMetaFiles(arqDados, i) != 0)
        {
            return errorDirectoryNotEmpty(arg_copy, clus, dir, arqDados);
        }

        // se o cluster removido for o mesmo em que o programa estava quando chamou esse comando, eu volto pro cluster pai
        // isso serve pra n atrapalhar no console e os comandos posteriores
        if ((arguments->cluster_atual == i))
            arguments->cluster_atual = clus->cluster_pai;
    }

    // loop para apagar todos os cluster do arquivo
    do
    {
        if (getValorIndex(i, arqDados, &j))
        {
            free(arg_copy);
            return errorGettingIndexValue(clus, dir, arqDados);
        }

        // clusters que continham o arquivo a ser removido apontam para vazio na tabela para indicar que o programa pode usar eles
        if (mudaEstadoIndex(i, VAZIO, arqDados))
        {
            return errorEditingIndexTable(arg_copy, clus, dir, arqDados);
        }
        i = j;
    }
    while (j != END_OF_FILE);

    // invalida os metadados do arq/dir no diretório pai
    estado = INVALIDO;
    offset = 1 + TAM_NOME_MAX + TAM_EXTENSAO + (sizeof(MetaFiles) * index);
    if (writeBlockOfData(cluster_atual, offset, 1, &estado, arqDados))
    {
        free(arg_copy);
        return errorWritingData(clus, dir, arqDados);
    }

    free(clus);
    free(dir);
    fclose(arqDados);
    printf("File/Directory '%s' removed\n\n", arg_copy);
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
    char *teste = (char *)malloc(sizeof(char) * strlen(arguments->args));
    char nome[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO] = "dir";
    int i, metafile_n, offset;
    BYTE *buffer;
    BYTE index, dirPai;

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

    // pega o nome do dir
    strcpy(teste, strtok(arguments->args, ". \n"));
    if (strlen(teste) >= TAM_NOME_MAX) // garante q o nome tem um tam aceito
    {
        truncaNome(teste);
    }
    strcpy(nome, teste);
    // checa se ja existe dir com esse nome
    if ((metafile_n = getIndexMeta(dir, nome, extensao)) != -1)
    {
        if (validMetafile(dir->metafiles[metafile_n]))
            return errorDirAlreadyExist(clus, dir, arqDados);
    }

    i = 0;
    while (i < NUM_METAFILES && (dir->metafiles[i].valida == VALIDO))
        i++;
    metafile_n = i;

    // vai ser o cluster onde o diretorio criado vai estar
    if (criaCluster(extensao, arqDados, &index) != 0)
    {
        return errorAllocatingCluster(clus, dir, arqDados);
    }
    else
    {
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
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
int EDIT_function(Arguments *arguments)
{
    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    char nome[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO] = "bin"; // caso usuario nao de extensao
    char arquivo[TAM_NOME_MAX + TAM_EXTENSAO + 1];
    char *teste = (char *)malloc(sizeof(char) * strlen(arguments->args) + 1);
    ;
    char *caminho_arquivo = (char *)malloc(sizeof(char) * strlen(arguments->args) + 1);
    char *conteudo_arquivo = (char *)malloc(sizeof(char) * strlen(arguments->args) + 1);
    char *token = strtok(arguments->args, " \"");

    int offset, metafile_n, tam;
    BYTE index, i = 0, cluster_atual, aux, value;

    if (arguments->num_args != arguments->owner->expected_args)
    {
        printf("[ERROR] Expected %u arguments but got %u: '%s'\n\n", arguments->owner->expected_args, arguments->num_args, arguments->args);
        free(teste);
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
        free(teste);
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

    cluster_atual = arguments->cluster_atual;
    // faz CD na pasta onde está o arquivo para editar
    if (CD_function(arguments))
    {
        free(clus);
        free(teste);
        free(dir);
        free(caminho_arquivo);
        free(conteudo_arquivo);
        return 1;
    }
    // essa funcao não deve alterar o cluster que o programa se encontra (arguments->cluster_atual)
    aux = arguments->cluster_atual;
    arguments->cluster_atual = cluster_atual;
    cluster_atual = aux;

    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        free(caminho_arquivo);
        free(conteudo_arquivo);
        free(teste);
        return errorOpeningFile(clus, dir, arqDados);
    }

    // pega o cluster do arqDados e coloca no clus
    if (buscarCluster(cluster_atual, clus, arqDados) != 0)
    {
        free(caminho_arquivo);
        free(teste);
        free(conteudo_arquivo);
        return errorGettingCluster(clus, dir, arqDados);
    }

    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    // separa nome da extensao
    strcpy(teste, strtok(arquivo, ". \n"));
    if (strlen(teste) >= TAM_NOME_MAX) // garante q o nome tem um tam aceito
    {
        truncaNome(teste);
    }
    strcpy(nome, teste);

    // testa se o usuario colocou sequer uma extensao
    if ((teste = strtok(NULL, ". \n")) != NULL)
        strcpy(extensao, teste);
    // procura o arquivo a ser editado na pasta indicada
    if ((metafile_n = getIndexMeta(dir, nome, extensao)) == -1)
    {
        free(conteudo_arquivo);
        free(teste);
        return errorFileDoesNotExist(caminho_arquivo, clus, dir, arqDados);
    }
    // se achou pelo nome, testa se o arquivo eh valido
    if ((validMetafile(dir->metafiles[metafile_n])))
    {
        // apontar o i pro proximo cluster que tem o conteudo do txt
        i = dir->metafiles[metafile_n].cluster_inicial;
    }
    else
    {
        free(conteudo_arquivo);
        free(teste);
        return errorFileDoesNotExist(caminho_arquivo, clus, dir, arqDados);
    }
    // colocar em clus o cluster do txt a ser editado
    if (buscarCluster(i, clus, arqDados) != 0)
    {

        free(caminho_arquivo);
        free(teste);
        free(conteudo_arquivo);
        return errorGettingCluster(clus, dir, arqDados);
    }
    // se for dir da error pq so pode editar .txt
    if (clus->cluster_type == CLUSTER_TYPE_DIRECTORY_TABLE) // testa se eh diretorio
    {
        free(teste);
        return errorCannotEditDir(caminho_arquivo, conteudo_arquivo, clus, dir, arqDados);
    }
    tam = strlen(conteudo_arquivo) + 1;
    // se for maior que o conteudo do cluster suporta, alocar mais cluster pra esse txt
    while (tam >= (sizeof(Cluster) - 3))
    {
        //cria cluster e devolve em index o numero do cluster criado
        if (criaCluster(extensao, arqDados, &index))
        {
            free(caminho_arquivo);
            free(conteudo_arquivo);
            free(teste);
            return errorAllocatingCluster(clus, dir, arqDados);
        }
        //muda tabela de indices pra fazer apontamento correto
        if(mudaEstadoIndex(i,index,arqDados))
        {
            free(conteudo_arquivo);
            return errorEditingIndexTable(caminho_arquivo,clus,dir,arqDados);
        }
        // escreve no cluster a string dada de entrada
        offset = 1;
        if (writeBlockOfData(i, offset, (sizeof(Cluster) - 3), (BYTE *)conteudo_arquivo, arqDados))
        {
            free(caminho_arquivo);
            free(conteudo_arquivo);
            free(teste);
            return errorWritingData(clus, dir, arqDados);
        }
        //pega o valor da tabela de indices pra usar quando for alocar proximo cluster
        if(getValorIndex(i,arqDados,&value))
        {
            free(caminho_arquivo);
            free(conteudo_arquivo);
            free(teste);
            return errorGettingIndexValue(clus,dir,arqDados);
        }
        i = value;
        tam = tam - ((sizeof(Cluster) - 3));
        conteudo_arquivo = conteudo_arquivo + (sizeof(Cluster) - 3);

    }
    // escreve no cluster a string dada de entrada
    offset = 1;
    if (writeBlockOfData(i, offset, sizeof(char) * (strlen(conteudo_arquivo)) + 1, (BYTE *)conteudo_arquivo, arqDados))
    {
        free(caminho_arquivo);
        free(teste);
        free(conteudo_arquivo);
        return errorWritingData(clus, dir, arqDados);
    }
    printf("File '%s' edited\n\n", caminho_arquivo);
    free(clus);
    free(dir);
    free(caminho_arquivo);
    free(conteudo_arquivo);
    fclose(arqDados);
    return 0;
}
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
int RENAME_function(Arguments *arguments)
{
    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    char nome[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO] = "dir"; // caso usuario nao de extensao
    char arquivo[TAM_NOME_MAX + TAM_EXTENSAO + 1];
    char *teste;
    int index;
    int offset, i = 0;
    char *arg_copy = (char *)malloc(sizeof(char) * strlen(arguments->args) + 1);
    char *caminho_arquivo = (char *)malloc(sizeof(char) * strlen(arguments->args) + 1);
    char *novo_nome_arquivo;
    BYTE cluster_atual, aux;

    // checa se o numero de argumentos está de acordo
    if (arguments->num_args != arguments->owner->expected_args)
    {
        free(caminho_arquivo);
        free(arg_copy);
        return errorNumArguments(arguments, clus, dir);
    }

    strcpy(arg_copy, arguments->args);
    strcpy(caminho_arquivo, strtok(arguments->args, " "));

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

    // garante q nao esta tentando renomear a root
    if (strcmp(caminho_arquivo, "root") == 0)
    {
        free(caminho_arquivo);
        return errorCannotAlterRoot(arg_copy, clus, dir);
    }
    // separa o argumento em dois, o caminho até oq queremos renomear e o arquivo que queremos renomear
    getPathToFile(caminho_arquivo, arquivo);


    strcpy(arguments->args, caminho_arquivo);
    // pra dar mensagem de error corretamente
    strcat(caminho_arquivo, "/");
    strcat(caminho_arquivo, arquivo);

    cluster_atual = arguments->cluster_atual;
    if (CD_function(arguments))
    {
        free(clus);
        free(dir);
        free(arg_copy);
        free(caminho_arquivo);

        return 1;
    }
    // essa funcao não deve alterar o cluster que o programa se encontra (arguments->cluster_atual)
    aux = arguments->cluster_atual;
    arguments->cluster_atual = cluster_atual;
    cluster_atual = aux;

    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        free(caminho_arquivo);
        free(arg_copy);
        return errorOpeningFile(clus, dir, arqDados);
    }
    if (buscarCluster(cluster_atual, clus, arqDados) != 0)
    {
        free(caminho_arquivo);
        free(arg_copy);
        return errorGettingCluster(clus, dir, arqDados);
    }
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    // separa nome da extensao
    teste = strtok(arquivo, ". \n");
    if (strlen(teste) >= TAM_NOME_MAX) // garante q o nome tem um tam aceito
    {
        truncaNome(teste);
    }
    strcpy(nome, teste);
    // testa se o usuario colocou sequer uma extensao
    if ((teste = strtok(NULL, ". \n")) != NULL)
        strcpy(extensao, teste);

    if ((index = getIndexMeta(dir, nome, extensao)) == -1)
    {
        free(arg_copy);
        return errorFileDoesNotExist(caminho_arquivo, clus, dir, arqDados);
    }

    // se nao é valida entao nao existe
    if (!validMetafile(dir->metafiles[index]))
    {
        free(arg_copy);
        return errorFileDoesNotExist(caminho_arquivo, clus, dir, arqDados);
    }
    offset = 1 + TAM_NOME_MAX + TAM_EXTENSAO + (index * (sizeof(MetaFiles)) + 1); // coloca o offset no inicio do nome

    if (strcmp(extensao, "dir") == 0) // caso seja alterado o nome de um diretorio
    {
        novo_nome_arquivo = strtok(novo_nome_arquivo, ".");
        // se tiver extensao é error
        if ((teste = strtok(NULL, "\0")) != NULL)
        {
            return errorInvalidExtension(caminho_arquivo, arg_copy, clus, dir, arqDados);
        }
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
        // checa se ja existe file/dir com esse nome que quer mudar
        if ((index = getIndexMeta(dir, novo_nome_arquivo, extensao)) != -1)
        {
            if (validMetafile(dir->metafiles[index]))
                return errorDirAlreadyExist(clus, dir, arqDados);
        }
        // garante q o novo nome tenha um tamanho aceito
        if (strlen(novo_nome_arquivo) >= TAM_NOME_MAX)
        {
            truncaNome(novo_nome_arquivo);
        }

        if (writeBlockOfData(cluster_atual, offset, strlen(novo_nome_arquivo) + 1, (BYTE *)novo_nome_arquivo, arqDados)) // escreve o novo nome nos metafiles do dir pai
        {
            free(caminho_arquivo);
            free(arg_copy);
            return errorWritingData(clus, dir, arqDados);
        }

        offset = 1; // coloca o offset no inicio do nome

        if (writeBlockOfData(clus->cluster_number, offset, strlen(novo_nome_arquivo) + 1, (BYTE *)novo_nome_arquivo, arqDados)) // altera o nome do diretorio dentro do seu cluster
        {
            free(caminho_arquivo);
            free(arg_copy);
            return errorWritingData(clus, dir, arqDados);
        }
        printf("Directory '%s' renamed to '%s'\n\n", caminho_arquivo, novo_nome_arquivo);
    }
    else // caso seja alterado um arq de texto
    {
        novo_nome_arquivo = strtok(novo_nome_arquivo, "."); // coloca um \0 no final do nome
        // se usuario nem botou extensao no nome novo dar erro
        if ((teste = strtok(NULL, "\0")) == NULL)
        {
            return errorInvalidExtension(caminho_arquivo, arg_copy, clus, dir, arqDados);
        }

        if (strcmp(teste, "txt")) // garante que a extensao digitada eh suportada
        {
            free(caminho_arquivo);
            return errorFileDoesNotExist(arg_copy, clus, dir, arqDados);
        }
        // checa se ja existe file/dir com esse nome que quer mudar
        if ((index = getIndexMeta(dir, novo_nome_arquivo, extensao)) != -1)
        {
            if (validMetafile(dir->metafiles[index]))
                return errorFileAlreadyExist(clus, dir, arqDados);
        }
        // garante q o novo nome tenha um tamanho aceito
        if (strlen(novo_nome_arquivo) >= TAM_NOME_MAX)
        {
            truncaNome(novo_nome_arquivo);
        }
        if (writeBlockOfData(cluster_atual, offset, strlen(novo_nome_arquivo) + 1, (BYTE *)novo_nome_arquivo, arqDados)) // escreve o novo nome nos metafiles do dir pai
        {
            free(arg_copy);
            free(caminho_arquivo);
            return errorWritingData(clus, dir, arqDados);
        }
        printf("File '%s' renamed to '%s.%s'\n\n", caminho_arquivo, novo_nome_arquivo, teste);
    }

    free(clus);
    free(dir);
    fclose(arqDados);
    free(arg_copy);
    free(caminho_arquivo);

    return 0;
}
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
int MOVE_function(Arguments *arguments)
{
    FILE *arqDados;
    Cluster *clus = (Cluster *)malloc(sizeof(Cluster));
    DirectoryFile *dir = (DirectoryFile *)malloc(sizeof(DirectoryFile));
    MetaFiles *meta = (MetaFiles *)malloc(sizeof(MetaFiles));
    char *arg_cpy = (char *)malloc((sizeof(char) * strlen(arguments->args)) + 1);
    char nome[TAM_NOME_MAX];
    char extensao[TAM_EXTENSAO] = "dir"; // caso usuario nao de extensao
    char arquivo[TAM_NOME_MAX + TAM_EXTENSAO];
    char *path_dir = (char *)malloc((sizeof(char) * strlen(arguments->args)) + 1);
    char *teste;
    char *path_file = (char *)malloc((sizeof(char) * strlen(arguments->args)) + 1);
    int i, metafile_n, offset, numero_metafile_invalidar;
    BYTE estado, cluster_atual, aux, dirPai, cluster_para_invalidar;
    BYTE *buffer;

    if (arguments->num_args != arguments->owner->expected_args)
    {
        free(arg_cpy);
        free(meta);
        return errorNumArguments(arguments, clus, dir);
    }
    cluster_atual = arguments->cluster_atual;

    strcpy(arg_cpy, arguments->args);
    strcpy(path_file, strtok(arguments->args, " "));
    teste = strtok(NULL, "\0");
    // trim pra caso tenho espacos em branco entre argumentos
    i = 0;
    while (teste[i] == ' ')
    {
        teste++;
    }
    i = strlen(teste) - 1;
    while (teste[i] == ' ')
    {
        teste[i] = '\0';
        i--;
    }
    if (strlen(teste) >= TAM_NOME_MAX) // garante q o nome tem um tam aceito
    {
        truncaNome(teste);
    }
    strcpy(path_dir, teste);
    // separa o argumento em dois, o caminho até oq queremos mover e o arquivo em si que queremos mover
    getPathToFile(path_file, arquivo);
    strcpy(arguments->args, path_file);
    // pra dar mensagem de error corretamente
    strcat(path_file, "/");
    strcat(path_file, arquivo);

    if (CD_function(arguments))
    {
        free(clus);
        free(dir);
        free(meta);
        free(arg_cpy);
        free(path_file);
        return 1;
    }
    // essa funcao não deve alterar o cluster(geralmente) que o programa se encontra (arguments->cluster_atual)
    aux = arguments->cluster_atual;
    arguments->cluster_atual = cluster_atual;
    cluster_atual = aux;

    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        free(meta);
        free(arg_cpy);
        free(path_file);
        return errorOpeningFile(clus, dir, arqDados);
    }
    // está no cluster onde é´pra estar o arquivo ou pasta a ser movida
    if (buscarCluster(cluster_atual, clus, arqDados) != 0)
    {
        free(meta);
        free(arg_cpy);
        free(path_file);
        return errorGettingCluster(clus, dir, arqDados);
    }
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    // separa nome da extensao
    strcpy(nome, strtok(arquivo, ". \n"));
    // testa se o usuario colocou sequer uma extensao
    if ((teste = strtok(NULL, ". \n")) != NULL)
        strcpy(extensao, teste);

    // checa se a file que queremos mover sequer existe
    if ((metafile_n = getIndexMeta(dir, nome, extensao)) == -1)
    {
        free(meta);
        free(arg_cpy);
        return errorFileDoesNotExist(path_file, clus, dir, arqDados);
    }
    // se nao é valida então ela nao existe: erro
    if (!validMetafile(dir->metafiles[metafile_n]))
    {
        free(meta);
        free(arg_cpy);
        return errorFileDoesNotExist(path_file, clus, dir, arqDados);
    }
    // copia os dados daquele metafile para depois colar no endereço onde quer mover
    memcpy(meta, &(dir->metafiles[metafile_n]), sizeof(MetaFiles));

    // invalida esse metafile no dir que estamos agora(estamos no dir pai do arquivo/pasta que queremos mover)
    // só que vvamos fazer isso no final do programa, pq caso o path do segundo argumento seja invalido ou coisa do tipo
    // daí nao acontece de esse cluster ser removido e o programa falhar
    cluster_para_invalidar = cluster_atual;
    numero_metafile_invalidar = metafile_n;
    dirPai = clus->cluster_number;
    // fecha pra chamar a CD
    fclose(arqDados);
    strcpy(arguments->args, path_dir);

    cluster_atual = arguments->cluster_atual;
    if (CD_function(arguments))
    {
        free(clus);
        free(dir);
        free(meta);
        free(arg_cpy);
        free(path_file);
        return 1;
    }
    // essa funcao não deve alterar o cluster(geralmente) que o programa se encontra (arguments->cluster_atual)
    aux = arguments->cluster_atual;
    arguments->cluster_atual = cluster_atual;
    cluster_atual = aux;


    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        free(meta);
        free(arg_cpy);
        free(path_file);
        return errorOpeningFile(clus, dir, arqDados);
    }

    // está no cluster onde é pra receber a pasta/file que se queria mover
    if (buscarCluster(cluster_atual, clus, arqDados) != 0)
    {
        free(meta);
        free(arg_cpy);
        free(path_file);
        return errorGettingCluster(clus, dir, arqDados);
    }
    memcpy(dir, clus->conteudo, sizeof(DirectoryFile));

    if(clusIsInsideOfClusN(clus,meta->cluster_inicial,arqDados))
    {
        return errorHigherHierarchyToLower(meta,arg_cpy,path_file,clus,dir,arqDados);
    }
    // checa se ja existe file/pasta com esse nome
    if ((metafile_n = getIndexMeta(dir, nome, extensao)) != -1)
    {
        if (validMetafile(dir->metafiles[metafile_n]))
        {
            free(meta);
            free(arg_cpy);
            free(path_file);
            return errorFileAlreadyExist(clus, dir, arqDados);
        }
    }
    // se o cluster movido for o mesmo em que o programa estava quando chamou esse comando, eu volto pro cluster pai
    if (isDirectory(*meta) && (arguments->cluster_atual == meta->cluster_inicial))
    {
        if(dirPai != END_OF_FILE)
            arguments->cluster_atual = dirPai;
        else
            arguments->cluster_atual = 0x00;
    }
    i = 0;
    // escolhe um lugar que não seja valido pra colocar a file/pasta que estamos movendo
    while (i < NUM_METAFILES && (dir->metafiles[i].valida == VALIDO))
        i++;
    metafile_n = i;
    buffer = makeByteBuffer(sizeof(MetaFiles));
    memcpy(buffer, meta, sizeof(MetaFiles));
    offset = 1 + TAM_NOME_MAX + TAM_EXTENSAO + (sizeof(MetaFiles) * metafile_n);
    if (writeBlockOfData(cluster_atual, offset, sizeof(MetaFiles), buffer, arqDados))
    {
        free(meta);
        free(arg_cpy);
        free(path_file);
        free(buffer);
        return errorWritingData(clus, dir, arqDados);
    }
    free(buffer);

    dirPai = clus->cluster_number;
    // mudar a o pai da pasta/file que foi movida pra pasta que ele ta dentro agora
    offset = 1 + sizeof(clus->conteudo);
    if (writeBlockOfData(meta->cluster_inicial, offset, 1, &dirPai, arqDados))
    {
        free(meta);
        free(arg_cpy);
        free(path_file);
        return errorWritingData(clus, dir, arqDados);
    }
    //invalidar metafile que estava no diretorio de origem do file/dir movido
    estado = INVALIDO;
    offset = 1 + TAM_NOME_MAX + TAM_EXTENSAO + (sizeof(MetaFiles) * numero_metafile_invalidar);
    if (writeBlockOfData(cluster_para_invalidar, offset, 1, &estado, arqDados))
    {
        free(meta);
        free(arg_cpy);
        free(path_file);
        return errorWritingData(clus, dir, arqDados);
    }


    printf("File/directory '%s' moved to '%s'\n\n", path_file, path_dir);
    free(clus);
    free(dir);
    free(meta);
    free(arg_cpy);
    free(path_file);
    fclose(arqDados);
    return 0;
}
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
int EXIT_function(Arguments *arguments)
{
    printf("\n Desligando... \n");
    exit(0);
}
//---------------------------------------------------------------------------------------
int RESET_function(Arguments *arguments)
{
    FileSystem *arq = (FileSystem *)malloc(sizeof(FileSystem));
    FILE *arqDados;

    if ((arqDados = fopen("arqDados", "rb+")) == NULL)
    {
        printf("[ERROR] Opening file error\n\n");
        return 1;
    }
    inicializaArquivo(arq);
    arguments->cluster_atual = 0x00;//vira o root
    printf("File System reseted \n\n");
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
        .func = &MOVE_function
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
    },
    {
        .name = "RESET",
        .expected_args = 0u,
        .func = &RESET_function
    }
};
