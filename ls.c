/*****************************************
 * Zeid Al-Ameedi
 * CPTS 460 Final project.
 * Information (Implementation) - 
 * can be found on https://www.eecs.wsu.edu/~cs460/last.html
 * 
 * Collab : Dr. KC Wang
 * Resources : stackoverflow &  
 * Book1: Design and Implementation of the MTX Operating System
 * Book2: Embedded Real Time Operating Systems
 * Book3: Systems Programming in Unix:Linux
 * 
 * All my code, commits, projects and labs for the course can be found at
 * www.github.com/zalameedi
*****************************************/

#include "ucode.c"

// Pretty much my ls from my 360 days.. ahh good times :)

STAT utat, *sp;
int fd, n;
DIR *dp;
char f[32], cwdname[64], file[64];
char buf[1024];

DIR *dp;
char *cp;

int main(int argc, char *argv[])
{
    int r, i;
    sp = &utat;
    if (argc == 1)
    { 
        strcpy(file, "./");
    }
    else
    {
        strcpy(file, argv[1]);
    }
    if (stat(file, sp) < 0)
    {
        printf("cannot stat %s\n", argv[1]);
        exit(2);
    }
    if ((sp->st_mode & 0xF000) == 0x8000)
    {
        ls_file(sp, file, file);
    }
    else
    {
        if ((sp->st_mode & 0xF000) == 0x4000)
        {
            ls_dir(sp, file);
        }
    }
    exit(0);
}

void ls_file(STAT *sp, char *name, char *path)
{
    u16 mode;
    int mask, k, len;
    char fullname[32], linkname[60];
    mode = sp->st_mode;
    if ((mode & 0xF000) == 0x4000)
        mputc('d');
    if ((mode & 0xF000) == 0xA000)
        mputc('s');
    else if ((mode & 0xF000) == 0x8000)
        mputc('-');

    mask = 000400;
    for (k = 0; k < 3; k++)
    {
        if (mode & mask)
            mputc('r');
        else
            mputc('-');
        mask = mask >> 1;
        if (mode & mask)
            mputc('w');
        else
            mputc('-');
        mask = mask >> 1;
        if (mode & mask)
            mputc('x');
        else
            mputc('-');
        mask = mask >> 1;
    }

    if (sp->st_nlink < 10)
        printf("  %d ", sp->st_nlink);
    else
        printf(" %d ", sp->st_nlink);

    printf(" %d  %d", sp->st_uid, sp->st_gid);
    printf("%d ", sp->st_size);


    printf("%s", name);

    if ((mode & 0xF000) == 0xA000)
    {
        strcpy(fullname, path);
        strcat(fullname, "/");
        strcat(fullname, name);
        len = readlink(fullname, linkname);
        printf(" -> %s", linkname);
    }

    printf("\n\r");
}

void pdate(t) u8 t[4];
{
    printf("%c%c%c%c-%c%c-%c%c  ",
           (t[0] >> 4) + '0', (t[0] & 0x0F) + '0',
           (t[1] >> 4) + '0', (t[1] & 0x0F) + '0',
           (t[2] >> 4) + '0', (t[2] & 0x0F) + '0',
           (t[3] >> 4) + '0', (t[3] & 0x0F) + '0');
}

void ptime(t) u8 t[4];
{
    printf("%c%c:%c%c:%c%c  ",
           (t[0] >> 4) + '0', (t[0] & 0x0F) + '0',
           (t[1] >> 4) + '0', (t[1] & 0x0F) + '0',
           (t[2] >> 4) + '0', (t[2] & 0x0F) + '0');
}

void ls_dir(STAT *sp, char *path)
{
    STAT dstat, *dsp;
    long size;
    char temp[32];
    int r;
    size = sp->st_size;
    fd = open(file, O_RDONLY); 
    while ((n = read(fd, buf, 1024)))
    {
        cp = buf;
        dp = (DIR *)buf;
        while (cp < buf + 1024)
        {
            dsp = &dstat;
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = 0;
            f[0] = 0;
            strcpy(f, file);
            strcat(f, "/");
            strcat(f, temp);
            if (stat(f, dsp) >= 0)
                ls_file(dsp, temp, path);
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }
    close(fd);
}