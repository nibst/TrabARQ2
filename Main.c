
#include <stdio.h>
#include <conio.h>
#include <string.h>

int checkCommand(char instruction[])
{
    char *command;
    command = strtok(instruction," \n"); //pega o comando
    if (!strcmp(command, "CD") || !strcmp(command, "DIR") || !strcmp(command, "RM") || !strcmp(command, "MKDIR") || !strcmp(command, "MKFILE") || !strcmp(command, "EDIT") || !strcmp(command, "MOVE") || !strcmp(command, "RENAME"))
        return 0;
    else
        return 1;
}

void emulaCMD()
{
    char dirName[] = "dir";
    char instruction[300];

    while (1)
    {
        printf("root/");
        printf("%s", dirName);
        printf("/>");
        fgets(instruction, 300, stdin);
        //chama funcao que analisa se ta correto o comando
        if (instruction[0] != 10)
        {
            if (checkCommand(instruction))
                printf("INVALID COMMAND");
            printf("\n");
        }

    }
}

int main()
{

    //inicializar a estrutura de arquivos de dados
    //emular o cmd, fazer coisas tipo root\dir> <tal comando>
    emulaCMD();
    return 0;
}
