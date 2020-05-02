//INIT.C

#include "ucode.c"

int console, serial0, serial1;

int parent()
{
    int pid, status;
    while(1)
    {
        pid = wait(&status);
        if(pid == console) //check process id on console to decide which one to execute
        {
            console = fork();
            if(console)
                continue;
            else
                exec("login /dev/tty0");
        }
        if(pid == serial0)//ttyS0 login process
        {
            serial0 = fork();

            if(serial0)
                continue;
            else
                exec("login /dev/ttyS0"); //proceed with the login process 

        }
        if(pid == serial1) //ttyS1 login process
        {
            serial1 = fork();
            if(serial1)
                continue;
            else
                exec("login /dev/ttyS1");//new login process
        }
    }
}

int main(int argc, char *argv[ ])
{
    int in, out;
    //1. open the file descriptors for serial port
    in = open("/dev/tty0", O_RDONLY);//open file descriptors for console so we can read and write to it
    out = open("/dev/tty0", O_WRONLY);


    //2. Fork process
    console = fork();

    //3. Decide serial port terminal to run login on

    if(console)//if console is non-zero then it's the parent PROC
    {
        serial0 = fork();
        if(serial0)
        {
            serial1 = fork();
            if(serial1)
                parent();
            else
                exec("login /dev/ttyS1");
        }
        else
            exec("login /dev/ttyS0");
    }
    else //last serial port
        exec("login /dev/tty0");
}
