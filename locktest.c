#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>


int main(int argc, char **argv)
{
	char	szBuf[2048], chWho;
	int		fd, x, y;
	struct flock	lockIt;


	if ( argc != 2 )
	{
		fprintf( stderr, "usage: %s outputFile\n", argv[0] );
		return 0;
	}

	fd = open(argv[1], _O_CREAT | _O_WRONLY | _O_TEXT);
	if ( fd < 0 )
	{
		fprintf( stderr, "%s: can't open %s\n", argv[0], argv[1]);
		return 0;
	}

	chWho = (fork() > 0) ? 'c' : 'p';

	lockIt.l_whence = SEEK_CUR;
	lockIt.l_start = 0;
	lockIt.l_len = 16;
	lockIt.l_pid = getpid();

	for ( x = 0; x < 10; ++x )
	{
		sprintf(szBuf, "%c %4d\n", chWho, x);
		//lockIt.l_type = F_WRLCK;
		//fcntl(fd, F_SETLKW, &lockIt);
		write(fd, szBuf, strlen(szBuf));
		//lockIt.l_type = F_UNLCK;
		//fcntl(fd, F_SETLKW, &lockIt);
	}
	
	close(fd);

	return 0;
}
