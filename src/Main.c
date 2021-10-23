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
#define CD 0
#define DIR 1
#define RM 2
#define MKDIR 3
#define MKFILE 4
#define EDIT 5
#define MOVE 6
#define RENAME 7
#define ROOTNAME "root"
#define MAX_INSTRUCTION_SIZE 1000

//s� os espa�os em branco da esquerda
char* leftTrim(char *args)

{
    int i = 0;
    while(args[i]== ' ')
    {
        args++;
    }
    return args;
}
//tira os espa�oes em branco da direita
char *rightTrim(char *args)
{
    int i = strlen(args)-1;
    while(args[i] == ' ')
    {
        args[i] = '\0';
        i--;
    }
    return args;
}
char* trim(char *args)
{
    args = leftTrim(args);
    return rightTrim(args);
}

int countArguments(char *args)
{
    int i;
    int arg_count = 0;
    //fazer algo pra considerar entre aspas

    //sem considerar aspas:
    i=0;
    while(args[i]!= '\0')
    {
        //passa atraves dos argumentos
        while((args[i]!= ' ') && (args[i]!= '\0'))
            i++;
        //passa pelos espaços em branco entre argumentos
        //não precisa testar '\0' pq args ja vai estar trimmed ent�o n�o existe
        //como ter ' ' seguido de '\0'
        while((args[i]) == ' ')
            i++;
        //conta o primeiro arg
        arg_count++;
    }
    return arg_count;
}

Arguments get_command_and_args(char *instruction_line, Arguments instruction)
{
    char *instruction_line_copy = (char *)malloc(sizeof(char) * strlen(instruction_line) + 1);
    //fazer copia da linha pq strtok modifica ela
    strcpy(instruction_line_copy, instruction_line);
    instruction.num_args = 0u;
    instruction.command_name = strtok(instruction_line_copy, " ");
    //se não há argumentos
   if (instruction.command_name[strlen(instruction.command_name) - 1] == '\n')
        //tira o \n, bota um null no lugar
        instruction.command_name[strlen(instruction.command_name) - 1] = '\0';

    else
    {

        //pega os argumentos, todos juntos tho, com o \n no final
        instruction.args = instruction_line + strlen(instruction.command_name)+1;
        //tirar o \n
        if (instruction.args[strlen(instruction.args) - 1] == '\n')
        //tira o \n, bota um null no lugar
            instruction.args[strlen(instruction.args) - 1] = '\0';
       instruction.args = trim(instruction.args);
       instruction.num_args = countArguments(instruction.args);
        //!fazer uma funcao para contar os argumentos, considerar argumento entre aspas como um s� -> comando editar
    }
    //free(instruction_line_copy);
    return instruction;
}

void emulaCMD()
{
    char *dirName = (char *)malloc((sizeof(char) * strlen(ROOTNAME)) + 1);//para armazenar ROOTNAME
    strcpy(dirName,ROOTNAME);
    char *instruction_line = (char *)malloc((sizeof(char) * MAX_INSTRUCTION_SIZE) + 1) ;
    int i=-1;
    BYTE ok = 1;//se rodou o comando corretamente = 0,senao = 1
    Arguments instruction;
    instruction.cluster_atual = 0x00;

    int ex = 0;
    while (!ex)
    {

          if(ok == 0 && i == CD)
        {
            free(dirName);
            dirName = (char *)malloc((sizeof(char) * strlen(instruction.args)) + 1);
            strcpy(dirName,instruction.args);
        }

        printf("%s",dirName);
        printf(">");
        fgets(instruction_line, MAX_INSTRUCTION_SIZE, stdin);
        //se o primeiro caractere não for enter
        if (instruction_line[0] != 10)
        {
           instruction = get_command_and_args(instruction_line,instruction);

           int p;   ///Pra aceitar letra minuscula tbm.
           for (p=0; instruction.command_name[p] != '\0'; p++)
           {
               instruction.command_name[p] = toupper(instruction.command_name[p]);
           }

            for(i =0; i<NCOMMANDS; i++)
            {



                //se comando digitado tiver na lista de comandos la
                if(!strcmp(commands[i].name,instruction.command_name))
                {
                    instruction.owner = &commands[i];
                    //roda funcao daquele comando
                    ok = (commands[i].func)(&instruction);
                    //a instruction_line_copy da funcao acima vira instruction.command_name na funcao acima
                    //tem que dar free nela.
                    free(instruction.command_name);
                    break;
                }
            }
            //nao achou comando
            if(i==NCOMMANDS)
                printf("INVALID COMMAND: '%s'\n",instruction.command_name);
        }
    }
    free(instruction_line);
}

int main()
{

    //inicializar a estrutura de arquivos de dados
    //emular o cmd, fazer coisas tipo root\dir> <tal comando>
    FileSystem *arq = (FileSystem *)malloc(sizeof(FileSystem));
    inicializaArquivo(arq);
    free(arq);

    emulaCMD();

   // printf("%d\n",sizeof(fileSystem));
   // printf("%d\n",sizeof(directoryFile));
   // printf("%d\n",sizeof(metaFiles));
   // printf("%d\n",sizeof(metaDados));
   // printf("%d\n",sizeof(cluster));
    return 0;
}
