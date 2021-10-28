///             Arquitetura e Organização de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler
///                     (00323741)  Nikolas Padão
///                     (00275960)  Pedro Afonso Tremea Serpa
///                     (00325735)  Ricardo Hermes Dalcin

#include "commands.h"
#include "arquivos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define CD 0
#define DIR 1
#define RM 2
#define MKDIR 3
#define MKFILE 4
#define EDIT 5
#define MOVE 6
#define RENAME 7
#define EXIT 8
#define RESET 9
#define ROOTNAME "root"
#define MAX_INSTRUCTION_SIZE 1000

// s� os espa�os em branco da esquerda
char *leftTrim(char *args)

{
    int i = 0;
    while (args[i] == ' ')
    {
        args++;
    }
    return args;
}
// tira os espa�oes em branco da direita
char *rightTrim(char *args)
{
    int i = strlen(args) - 1;
    while (args[i] == ' ')
    {
        args[i] = '\0';
        i--;
    }
    return args;
}
char *trim(char *args)
{
    args = leftTrim(args);
    return rightTrim(args);
}

int countArguments(char *args)
{
    int i;
    int arg_count = 0;

    char separador;

    i = 0;
    while (args[i] != '\0')
    {
        // passa através dos argumentos
        while ((args[i] != ' ') && (args[i] != '"') && (args[i] != '\0'))
            i++;

        // atribui o separador encontrado
        separador = args[i];

        // se o separador for aspas, procura o fechamento
        if (separador == '"')
        {
            // pula a primeira '"'
            i++;

            // procura a próxima aspa até acabar a string
            while ((args[i] != separador) && (args[i] != '\0'))
                i++;

            // pula a última '"' (se tiver)
            if (args[i] != '\0')
                i++;
        }

        // passa pelos espaços em branco entre argumentos
        // não precisa testar '\0' pq args ja vai estar trimmed entao nao existe
        // como ter ' ' seguido de '\0'
        while ((args[i]) == ' ')
            i++;

        // conta o argumento
        arg_count++;
    }

    return arg_count;
}

Arguments get_command_and_args(char *instruction_line, Arguments instruction)
{
    char *instruction_line_copy = (char *)malloc(sizeof(char) * strlen(instruction_line) + 1);
    // fazer copia da linha pq strtok modifica ela
    strcpy(instruction_line_copy, instruction_line);
    instruction.num_args = 0u;
    instruction.command_name = strtok(instruction_line_copy, " ");
    // se não há argumentos
    if (instruction.command_name[strlen(instruction.command_name) - 1] == '\n')
    {
        // tira o \n, bota um null no lugar
        instruction.command_name[strlen(instruction.command_name) - 1] = '\0';
        // apontar instruction.args pra algum lugar pra nao bugar depois
        instruction.args = instruction_line + strlen(instruction.command_name) + 1;
        // como não ha argumentos vou deixar como "" o argumento pra evitar bugs
        strcpy(instruction.args, "");
    }
    else
    {
        // pega os argumentos, todos juntos tho, com o \n no final
        instruction.args = instruction_line + strlen(instruction.command_name) + 1;
        // tirar o \n
        if (instruction.args[strlen(instruction.args) - 1] == '\n')
            // tira o \n, bota um null no lugar
            instruction.args[strlen(instruction.args) - 1] = '\0';
        instruction.args = trim(instruction.args);
        instruction.num_args = countArguments(instruction.args);
        //! fazer uma funcao para contar os argumentos, considerar argumento entre aspas como um s� -> comando editar
    }
    // free(instruction_line_copy);
    return instruction;
}

void emulaCMD()
{
    char *path = (char *)malloc((sizeof(char) * strlen(ROOTNAME)) + 1); // para armazenar ROOTNAME
    strcpy(path, ROOTNAME); //comeca com root só como caminho
    char *instruction_line = (char *)malloc((sizeof(char) * MAX_INSTRUCTION_SIZE) + 1);
    int i = -1;//numero da operacao
    BYTE ok = 1; // se rodou o comando corretamente = 0,senao = 1
    Arguments instruction;
    instruction.cluster_atual = 0x00;
    while (1)
    {

        // funcoes que potencialmente mudam o nome do caminho no console
        if (ok == 0 && (i == CD || i == RM || i == RENAME || i == MOVE || i == RESET))
        {
            free(path);
            path = (char *)malloc(sizeof(char) * MAX_INSTRUCTION_SIZE);
            getPathFromClusToRoot(instruction.cluster_atual, path);
        }

        printf("%s", path);
        printf(">");
        fgets(instruction_line, MAX_INSTRUCTION_SIZE, stdin);
        // se o primeiro caractere não for enter
        if (instruction_line[0] != 10)
        {
            instruction = get_command_and_args(instruction_line, instruction);

            int p; // Pra aceitar letra minuscula tbm.
            for (p = 0; instruction.command_name[p] != '\0'; p++)
            {
                instruction.command_name[p] = toupper(instruction.command_name[p]);
            }

            for (i = 0; i < NCOMMANDS; i++)
            {
                // se comando digitado tiver na lista de comandos la
                if (!strcmp(commands[i].name, instruction.command_name))
                {
                    instruction.owner = &commands[i];
                    // roda funcao daquele comando
                    ok = (commands[i].func)(&instruction);
                    // a instruction_line_copy da funcao acima vira instruction.command_name na funcao acima
                    // tem que dar free nela.
                    free(instruction.command_name);
                    break;
                }
            }
            // nao achou comando
            if (i == NCOMMANDS)
                printf("INVALID COMMAND: '%s'\n", instruction.command_name);
        }
    }
    free(path);
    free(instruction_line);
}

int main()
{

    FILE *arqDados;
    FileSystem *arq = (FileSystem *)malloc(sizeof(FileSystem));
    if (((arqDados = fopen("arqDados", "rb")) == NULL))
    {
        fclose(arqDados);
        inicializaArquivo(arq);
        free(arq);
    }
    free(arq);
    emulaCMD();

    // printf("%d\n",sizeof(fileSystem));
    // printf("%d\n",sizeof(directoryFile));
    // printf("%d\n",sizeof(metaFiles));
    // printf("%d\n",sizeof(metaDados));
    // printf("%d\n",sizeof(cluster));
    return 0;
}
