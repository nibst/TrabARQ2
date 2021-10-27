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
    printf("File/Directory '%s/%s.%s' removed\n\n", arguments->args,nome,extensao);
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

    //char teste64[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut non felis ut dolor posuere fermentum a vel erat. Quisque sed lacus non tellus vulputate cursus. Sed ac porttitor quam, non facilisis odio. Vestibulum nulla justo, facilisis non est nec, rutrum egestas mi. Curabitur sed nibh magna. Nullam ac libero ornare, elementum ex sagittis, auctor velit. Nulla id auctor velit, eget congue elit. Maecenas auctor urna vestibulum ante varius commodo. Nam lacus justo, elementum eget aliquam sed, euismod id lacus. Duis sed elementum augue, quis porttitor arcu. Nunc commodo tellus lectus. Aenean auctor ornare justo, in pellentesque justo egestas a. Suspendisse maximus velit velit, vel eleifend dui lacinia eget. Morbi quis arcu nec massa vehicula scelerisque. Etiam ligula massa, pulvinar aliquam nibh id, dignissim aliquam nisl.  Duis elementum odio sed ultrices varius. Nulla quis nunc efficitur, consequat neque vitae, aliquet enim. Cras non molestie lorem. Cras finibus lorem eu imperdiet fringilla. Nunc facilisis justo metus, nec sodales purus sollicitudin vel. Vestibulum elementum accumsan sagittis. Nullam sed pharetra erat. Ut posuere leo vel mi placerat, id bibendum tortor venenatis. Cras dolor lorem, iaculis vitae sodales nec, venenatis id ante. Fusce metus ligula, aliquet eget ornare a, venenatis a lectus.  Suspendisse non venenatis magna. Sed elementum, dui sed sodales varius, massa tortor eleifend justo, et aliquam velit ipsum eget risus. Etiam erat sem, fringilla in ipsum fermentum, congue efficitur ipsum. Donec id mauris et mauris lobortis congue. Donec ornare metus non orci rutrum congue. Pellentesque efficitur magna non felis condimentum sollicitudin. Proin iaculis viverra diam. Maecenas eleifend ipsum eget mi feugiat, eget placerat risus maximus. Nunc molestie pellentesque suscipit. Phasellus ultricies sit amet ipsum id ultricies. Lorem ipsum dolor sit amet, consectetur adipiscing elit.  In maximus tincidunt lobortis. Donec non tristique urna. In nisl nulla, condimentum feugiat vestibulum ac, suscipit nec lacus. Vestibulum vehicula, odio sed vehicula dapibus, metus velit blandit felis, in consequat ex massa cursus nisl. Nulla nec finibus nisi. Proin nec urna quis tellus auctor egestas. Pellentesque vehicula tincidunt urna, vel placerat libero gravida dictum. Sed mollis consequat purus, vitae aliquet lectus vestibulum et. Praesent quis leo id sapien mollis eleifend eu a velit. Proin tristique, dui sit amet tincidunt rhoncus, odio dui vestibulum justo, ut pellentesque nisi turpis sit amet nisi. In nec tellus elit. Mauris dapibus nibh a tincidunt vehicula. Sed aliquet massa velit, id consectetur velit consectetur in. Nam sapien augue, lacinia at tempus ac, viverra eget velit. Vivamus tempor elit et velit venenatis aliquet. Proin tempor eu ipsum ac malesuada.  Cras ante ligula, posuere quis mattis non, interdum luctus sapien. In rhoncus lacus neque, quis euismod leo semper eleifend. Suspendisse sit amet nibh viverra, vehicula urna sed, hendrerit lorem. Nunc eu eros erat. Proin vitae nisl id turpis eleifend lobortis. Sed ac aliquam velit. Nunc eleifend tincidunt sapien, et mollis dolor vehicula ac. Nullam lacinia mollis mi vel consequat. Fusce id lectus malesuada, varius ipsum ac, elementum mauris. Nulla congue tellus magna, nec vulputate est malesuada eu.  Maecenas sollicitudin libero est, non rutrum odio tristique nec. Donec convallis orci eu justo vestibulum ullamcorper. Mauris nec ullamcorper nibh. Aliquam malesuada elementum massa non lobortis. Aliquam elementum odio non ex finibus finibus. Maecenas vitae ultrices orci, ut placerat nulla. Integer quis ipsum dapibus, gravida lorem et, laoreet sapien. Maecenas ut aliquet augue. Interdum et malesuada fames ac ante ipsum primis in faucibus. Phasellus consequat nisi sed purus efficitur, tempor finibus nibh efficitur. Curabitur et eros vestibulum, cursus orci sit amet, iaculis felis.  Phasellus ante lacus, iaculis id purus eget, auctor maximus enim. Praesent at luctus tellus. Maecenas convallis mi vel nibh efficitur, non rhoncus eros blandit. Cras et nisl non lorem mollis maximus non et lacus. Morbi bibendum velit arcu, sed finibus orci maximus blandit. Aenean id risus metus. Ut neque dui, vulputate quis leo sit amet, sodales auctor nisl. Vivamus id molestie dui. Proin faucibus cursus odio, at vehicula nulla cursus vel. Vivamus gravida rhoncus diam, id lobortis mauris iaculis at. Fusce ante risus, ullamcorper non tincidunt ut, vehicula sit amet diam. Nulla a orci enim. Fusce nec justo urna. Nulla molestie tempor tellus, eu eleifend est varius non.  Nam leo velit, luctus vel est sit amet, porta imperdiet turpis. Vestibulum erat enim, congue facilisis sem et, dapibus tempor turpis. Praesent eget interdum augue, eu posuere est. Nulla facilisi. Aliquam aliquam eros a viverra fringilla. Nullam venenatis, lorem dictum maximus pharetra, felis eros egestas enim, non volutpat metus sapien vitae elit. Ut mattis libero sit amet porttitor semper. Nunc ac dui vel velit consectetur ultrices. Integer lobortis purus vitae metus efficitur, sed sodales eros posuere. Aliquam ut quam ac ligula sodales sollicitudin eu quis turpis. Proin turpis dui, semper eu tincidunt sit amet, ultrices id velit. Vestibulum suscipit, nibh nec hendrerit luctus, lacus tortor dictum mi, id luctus ex leo ac nulla. Integer hendrerit erat at lacinia ultrices. Vivamus eget imperdiet tellus.  Mauris vitae scelerisque justo. Ut eget commodo massa. Nunc euismod, odio at interdum elementum, est orci sollicitudin nisl, at mattis ex elit viverra ligula. Donec quis euismod tortor. Vestibulum pharetra convallis urna, nec lacinia diam porttitor at. Mauris lacinia libero quis massa mattis, id ornare quam pellentesque. Aliquam porta fringilla tincidunt. Etiam lacus risus, commodo quis imperdiet eget, eleifend vitae tellus. In nec lacus ut mauris lobortis hendrerit. Vestibulum ut posuere urna. Sed tempor libero neque, sollicitudin fringilla sem auctor eu. Aliquam aliquam eu ligula a porttitor. Maecenas mattis diam vitae enim dignissim, et pharetra eros pharetra.  In sed nibh a nisi rhoncus tincidunt. Etiam facilisis est quis mauris porta imperdiet. Nulla sed turpis ut ligula blandit consectetur sed non nulla. Sed ut iaculis nibh. Curabitur accumsan, purus sit amet laoreet elementum, ex dui blandit felis, nec semper eros mi in massa. Interdum et malesuada fames ac ante ipsum primis in faucibus. Suspendisse sodales blandit tincidunt. Proin luctus dignissim neque imperdiet molestie. Donec at condimentum mi.  Vivamus consectetur blandit aliquet. Vestibulum et turpis mattis, ornare nisl a, egestas ligula. Ut ullamcorper interdum leo. Curabitur interdum sodales tellus porttitor suscipit. Donec eu rutrum turpis, ut luctus magna. Pellentesque rutrum euismod turpis, vitae cursus metus convallis at. Ut arcu augue, vehicula id diam nec, malesuada condimentum nulla. Proin facilisis venenatis iaculis. Curabitur risus enim, auctor sed vestibulum porttitor, egestas id enim. Duis a dignissim tellus, ac pretium velit. Pellentesque imperdiet libero in turpis feugiat, dictum molestie sapien egestas. Integer ullamcorper lorem ac mi ultrices ullamcorper. Suspendisse risus elit, sodales sit amet urna et, accumsan vestibulum velit. Maecenas sit amet diam ut enim egestas laoreet egestas in urna.  Nam vestibulum, ligula venenatis lobortis commodo, odio ex vehicula odio, vitae fermentum nulla risus at lacus. Fusce eu pulvinar metus. Mauris sed pulvinar orci. Nulla facilisi. Nulla efficitur posuere mollis. Phasellus ut risus id ipsum tristique porta. Fusce auctor enim mi, sed ullamcorper nunc efficitur feugiat. Nam sodales felis vel eros tempus, a mollis nunc imperdiet. Morbi in congue eros. Morbi iaculis blandit ante, at hendrerit ex bibendum sit amet. Donec dapibus aliquam blandit. Donec vel interdum dui.  Fusce eget sem massa. Aliquam consectetur sollicitudin ante, quis consectetur erat pretium vel. Proin nulla turpis, volutpat non luctus convallis, tincidunt nec nisi. Nulla facilisi. Nam eu tincidunt ex. Phasellus posuere justo id tempus lacinia. Vestibulum interdum ex vitae mauris pulvinar, nec tristique lacus gravida. Aenean eleifend blandit nisi, vitae feugiat risus tempus in. Ut dapibus ultrices nibh eget congue.  Integer malesuada urna quis felis tempor, eget bibendum erat tempor. Proin tempus pretium justo, ac maximus lorem euismod ac. Cras vel ultrices sapien. Ut in enim auctor, malesuada ex volutpat, sollicitudin leo. Curabitur dignissim velit eu sagittis euismod. In gravida aliquam arcu, a ullamcorper nibh. Maecenas ut tristique velit. Nunc ac tincidunt odio.  Phasellus pharetra posuere ultricies. Integer vulputate placerat blandit. Etiam pharetra venenatis laoreet. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Fusce non quam nisl. In malesuada ex quis porttitor aliquet. Donec elit purus, rutrum sed lacinia eu, tempus viverra elit. Etiam at odio ut arcu sodales mollis. Curabitur nec odio fringilla, vestibulum enim sit amet, congue mi. Vestibulum quis velit placerat, congue odio eu, sodales arcu. Quisque posuere magna ac lacus dictum sollicitudin. Aliquam erat volutpat.  Vivamus porta nisl eu nunc sagittis, in hendrerit mauris sollicitudin. Nam scelerisque, est in vestibulum ornare, erat erat aliquet lacus, in consequat ipsum odio vitae erat. Interdum et malesuada fames ac ante ipsum primis in faucibus. Integer sodales et lorem ut sagittis. Aenean tempor aliquam ullamcorper. Donec sit amet neque at nunc consequat rutrum. Sed rutrum orci odio, eu luctus odio commodo at. Suspendisse et eleifend massa. Morbi in commodo eros, a faucibus turpis. Suspendisse sit amet libero odio. Nullam semper et diam vel dapibus.  Fusce ultricies urna quis enim mattis ullamcorper. Sed ut commodo sapien. Praesent interdum egestas semper. Pellentesque vulputate tempor elit mattis rhoncus. Proin tincidunt, sapien at porttitor semper, nibh est lacinia quam, eu luctus sapien tortor vitae turpis. Sed ut ipsum lectus. Nam tristique augue quis tincidunt laoreet.  Suspendisse mollis nulla dolor, eu eleifend tellus dictum quis. Aliquam cursus orci nulla, non efficitur urna vestibulum in. Aenean imperdiet justo nec metus congue condimentum. Ut fermentum imperdiet urna, eget sodales felis. Sed id tincidunt justo. Duis in turpis sit amet diam dignissim laoreet ut vel augue. Quisque ullamcorper vitae est in finibus. Vivamus quis mattis arcu. Sed sed orci arcu. Aenean varius posuere ante et lacinia.  Donec condimentum quam dui, et pellentesque elit tempus at. Aliquam blandit purus eget aliquam sollicitudin. Donec consequat augue vitae porttitor interdum. Donec interdum, elit porttitor hendrerit aliquam, ligula mauris cursus enim, sit amet hendrerit orci tortor sed mauris. Suspendisse consectetur euismod lectus, sed vehicula felis malesuada vel. Duis ac tellus scelerisque lorem mollis lacinia. Sed eget metus nisl. Curabitur tincidunt turpis at nunc cursus auctor et ac velit. Aliquam laoreet quam non placerat bibendum. Nulla consequat pulvinar ligula, sit amet pharetra quam bibendum sed. Ut neque magna, semper nec finibus auctor, vulputate at est. Ut vestibulum tempus ipsum, ut vestibulum tellus euismod ac. Sed neque eros, pellentesque sit amet leo ut, cursus commodo justo. Mauris bibendum dignissim sapien lobortis dignissim. Vivamus sapien velit, mollis vel imperdiet in, congue ut lectus. Nam feugiat venenatis purus, vel efficitur ante consequat ac.  Ut volutpat placerat ligula sit amet fringilla. Proin ultrices diam vel ligula interdum tincidunt a mattis est. Aliquam tempus felis eget eros pretium, ut rhoncus metus fermentum. Mauris in tempus magna. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Phasellus blandit, turpis et gravida tempus, nunc erat maximus ipsum, at lobortis purus libero sed enim. Curabitur id massa ex. Nunc consequat pellentesque pulvinar. Cras finibus enim id consequat auctor.  Suspendisse potenti. Nunc at condimentum neque. In suscipit lacinia vulputate. Mauris augue nunc, porta in condimentum et, tincidunt quis felis. Suspendisse quis augue vel tellus interdum laoreet. Pellentesque varius condimentum purus, vel aliquam tellus ultricies nec. Quisque interdum odio nec blandit accumsan. Nam eget vehicula elit, ac fermentum lacus. Nulla facilisi. Cras fermentum, tortor sit amet tincidunt varius, nibh est vehicula urna, non pharetra dui elit vitae magna. Donec erat tellus, accumsan sed hendrerit sed, posuere in nibh. Nam ac ligula eget diam pretium sagittis ac at arcu. Nunc quis hendrerit massa. Praesent id convallis ipsum. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Vestibulum quis congue purus.  Praesent vestibulum lacus turpis, ut pulvinar ex semper at. Donec at urna sit amet lectus bibendum tincidunt. Morbi ac leo nisl. Praesent eu magna lacinia, ornare orci vel, hendrerit libero. Suspendisse potenti. Nunc ornare diam vitae placerat tempor. Integer eleifend magna sed gravida porttitor. Pellentesque suscipit dignissim urna vel accumsan. Suspendisse a diam cursus, lobortis nibh vel, congue mauris. Sed non feugiat tortor. Donec non tristique dolor. Etiam dapibus risus vitae orci fringilla vehicula. Donec sit amet est ut velit vestibulum tempus. Nunc tortor magna, feugiat id iaculis sit amet, sodales ut ipsum. Fusce varius massa felis, at vehicula elit porta vel. Aliquam sodales cursus felis ac sagittis.  Sed aliquet elit dolor, et dapibus sem tristique at. Curabitur vestibulum a metus vel rhoncus. Etiam vestibulum imperdiet eleifend. Suspendisse faucibus augue ut elementum accumsan. Nam vitae tristique dolor. Ut congue volutpat rutrum. Pellentesque condimentum posuere volutpat.  Vivamus eros urna, posuere eu tellus et, dictum viverra odio. Nam a pharetra velit, ut semper elit. Nulla ornare massa at arcu venenatis commodo. Proin libero tortor, facilisis molestie urna et, ornare rutrum felis. Nunc eget lectus sollicitudin, aliquam felis in, luctus nunc. Praesent sed lorem in velit commodo elementum ac ac dui. Proin ultrices, odio vitae dapibus rhoncus, elit diam dapibus ipsum, sit amet rutrum orci velit et nibh.  Aenean ullamcorper in dolor in scelerisque. Proin euismod velit nec arcu ultricies mollis. Duis congue mauris nec orci cursus, a sagittis quam ultrices. Maecenas feugiat magna et libero fringilla, ac dapibus lorem cursus. In ac vulputate magna, nec tempus ligula. Fusce dignissim maximus consectetur. In accumsan risus a sem cursus mollis. Nulla dapibus ligula et porta condimentum. Fusce consectetur eget odio in aliquet. Etiam maximus nunc sit amet mattis tincidunt. Nunc quis semper turpis, porta ultrices arcu. Nullam eget sem metus. Maecenas sed elit finibus, commodo ante non, laoreet leo. Suspendisse consequat arcu bibendum, blandit ante semper, fringilla ex. Ut suscipit, justo eu dignissim lobortis, nisl lectus iaculis eros, a accumsan augue orci vitae tellus. Nunc tempor urna a metus varius, vel porta felis convallis.  Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Nullam mi enim, ullamcorper non ullamcorper sed, tempus lobortis tortor. Nam nunc felis, venenatis quis dignissim ullamcorper, molestie sit amet tortor. Vestibulum commodo nunc sit amet urna maximus lacinia id vel felis. Duis ullamcorper vestibulum mollis. Vestibulum interdum nisi massa, sed commodo tellus interdum ut. Ut at rhoncus mi, viverra sollicitudin magna. Ut ullamcorper orci at tristique condimentum. Nullam leo ex, venenatis id nibh vitae, dictum vehicula massa. Aliquam tellus sem, euismod id lorem a, egestas aliquet nunc. Donec id laoreet neque. Duis sed convallis orci. Aliquam elementum orci in justo rhoncus egestas. Nulla eleifend turpis bibendum nisl fringilla placerat. Aenean consectetur risus a ligula faucibus, sed molestie mi dapibus.  Praesent mi dolor, tincidunt in pulvinar eget, semper in turpis. Phasellus accumsan elit ligula, id ornare purus rhoncus et. Sed ac interdum dui, quis accumsan libero. Aliquam a dolor at leo auctor luctus. Vestibulum sed enim urna. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. In sollicitudin mauris suscipit neque sollicitudin, quis facilisis turpis viverra. Maecenas vel pulvinar dolor. Quisque quis ligula augue. Etiam consectetur purus dolor, a ornare sapien vulputate id. Praesent est neque, auctor nec lacus eget, vestibulum ultrices mi. Pellentesque nec risus consequat, dignissim nunc sed, cursus ex. Maecenas mollis purus ligula, nec tempus sem condimentum eget. Pellentesque at leo pellentesque, auctor enim at, rutrum nulla.  Morbi ex lacus, lacinia vel molestie interdum, fermentum sed massa. Integer nec nisl sed elit volutpat pellentesque ac sed urna. Morbi eleifend fermentum velit ut viverra. Mauris in lacinia ex, eget faucibus libero. Maecenas commodo nisi nec mauris euismod, a viverra mauris dignissim. Pellentesque id cursus tortor. Integer sit amet nibh vel ante varius hendrerit. Aenean aliquam porttitor hendrerit. Nullam accumsan, nulla eu sodales tempor, erat lacus pulvinar ligula, sed fringilla metus felis ac ligula.  Nam nulla urna, venenatis eget quam id, finibus finibus sapien. Sed dictum leo sit amet nunc posuere facilisis. Ut bibendum iaculis vestibulum. Ut lectus ante, fermentum nec felis nec, varius pellentesque massa. Nunc hendrerit volutpat augue id scelerisque. Duis velit turpis, lacinia non ligula interdum, euismod feugiat massa. Cras a ipsum cursus, volutpat libero eu, sodales ex. Suspendisse eros leo, semper ac dictum ut, lobortis et turpis. Nunc placerat euismod rhoncus. Aenean sodales elit libero, sed euismod magna ultrices ac. Mauris porttitor egestas turpis, eget rhoncus dolor luctus nec. Proin neque ligula, sagittis et posuere eget, tincidunt ut ante. Etiam eget sapien tincidunt, tincidunt nulla id, venenatis neque.  Donec eget scelerisque magna. Cras iaculis cursus iaculis. Fusce leo massa, volutpat ac magna in, tincidunt varius arcu. Etiam congue vulputate enim, ut vulputate sapien tincidunt at. Integer facilisis ullamcorper congue. Donec sem lacus, faucibus ac dolor et, porttitor ultrices lorem. Sed turpis dolor, dictum eu tellus eu, tincidunt viverra velit. Cras fermentum turpis orci. Sed nisl tortor, cursus et odio eget, porttitor ultricies ex.  Vestibulum tristique nulla feugiat augue fermentum efficitur. Nullam lacinia nisl ipsum, a volutpat lectus tincidunt mollis. Ut congue est leo, vel condimentum velit rutrum quis. In pretium est quam, et lobortis nibh varius sit amet. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Aenean vehicula consectetur laoreet. Phasellus sollicitudin mi eget libero iaculis elementum. In aliquam, enim quis finibus faucibus, libero tortor rutrum arcu, quis venenatis lacus nulla ac nisi. Maecenas interdum non orci at luctus. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum sit amet convallis metus. Nulla facilisi.  Morbi nec risus in turpis pellentesque consequat faucibus rutrum urna. Morbi feugiat, ex non pellentesque lacinia, magna massa dapibus massa, nec rutrum ex lectus sed ex. Proin quam quam, vehicula in dolor a, imperdiet hendrerit erat. Quisque vitae finibus ipsum. Nam volutpat nunc vel fringilla ultricies. Nullam id quam sollicitudin nulla dapibus dictum. Curabitur rhoncus mi purus, in iaculis orci condimentum id. Integer ornare ullamcorper odio, in bibendum libero cursus ut. Mauris commodo eu libero sed tristique. Aliquam vulputate varius velit vulputate finibus. Nulla id molestie arcu, et iaculis arcu. Sed quis pulvinar mi. Donec ullamcorper dui felis, a lacinia est feugiat vel.  Pellentesque in ex diam. Ut consequat sollicitudin lectus eu pellentesque. Donec vitae varius elit. Nunc purus lacus, blandit quis dignissim vitae, porta id massa. Duis ornare in dolor vulputate ultricies. Vivamus in eleifend quam. Nunc tempor arcu urna, et aliquet ex blandit a. Ut efficitur tellus a efficitur scelerisque. Fusce eget arcu sit amet ex interdum porta. Fusce at odio sit amet erat ornare convallis ut nec nisi.  Proin tincidunt eget lorem vel egestas. Nam euismod metus nec elit interdum euismod. In hac habitasse platea dictumst. Nulla ex purus, vehicula vel condimentum in, pharetra quis risus. In a neque nec risus pulvinar finibus. Integer scelerisque mi eget scelerisque sollicitudin. Donec ornare fermentum semper.  Suspendisse ac commodo metus. Donec et dolor luctus, imperdiet neque at, maximus ex. Suspendisse quis tortor bibendum, porta ligula fringilla, dignissim lectus. In enim purus, facilisis ac gravida quis, pulvinar quis augue. Cras sagittis vestibulum nisi, vel sollicitudin est blandit sed. Pellentesque lorem eros, blandit at lacus id, lacinia faucibus ante. Donec lobortis felis sit amet diam faucibus, fringilla viverra est facilisis. Nullam mollis quis nulla et fringilla. Cras sed mi non sapien feugiat pharetra. Pellentesque pellentesque elit eu ex finibus vulputate. Phasellus at massa in nibh euismod varius. Fusce finibus risus libero. Maecenas sagittis, orci sed pharetra sollicitudin, nibh nisl feugiat urna, non vulputate quam felis id nisl. Maecenas in leo id mi porttitor ultricies id vulputate justo. Nam et risus vitae tortor condimentum congue.  Curabitur condimentum semper feugiat. Pellentesque lorem arcu, euismod non fermentum at, mollis sit amet sapien. Sed consequat diam metus, sed malesuada lacus interdum vel. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Sed sollicitudin eros eget diam faucibus fermentum. Sed pellentesque magna nisl. In nec dolor eu ante euismod volutpat et quis tortor. Vestibulum ultricies blandit nibh, sed ultrices mi rhoncus id. Etiam vitae nisl id dolor aliquam vulputate id id massa. Ut ullamcorper orci nec diam porta semper. Fusce libero quam, imperdiet non sodales vitae, facilisis sagittis felis. Etiam mattis pulvinar lectus, a fringilla lectus. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Praesent mattis nunc ligula. Maecenas at cursus lorem, semper iaculis erat. Quisque et accumsan ex.  Quisque placerat eget eros in laoreet. Nulla sit amet neque at velit venenatis efficitur nec in ante. Aenean ornare ut dui non pulvinar. Mauris pharetra sed sapien in venenatis. Nunc eget mi nec quam ullamcorper iaculis a nec ipsum. Nunc neque erat, volutpat nec quam vel, pharetra feugiat purus. In pharetra maximus volutpat. Donec venenatis, quam id congue interdum, enim tortor interdum magna, vitae semper tortor dolor a tellus. Pellentesque vel libero ac metus finibus pellentesque. Vivamus aliquam porta lectus at posuere. Quisque sed tortor eu erat lacinia semper. Duis volutpat diam tellus, eu sagittis velit cursus vitae. Quisque cursus dolor eu velit euismod tristique sit amet ut felis. Proin scelerisque velit leo, eu placerat odio venenatis nec.  Curabitur nunc nunc, vulputate at vulputate blandit, eleifend a metus. Curabitur ut nisi in nulla maximus semper. Mauris ac enim lacinia, congue est eu, semper orci. Vivamus hendrerit ex tristique euismod interdum. Phasellus quis hendrerit augue. Nullam consequat fringilla nulla eget posuere. Duis enim orci, commodo nec magna ut, convallis rhoncus nibh. Nulla facilisi. Nunc facilisis aliquam posuere. Maecenas euismod justo pharetra nulla tempor, tempus sollicitudin arcu venenatis. Vestibulum euismod ex eget tortor cursus venenatis. Suspendisse vel auctor ipsum, eu suscipit diam. Nulla facilisi.  Maecenas non risus ac purus mollis pretium vel in lacus. Phasellus quis purus at leo ullamcorper consequat tincidunt id lorem. Donec vel turpis tristique, facilisis mauris tristique, pellentesque urna. Fusce hendrerit sit amet ligula quis imperdiet. Etiam turpis dolor, pellentesque vel est eu, tempus pellentesque erat. Donec vel libero lectus. Suspendisse accumsan, velit et viverra interdum, mi velit volutpat quam, cursus aliquam ex risus eget nisi. Aenean laoreet sapien eget justo molestie, quis pretium quam aliquet. In a sem arcu. Nunc sed tincidunt ipsum. Morbi ac molestie nisl. Ut facilisis eget quam ac tempus. Nunc accumsan sagittis quam, eget malesuada magna dictum sed.  Nam facilisis lorem nec vulputate dictum. Morbi nibh ex, accumsan sed volutpat ut, blandit et odio. Praesent quis risus et orci fermentum facilisis. Vestibulum orci mauris, posuere a sapien eget, posuere lacinia purus. Aenean et nisl sit amet felis cursus varius. Nunc vitae laoreet nisl. Cras commodo quis risus vitae gravida. Integer eu enim velit.  Fusce sed ornare diam, sed bibendum enim. Curabitur blandit scelerisque augue, quis tempus ex ultrices non. Donec convallis condimentum purus, euismod rhoncus risus aliquam bibendum. Vivamus nisl nulla, iaculis eu egestas aliquam, laoreet pellentesque lectus. Pellentesque rhoncus dignissim arcu, eget ultricies massa vestibulum a. Integer et blandit neque. Integer congue congue nunc dictum faucibus. Fusce eu varius lacus. In lacinia convallis ex eget laoreet. Etiam eu convallis arcu, at pretium libero. In quis sem vulputate, sollicitudin massa quis, finibus sapien. In semper nulla vel massa dapibus, sit amet sagittis lacus pretium. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Ut tincidunt diam tellus, ut mollis sem pulvinar nec.  Nam fermentum mi lacus, nec laoreet elit finibus non. Nulla in pulvinar leo. Mauris et eros fringilla quam dictum consectetur. Phasellus id gravida odio. Quisque egestas condimentum lobortis. Ut scelerisque at odio at vulputate. Pellentesque dolor ipsum, ornare non ultrices vitae, bibendum et mi. Nullam dolor magna, pretium at mauris vitae, rhoncus varius mauris. Nulla in tellus sit amet enim tempor faucibus vitae vitae nisi. Donec ut sollicitudin turpis. Donec quis quam tellus. Duis cursus metus leo, ac suscipit ex luctus in. Fusce semper, purus et volutpat sollicitudin, enim sem hendrerit metus, ac ultrices diam justo nec velit. Morbi aliquet sollicitudin augue.  Aliquam tincidunt ullamcorper nibh, sit amet dapibus nulla egestas vitae. Sed at tempus velit. Maecenas sed nisi finibus, hendrerit ipsum in, pretium quam. In nec erat eleifend erat imperdiet aliquam in id urna. Vivamus sollicitudin elementum sapien eu fringilla. Aenean eget erat dignissim, elementum orci a, tempus nulla. Aliquam dapibus id lacus eget rhoncus. Donec eget odio nisl. Quisque malesuada lacus orci, et facilisis odio aliquam non. Morbi fringilla nisi non mattis lobortis. Aliquam quis rutrum nisl. Proin id lacus quis neque venenatis vehicula. Donec nec pharetra libero. Proin elementum in nisi et dictum. Cras porta ultricies dolor, ac scelerisque libero posuere ac.  Morbi nunc ipsum, tempus eu enim eu, porttitor imperdiet ante. Curabitur turpis augue, laoreet at condimentum vitae, consectetur nec lacus. Phasellus hendrerit nisl odio, non ultrices turpis tempus id. Suspendisse blandit egestas urna ac efficitur. Maecenas feugiat congue ultricies. Nunc eu erat vel risus faucibus vehicula. Suspendisse potenti. Aliquam erat volutpat. Curabitur interdum, urna eu sagittis condimentum, turpis ipsum viverra nulla, eu sodales nunc arcu sit amet justo. In nunc velit, interdum ac metus non, lobortis fermentum dolor. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Praesent dictum rutrum libero, nec hendrerit lacus ultrices sit amet. Nulla varius nibh commodo tellus tincidunt viverra. Aenean quis malesuada nisi.  Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Pellentesque elementum nec libero in aliquet. Duis sed pretium purus. Quisque finibus consectetur eros et elementum. Suspendisse auctor fermentum magna a maximus. Donec dignissim tristique lectus, vel ultricies eros sollicitudin finibus. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Ut ex sem, condimentum nec tempus vitae, scelerisque nec mauris. Integer elit arcu, bibendum a libero rutrum, convallis malesuada risus. Aliquam nec nisl dapibus, auctor enim non, euismod urna. Nam vitae fringilla augue, at iaculis metus. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Nulla convallis commodo orci, nec iaculis nibh iaculis id. Donec euismod euismod mi. Nullam finibus turpis mi, eget elementum nisi ullamcorper non.  Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Cras imperdiet leo turpis, eu mollis mi vehicula sed. Morbi eu vehicula elit, eu luctus elit. Aliquam odio arcu, mollis id lacus non, blandit tincidunt sapien. Ut maximus ex lorem, vitae condimentum nunc accumsan in. Nam nec nibh neque. Aliquam ut elementum purus. Integer ultrices tempor purus vel dapibus. Duis porta ligula justo, vitae suscipit ipsum maximus eget. Etiam volutpat lacinia dolor eleifend malesuada. Suspendisse commodo dignissim mi, lacinia venenatis ex sollicitudin vitae. In ac dictum turpis. Pellentesque hendrerit interdum sagittis. Nunc vel leo dolor. In commodo tempor mauris, ac sodales est gravida vitae.  Suspendisse faucibus ex id erat blandit porta. Praesent dignissim ac magna eget scelerisque. In at est dui. Nam vitae mauris sit amet arcu congue efficitur nec non ex. Etiam vehicula, arcu a lacinia imperdiet, augue lectus maximus eros, ut scelerisque ex libero a turpis. In ac lectus orci. Proin gravida lobortis nibh, at sodales odio iaculis quis. Cras sodales arcu nec pretium mattis. Fusce in lacus ornare, ultricies ex ut, tincidunt dui. Integer pellentesque, turpis sit amet tincidunt aliquam, odio nisl pharetra nunc, vehicula hendrerit metus nulla nec ante. Pellentesque laoreet ipsum et lectus ultrices, ut tempor nibh lacinia.  Proin eleifend accumsan dolor finibus eleifend. Nunc in justo turpis. Nullam sed justo aliquam, tristique lectus at, varius mi. Etiam in mattis eros, nec convallis diam. Donec et pharetra tellus. Donec rhoncus sollicitudin mauris at tincidunt. Mauris dignissim posuere risus, id scelerisque quam dignissim a. Quisque lorem purus, efficitur et quam quis, semper vestibulum enim. Sed porttitor, risus at tristique egestas, nisi massa placerat turpis, id facilisis elit massa sed nisl. Etiam et mollis ante. Curabitur condimentum, ante suscipit pellentesque dignissim, tortor eros volutpat quam, vel molestie nunc orci id dolor. Quisque bibendum erat vel neque vulputate egestas ut nec justo. Suspendisse potenti.  Pellentesque condimentum iaculis diam, nec venenatis elit luctus in. Nunc luctus mauris mi, id iaculis leo accumsan id. Nullam eget consectetur sapien. Etiam auctor, quam eget pellentesque dapibus, nisi lectus tempor nisl, ut vehicula arcu purus ac odio. Aenean tortor metus, rhoncus vitae iaculis vitae, feugiat eget quam. Cras luctus feugiat pellentesque. Quisque eget tellus lorem. Ut condimentum ante nibh. Duis non bibendum turpis. Aenean rhoncus sagittis efficitur. In tristique nec dolor non molestie. Curabitur nibh tortor, sagittis vitae consectetur eget, sollicitudin cursus justo. In aliquam eu lacus a rutrum.  Suspendisse aliquam quam mi, ac facilisis ligula tincidunt sit amet. Vestibulum congue magna vel ligula mattis elementum ut quis nulla. Cras mattis condimentum ante, ac imperdiet justo finibus eu. Vestibulum pellentesque nibh eu ipsum posuere ultricies. Praesent molestie fringilla turpis, quis faucibus mi facilisis at. Aenean eu ultricies dui. Phasellus a iaculis felis. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; In sit amet lacus magna. Nulla lorem leo, rutrum et tellus aliquam, ultrices ultrices justo.  Pellentesque finibus nunc et neque facilisis, et aliquet risus placerat. Cras quis dolor non nunc faucibus maximus. Ut non consequat mauris, ut eleifend libero. Morbi fringilla lorem sed elit posuere fermentum. Maecenas molestie, ex vel bibendum tincidunt, sem ante tempor lorem, eget maximus mi felis eget dolor. Pellentesque vestibulum magna id sollicitudin malesuada. Sed a metus eu est facilisis laoreet nec eu metus.  Pellentesque congue eu mi eget dignissim. Fusce fringilla egestas ligula ac vulputate. Duis finibus, nisl vel elementum condimentum, tortor urna luctus urna, quis vehicula risus arcu accumsan lectus. Integer odio velit, eleifend ac feugiat vitae, blandit eu felis. Maecenas eget tellus eu tortor sollicitudin venenatis. Aliquam in est a magna finibus dignissim. Etiam malesuada, est nec tristique semper, turpis lectus faucibus ante, tempus auctor lorem massa vel nulla. Suspendisse potenti. Nulla et tincidunt tortor, vel scelerisque tellus. In eget nisi et leo luctus porta et iaculis lectus. Maecenas vel nulla fringilla, tincidunt lorem in, lobortis nisi. Interdum et malesuada fames ac ante ipsum primis in faucibus. Praesent eget vulputate enim. Vivamus accumsan nulla tortor, ac suscipit libero consequat quis.  Sed vitae lobortis metus, eu condimentum neque. Donec in congue massa, non eleifend elit. Mauris a purus et arcu gravida congue. Ut vehicula nibh erat, ut varius neque elementum quis. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. In quis ante in metus ullamcorper cursus id a odio. Praesent lobortis fermentum odio non pellentesque. Sed faucibus malesuada nulla id pretium. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Vestibulum sit amet felis non sem molestie gravida. Pellentesque eget euismod enim.  Curabitur lobortis ex ante, ut faucibus quam placerat pulvinar. Curabitur efficitur odio non risus lacinia, nec sodales augue eleifend. Aliquam a gravida massa, eget rhoncus quam. Duis elementum velit id velit vulputate dignissim. In aliquam dui diam, id blandit massa auctor in. Ut tempor et nisl a hendrerit. Aenean sed finibus enim. Nullam tempor nisi id felis porttitor sodales. Proin a ipsum in magna commodo tincidunt ac a ligula. Aenean rutrum sem eros, imperdiet condimentum urna sagittis non. Vivamus ante enim, ultrices non porta vitae, malesuada ac felis.  Mauris aliquam pellentesque enim nec accumsan. Nullam sem lectus, scelerisque ut posuere a, maximus eu nibh. Praesent rhoncus scelerisque sapien, sit amet faucibus nisi dictum in. Phasellus sed ligula eu ante finibus tempor. Suspendisse vitae lacinia metus, dapibus rhoncus nibh. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Proin sodales sodales libero, a sollicitudin justo varius vel. Vivamus sit amet metus blandit, varius velit eget, interdum nunc. Duis nec consectetur est, ac sagittis lorem. Donec sit amet neque arcu. Maecenas quis enim lorem.  Cras velit turpis, vulputate quis tincidunt a, tincidunt non nibh. Etiam placerat nulla quis lorem consequat luctus. In elementum sed dolor a condimentum. Nulla sollicitudin est est, quis consequat quam euismod feugiat. Suspendisse in ligula congue nisl lacinia pretium. Etiam in ante vel quam bibendum iaculis eu et lorem. Duis ipsum leo, tincidunt in ipsum ut, malesuada efficitur lectus. Aenean convallis malesuada tellus, eu lobortis risus consequat eu. Donec et tortor interdum, rutrum leo et, iaculis felis. Phasellus hendrerit rhoncus nulla nec hendrerit. Curabitur auctor leo magna, in convallis urna finibus et. Sed non libero mattis erat placerat molestie. Ut nec consectetur turpis, vitae euismod lacus. Nunc sodales ultrices dui, quis luctus odio fringilla sed.  Nulla facilisi. Maecenas a semper sapien. Cras ornare enim in interdum convallis. Curabitur a efficitur libero. Ut commodo felis eget blandit tincidunt. Maecenas tempor nec sapien nec vestibulum. Nam condimentum est quis consequat consectetur. Mauris ut viverra ipsum, vitae fermentum metus. Maecenas metus lectus, gravida non enim id, ultricies congue sem. Curabitur consequat commodo mauris, ut condimentum eros dictum in. Nulla vitae tempor erat, sit amet tristique ante. Cras faucibus fermentum nisi, a mattis sem auctor in.  Integer vitae facilisis massa, ac euismod tellus. Integer sit amet justo nunc. Aliquam at massa gravida, pretium turpis vitae, lacinia enim. Praesent mattis et nunc vitae vulputate. Pellentesque aliquam ac risus nec egestas. Donec egestas volutpat nibh et eleifend. Donec accumsan imperdiet maximus. Etiam tellus quam, interdum quis pulvinar in, sollicitudin quis erat.  Fusce posuere nisi posuere justo mattis fringilla. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Donec sed lectus justo. Sed sagittis posuere velit ut semper. Aenean vitae mollis mauris, eu placerat lectus. Quisque non congue mauris. Vivamus metus lorem, semper suscipit enim ac, congue venenatis lorem. Pellentesque porttitor, lorem a convallis accumsan, risus ex pulvinar leo, laoreet tempor mi lacus in nisi. Sed ut turpis vulputate, ultricies lacus id, faucibus dui. Donec et justo quis neque tincidunt porta. Nulla facilisi.  Etiam molestie ligula ut fringilla convallis. Suspendisse molestie egestas metus, non semper arcu mollis non. Nunc non semper est, gravida fringilla nunc. Vivamus in felis turpis. Sed imperdiet euismod pretium. Mauris hendrerit feugiat augue, nec porttitor orci rutrum ut. Fusce eu ligula felis. Integer quis tincidunt ante. Quisque at tristique neque. Vestibulum dictum dui vel massa aliquet finibus. Duis ut consequat ante, ac finibus velit. Donec egestas urna a semper iaculis.  Nunc euismod nulla in facilisis tincidunt. Cras sapien nunc, mattis vitae convallis quis, tristique sed nunc. Nullam sodales gravida libero. Nam vitae mauris a augue feugiat tristique. Praesent justo mauris, sagittis vel ultricies ac, pellentesque ac ex. Donec at eros at purus ultrices tincidunt. Maecenas semper tortor id sapien tristique, sit amet convallis arcu pulvinar. Praesent aliquet sem neque, sed consectetur ipsum ultrices at. Aliquam et quam nec turpis auctor cursus suscipit sit amet orci. Vivamus eros lectus, egestas ac leo vitae, tristique feugiat eros. Donec in molestie sem. Mauris sed interdum orci, id vulputate augue.  Phasellus at eros sapien. Etiam consectetur, nulla ut vulputate tempor, sem leo malesuada est, eu scelerisque urna ipsum et arcu. Sed pretium ipsum id odio tempor, eu ultricies mi semper. Integer tempor lorem at ante maximus, pharetra ultricies ex rhoncus. Morbi eu libero suscipit, elementum nunc ac, sodales massa. Pellentesque at tincidunt enim, sit amet suscipit orci. Etiam vestibulum sapien in felis facilisis, id pulvinar turpis ultrices. Sed ac tortor vel nisl eleifend imperdiet. Mauris ornare turpis vitae tortor malesuada, ac dapibus ex volutpat. In sit amet porta quam. Nulla aliquet est non tortor tempus accumsan. Integer quis erat eget sem tincidunt viverra. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. In congue mauris sed orci iaculis, vel imperdiet neque lacinia.  Aliquam venenatis eros ipsum, vel vestibulum leo fringilla nec. Nulla facilisi. Quisque quis bibendum nisi. Curabitur lacinia in eros in pharetra. Quisque in arcu at nunc venenatis consequat eu eu velit. Etiam dignissim mattis interdum. Ut mauris erat, finibus at placerat non, gravida eu ante. Nulla sed felis auctor, lacinia libero ac, placerat felis. Fusce ut massa convallis, convallis neque non, aliquam lectus. Pellentesque pretium commodo aliquam. Cras malesuada sapien sit amet felis pellentesque, vel suscipit lacus tempor. Nunc faucibus massa quis magna eleifend, vel eleifend eros eleifend. Nullam ornare augue tellus, quis bibendum nisl faucibus ac. Vivamus ut efficitur magna. Mauris interdum cursus scelerisque.  Donec malesuada pretium ex, at posuere odio semper ut. Curabitur vitae magna efficitur nisi porta rhoncus sit amet vel tellus. Quisque ultricies magna et lacinia dictum. Duis nec nibh ac sem semper venenatis vel ut leo. Nunc vulputate id dui in pellentesque. Nam non velit placerat, pulvinar massa in, lacinia quam. Aliquam malesuada vehicula nulla, in lobortis ante porta id. In rutrum orci eu fermentum consectetur. Nullam sit amet dolor in enim vulputate posuere. Etiam vehicula lobortis luctus. Quisque condimentum ex id elit accumsan, vitae mollis felis convallis.  Aenean molestie convallis tincidunt. Nullam iaculis nulla at ex congue suscipit. Fusce egestas turpis molestie lorem feugiat, eu fringilla turpis interdum. Sed id justo porta, facilisis velit interdum, accumsan magna. Maecenas nisl elit, eleifend in metus nec, tincidunt ultricies nibh. Phasellus tincidunt, neque a suscipit lacinia, purus nibh tempus sapien, non egestas enim turpis a felis. Donec imperdiet fermentum mauris at rhoncus. Sed ac massa eu ipsum imperdiet dignissim ac in felis.  In nulla magna, pulvinar nec elementum eu, consequat non dolor. Pellentesque sit amet risus vitae risus interdum lacinia eget fermentum lectus. Mauris sit amet tortor convallis eros luctus cursus. Vivamus posuere id justo ut fringilla. Quisque id faucibus nisl, ut gravida turpis. Nulla facilisi. Donec sit amet dui commodo, scelerisque ex at, venenatis mauris. Sed fermentum in enim lobortis efficitur. Nam vel interdum purus.  Integer quis lacus tristique, lobortis velit suscipit, tempor ante. Phasellus sit amet felis vitae purus sagittis viverra. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Curabitur sollicitudin placerat mauris, a pharetra est luctus sed. Nullam at lacus neque. Maecenas suscipit nulla sed interdum dignissim. Curabitur tempus lorem vitae commodo hendrerit. Nullam a sapien pharetra, elementum felis id, consectetur urna. In lobortis nisi egestas ipsum sodales, ac placerat arcu molestie. Cras id elementum mauris. Ut laoreet sapien vitae porta maximus. Vestibulum nulla urna, lobortis a velit nec, euismod tempor tortor. Fusce non nunc lectus. Integer posuere ex sit amet lacus molestie, quis ultrices metus convallis. Sed aliquet libero turpis, id rhoncus purus pellentesque eu. In fringilla dui posuere, posuere ipsum vitae, eleifend quam.  Aliquam ut maximus neque. Praesent sagittis diam nec orci volutpat scelerisque. Sed aliquam vestibulum eros, sed porttitor tellus eleifend bibendum. Nullam vulputate tincidunt urna ac consequat. Vivamus auctor commodo diam, nec elementum ex pretium ac. Fusce nunc mi, maximus quis est eu, dignissim interdum lectus. Etiam a orci at libero luctus vestibulum sed at libero. Proin mollis congue congue. Curabitur metus turpis, volutpat sit amet lacus ut, tincidunt feugiat mauris. Maecenas elit odio, placerat id nisi nec, congue ullamcorper lorem. Mauris id tortor varius, accumsan ligula nec, commodo sapien. Nam fringilla enim id tellus rutrum facilisis. Donec ex purus, faucibus in libero a, ullamcorper pharetra elit. Etiam tincidunt eget quam non sodales. Donec leo orci, porta finibus facilisis quis, iaculis nec augue. Curabitur purus velit, lacinia eget odio eget, porttitor ullamcorper metus.  Sed elementum arcu ac augue porta ornare. Suspendisse odio dolor, lobortis quis luctus mollis, consectetur vitae magna. Praesent pharetra blandit sapien, non viverra enim volutpat in. In scelerisque ultricies felis, eget fermentum leo volutpat et. Vestibulum et lobortis est, at venenatis risus. Cras ut rhoncus libero, ac condimentum tellus. Maecenas hendrerit nisi ac risus interdum, eu porta tellus dignissim. Morbi cursus lorem a ligula lacinia euismod. Duis in libero magna. Suspendisse aliquam erat a nunc interdum semper. Sed laoreet nunc ac mauris auctor ultrices. Duis eget commodo turpis, sed euismod est. Nulla sed commodo dolor, at imperdiet eros. Mauris hendrerit elementum neque in interdum. Suspendisse sapien quam, imperdiet vitae eros vel, ullamcorper tristique ligula.  Integer accumsan non tellus eu elementum. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Ut quis luctus ipsum, sit amet mollis nisi. Aliquam condimentum, magna ut euismod lacinia, ex ex cursus ligula, et porta est est convallis lorem. Integer aliquet sed nibh nec rutrum. Aliquam ut urna sed orci porttitor rhoncus. In ex nulla, porttitor non sollicitudin gravida, pulvinar at erat. Proin sit amet varius magna.  Proin sagittis ante turpis, vitae pharetra erat varius eget. Suspendisse nunc nisl, mattis at purus eu, malesuada gravida orci. Curabitur facilisis mauris in ipsum imperdiet, eleifend rhoncus augue fermentum. Nulla ornare feugiat semper. Vestibulum eu sem tempor, congue nibh at, egestas magna. In facilisis varius cursus. Mauris placerat nisl sed sagittis ultricies. Nam facilisis turpis felis, sed aliquet lacus mattis sit amet. In ut rutrum mauris. Ut quis molestie massa. In ornare, nisl a placerat convallis, nunc augue suscipit magna, ut vestibulum magna nisi tempor nunc. Integer rhoncus erat non bibendum mollis. Suspendisse viverra diam at nisi consectetur, eu consequat augue aliquam.  Donec nec ipsum tincidunt, molestie orci non, lacinia massa. Vivamus consequat posuere maximus. Vivamus viverra massa id lacus finibus dapibus. Etiam ornare varius lacus, eu suscipit libero dictum vel. Praesent sed nisi a sem sodales pretium eu vitae tellus. Maecenas sit amet augue non ante malesuada maximus. Proin nisl magna, interdum a dignissim dignissim, consectetur nec erat. Integer facilisis orci diam, nec vulputate mauris pharetra porta. Ut tempor leo urna, quis commodo ipsum fermentum sed. Morbi sed ipsum volutpat, tempor orci non, ornare erat. Etiam pulvinar ornare fermentum. Cras a molestie nunc. Donec interdum neque in felis egestas feugiat.  Morbi elementum sem at sem sodales finibus. Vivamus accumsan ultrices mauris sed condimentum. Fusce elit quam, molestie quis nisi vel, lacinia porttitor justo. Curabitur massa tellus, dictum sed pretium vel, mollis sit amet purus. Mauris vitae tincidunt nisi. Suspendisse potenti. Praesent imperdiet, augue id volutpat tristique, dolor ante vulputate diam, id viverra lacus lectus tincidunt urna. Suspendisse nulla nisi, semper ac ultricies at, iaculis id magna. Sed ut ante ut nisi tristique eleifend.  Nulla arcu risus, sollicitudin nec orci ut, fermentum dictum odio. Nam luctus malesuada metus, eu gravida velit pulvinar ut. Ut molestie ipsum ante, et rhoncus ex auctor eu. Pellentesque in mauris sit amet ligula commodo hendrerit. Aliquam in tortor risus. Quisque porta mollis ipsum et commodo. Phasellus elit ligula, consequat at dapibus eu, porttitor vitae lectus. Vestibulum quis sagittis elit, sed dapibus tortor. Phasellus feugiat in ligula vel tempus. Suspendisse quis congue odio, eget elementum massa. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.  Morbi lacinia finibus metus, sed mollis ipsum aliquet id. Nulla ut dolor neque. Curabitur vel viverra ante. Nunc at dictum quam. Donec facilisis purus dolor, a lobortis libero maximus at. Phasellus pulvinar dolor in nulla sagittis tincidunt. Etiam nec nulla vestibulum, facilisis nunc sit amet, posuere tortor. Proin hendrerit urna dolor, quis cursus quam lobortis in. Fusce interdum molestie lectus, a porta risus. Etiam enim nisl, varius id sodales at, scelerisque in ligula. Nullam placerat turpis mi, at vehicula nunc blandit vel. Aenean venenatis commodo est ut elementum. Ut pulvinar ut ipsum sed tincidunt. Duis nec est et lectus semper gravida ac lobortis lorem. Aenean ut mauris risus. Nunc nulla nunc, rutrum quis dapibus eget, suscipit nec neque.  Etiam erat ante, vehicula nec dignissim lacinia, semper et enim. Quisque id consectetur neque, et molestie massa. Quisque nunc velit, mattis ac tincidunt nec, cursus eu libero. Curabitur lobortis finibus purus, nec luctus lacus fringilla sed. Suspendisse laoreet nulla rutrum lacus accumsan congue. Nunc interdum dolor et aliquam semper. Integer varius scelerisque mauris at suscipit. Sed pharetra eros a aliquam laoreet. Ut lacinia suscipit ligula. Pellentesque ipsum lacus, tempus sit amet tempus vulputate, tristique et nisi. Cras dapibus turpis velit, nec vehicula arcu ullamcorper sed. Sed mollis mollis fermentum. Quisque eget rhoncus eros. Vivamus at aliquet nunc.  Integer vitae dapibus orci. In id imperdiet lectus, in rhoncus diam. Etiam risus felis, elementum et mi at, laoreet suscipit nisl. Duis vehicula imperdiet bibendum. Duis rutrum ante congue augue imperdiet, eu congue justo pellentesque. Curabitur pretium sollicitudin sapien in finibus. Aliquam leo nulla, hendrerit in eros ut, blandit elementum ligula.  Vestibulum dictum aliquam urna, nec pellentesque elit venenatis eget. Vivamus non enim a nibh pulvinar cursus. Integer tincidunt eros et neque tristique scelerisque. Nulla eget congue ante. Sed non augue sit amet lectus commodo varius eu id orci. Etiam sed tempor metus. Proin accumsan sodales urna, at tincidunt eros vehicula eleifend. Vivamus pellentesque risus et ante aliquet, ut eleifend nibh ornare. Maecenas lacinia pellentesque lectus id posuere. Maecenas vehicula pharetra purus, sit amet ultrices nulla blandit at. Nunc nisi nunc, sollicitudin a faucibus id, viverra bibendum lorem.  Vestibulum at viverra leo. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean arcu dolor, vehicula eu gravida quis, gravida id eros. Maecenas egestas pulvinar arcu, eu tincidunt mauris. Duis rutrum turpis a tellus iaculis, sit amet laoreet magna imperdiet. Mauris mattis malesuada elit, et bibendum tellus. Suspendisse commodo elit sit amet libero molestie, vel egestas enim lobortis. Vivamus at lorem ex. Morbi tincidunt lobortis nunc vel pretium. Sed massa diam, maximus eget nulla id, feugiat molestie enim. Cras malesuada ac metus commodo blandit. Sed elementum risus id dui vehicula, pretium aliquam magna efficitur. Phasellus erat urna, posuere quis diam et, semper cursus quam.  Ut id massa dignissim, fringilla justo at, posuere odio. Aliquam convallis cursus turpis. Vivamus at nibh ac est gravida interdum a nec tellus. Curabitur sed tellus at enim sagittis vehicula et a ligula. Nulla facilisi. Praesent ornare tellus ac felis lobortis aliquet. Aliquam erat volutpat. Fusce dictum vestibulum sagittis. Nullam egestas mauris ipsum. Sed vitae sapien lorem. Maecenas sagittis gravida augue, et mattis enim lacinia id. Proin tincidunt quam a luctus eleifend. Donec eu varius justo, sed dictum ipsum. Vivamus sed elit enim. Vestibulum consectetur viverra metus, in euismod elit malesuada a. Duis massa augue, ullamcorper vel ipsum ut, imperdiet facilisis nulla.  Nulla rutrum magna eu suscipit cursus. Phasellus quis dapibus leo. Nulla diam leo, sollicitudin ac cursus sed, tristique nec nunc. Vivamus et fermentum ex. Praesent ac purus velit. Suspendisse a aliquet turpis. Sed mollis sagittis justo, a facilisis eros fermentum ac. Nulla posuere odio vel diam iaculis tincidunt. Vestibulum suscipit fringilla vehicula.  Donec sem sem, varius at iaculis sit amet, vulputate eu sapien. Maecenas dui velit, feugiat quis enim quis, vehicula posuere libero. Cras viverra neque nibh, in mollis diam mollis sit amet. Suspendisse pulvinar sem sed blandit euismod. Ut molestie, turpis nec volutpat dictum, turpis tellus mollis turpis, vitae semper dolor dolor at nulla. Integer vel elit nec enim porttitor blandit. Maecenas id lectus eu mi convallis finibus.  Morbi vel libero imperdiet, aliquet risus posuere, lacinia massa. Etiam nec lacus ac elit commodo vulputate quis a erat. Morbi odio massa, condimentum in luctus et, suscipit id lacus. Phasellus porttitor est sed orci tempus, id finibus libero pharetra. Morbi sodales tellus turpis, quis posuere nibh convallis sed. Proin at ligula et eros lobortis vestibulum. Nullam lacinia auctor elit in mollis. Maecenas tempor, neque eu lacinia vestibulum, mauris quam tristique eros, tempus varius risus lorem ac lacus. Interdum et malesuada fames ac ante ipsum primis in faucibus. Curabitur bibendum elit ut dui consectetur tincidunt. Praesent non metus iaculis ipsum congue aliquet vitae sit amet urna. Praesent quam mi, sagittis eu nisl rhoncus, tincidunt fringilla nibh. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae;  Fusce in est eget enim rutrum convallis ac nec enim. Nulla et iaculis metus. Nullam non finibus magna. Suspendisse consectetur commodo ipsum. Cras sagittis suscipit justo, eget laoreet lacus scelerisque at. Donec et turpis id arcu pellentesque commodo at ut risus. Cras mollis posuere feugiat. Aliquam tellus risus, molestie finibus ante ac, mattis mattis lectus. Etiam diam orci, finibus in sapien at, mollis tristique lacus. Praesent a neque consequat est pretium fermentum. Suspendisse scelerisque, ex ac scelerisque sodales, quam odio tincidunt enim, cursus pellentesque ex nibh in erat. In turpis ligula, tempus non sagittis eget, tempor nec mauris. Vivamus risus arcu, imperdiet vel augue in, gravida lobortis sem. Proin eu velit tincidunt, scelerisque erat ac, mattis sapien. Quisque eleifend ut est ut auctor.  Sed a nisi tempus, pharetra massa vitae, pulvinar odio. Aenean tincidunt urna ac neque accumsan accumsan. Aliquam dui enim, eleifend ac nibh id, pellentesque pellentesque urna. Maecenas et massa dui. Aenean hendrerit eu augue at hendrerit. Aliquam interdum risus nec facilisis dictum. Cras sit amet varius diam. Aenean fringilla imperdiet odio, nec placerat purus consectetur in. Sed rhoncus magna ac metus consectetur, a posuere orci porttitor.  Ut consectetur accumsan orci, at pulvinar elit pulvinar ac. Vestibulum sed tortor semper odio ornare viverra. Donec tempor, ipsum non imperdiet fermentum, quam ipsum vestibulum eros, placerat rhoncus est enim vel urna. Aliquam auctor mi lacus, vitae feugiat odio rhoncus ut. Pellentesque blandit vehicula suscipit. In ac volutpat mauris. Vivamus facilisis tellus quis euismod vehicula. Proin egestas egestas velit at aliquam. Nullam nisi leo, maximus a convallis at, consequat sed diam. Fusce sed gravida massa. Praesent sagittis tincidunt metus.  Mauris facilisis in arcu nec rhoncus. Nunc malesuada aliquet ipsum non consectetur. Suspendisse ut massa quis tortor egestas porttitor. Donec malesuada leo eu pellentesque sagittis. Suspendisse tellus dui, congue condimentum sem ac, vestibulum ullamcorper tellus. Mauris molestie massa vitae diam egestas, eget vestibulum metus consectetur. Sed aliquet lorem cursus pellentesque lobortis.  Nulla facilisi. Sed rutrum egestas odio, a varius libero. In hac habitasse platea dictumst. Sed tortor justo, scelerisque in orci a, facilisis euismod quam. Etiam eu turpis vitae lorem ultrices tincidunt vitae vitae massa. Proin placerat sit amet tellus nec suscipit. Nulla ultrices efficitur felis eu faucibus. Pellentesque porta finibus diam, non venenatis justo fermentum nec. Ut aliquam finibus magna nec maximus. Vestibulum ut augue nec nisi volutpat efficitur. Pellentesque sit amet vehicula mi, in rutrum libero. Mauris non neque et nisl varius efficitur. Vestibulum vitae nunc nulla. Morbi vitae tortor orci. Suspendisse ut auctor risus. Quisque imperdiet urna vitae leo finibus lobortis.  Pellentesque aliquam efficitur nisi, nec dignissim quam scelerisque at. Nulla pulvinar neque in faucibus fermentum. Praesent malesuada convallis tellus finibus pulvinar. Nunc ultricies risus quam, porta sagittis nisl accumsan et. Nunc maximus tortor nec tellus vulputate vehicula. Donec rutrum mauris ut massa elementum venenatis. Sed facilisis placerat dui, sed pulvinar diam porta ut. Cras tempus ex nec ligula suscipit eleifend. Etiam id elementum ipsum. Nam maximus suscipit quam, at sollicitudin est euismod sit amet.  Phasellus semper in massa sit amet hendrerit. Ut et odio rutrum, varius felis ac, euismod ligula. Mauris ut urna tristique, tristique magna at, sodales velit. Sed at congue nulla. Quisque ut suscipit lectus, at fringilla justo. Praesent eget lacinia tortor. Mauris venenatis sit amet lorem in dapibus. Morbi tincidunt, lacus et volutpat mollis, ipsum urna laoreet magna, sed accumsan lacus sem eget leo. Pellentesque dictum est et accumsan tincidunt. Nam scelerisque rhoncus lobortis.  Donec eget maximus ante, quis accumsan justo. Aliquam finibus, velit et tincidunt mollis, eros velit sollicitudin lorem, mattis rutrum est leo et velit. Donec sagittis justo a pulvinar sodales. In rhoncus viverra magna, egestas pharetra urna vulputate in. Phasellus pretium eros id semper imperdiet. Fusce vestibulum sem at diam bibendum, eu tincidunt lorem dapibus. Aenean quis orci tellus. Donec eget leo faucibus, dignissim ligula quis, egestas ipsum.  Praesent rutrum tortor non nibh vestibulum, in faucibus risus convallis. Curabitur commodo mi diam, sit amet euismod risus imperdiet nec. Pellentesque quis dictum quam, eget luctus enim. Duis ac lorem sed quam sodales condimentum. Suspendisse et cursus ipsum, sed vestibulum leo. Vivamus in felis imperdiet, sagittis arcu non, sagittis risus. Vestibulum tristique ipsum sit amet posuere auctor. Pellentesque tempus massa non augue sagittis posuere.  Cras purus risus, vestibulum id odio in, finibus tempus dui. Pellentesque mollis vulputate turpis. Mauris eu maximus urna, eu viverra neque. Fusce tincidunt mauris mauris, at commodo sapien commodo vitae. In nisi ipsum, vulputate et quam non, elementum vehicula neque. Pellentesque condimentum dignissim magna id vehicula. Suspendisse in mauris vitae diam tristique dapibus. Curabitur gravida commodo sapien, ac porta elit dictum ut. Sed quis sagittis odio. Nunc elit nisl, hendrerit nec nulla ac, vulputate gravida risus. Maecenas nec leo eget urna cursus eleifend a at odio. Duis posuere nisi eget magna pulvinar, a maximus tellus finibus. Sed elementum nunc nisi, sed dignissim nunc commodo id. Cras facilisis lectus nec felis tempus, eu pharetra neque aliquet.  Maecenas luctus sem eu purus fermentum, sit amet tempus nunc finibus. Sed laoreet vel quam at finibus. Duis tortor arcu, cursus quis facilisis id, semper ut dui. Vestibulum pretium congue dolor, et consequat elit sagittis ut. Cras ac aliquam risus. Mauris in interdum risus. In ac posuere massa. Phasellus non blandit dolor, nec scelerisque ligula.  Cras id metus condimentum, aliquam justo non, pulvinar augue. Fusce tortor mauris, fermentum vel lectus quis, iaculis viverra urna. Curabitur sed lorem in libero tristique porta. Etiam efficitur dolor ac odio tempor, sit amet gravida felis vulputate. Mauris ut tortor vitae ipsum sollicitudin tincidunt vel vitae nisl. Duis fringilla eleifend urna, sed faucibus felis feugiat eu. Vestibulum venenatis malesuada ex, sit amet rutrum orci viverra at. Suspendisse potenti. Ut orci nisl, consequat ac neque quis, tempus lobortis lacus. Aliquam erat volutpat. Nam at viverra ante, sed tristique arcu. Quisque ut efficitur arcu. Etiam hendrerit vel turpis ut laoreet. Vivamus viverra magna at diam pharetra pharetra. Fusce non molestie libero, eget gravida sapien.  Duis porttitor bibendum congue. Sed vestibulum ante ac leo auctor malesuada. Fusce nisl nisi, molestie eget nisl at, consequat dictum neque. Ut molestie ultrices justo vel aliquet. Integer ornare est sed augue imperdiet laoreet. Etiam tristique tellus et dapibus rhoncus. Integer condimentum commodo tellus, nec aliquet mauris euismod vitae. Vestibulum convallis mi elit. Pellentesque mollis velit orci, a auctor purus suscipit et. Mauris imperdiet, odio a consectetur luctus, velit enim vestibulum mauris, ut scelerisque purus nisi eget metus. Vivamus semper augue sed sapien consectetur, nec faucibus libero maximus. Maecenas vel lacus nunc. Sed purus tortor, elementum at nunc quis, laoreet rhoncus urna. Donec aliquet sit amet risus nec luctus. Phasellus elementum fermentum ante, vitae vulputate sem cursus non. Integer lobortis felis in mi dictum scelerisque.  Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Maecenas blandit hendrerit eros ac suscipit. Pellentesque nec blandit nunc. Etiam magna nisi, luctus non magna quis, condimentum elementum neque. Suspendisse eu faucibus tellus, sed elementum justo. Sed sit amet metus pellentesque, interdum mi ac, scelerisque orci. Morbi lobortis maximus neque. Mauris vel enim vel risus pulvinar aliquam. Quisque tincidunt fringilla dui et luctus. Suspendisse potenti.  Duis auctor ligula rutrum nibh laoreet malesuada. Fusce eros nisl, imperdiet sollicitudin urna in, pulvinar luctus lectus. Praesent ac tortor mauris. Pellentesque a sollicitudin arcu. Phasellus enim justo, finibus ut facilisis sed, laoreet ut ipsum. Vestibulum id iaculis mi, et tempus ante. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Donec dapibus elit sem, ut sagittis lorem ultricies ut. Praesent luctus congue scelerisque. Morbi in dui libero. Phasellus elementum tortor dolor, sed fermentum eros porttitor vel. Curabitur sit amet lobortis dui, in ultrices dui.  Cras accumsan dolor ac neque mattis ornare. Aliquam fringilla ante magna, vitae convallis libero lobortis ut. Nam rutrum ante purus, sit amet facilisis sapien porta nec. Sed maximus erat vitae volutpat lobortis. Praesent ultrices, elit sit amet blandit semper, metus mauris aliquam dolor, vel ullamcorper nulla risus a felis. Quisque sodales, velit vel lobortis pretium, risus nisi vulputate urna, vitae ornare est nunc ut lectus. Vivamus urna odio, egestas id massa in, euismod sagittis dolor. Ut dignissim felis nisi, vel bibendum neque dignissim sed. Sed malesuada sapien nisl, ut pellentesque augue bibendum commodo. Pellentesque sed leo et nisi varius aliquam. Mauris ligula ex, ultricies in blandit vitae, accumsan a mi. Sed pretium pretium quam, sit amet aliquam lorem dapibus in. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Duis ex enim, feugiat ut tortor sollicitudin, auctor efficitur quam. Nunc eros arcu, viverra at lorem at, rhoncus sollicitudin ligula.  Maecenas sem erat, aliquam eget finibus vitae, efficitur sit amet nisl. Donec in felis ornare, scelerisque ipsum sed, rhoncus sem. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Duis aliquam diam nec tellus elementum, nec rutrum odio sollicitudin. Etiam bibendum nisl augue. Morbi sed auctor felis. Pellentesque a magna vitae purus ornare euismod. Etiam facilisis sit amet orci in semper. Nullam suscipit et massa in pharetra.  Nulla facilisi. Curabitur vel ultrices magna, fermentum iaculis elit. Nunc scelerisque nulla elit, vestibulum varius purus pharetra in. Morbi tempor nibh non est porttitor, eu interdum neque tempus. Sed et aliquet dui. Suspendisse ut mauris at mauris elementum pellentesque luctus vitae urna. Proin in blandit tortor. Fusce egestas pretium nisl nec aliquet.  In id molestie orci. Phasellus eget sagittis felis, sed varius neque. Morbi commodo lacus vitae ligula mattis mattis. Morbi et sem quis dolor lacinia faucibus. Vivamus efficitur dignissim elit ac posuere. Duis a ipsum fringilla, rutrum ex sed, placerat dui. Duis nec varius neque, nec fringilla magna. Nulla id efficitur nisl. Ut eget vestibulum velit, at euismod velit. Curabitur facilisis eros quis arcu vehicula, vel egestas tortor mattis. Morbi at lectus sagittis, euismod velit semper, suscipit ipsum. Nulla ullamcorper vel mauris molestie viverra. Quisque aliquet mi ut massa auctor dictum.  Aliquam erat volutpat. Nullam laoreet purus sed ullamcorper fringilla. Sed auctor mollis leo, sed sagittis leo congue in. Aenean ultrices dapibus dapibus. Aenean efficitur, sem id cursus efficitur, libero massa elementum erat, sit amet convallis felis ligula non neque. Curabitur volutpat dolor sed enim accumsan eleifend. Vivamus condimentum porttitor eros. Nullam ac orci non nulla viverra faucibus. Cras id erat lectus. Duis nisi tellus, rhoncus sed metus volutpat, egestas interdum ipsum.  Etiam nisl tortor, malesuada id imperdiet eu, dignissim ut lacus. Aenean faucibus ipsum et nulla efficitur hendrerit. Etiam tempus fringilla elementum. Ut leo dui, ultrices at neque at, interdum vulputate eros. Donec egestas viverra rhoncus. Morbi molestie augue justo. Aenean ac est augue. Pellentesque fringilla eget magna ac lacinia.  Pellentesque pellentesque, odio quis sodales fermentum, magna dui lobortis mauris, at varius magna felis id est. Ut vestibulum volutpat orci vitae posuere. Pellentesque id sapien tempus, laoreet dui vel, consequat libero. Cras tellus enim, ultrices eget leo id, rhoncus faucibus mauris. Curabitur rhoncus dui sed quis.";
    //free(conteudo_arquivo);
    //conteudo_arquivo = teste64;
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
    // faz o trim do caminho pro dir
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

    // se o cluster movido for o mesmo em que o programa estava quando chamou esse comando, eu volto pro cluster pai
    if (isDirectory(*meta) && (arguments->cluster_atual == meta->cluster_inicial))
        arguments->cluster_atual = clus->cluster_pai;

    // invalida esse metafile no dir que estamos agora(estamos no dir pai do arquivo/pasta que queremos mover)
    // só que vvamos fazer isso no final do programa, pq caso o path do segundo argumento seja invalido ou coisa do tipo
    // daí vamo so remover esse cluster daqui e ngm mais vai poder acessar ele
    cluster_para_invalidar = cluster_atual;
    numero_metafile_invalidar = metafile_n;
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
int EXIT_function(Arguments *arguments)
{
    printf("\n Desligando... \n");
    exit(0);
}
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
