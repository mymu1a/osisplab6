#include <stdio.h>
#include <stdlib.h>

#include "gengetopt/cmdline.h"

struct gengetopt_args_info	config;

int main(int argc, char** argv)
{
    if (cmdline_parser(argc, argv, &config) != 0)
    {
        exit(1);
    }

    printf("config.memsize: %d\n", config.memsize_arg);
    printf("config.blocks: %d\n", config.blocks_arg);
    printf("config.threads: %d\n", config.threads_arg);
    printf("config.filename: %s\n", config.filename_arg);

    return 0;
}
