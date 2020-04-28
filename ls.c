#include "ucode.c"

#define BLKSIZE 1024

enum TRUTH_VALUES
{
    FALSE,
    TRUE
};

#define S_ISDIR(m) ((m & 0170000) == 0040000) /* directory */
#define S_ISLNK(m) ((m & 0170000) == 0120000) /* symbolic link */
#define S_ISREG(m) ((m & 0170000) == 0100000) /* regular file */

/*
    Name: ls_file
    Description: grabs a files state pointer and checks byte permissions and displays content to user.
*/
void ls_file(STAT *stat_ptr, char *name, char *path)
{
    char full_path[32], link_name[60];
    char *t1 = "xwrxwrxwr-------";
    char *t2 = "----------------"; 

    //check the mode
    if (S_ISDIR(stat_ptr->st_mode)) //print file type as d
        printf("%c", 'd');
    if (S_ISLNK(stat_ptr->st_mode)) //print file type as s
        printf("%c", 's');
    if (S_ISREG(stat_ptr->st_mode)) //print file type as -
        printf("%c", '-');

    for (int i = 8; i >= 0; i--) // check the permissions 
    {
        if (stat_ptr->st_mode & (1 << i))
            printf("%c", t1[i]); // display valid permissions
        else
            printf("%c", t2[i]); // dash invalid permissions
    }

    // print file details
    // link count, uid, gid, filesize, name
    printf(" %d  %d  %d  %d  %s", stat_ptr->st_nlink, stat_ptr->st_uid, stat_ptr->st_gid, stat_ptr->st_size, name);

    if (S_ISLNK(stat_ptr->st_mode)) // symlink file need to print linked file
    {
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, name);
        readlink(full_path, link_name);
        printf(" -> %s", link_name); // linked path name
    }
    printf("\n\r");
}

/*
    Name: ls_dir
    Description: using a directorys stat pointer read the bytes of the directory file and ls all files in the directory
*/
void ls_dir(char *name, char *path)
{
    char ls_path[32];
    char buf[1024];
    DIR *dir_ptr;
    char *char_ptr;
    int dir_fd, can_read;
    STAT dstat, *dirstat_ptr;
    
    char temp[255];


    dir_fd = open(name, O_RDONLY); //open dir file for read


    while ((can_read = read(dir_fd, buf, BLKSIZE))) // read from the directory until there is nothing left
    {
        char_ptr = buf; 
        dir_ptr = (DIR *)buf; // directory pointer on the entries in the directory
        while (char_ptr < buf + BLKSIZE) // go through each entry of the directory file
        {
            // going to print the contents of the directory that we are looking at
            dirstat_ptr = &dstat; 
            strncpy(temp, dir_ptr->name, dir_ptr->name_len); //copy name
            temp[dir_ptr->name_len] = 0;
            ls_path[0] = 0; // reset ls_path

            // Build path to ls
            strcpy(ls_path, name);
            strcat(ls_path, "/");
            strcat(ls_path, temp);

            if (stat(ls_path, dirstat_ptr) >= 0) // check item is a real file and return stat 
                ls_file(dirstat_ptr, temp, path); 

            char_ptr += dir_ptr->rec_len; // Grab next file to ls (everything is a file in linux!)
            dir_ptr = (DIR *)char_ptr; 
        }
    }
    close(dir_fd);
}

int main(int argc, char *argv[])
{
    STAT filestat, *stat_ptr = &filestat;
    int stat_success;
    char *path;
    char file_name[BLKSIZE];

    switch (argc)
    {
        case 1: // ls CWD
            path = "./";
            break;

        case 2:
            path = argv[1]; // ls on a file
            break;

        default: // Validate arguments
            exit(0);
    }
    
    if (stat_success = stat(path, stat_ptr) < 0) // grab file stat 
    {
        printf("cannot stat %s\n", argv[1]);
        exit(2);
    }
    strcpy(file_name, path); // grab the name of the argument
    if (S_ISDIR(stat_ptr->st_mode))
    {
        ls_dir(file_name, file_name); 
    }
    else
    {
        ls_file(stat_ptr, file_name, file_name);
    }
    exit(0); // Leave success
}