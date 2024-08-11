#include <stdio.h>
#include <string.h>

#include "create.h"
#include "delete.h"
#include "read.h"
#include "write.h"
#include "copy.h"
// #include "list.h"
// #include "copy.h"
#include "list.h"
#include "get.h"

int main()
{

    printf(" If you want to exit the write 'exit'\n");
    while (1)
    {
        char command[100];
        printf("ENTER THE COMMAND (all caps)\n");
        fgets(command, sizeof(command), stdin);
        if (strcmp(command, "exit\n") == 0)
        {
            break;
        }
        else if (strcmp(command, "CREATE\n") == 0)
        {
            create();
        }
        else if (strcmp(command, "DELETE\n") == 0)
        {
            delete_();
        }
        // for reading files
        else if (strcmp(command, "READ\n") == 0)
        {
            read_();
        }
        // for writing files
        if (strcmp(command, "WRITE\n") == 0)
        {
            write_();
        }
        if (strcmp(command, "COPY\n") == 0)
        {
            copy();
        }
        if (strcmp(command, "LIST\n") == 0) {
            list();
        }
        if (strcmp(command, "GET\n") == 0)
        {
            gett();
        }
    }

    printf("Thank you, Client is closed, Happy diwali\n");
}