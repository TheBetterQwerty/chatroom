#include <string.h>
#include <stdlib.h>

#include "argparse.h"

void argparse(int args, char** argv, char* ip, int port, char* username) {
    for (int i = 0; i < args; i++) {
        if (!strcmp("-ip", argv[i]) && (i+1 < args))
            strcpy(ip, argv[i+1]);
        if (!strcmp("-p", argv[i]) && (i+1 < args))
            port = atoi(argv[i+1]);
        if (!strcmp("-u", argv[i]) && (i+1 < args))
            strcpy(username, argv[i+1]);
    }
}
