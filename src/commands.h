

///             Arquitetura e Organização de Computadores II
///                   Trabalho 2: Light File System
///
///             Alunos:
///                     (00326477)  Felipe Kaiser Schnitzler    
///                     (00323741)  Níkolas Padão               
///                     (00275960)  Pedro Afonso Tremea Serpa   
///                     (00xxxxxx)  Ricardo




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
    char name[10];               //nome de comando aceito pelo terminal
    unsigned int expected_args; //numero de args que o comando vai requerir
    int (*func)(Arguments *);  //funcao que implementa esse comando
} Command;


//só teste das funcoes, nao implementar elas aqui, so colocar header
int CD_function(Arguments *arguments);
int DIR_function(Arguments *arguments);
int RM_function(Arguments *arguments);
int MKDIR_function(Arguments *arguments);
int MKFILE_function(Arguments *arguments);
int EDIT_function(Arguments *arguments);
int MOVE_function(Arguments *arguments);
int RENAME_function(Arguments *arguments);

Command commands[NCOMMANDS];
