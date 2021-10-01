#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROOTNAME "root"
#define MAX_INSTRUCTION_SIZE 1000

Arguments get_command_and_args(char *instruction_line)
{
    char *instruction_line_copy = (char *)malloc(sizeof(char) * strlen(instruction_line));
    char *arg;
    //fazer copia da linha pq strtok modifica ela
    strcpy(instruction_line_copy, instruction_line);
    Arguments instruction;
    instruction.num_args = 0u;
    instruction.command_name = strtok(instruction_line_copy, " ");
    //se não há argumentos
   if (instruction.command_name[strlen(instruction.command_name) - 1] == '\n')
        //tira o \n, bota um null no lugar
        instruction.command_name[strlen(instruction.command_name) - 1] = 0;

    else
    {
        /*ajeitar isso pq no EDIT recebe uma string como argumento
        e ela tem espaçoes entre o mesmo argumento
        Outra coisa - se tiver mais de um espaço entre argumentos ou entre comando-argumento
        o strtok vai pegar um token vazio e vai dar  ruim*/
        arg = strtok(NULL, " ");
        while (arg != NULL)
        {
            arg = strtok(NULL, " ");
            instruction.num_args++;
        }
        //pega toda linha, ia fazer pra pega so argumentos, separando do comando mas to com sono
        instruction.args = instruction_line;
    }
    //free(instruction_line_copy);
    return instruction;
}

void emulaCMD()
{
    char dirName[] = "dir";
    char *instruction_line = (char *)malloc((sizeof(char) * MAX_INSTRUCTION_SIZE) + 1) ;
    int i;
    while (1)
    {
        printf(ROOTNAME);
        printf("/%s>", dirName);
        fgets(instruction_line, MAX_INSTRUCTION_SIZE, stdin);
        //se o primeiro caractere não for enter
        if (instruction_line[0] != 10)
        {
            Arguments instruction = get_command_and_args(instruction_line);
            for(i =0; i<NCOMMANDS; i++)
            {
                //se comando digitado tiver na lista de comandos la
                if(!strcmp(commands[i].name,instruction.command_name))
                {
                    instruction.owner = &commands[i];
                    //roda funcao daquele comando
                    (commands[i].func)(&instruction);
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
    emulaCMD();
    return 0;
}
