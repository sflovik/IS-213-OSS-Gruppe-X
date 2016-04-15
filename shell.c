// Let's do this boys!
int main (int argc, char **argv)
{
	//Locality-sensitive hashing
	//laster inn config filer hvis noen finnes

	//lsh looper tolk av kommandoer
	lsh.loop();

	return EXIT_SUCCESS;
}
void lsh_loop(void)
{
	char *line;
	char **args;
	int status;

//leser, analyserer og utfører 
	do {
		printf("> ");
		line = lsh_read_line();
		args = lsh_split_line(line);
		status = lsh_execute(args);

		free(line);
		free(args);
	} while (status);
}
#define LSH_RL_BUFSIZE 1024
char *lsh_read_line(void)
{
	int buffsize = LSH_RL_BUFSIZE
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;

	if (!buffer) {
		fprintf(stderr, "lsh: allocation error\n", );
		exit(EXIT_FAILURE);
	}
	while (1) {
		//Les en karakter
		c = getchar();

		
	}
}