#include "ucode.c"
#define BLKSIZE 1024
#define TERMINAL_ROWS 25
#define ENTER '\r'
#define NEWLINE '\n'
#define SPACE ' ' // May the force be with you

enum TRUTH_VALUES
{
    FALSE,
    TRUE
};

int validate_serport(int *serport);
int validate_infile(int *file);
int more_file(int infile, int in_port);
int more_terminal(int in_port);

main(int argc, char *argv[])
{
    printf("********** Noah's more **********\n\r");

    int infile, in_port;

    validate_serport(&in_port);
    validate_infile(&infile);

    if (infile) // if more on file
    {
        more_file(infile, in_port);
    }
    else // more on STDIN kinda buggy because its not a real command
    {
        more_terminal(in_port);
    }
}

/*
    Name: validate_serport
    Description: grab serial port and open it for terminal 
*/
int validate_serport(int *serport)
{
    char serport_name[64];
    gettty(serport_name);
    *serport = open(serport_name, 0);
}

/*
    Name: validate_infile
    Description: open either stdin or read from infile
*/
int validate_infile(int *file)
{
    *file = 0; // default infile = stdin
    if (argv[1])
    {
        *file = open(argv[1], 0); // open input file for READ
        if (*file < 0)            // Validate file opened correctly
            exit(1);
    }
}

/*
    Name: more_file
    Description: print a file to the screen in pages or by line based on input
*/
more_file(int infile, int in_port)
{
    char buf[1024];
    int bytes_read, lines = 0;
    char user_input, character, cartridge_return = '\r';

    bytes_read = read(infile, buf, BLKSIZE); // read a block size from the file into the buffer
    while (bytes_read > 0)                   // If bytes were read
    {
        for (int i = 0; i < bytes_read; i++)
        {
            character = buf[i];       // Grab character from buffer
            write(1, &character, 1);  // Write character to screen
            if (character == NEWLINE) // Correctly append cartridge return
            {
                lines++;                        
                write(2, &cartridge_return, 1); // Fix that dang terminal screen when you do a newline
            }
            if (lines > TERMINAL_ROWS) // Terminal Screen is full and need to read user input for page or row
            {
                read(in_port, &user_input, 1); // read user input for next selection

                switch (user_input)
                {
                case ENTER: // reduce lines by 1 if ENTER was hit (1 more line)
                    lines--;
                    break;
                case NEWLINE: // reduce lines by 1 if NEWLINE was hit (1 more line)
                    lines--;
                    break;
                case SPACE: // Allow full page for SPACE
                    lines = 0;
                    break;
                default:
                    lines--;
                    break;
                }
            }
        }
        bytes_read = read(infile, buf, BLKSIZE); // Grab another chunk of text
    }
}

/*
    Name: more_terminal
    Description: read from the user input and print it to the screen
*/
more_terminal(int in_port)
{
    char buf[BLKSIZE], user_input;
    int bytes_read, n_line;
    printf("\n\r");             // Clear screen row
    while (getline(buf))        // Grab user input
    {                           // getline() does not show input chars
        printf(" %s\n\r", buf); // Get that input back to the screen
        n_line++;
        if (n_line > TERMINAL_ROWS)
        {
            bytes_read = read(in_port, &user_input, 1);

            switch (user_input)
            {
            case ENTER: // reduce lines by 1 if ENTER was hit (1 more line)
                n_line--;
                break;
            case NEWLINE: // reduce lines by 1 if NEWLINE was hit (1 more line)
                n_line--;
                break;
            case SPACE: // Allow full page for SPACE
                n_line = 0;
                break;
            default:
                n_line--;
                break;
            }
        }
    }
}