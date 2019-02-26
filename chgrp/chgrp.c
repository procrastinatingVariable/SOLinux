/*
1. Implementati comanda 'chgrp GROUP FILE...'
 (modifica grupul fisierelor specificate de 'FILE...' in 'GROUP', dat
 ca nume).
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>

gid_t newgid;
int filec;
char** files;

gid_t parsegid(char* gids) {
    char* tail;

    errno = 0;
    gid_t gid = strtol(gids, &tail, 0);
    if (errno || *tail != '\0') {
        return -1;
    }

    return gid;
}

gid_t gnametoid(char* gname) {
    struct group* grp;

    gid_t gid;
    if ((gid = parsegid(gname)) != -1) {
        if ((grp = getgrgid(gid)) == NULL) {
            return -1;
        }
    } 
    else {    
        if ((grp = getgrnam(gname)) == NULL) {
            return -1;
        }
    }

    return grp->gr_gid;
}

void argparse(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Not enough arguments\nUSAGE: chgrp GROUP FILE...\n");
        exit(EXIT_FAILURE);
    }

    errno = 0;
    char* gname = argv[1];
    if ((newgid = gnametoid(gname)) == -1) {
        // on some implementations, errno is not set when the given group is not found
        if (errno) {
            perror(gname);
        } 
        else {
            fprintf(stderr, "invalid group: %s\n", gname);
        }
        exit(EXIT_FAILURE);
    }

    filec = argc - 2;
    files = argv + 2;
}

uid_t get_uid(const char* filename) {
    struct stat info;

    if (stat(filename, &info)) {
        return -1;
    }

    return info.st_uid;
}

int chgrp(char * filename, gid_t newgid) {
    uid_t curruid;
    if ((curruid = get_uid(filename)) == -1) {
        return -1;
    }
    
    return chown(filename, curruid, newgid);
}

void procfiles(void) {
    int i;
    char* fname;
    for (i = 0; i < filec; i++) {
        fname = files[i];

        if (chgrp(fname, newgid) == -1) {
            perror(fname);
            continue;
        }
    }
}

int main(int argc, char** argv) {
    argparse(argc, argv);

    procfiles();

    return 0;
}