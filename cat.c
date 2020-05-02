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

char *cp, mytty[64];

char realname[64], linkname[64];
char newline = '\n';
char CR = '\r';

int main(int argc, char *argv[])
{
	int fd, i, n;
	char buf[1244], dummy;
	char *cp;

	fd = 0;
	if (argc > 1)
	{
		fd = open(argv[1], O_RDONLY);
		if (fd < 0)
		{
			printf("cat %s error\n", argv[1]);
			exit(0);
		}
	}

	if (argc < 2)
	{
		while (gets(buf))
		{
			print2f(buf);
			print2f("\n\r");
		}
		exit(0);
	}

	while ((n = read(fd, buf, 1024)))
	{
		buf[n] = 0;
		cp = buf;
		if (fd)
		{
			for (i = 0; i < n; i++)
			{
				write(1, &buf[i], 1);
				if (buf[i] == '\n')
					write(2, &CR, 1);
			}
		}
		else
		{ // fd=0 case
			cp = buf;
			if (*cp == '\r')
				write(2, &newline, 1);
			write(1, cp, 1);
		}
	}
	close(fd);
	exit(0);
}