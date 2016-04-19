
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
  Funksjonsdeklarasjoner for innebygde shell kommandoer 
 */
int gpx_cd(char **args);
int gpx_exit(char **args);

/*
  Lister opp innebygde kommandoer som kan brukes i terminalen
 */
char *builtin_str[] = {
  "cd",
  "exit"
};
/*
Dette er en array av funksjonspekere, som tar en array av strings og returnerer en int. 
Hvis ikke shell gjenkjenner en kommando som innebygd kommando (f.eks er ls innebygd), vil argumenter bli satt på linje 128,
og en return statement vil fylle ut og kalle builtin_func, og deretter kalle en av de innebygde funksjonene, her gpx_cd eller gpx_exit 

*/
int (*builtin_func[]) (char **) = {
  &gpx_cd,
  &gpx_exit
};
// Henter antall av innebygde funksjoner, builtin_str dividert på størrelsen av per karakterpeker
int gpx_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin funksjonsimplementasjon
*/

/**
   @brief Innebygd kommando: change directory
   @param args Liste av args.  args[0] er "cd".  args[1] er filbanen.
   @return retunerer alltid 1 for å fortsette å kjøre
 */
 
/*
  hvis ikke kommandoen cd inneholder noen argumenter, dvs. hvilken mappedestinasjon man vil flyttes til
  så skrives det ut "expected argument to cd"
  else, hvis argumentet ikke stemmer overens med en forventet directory, printer ut "no such file or directory".
*/
 
int gpx_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "gpx: expected argument to \"cd\"\n");
    //Om det ikke følger noe argument etter CD, printes feilbeskjed
    printf("Vi ønsket egentlig mulighet for å kunne gå \"hjem\" eller tilbake, men fikk ikke til å implementere dette\n");
  } else {
    // Kjører systemkallet chdir() på verdien av args (det som er skrevet etter cd, f.eks. cd Desktop)
    if (chdir(args[1]) != 0) {
      perror("gpx"); //print 
    }
  }
  return 1;
}


/**
   @brief Innebygd kommando: exit.
   @param args Liste av args
   @return Returnerer alltid 0 for å terminere shell
 */
int gpx_exit(char **args)
{
  return 0;
}

/**
  @brief Start et program og vent på at det skal termineres
  @param args Null terminert liste av argumenter
  @return Returnerer alltid 1 for å fortsette kjøring
 */
int gpx_launch(char **args)
{
  pid_t pid; //process id
  int status;
  
 //Forker prosessen, og lagrer return verdien. Når fork() returnerer , har vi to prosesser kjørende samdtidig. 
 // Child process
 //child prosessen tar den første betingelsen (pid == 0)
    
  pid = fork(); //processid = fork
  if (pid == 0) {

    if (execvp(args[0], args) == -1) {
      perror("gpx");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    // hvis prosessid er mindre enn 0, så er det ingen prosess, ingen prosess har / skal ha en id mindre en 0
    perror("gpx");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    // Child process skal utføre en prosess, så parent må vente på at denne blir ferdig
    // Bruker parametrene til waitpid til å vente til prosessene er avsluttede eller drept
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  // returnerer til slutt 1 som et signal til kallfunksjonen for å prompte om input igjen
  return 1;
}

/**
   @brief Execute shell-innebygd funksjon eller starter program.
   @param args Null terminert liste av argumenter
   @return 1 om shell skal fortsette å kjøre, 0 om det skal termineres
 */
int gpx_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // En tom kommando var gitt
    return 1;
  }
    // Sjekker om kommandoen er innebygd
    // Hvis den er innebygd returneres og kjøres den innebygde funksjonen, hvis ikke kalles gpx_launch
    // for å starte en ny prosess
  for (i = 0; i < gpx_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  // Ikke innebygd-funksjon, gpx_launch kalles for å starte ny prosess
  return gpx_launch(args);
}

#define GPX_RL_BUFSIZE 1024
/**
   @brief Leser en linje med input fra stdin
   @return linjnen fra stdin
 */
char *gpx_read_line(void)
{
  int bufsize = GPX_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int tegn;
  // om !buffer (ingen buffer), gi allokeringserror og exit
  if (!buffer) {
    fprintf(stderr, "gpx: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    /*
    While parameter 1 (true) vil si at løkka kjøres uendelig for å plukke 
    opp inputs/karakterer fra "user space" (getchar). Løkka blir først avsluttet
    når man kommer til EOF (end of file). (når bruker trykker enter) så starter løkka igjen på neste linje
    */
    tegn = getchar();
    
    /*
    // Om vi når enden på filen (EOF), byttes karakteren ut med en null karakter
    hvis det er slutt på tegn (brukeren trykker enter) blir tegn til linjeskift
    ny linje blir dermed opprettet hvor bufferposisjon er 0, returnerer buffer og linja blir dermed tom. 
    bufferen sin posisjon skal være på det tegnet man skriver. 
    */
    
    if (tegn == EOF || tegn == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = tegn;
    }
    position++;

    // hvis vi har overskredet bufferen, realloker
    
    if (position >= bufsize) {
      // Om position variabelen har en større verdi en vi satt som bufsize, må vi kjøre realloc
      bufsize += GPX_RL_BUFSIZE;
      // Fremfor bufsize = GPX_RL_BUFSIZE, er bufsize +
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "gpx: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define GPX_TOK_BUFSIZE 64
#define GPX_TOK_DELIM " \t\r\n\a"
/**
   @brief Splitter en linje til tokens
   @param line linjen
   @return Null-terminert array av tokens
 */
char **gpx_split_line(char *line)
{

  int bufsize = GPX_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  //allokerer minne tilsvarende størrelsen av variabelen char
  char *token, **tokens_backup;
  // Om tokens variabelen er ikke-eksisterende, allokeringserror og exit
  if (!tokens) {
    fprintf(stderr, "gpx: allocation error\n");
    exit(EXIT_FAILURE);
  }


//Ved starten av funksjonen, tokanizerer vi ved å kalle strtok./ 
//Returnerer en peker til den første token. strtok() returnerer pekere til innsiden av stringen man gir den
//og plasserer 0 bytes på slutten av hver token. Hver peker lagres i en array (eller buffer) av karakter-pekere

  token = strtok(line, GPX_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;
    // Om vi overgår bufferstørrelsen, bufsize, realloker. Gjentas til ingen token er returnert av strtok()
    if (position >= bufsize) {
      bufsize += GPX_TOK_BUFSIZE;
      // += betyr at bufsize = bufsize + GPX_TOK_BUFSIZE. For å øke bufsize
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      // Realloc bufsize multiplisert med størrelsen av char peker
     
      /*
        Om !tokens (at tokens er null), betyr det at allokeringen feilet og det printes en feilmelding, og exit() kalles
     */
      if (!tokens) {
        free(tokens_backup);
        fprintf(stderr, "gpx: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, GPX_TOK_DELIM);
  }
  // Når ingen tokens returneres fra strtok() null-termineres listen av tokens
  tokens[position] = NULL;
  return tokens;
 
}

/**
   @brief Loop får input og utfører handling. 
 */
void gpx_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    
    printf("Et eminent shell> ");
    // line karakter satt av read line, leser linjen
    line = gpx_read_line();
    // args karakter satt av split line, deler ordene opp og tolker de
    args = gpx_split_line(line);
    // status satt av execution av argumenter (args)
    status = gpx_execute(args);
    // frigjør variablene
    free(line);
    free(args);
  } while (status);
}

/**
   @brief Main Startpunkt
   @param argc Argument counter
   @param argv Argument vektor
   @return statuskode
 */
int main(int argc, char **argv)
{
  
  
  printf("Velkommen til vårt shell\n");
  printf("Fungerende kommandoer: Exit, cd og ls\n");
    
  // Kjører kommandoen loop
  gpx_loop();

  // Gjennomfør stopp

  return EXIT_SUCCESS;
}
