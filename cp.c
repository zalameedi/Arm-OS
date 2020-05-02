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

int main(int argc, char *argv[]) {
	int n;
	char buf[1024];
	int src = open(argv[1], 0);
	int dest = open(argv[2], 1);

	if(src < 0)
		return -1;


	if(dest < 0)
	{
		creat(argv[2]);
		dest = open(argv[2], 1);
	}

	while( (n = read(src, buf, 1024)) )
		write(dest, buf, n);

	close(src);
	close(dest);
}
