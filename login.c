#include "ucode.c"


int main(int argc, char *argv[ ])
{
    int in;
    int out;
    int err;
    int fd;
    int count;
    int i = 0, r = 0, un = 0, p = 0;
    char username[64], password[64], buf1[1024], hold[1024];
    char uname[64], pass[64];
    char token[64];

    //1. Close fd so we can set them later. 
    close(0);
    close(1);

    //2. Set the STDIN, STDOUT, STDERR
    in = open(argv[1], 0);
    out = open(argv[1], 1);
    err = open(argv[1], 2);
    fixtty(argv[1]);

    // 3. Authenticate by checking against our etc/passwd file
    fd = open("/etc/passwd", O_RDONLY);
    count = read(fd, buf1, 1024);
    strcpy(hold, buf1);

    // 4. Prompt user for login 
    while(1)
    {
        i = 0;
        prints("Please enter Username: ");
        gets(username);
        prints("Please enter Password: ");
        gets(password);
        printf("------------> Logging with %s <------------\n", argv[1]); //display serial port
        while(i < count)
        {
            // 5. Tokenize process
            r = 0;
            un = 0;
            p = 0;

            //username:password:gid:uid:fullname:HOMEDIR:program
            //e.g.    root:xxxxxxx:1000:0:superuser:/root:sh


            while(hold[i] != '\n' && hold[i])
            {
                token[r] = hold[i];
                i++;
                r++;
            }
            i++;
            while(token[un] != ':')
            {
                uname[un] = token[un];
                un++;
            }
            p = un + 1;
            un = 0;
            while(token[p] != ':')
            {
                pass[un] = token[p];
                p++;
                un++;
            }
            p++;
            if((strcmp(username, uname) == 0) && (strcmp(password, pass) == 0))
            {
                char uid[8], gid[8], dir[32], cmd[32];
                int U, G;
                un = 0;
                while(token[p] != ':')
                {
                    uid[un] = token[p];
                    p++;
                    un++;
                }
                un = 0;
                p++;
                while(token[p] != ':')
                {
                    gid[un] = token[p];
                    p++;
                    un++;
                }
                p++;
                un = 0;
                U = atoi(uid);
                G = atoi(gid);
                while(token[p] != ' ')
                {
                    p++;
                }
                while(token[p] != ':')
                {
                    p++;
                }
                p++;
                while(token[p] != ':')
                {
                    dir[un] = token[p];
                    un++;
                    p++;
                }
                p++;
                un = 0;

                while(token[p])
                {
                    cmd[un] = token[p];
                    un++;
                    p++;
                }

                //6. Change user ID to working proc's ID
                //7. Change to user home directory
                
                chuid(U,G);
                chdir(dir);
                close(fd);


                //8. launch SH using wanix sys call
                exec("sh");
                return 1;

            }

            //set info in memory
            memset(uname, 0, 64);
            memset(pass, 0, 64);
        }
        memset(password, 0, 64);
        memset(username, 0, 64);
    }
}
