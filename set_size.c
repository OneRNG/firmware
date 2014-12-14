#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
void
main(int argc, char**argv)
{
	struct stat s;

	stat(argv[1], &s);
	printf("%c%c", (int)s.st_size&0xff, (int)(s.st_size>>8)&0xff);
	exit(0);
}
