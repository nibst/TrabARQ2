#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NCOMMANDS 8

struct type_command;

typedef struct type_args
{
    struct type_command *owner; //o comando real que bate com oque o usuario digitou
    char *args;                 //argumentos do comando que usuario digitou
    char *command_name;         //comando que usuario digitou
    unsigned int num_args;      //numero de argumentos que usuario digitou
} Arguments;

typedef struct type_command
{
    char name[7];               //nome de comando aceito pelo terminal
    unsigned int expected_args; //numero de args que o comando vai requerir
    void (*func)(Arguments *);  //funcao que implementa esse comando
} Command;


//só teste das funcoes, nao implementar elas aqui, so colocar header
void CD_function(Arguments *arguments){
    printf("dale\n")
    return;
}
void DIR_function(Arguments *arguments){
    printf("ye\n");
}
void RM_function(Arguments *arguments);
void MKDIR_function(Arguments *arguments);
void MKFILE_function(Arguments *arguments);
void EDIT_function(Arguments *arguments);
void MOVE_function(Arguments *arguments);
void RENAME_function(Arguments *arguments);

Command commands[] =
    {
        {
            "CD",
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
