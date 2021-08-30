/* $Revision: 652 $ */
/* $Date: 2021-08-28 09:52:46 +0200 (sob, 28 avg 2021) $ */
/* This file (biddy-example-dictman.c) is a C file */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v2.0 and laters */

/* IF DICTIONARY INCLUDES NULL SYMBOL (I.E. WORDS HAVE DIFFERENT LENGTH) */
/* THEN DICTIONARY MUST INCLUDE EMPTY WORD. */
/* IF DICTIONARY DOES NOT INCLUDE NULL SYMBOL (I.E. ALL WORDS HAVE EQUAL LENGTH) */
/* THEN LENGTH 1 IS NOT ALLOWED. */

/* ON LINUX COMPILE WITH (ADD -lgmp IF USING STATIC BIDDY LIBRARY): */
/* gcc -DUNIX -DBIDDY -O2 -o biddy-example-dictman biddy-example-dictman.c biddy-example-dict-common.c -I. -L./bin -lbiddy */

/* ON MINGW COMPILE WITH (ADD -lgmp IF USING STATIC BIDDY LIBRARY): */
/* gcc -DMINGW -DBIDDY -O2 -o biddy-example-dictman biddy-example-dictman.c biddy-example-dict-common.c -I. -L./bin -lbiddy -Wl,-Bdynamic -lws2_32 */

/* ON LINUX PROFILE WITH (YOU NEED STATIC BIDDY LIBRARY): */
/* gcc -DUNIX -DBIDDY -O2 -ggdb -pg -o biddy-example-dictman biddy-example-dictman.c biddy-example-dict-common.c -I. -L./bin -lbiddy -lgmp -lm */
/* gprof biddy-example-dictman gmon.out > analysis.txt */

#define DEFAULT_FILENAME "./bddscout/DICTIONARY/words-TZBDD.bddview"
#define DEFAULT_ALPHABETSIZE 54
#define DEFAULT_MAXWORDLENGTH 24

/* TO MANIPULATE DICTIONARY GENERATED FOR BENCHMARK USED IN THE PAPER */
/* ./biddy-example-dictman ./bddscout/DICTIONARY/biddy-words-TZBDD.bddview 5 3 */

/* TO MANIPULATE DICTIONARY GENERATED FOR BENCHMARK 1 */
/* ./biddy-example-dictman ./bddscout/DICTIONARY/abcd-words-fixedsize-TZBDD.bddview 4 3 */

/* TO MANIPULATE DICTIONARY GENERATED FOR BENCHMARK 2 */
/* ./biddy-example-dictman ./bddscout/DICTIONARY/abcd-words-TZBDD.bddview 5 3 */

/* TO MANIPULATE DICTIONARY GENERATED FOR BENCHMARK 3 */
/* ./biddy-example-dictman ./bddscout/DICTIONARY/abcd-words-TZBDD.bddview 6 4 */

/* TO MANIPULATE DICTIONARY GENERATED FOR BENCHMARK 4 */
/* ./biddy-example-dictman ./bddscout/DICTIONARY/words12-TZBDD.bddview 54 5 */

/* TO MANIPULATE DICTIONARY GENERATED FOR BENCHMARK 5 */
/* ./biddy-example-dictman ./bddscout/DICTIONARY/words12-ascii-TZBDD.bddview 129 5 */

/* TO MANIPULATE DICTIONARY GENERATED FOR BENCHMARK 6 */
/* ./biddy-example-dictman ./bddscout/DICTIONARY/sgb-words-fixedsize-TZBDD.bddview 26 5 */

/* TO MANIPULATE DICTIONARY GENERATED FOR BENCHMARK 7 */
/* ./biddy-example-dictman ./bddscout/DICTIONARY/words-TZBDD.bddview 54 24 */

/* TO MANIPULATE DICTIONARY GENERATED FOR BENCHMARK 8 */
/* ./biddy-example-dictman ./bddscout/DICTIONARY/words-ascii-TZBDD.bddview 129 24 */

/* TO MANIPULATE DICTIONARY GENERATED FOR BENCHMARK 9 */
/* ./biddy-example-dictman ../donaj/donaj-TZBDD.bddview 31 25 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "biddy-example-dict-common.h"

void allWords(Biddy_Edge dictionary, VarTable *vartable, unsigned char word[], Biddy_Boolean usenullsymbol, unsigned char nullsymbol, unsigned int *wordnum, Biddy_String **wordlist);
void patternWords(Biddy_Edge dictionary, Biddy_String pattern, VarTable *vartable, unsigned char word[], Biddy_Boolean usenullsymbol, unsigned char nullsymbol, unsigned int *wordnum, Biddy_String **wordlist);
void writeTemplate(Biddy_Edge templ, VarTable *vartable, unsigned char word[], unsigned int *wordnum, Biddy_String **wordlist);
Biddy_Edge subgraphPattern(Biddy_String pattern, Biddy_Edge dictionary, unsigned char alphabet[], VarTable *vartable, Biddy_Boolean usenullsymbol, unsigned char nullsymbol, Biddy_Edge code[], Biddy_Edge codenot[], unsigned int alphabetsize, unsigned int maxwordlength);
Biddy_Edge subgraphHamming(Biddy_String word, unsigned int dist, Biddy_Edge dictionary, unsigned char alphabet[], VarTable *vartable, Biddy_Boolean usenullsymbol, unsigned char nullsymbol, Biddy_Edge code[], Biddy_Edge codenot[], unsigned int alphabetsize, unsigned int maxwordlength);
Biddy_Edge subgraphLcs(Biddy_String word, unsigned int dist, Biddy_Edge dictionary, unsigned char alphabet[], VarTable *vartable, Biddy_Boolean usenullsymbol, unsigned char nullsymbol, Biddy_Edge code[], Biddy_Edge codenot[], unsigned int alphabetsize, unsigned int maxwordlength);
Biddy_Edge subgraphLevenshtein(Biddy_String word, unsigned int dist, Biddy_Edge dictionary, unsigned char alphabet[], VarTable *vartable, Biddy_Boolean usenullsymbol, unsigned char nullsymbol, Biddy_Edge code[], Biddy_Edge codenot[], unsigned int alphabetsize, unsigned int maxwordlength);

/* our theory about templates is not finished, yet, thus this is currently not used */
Biddy_String readLevenshteinTemplate(unsigned int wordlength, unsigned int dist, unsigned int alphabetsize, unsigned int maxwordlength);
Biddy_Edge useLevenshteinTemplate(Biddy_String word, unsigned int dist, Biddy_String *templatetable, unsigned char alphabet[], VarTable *vartable, unsigned char nullsymbol, unsigned int alphabetsize, unsigned int maxwordlength);

#if (defined(_MSC_VER) || defined(MINGW))
#include <winsock2.h>
//#include <ws2tcpip.h>
#define SocketType SOCKET
#define SOCKETERROR INVALID_SOCKET
#else
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#define SocketType int
#define SOCKETERROR -1
#endif

/* This is from https://stackoverflow.com/questions/15701150/redirect-fprintf-output-to-a-port */
/* Thanks to Saswat Padhi */
#if (defined(_MSC_VER) || defined(MINGW))
void fprintfsock(SOCKET s, const char* f, ...)
{
  char *buf;
  va_list a;
  va_start(a,f);
  int l = vsnprintf(0,0,f,a);
  buf = (char*) malloc(l+1);
  va_start(a,f);
  vsnprintf(buf,l,f,a);
  send(s,buf,l,0);
  free(buf);
}
#endif

/* Limit for read is set to 1024 characters! */
#if (defined(_MSC_VER) || defined(MINGW))
int fscanfsock(SOCKET s, const char* f, ...)
{
  char buf[1024];
  va_list a;
  if (recv(s,buf,1024,0) == SOCKETERROR) return EOF;
  va_start(a,f);
  return(vsscanf(buf,f,a));
}
#endif

/*******************************************************************************
\brief Function initServer.

### Description
### Side effects
### More info
*******************************************************************************/

SocketType
initServer(int port)
{

  SocketType sock = SOCKETERROR;

#if (defined(_MSC_VER) || defined(MINGW))

  WSADATA wsaData;
  struct sockaddr_in server;

#else

  struct sockaddr_in server;

#endif

#if (defined(_MSC_VER) || defined(MINGW))

  /* initialize Winsock */
  if (WSAStartup(MAKEWORD(2,2),&wsaData) !=0) {
    printf("WSAStartup failed: %d\n",WSAGetLastError());
    exit(1);
  }

  /* create server socket */
  sock = socket(AF_INET,SOCK_STREAM,0);
  if (sock == SOCKETERROR) {
    printf("ERROR: create socket failed (%d)\n",WSAGetLastError());
    exit(1);
  }

  /* bind */
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);

  if (bind(sock,(struct sockaddr *)&server,sizeof(server)) == SOCKETERROR) {
    printf("ERROR: bind failed (%d)\n",WSAGetLastError());
    exit(1);
  }

#else

  /* create server socket */
  sock = socket(AF_INET,SOCK_STREAM,0);
  if (sock == SOCKETERROR) {
    printf("ERROR: create socket failed\n");
    exit(1);
  }

  /* bind */
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);
  if (bind(sock,(struct sockaddr *)&server,sizeof(server)) == SOCKETERROR) {
    printf("ERROR: bind failed\n");
    exit(1);
  }

#endif

  return sock;
}

/*******************************************************************************
\brief Function acceptClient.

### Description
### Side effects
### More info
*******************************************************************************/

void
acceptClient(SocketType sock, Biddy_Edge dictionary, Biddy_Boolean summarymode,
  unsigned char alphabet[], VarTable *vartable, Biddy_Boolean usenullsymbol,
  unsigned char nullsymbol, Biddy_Edge code[], Biddy_Edge codenot[],
  unsigned int alphabetsize, unsigned int maxwordlength)
{
  int c;
  SocketType clientsock;
  struct sockaddr_in client;
  Biddy_String w;
  unsigned int dist;
  Biddy_Boolean OK;
  Biddy_Edge subdictionary;
  unsigned char *word = (unsigned char *) calloc(maxwordlength+1,sizeof(unsigned char));
  unsigned int wordnum = 0;
  Biddy_String *wordlist = (Biddy_String *) malloc(sizeof(Biddy_String));
  Biddy_String *result = NULL;
  unsigned int i;
  double n;
  clock_t elapsedtime;

#if (defined(_MSC_VER) || defined(MINGW))
#else
  FILE * outputStream;
  FILE * inputStream;
#endif

  /* listen */
  listen(sock,1);

  //accept connection from an incoming client
  c = sizeof(struct sockaddr_in);
  clientsock = accept(sock,(struct sockaddr *)&client,&c);
  if (clientsock == SOCKETERROR)
  {
    printf("ERROR: accept failed\n");
    exit(1);
  }

#if (defined(_MSC_VER) || defined(MINGW))
#else
  if (!(inputStream = fdopen(dup(clientsock),"r"))) {
    printf("ERROR: fdopen failed for read\n");
    exit(1);
  }
  if (!(outputStream = fdopen(dup(clientsock),"w"))) {
    printf("ERROR: fdopen failed for write\n");
    exit(1);
  }
  setbuf(outputStream,NULL);
#endif

  printf("\nClient connected!\n");

  OK = TRUE;
  w = (Biddy_String) calloc(255,sizeof(char));
  dist = 0;

#if (defined(_MSC_VER) || defined(MINGW))
  if (fscanfsock(clientsock,"%s%d",w,&dist) < 2) OK = FALSE;
#else
  if (fscanf(inputStream, "%s%d", w, &dist) < 2) OK = FALSE;
#endif

  if (!strlen(w)) OK = FALSE;
  if (!dist) OK = FALSE;

  if (!OK) {
#if (defined(_MSC_VER) || defined(MINGW))
    closesocket(clientsock);
#else
    fclose(outputStream);
    fclose(inputStream);
    close(clientsock);
#endif
    return;
  }

  elapsedtime = clock();
  subdictionary = subgraphLevenshtein(w,dist,dictionary,alphabet,vartable,usenullsymbol,nullsymbol,code,codenot,alphabetsize,maxwordlength);

  if (summarymode) {
#ifdef _MSC_VER
    fprintfsock(clientsock,"%.0f\n\n",Biddy_CountMinterms(subdictionary,0));
#elif MINGW
    fprintfsock(clientsock,"%.0f\n",Biddy_CountMinterms(subdictionary,0));
#else
    fprintf(outputStream,"%.0f\n",Biddy_CountMinterms(subdictionary,0));
#endif
  } else {
    for (i = 0; i < maxwordlength; i++) {
      word[i] = '*';
    }
    wordlist[0] = NULL;
    allWords(subdictionary,vartable,word,usenullsymbol,nullsymbol,&wordnum,&wordlist);
    result = wordlist;
    while (result[0]) {
#ifdef _MSC_VER
      fprintfsock(clientsock,"%s\n\n",result[0]);
#elif MINGW
      fprintfsock(clientsock,"%s\n",result[0]);
#else
      fprintf(outputStream,"%s\n",result[0]);
#endif
      free(result[0]);
      result++;
    }
    free(word);
    free(wordlist);
  }

  elapsedtime = clock()-elapsedtime;
  if (summarymode) {
    printf("result returned in %.2f s\n",elapsedtime/(1.0*CLOCKS_PER_SEC));
  } else {
    if ((n=Biddy_CountMinterms(subdictionary,0)) == 1) {
      printf("%.0f word returned in %.2f s\n",n,elapsedtime/(1.0*CLOCKS_PER_SEC));
    } else {
      printf("%.0f words returned in %.2f s\n",n,elapsedtime/(1.0*CLOCKS_PER_SEC));
    }
  }

  SWEEP();

  free(w);

#if (defined(_MSC_VER) || defined(MINGW))
  closesocket(clientsock);
#else
  fclose(outputStream);
  fclose(inputStream);
  close(clientsock);
#endif

  printf("Client disconnected!\n");
}

/*******************************************************************************
\brief Function main.

### Description
### Side effects
### More info
*******************************************************************************/

int main(int argv, char ** argc) {
  FILE *bddfile;
  unsigned char buffer[256],bddtype[256];
  Biddy_Boolean servermode,summarymode;
  int serverport;
  Biddy_String filename,line,name;
  Biddy_String userinput;
  Biddy_Boolean stop;
  Biddy_Edge dictionary,tmp;
  unsigned int dictionary_idx,original_idx;
  unsigned int alphabetsize,maxwordlength;
  unsigned char *alphabet;
  Biddy_Boolean usenullsymbol;
  unsigned char nullsymbol;
  Biddy_Edge *code; /* BDD variables */
  Biddy_Edge *codenot; /* BDD variables */
  VarTable *vartable;
  Biddy_String *templatetable;
  unsigned int i,j;
  int n;
  clock_t elapsedtime;

  setbuf(stdout,NULL);
  filename = NULL;
  serverport = 4444;
  servermode = FALSE;
  summarymode = FALSE;

  n = 1;
  while ((n < argv) && argc[n][0] == '-') n++;
  if (n >= argv) {
    printf("USAGE: ./biddy-example-dictman options filename alphabetsize maxwordlength\n");
    filename = strdup(DEFAULT_FILENAME);
    alphabetsize = DEFAULT_ALPHABETSIZE;
    maxwordlength = DEFAULT_MAXWORDLENGTH;
    printf("Trying default: ./biddy-example-dictman %s %u %u\n",filename,alphabetsize,maxwordlength);
  }
  else {
    filename = strdup(argc[n]);
    n++;
    while ((n < argv) && argc[n][0] == '-') n++;
    if (n >= argv) {
      printf("USAGE: ./biddy-example-dictman options filename alphabetsize maxwordlength\n");
      exit(1);
    }
    sscanf(argc[n], "%u", &alphabetsize);
    n++;
    while ((n < argv) && argc[n][0] == '-') n++;
    if (n >= argv) {
      printf("USAGE: ./biddy-example-dictman options filename alphabetsize maxwordlength\n");
      exit(1);
    }
    sscanf(argc[n], "%u", &maxwordlength);
  }

  for (n = 1; n < argv; n++) {
    if (argc[n][0] == '-') {
      if (!strcmp(argc[n],"-server")) servermode = TRUE;
      else if (!strcmp(argc[n],"-summary")) summarymode = TRUE;
      else {
        printf("Option \"%s\" not recognized\n",argc[n]);
        printf("Valid options are: -server -summary\n");
      }
    }
  }

  /* CHECK BDD TYPE AND SET UP THE CORRECT SYSTEM */
  bddfile = fopen(filename,"r");
  if (!bddfile) {
    printf("File error (%s)!\n",filename);
    exit(1);
  }
  do {
    line = fgets((char *)buffer,255,bddfile);
  } while (line && strncmp(line,"type",4));
  if (!line) {
    printf("File error - missing type (%s)!\n",argc[1]);
    exit(1);
  }
  sscanf(&line[5],"%s",bddtype);
  fclose(bddfile);
  if (!strcmp((char *)bddtype,"robdd") || !strcmp((char *)bddtype,"ROBDD")) {
    Biddy_InitAnonymous(BIDDYTYPEOBDD);
  }
  else if (!strcmp((char *)bddtype,"robddce") || !strcmp((char *)bddtype,"ROBDDCE")) {
    Biddy_InitAnonymous(BIDDYTYPEOBDDC);
  }
  else if (!strcmp((char *)bddtype,"zbdd") || !strcmp((char *)bddtype,"ZBDD")) {
    Biddy_InitAnonymous(BIDDYTYPEZBDD);
  }
  else if (!strcmp((char *)bddtype,"zbddce") || !strcmp((char *)bddtype, "ZBDDCE")) {
    Biddy_InitAnonymous(BIDDYTYPEZBDDC);
  }
  else if (!strcmp((char *)bddtype,"tzbdd") || !strcmp((char *)bddtype,"TZBDD")) {
    Biddy_InitAnonymous(BIDDYTYPETZBDD);
  }
  else {
    printf("File error - wrong type (%s)!\n",argc[1]);
    exit(1);
  }

  /* READ COMPLETE BDD FROM THE FILE */
  printf("Reading %s...\n",filename);
  elapsedtime = clock();
  Biddy_ReadBddview(filename,(Biddy_String)"DICT");
  free(filename);
  elapsedtime = clock()-elapsedtime;

  /* INITIALIZE SOME VARIABLES AND STRUCTURES */
  /* THIS IS COMPLICATED BECAUSE VERY LITTLE DICTIONARY-ORIENTED DATA ARE STORED IN A bddview FILE */
  /* vartable CONTAINS THE TABLE OF ALL VARIABLES */
  /* vartable[0] IS NOT USED, vartable[i] CORRESPONDS TO VARIABLE Biddy_GetVariableName(i) */
  /* alphabet INCLUDES ALL LETTERS FROM DICTIONARY */
  /* code AND codenot INCLUDE BDD CODES FOR ALL LETTERS FROM DICTIONARY */
  /* THESE VARIABLES/STRUCTURES MUST NOT BE CHANGED EVEN IF VARIABLE ORDERING IS CHANGED (E.G. WITH SIFTING) */
  Biddy_FindFormula((Biddy_String)"DICT",&dictionary_idx,&dictionary);
  original_idx = MARK_X("DICT",dictionary);
  SWEEP();

  /* Biddy_VariableTableNum() counts all user variables + constant variable 1 */
  if (Biddy_VariableTableNum() != (alphabetsize*maxwordlength+1)) {
    printf("File error - wrong number of variables!\n");
    exit(1);
  }

  vartable = createVarTable(alphabetsize,maxwordlength);

  alphabet = (unsigned char *) malloc(alphabetsize*sizeof(unsigned char));
  j = 0;
  for (i=1; i<(alphabetsize*maxwordlength+1); i++) {
    if (vartable[i].y == 1) {
      if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC)) {
        alphabet[j++] = vartable[i].x;
      }
      else if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) {
        alphabet[alphabetsize-(j++)-1] = vartable[i].x;
      }
      else if ((Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC)) {
        alphabet[alphabetsize-(j++)-1] = vartable[i].x;
      }
    }
  }

  code = (Biddy_Edge *) malloc(alphabetsize*maxwordlength*sizeof(Biddy_Edge));
  codenot = (Biddy_Edge *) malloc(alphabetsize*maxwordlength*sizeof(Biddy_Edge));
  for (i=1; i<(alphabetsize*maxwordlength+1); i++) {
    for (j=0; j<alphabetsize; j++) {
      if (alphabet[j] == vartable[i].x) {
        code[(vartable[i].y-1)*alphabetsize+j] = Biddy_GetVariableEdge(i);
        codenot[(vartable[i].y-1)*alphabetsize+j] = Biddy_Not(Biddy_GetVariableEdge(i));
        MARK_0(codenot[(vartable[i].y-1)*alphabetsize+j]); /* do not use purge! */
      }
    }
  }

  /* DEBUGGING */
  /*
  reportAlphabet(alphabet,alphabetsize);
  reportCodes(alphabet,code,alphabetsize,maxwordlength);
  */

  /* CHECK IF WORD WITH LENGTH 1 EXIST */
  /* IF IT EXIST THEN ITS LETTER IS USED AS NULL SYMBOL */
  usenullsymbol = FALSE;
  nullsymbol = 0;
  tmp = dictionary;
  while (!Biddy_IsTerminal(Biddy_GetElse(tmp))) {
    tmp = Biddy_TransferMark(Biddy_GetElse(tmp),Biddy_GetMark(tmp),TRUE);
  }
  name = Biddy_GetTopVariableName(tmp); /* the variable we choose 'then' successor */
  tmp = Biddy_TransferMark(Biddy_GetThen(tmp),Biddy_GetMark(tmp),FALSE);
  while (!Biddy_IsTerminal(tmp)) {
    tmp = Biddy_TransferMark(Biddy_GetElse(tmp),Biddy_GetMark(tmp),TRUE);
  }
  if (!Biddy_GetMark(tmp)) {
    usenullsymbol = TRUE;
    if (strlen(name) == 3) nullsymbol = name[0];
    else if (strlen(name) == 5) nullsymbol = 100*(name[0]-'0')+10*(name[1]-'0')+(name[2]-'0');
    else {
      printf("File error - wrong variable name (%s)!\n",name);
      exit(1);
    }
    /* printf("EMPTY WORD EXISTS, NULL SYMBOL = <%c>(#%u)",nullsymbol,nullsymbol); */
  }

  if (usenullsymbol) {
    /* it would be simpler but less efficient to use tmp = Biddy_Not(nullword) */
    printf("Removing null word...\n");
    tmp = Biddy_GetConstantZero();
    for (i=0; i<alphabetsize-1; i++) {
      tmp = Biddy_Or(tmp,code[i]);
    }
    dictionary = Biddy_And(dictionary,tmp);
    original_idx = dictionary_idx = MARK_X("DICT",dictionary);
    SWEEP();
  }

  /* intitialize templates */
  templatetable = (Biddy_String *) calloc(maxwordlength*maxwordlength,sizeof(Biddy_String));
  for (i = 0; i < maxwordlength; i++) {
    for (j = 0; j < maxwordlength; j++) {
      templatetable[i*maxwordlength+j] = NULL;
    }
  }

  if (servermode) {
    SocketType sock;

    printf("TIME: %.2f s\n",elapsedtime/(1.0*CLOCKS_PER_SEC));

    /* FOR TZBDD, TOP EDGE OF dictionary MAY CHANGE DURING THE SIFTING */
    dictionary = Biddy_GetIthFormula(dictionary_idx);

    sock = initServer(serverport);
    printf("Server mode started, use e.g.: echo \"feri 1\" | nc localhost %d\n",serverport);

    while(1) {
      acceptClient(sock,dictionary,summarymode,alphabet,vartable,usenullsymbol,nullsymbol,code,codenot,alphabetsize,maxwordlength);
    }

#if (defined(_MSC_VER) || defined(MINGW))
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

  } else {

    userinput = strdup("...");

    do {

      /* FOR TZBDD, TOP EDGE OF dictionary MAY CHANGE DURING THE SIFTING */
      dictionary = Biddy_GetIthFormula(dictionary_idx);

      if ((toupper(userinput[0]) != 'Y') && (toupper(userinput[0]) != 'O') && (toupper(userinput[0]) != 'F')
           && (toupper(userinput[0]) != 'G')
           && (toupper(userinput[0]) != 'D') && (toupper(userinput[0]) != 'A') && (toupper(userinput[0]) != 'P')
           && (toupper(userinput[0]) != 'T') && (toupper(userinput[0]) != 'I'))
      {
        printf("TIME: %.2f s\n",elapsedtime/(1.0*CLOCKS_PER_SEC));
        reportSystem();
        reportDictionary(dictionary,alphabetsize,maxwordlength);
      }

      printf("\n[D]ICTIONARY / [T]EMPLATES / S[Y]STEM REPORT / [O]RDERING / SAVE TO [F]ILE / REVERT TO ORI[G]INAL / E[X]IT: ");
      if (!scanf("%s",userinput)) printf("ERROR\n");

      elapsedtime = clock();

      if (toupper(userinput[0]) == 'X') {
        stop = TRUE;
      } else if (toupper(userinput[0]) == 'D') {
        printf("\nOPERATIONS ON DICTIONARY: [A]LL WORDS / [P]ATTERN / S[U]BGRAPH  / [H]AMMING / LC[S] / [L]EVENSHTEIN\n");
        stop = FALSE;
      } else if (toupper(userinput[0]) == 'T') {
        printf("\nOPERATIONS ON TEMPLATES: [R]EAD TEMPLATES / L[I]ST TEMPLATES / [W]RITE TEMPLATE\n");
        stop = FALSE;
      } else if (toupper(userinput[0]) == 'Y') {
        printf("\n");
        Biddy_PrintInfo(stdout);
        printf("\n");
        stop = FALSE;
      } else if (toupper(userinput[0]) == 'O') {
        printf("\n");
        reportOrdering();
        printf("\n");
        stop = FALSE;
      } else if (toupper(userinput[0]) == 'F') {
        /* DEBUGGING */
        /*
        Biddy_WriteDot("dictionary.dot",dictionary,"DICT",-1,FALSE);
        printf("\nCreated graph in dot format.\n");
        printf("Use 'dot -y -Tpng -O dictionary.dot' to visualize BDD for dictionary.\n");
        */
        Biddy_WriteBddview("dictionary.bddview",dictionary,"DICT",NULL);
        printf("\nBDD saved to file dictionary.bddview.\n");
        printf("Use Bdd Scout to view and manipulate this file.\n");
        stop = FALSE;
      } else if (toupper(userinput[0]) == 'G') {
        dictionary_idx = original_idx;
        dictionary = Biddy_GetIthFormula(dictionary_idx);
        printf("\n");
        reportDictionary(dictionary,alphabetsize,maxwordlength);
        stop = FALSE;
      } else if (toupper(userinput[0]) == 'A') {
        unsigned char *word = (unsigned char *) calloc(maxwordlength+1,sizeof(unsigned char));
        unsigned int wordnum = 0;
        Biddy_String *wordlist = (Biddy_String *) malloc(sizeof(Biddy_String));
        Biddy_String *result = NULL;
        double num;
        num = Biddy_CountMinterms(dictionary,0);
        if (num > 1024) {
          printf("\nThere are %.0f words - to many to report them!\n",num);
        } else {
          for (i = 0; i < maxwordlength; i++) {
            word[i] = '*';
          }
          wordlist[0] = NULL;
          allWords(dictionary,vartable,word,usenullsymbol,nullsymbol,&wordnum,&wordlist);
          if (wordnum == 1) {
            printf("\nThere is %u word:\n",wordnum);
          } else {
            printf("\nThere are %u words:\n",wordnum);
          }
          result = wordlist;
          while (result[0]) {
            printf("%s\n",result[0]);
            free(result[0]);
            result++;
          }
        }
        free(wordlist);
        free(word);
        stop = FALSE;
      } else if (toupper(userinput[0]) == 'P') {
        unsigned int wordnum = 0;
        Biddy_String *result = NULL;
        Biddy_String *wordlist = (Biddy_String *) malloc(sizeof(Biddy_String));
        unsigned char *word = (unsigned char *) calloc(maxwordlength+1,sizeof(unsigned char));
        unsigned char *p = (unsigned char *) calloc(maxwordlength+1,sizeof(unsigned char));
        printf("Pattern (e.g. **A): ");
        while (!(scanf("%s",p)) || (strlen((char *)p) > maxwordlength)) {
          printf("Enter valid pattern (e.g. **A): ");
        }
        for (i = 0; (i < strlen((char *)p)) && (i < maxwordlength); i++) {
          word[i] = '*';
        }
        wordlist[0] = NULL;
        patternWords(dictionary,(Biddy_String)p,vartable,word,usenullsymbol,nullsymbol,&wordnum,&wordlist);
        if (wordnum > 1024) {
          printf("\nThere are %u words - to many to report them!\n",wordnum);
          result = wordlist;
          while (result[0]) {
            free(result[0]);
            result++;
          }
        } else {
          if (wordnum == 1) {
            printf("\n%u word for pattern \"%s\":\n",wordnum,p);
          } else {
            printf("\n%u words for pattern \"%s\":\n",wordnum,p);
          }
          result = wordlist;
          while (result[0]) {
            printf("%s\n",result[0]);
            free(result[0]);
            result++;
          }
        }
        free(wordlist);
        free(word);
        free(p);
        stop = FALSE;
      } else if (toupper(userinput[0]) == 'U') {
        unsigned char *p = (unsigned char *) calloc(maxwordlength+1,sizeof(unsigned char));
        printf("Subgraph for pattern (e.g. ab*): ");
        while (!(scanf("%s",p)) || (strlen((char *)p) > maxwordlength)) {
          printf("Enter valid pattern (e.g. ab*): ");
        }
        dictionary = subgraphPattern((Biddy_String)p,dictionary,alphabet,vartable,usenullsymbol,nullsymbol,code,codenot,alphabetsize,maxwordlength);
        dictionary_idx = MARK_X("SUBDICT",dictionary);
        SWEEP();
        printf("Subgraph for pattern \"%s\" created.\n\n",p);
        free(p);
        stop = FALSE;
      } else if (toupper(userinput[0]) == 'H') {
        unsigned char *w = (unsigned char *) calloc(maxwordlength+1,sizeof(unsigned char));
        unsigned int dist;
        printf("Hamming for word: ");
        while (!(scanf("%s",w)) || (strlen((char *)w) > maxwordlength)) {
          printf("Enter valid word: ");
        }
        printf("Max Distance: ");
        while (!(scanf("%u",&dist)) || dist > (strlen((char *)w))) {
          printf("Enter valid distance: ");
        }
        dictionary = subgraphHamming((Biddy_String)w,dist,dictionary,alphabet,vartable,usenullsymbol,nullsymbol,code,codenot,alphabetsize,maxwordlength);
        dictionary_idx = MARK_X("SUBDICT",dictionary);
        SWEEP();
        printf("Subgraph for word \"%s\" created.\n\n",w);
        free(w);
        stop = FALSE;
      } else if (toupper(userinput[0]) == 'S') {
        unsigned char *w = (unsigned char *) calloc(maxwordlength+1,sizeof(unsigned char));
        unsigned int dist;
        printf("LCS for word: ");
        while (!(scanf("%s",w)) || (strlen((char *)w) > maxwordlength)) {
          printf("Enter valid word: ");
        }
        printf("Max Distance: ");
        while (!(scanf("%u",&dist)) || dist > 2*maxwordlength) {
          printf("Enter valid distance: ");
        }
        dictionary = subgraphLcs((Biddy_String)w,dist,dictionary,alphabet,vartable,usenullsymbol,nullsymbol,code,codenot,alphabetsize,maxwordlength);
        dictionary_idx = MARK_X("SUBDICT",dictionary);
        SWEEP();
        printf("Subgraph for word \"%s\" created.\n\n",w);
        free(w);
        stop = FALSE;
      } else if (toupper(userinput[0]) == 'L') {
        unsigned char *w = (unsigned char *) calloc(maxwordlength+1,sizeof(unsigned char));
        unsigned int dist;
        printf("Levenshtein for word: ");
        while (!(scanf("%s",w)) || (strlen((char *)w) > maxwordlength)) {
          printf("Enter valid word: ");
        }
        printf("Max Distance: ");
        while (!(scanf("%u",&dist)) || dist > maxwordlength) {
          printf("Enter valid distance: ");
        }
        dictionary = subgraphLevenshtein((Biddy_String)w,dist,dictionary,alphabet,vartable,usenullsymbol,nullsymbol,code,codenot,alphabetsize,maxwordlength);
        dictionary_idx = MARK_X("SUBDICT",dictionary);
        SWEEP();
        printf("Subgraph for word \"%s\" created.\n\n",w);
        free(w);
        stop = FALSE;
      } else if (toupper(userinput[0]) == 'R') {
        Biddy_String template;
        n = 0;
        for (i = 0; (i < 10) && i < maxwordlength; i++) {
          printf("[%u]",i+1);
          for (j = 0; j < maxwordlength; j++) {
            template = readLevenshteinTemplate(i+1,j+1,alphabetsize,maxwordlength);
            if (template) {
              templatetable[i*maxwordlength+j] = template;
              n++;
            } else {
              templatetable[i*maxwordlength+j] = NULL;
            }
          }
        }
        printf("\n%u templates loaded\n",n);
        printf("\n");
        stop = FALSE;
      } else if (toupper(userinput[0]) == 'I') {
        unsigned int tgraph_idx;
        Biddy_Edge tgraph;
        unsigned int wordlength;
        printf("Enter word length: ");
        while (!(scanf("%u",&wordlength)) || !wordlength || (wordlength > maxwordlength)) {
          printf("Enter valid length: ");
        }
        printf("\nList of known templates:\n");
        i = wordlength-1;
        for (j = 0; j < maxwordlength; j++) {
          printf("Levenshtein template for word length %u and distance %u: ",i+1,j+1);
          if (templatetable[i*maxwordlength+j]) {
            tgraph = Biddy_GetConstantZero();
            Biddy_FindFormula(templatetable[i*maxwordlength+j],&tgraph_idx,&tgraph);
            printf("%u nodes, %.0f patterns\n",Biddy_CountNodes(tgraph),Biddy_CountMinterms(tgraph,0));
          } else {
            printf("NULL\n");
          }
        }
        stop = FALSE;
      } else if (toupper(userinput[0]) == 'W') {
        unsigned int tgraph_idx;
        Biddy_Edge tgraph;
        unsigned int wordlength,dist;
        unsigned char *word = (unsigned char *) calloc(maxwordlength+1,sizeof(unsigned char));
        unsigned int wordnum = 0;
        Biddy_String *wordlist = (Biddy_String *) malloc(sizeof(Biddy_String));
        Biddy_String *result = NULL;
        double num;
        printf("Levenshtein template for word length: ");
        while (!(scanf("%u",&wordlength)) || !wordlength || (wordlength > maxwordlength)) {
          printf("Enter valid length: ");
        }
        printf("Enter distance: ");
        while (!(scanf("%u",&dist)) || !dist || (dist > maxwordlength)) {
          printf("Enter valid distance: ");
        }
        if (templatetable[(wordlength-1)*maxwordlength+(dist-1)]) {
          tgraph = Biddy_GetConstantZero();
          Biddy_FindFormula(templatetable[(wordlength-1)*maxwordlength+(dist-1)],&tgraph_idx,&tgraph);
        } else {
          tgraph = Biddy_GetConstantZero();
        }
        num = Biddy_CountMinterms(tgraph,0);
        if (num > 1024) {
          printf("\nThere are %.0f patterns - to many to report them!\n",num);
        } else {
          for (i = 0; i < maxwordlength; i++) {
            word[i] = '*';
          }
          wordlist[0] = NULL;
          writeTemplate(tgraph,vartable,word,&wordnum,&wordlist);
          if (wordnum == 1) {
            printf("\nThere is %u pattern:\n",wordnum);
          } else {
            printf("\nThere are %u patterns:\n",wordnum);
          }
          result = wordlist;
          while (result[0]) {
            printf("%s\n",result[0]);
            free(result[0]);
            result++;
          }
        }
        printf("\n");
        free(wordlist);
        free(word);
        stop = FALSE;
      } else {
        stop = FALSE;
      }

      elapsedtime = clock()-elapsedtime;

    } while (!stop);

    free(userinput);
  }

  free(vartable);

  /* EXIT */
  Biddy_Exit();
}


/*******************************************************************************
\brief Function allWords returns list of all words in the given dictionary.

### Description
### Side effects
### More info
*******************************************************************************/

void
allWords(Biddy_Edge dictionary, VarTable *vartable, unsigned char word[],
  Biddy_Boolean usenullsymbol, unsigned char nullsymbol,
  unsigned int *wordnum, Biddy_String **wordlist)
{
  Biddy_Variable v;
  unsigned char c,old;

  if (Biddy_IsTerminal(dictionary)) {
    if (!Biddy_GetMark(dictionary)) {
      /* printf("WORD COMPLETED: %s\n",word); */
      (*wordlist)[*wordnum] = strdup((char *)word);
      (*wordnum)++;
      *wordlist = (Biddy_String *) realloc(*wordlist, (*wordnum+1) * sizeof(Biddy_String));
      (*wordlist)[*wordnum] = NULL;
    }
  } else {
    /* it could be easier but we want to have a correct order of words */
    v = Biddy_GetTopVariable(dictionary);
    c = vartable[v].x;
    if (usenullsymbol && isNullSymbol(v,vartable,nullsymbol)) c = 0; /* do not print null symbols */
    old = word[vartable[v].y-1];
    word[vartable[v].y-1] = c;
    allWords(Biddy_TransferMark(Biddy_GetThen(dictionary),Biddy_GetMark(dictionary),FALSE),
             vartable,word,usenullsymbol,nullsymbol,wordnum,wordlist);
    word[vartable[v].y-1] = old;
    allWords(Biddy_TransferMark(Biddy_GetElse(dictionary),Biddy_GetMark(dictionary),TRUE),
             vartable,word,usenullsymbol,nullsymbol,wordnum,wordlist);
  }
  return;
}

/*******************************************************************************
\brief Function patternWords returns list of all words in the given dictionary
        which correspond to the given pattern.

### Description
### Side effects
### More info
*******************************************************************************/

void
patternWords(Biddy_Edge dictionary, Biddy_String pattern, VarTable *vartable,
  unsigned char word[], Biddy_Boolean usenullsymbol, unsigned char nullsymbol,
  unsigned int *wordnum, Biddy_String **wordlist)
{
  Biddy_Variable v;
  unsigned char p,c,old;

  if (Biddy_IsTerminal(dictionary)) {
    if (!Biddy_GetMark(dictionary)) {
      /* printf("WORD COMPLETED: %s\n",word); */
      (*wordlist)[*wordnum] = strdup((char *)word);
      (*wordnum)++;
      *wordlist = (Biddy_String *) realloc(*wordlist, (*wordnum+1) * sizeof(Biddy_String));
      (*wordlist)[*wordnum] = NULL;
    }
  } else {
    /* it could be easier but we want to have a correct order of words */
    v = Biddy_GetTopVariable(dictionary);
    c = vartable[v].x;
    if (usenullsymbol && isNullSymbol(v,vartable,nullsymbol)) c = 0; /* do not print null symbols */
    if (vartable[v].y > strlen(pattern)) {
      p = 0;
    } else {
      p = pattern[vartable[v].y-1];
    }
    if (((p == '*') && c) || (p == c)) {
      old = word[vartable[v].y-1];
      word[vartable[v].y-1] = c;
      patternWords(Biddy_TransferMark(Biddy_GetThen(dictionary),Biddy_GetMark(dictionary),FALSE),
                   pattern,vartable,word,usenullsymbol,nullsymbol,wordnum,wordlist);
      word[vartable[v].y-1] = old;
    }
    patternWords(Biddy_TransferMark(Biddy_GetElse(dictionary),Biddy_GetMark(dictionary),TRUE),
                 pattern,vartable,word,usenullsymbol,nullsymbol,wordnum,wordlist);
  }
  return;
}

/*******************************************************************************
\brief Function writeTemplate returns list of all words in the given template.

### Description
### Side effects
### More info
*******************************************************************************/

void
writeTemplate(Biddy_Edge templ, VarTable *vartable,
  unsigned char word[], unsigned int *wordnum, Biddy_String **wordlist)
{
  Biddy_Variable v;
  unsigned char c,old;

  if (Biddy_IsTerminal(templ)) {
    if (!Biddy_GetMark(templ)) {
      /* printf("WORD COMPLETED: %s\n",word); */
      (*wordlist)[*wordnum] = strdup((char *)word);
      (*wordnum)++;
      *wordlist = (Biddy_String *) realloc(*wordlist, (*wordnum+1) * sizeof(Biddy_String));
      (*wordlist)[*wordnum] = NULL;
    }
  } else {
    /* it could be easier but we want to have a correct order of words */
    v = Biddy_GetTopVariable(templ);
    c = vartable[v].x;
    old = word[vartable[v].y-1];
    word[vartable[v].y-1] = c;
    writeTemplate(Biddy_TransferMark(Biddy_GetThen(templ),Biddy_GetMark(templ),FALSE),
             vartable,word,wordnum,wordlist);
    word[vartable[v].y-1] = old;
    writeTemplate(Biddy_TransferMark(Biddy_GetElse(templ),Biddy_GetMark(templ),TRUE),
             vartable,word,wordnum,wordlist);
  }
  return;
}

/*******************************************************************************
\brief Function subgraphPattern creates a sub-dictionary of all words which
        correspond to the given pattern.

### Description
### Side effects
### More info
*******************************************************************************/

Biddy_Edge
subgraphPattern(Biddy_String pattern, Biddy_Edge dictionary, unsigned char alphabet[],
  VarTable *vartable, Biddy_Boolean usenullsymbol, unsigned char nullsymbol,
  Biddy_Edge code[], Biddy_Edge codenot[], unsigned int alphabetsize,
  unsigned int maxwordlength)
{
  Biddy_Edge pgraph,cube,result;
  unsigned char p[32];
  unsigned int i,j;

  for (i=0; i<strlen(pattern); i++) {
    if (pattern[i] == '*') {
      p[i] = alphabet[1]; /* symbol * may not be included into alphabet */
    } else {
      p[i] = pattern[i];
    }
  }
  p[i] = 0;

  /* DEBUGGING */
  /*
  printf("\nCREATING WORD %s FOR PATTERN %s\n",p,pattern);
  */

  pgraph = encodeOneWord(p,alphabet,usenullsymbol,nullsymbol,code,codenot,alphabetsize,maxwordlength);

  /* DEBUGGING */
  /*
  printf("\nDICTIONARY FOR WORD %s:\n",p);
  reportDictionary(pgraph,alphabetsize,maxwordlength);
  */

  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
    cube = Biddy_GetBaseSet(); else cube = Biddy_GetUniversalSet();
  for (i=0; i<strlen(pattern); i++) {
    if (pattern[i] == '*') {
      for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
        if (((vartable[j].y-1) == i) && (!usenullsymbol || !isNullSymbol(j,vartable,nullsymbol))) {
          if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
            cube = Biddy_Change(cube,j); else cube = Biddy_And(cube,Biddy_GetVariableEdge(j));
          /* printf("<%s>",Biddy_GetVariableName(j)); */
        }
      }
    }
  }
  pgraph = Biddy_ExistAbstract(pgraph,cube);

  /* DEBUGGING */
  /*
  printf("\nDICTIONARY FOR WORD %s AFTER EXTRACT:\n",p);
  reportDictionary(pgraph,alphabetsize,maxwordlength);
  */

  result = Biddy_Intersect(dictionary,pgraph);
  return result;
}

/*******************************************************************************
\brief Function subgraphHamming creates a subdictionary of all words which
        can be obtained by a given number of substitutions.

### Description
### Side effects
### More info
*******************************************************************************/

Biddy_Edge
subgraphHamming(Biddy_String word, unsigned int dist, Biddy_Edge dictionary,
  unsigned char alphabet[], VarTable *vartable, Biddy_Boolean usenullsymbol,
  unsigned char nullsymbol, Biddy_Edge code[], Biddy_Edge codenot[],
  unsigned int alphabetsize, unsigned int maxwordlength)
{
  Biddy_Edge hgraph,result;
  unsigned int i;

  /* DEBUGGING */
  /*
  printf("\nHAMMING FOR WORD %s\n",word);
  */

  hgraph = encodeOneWord((unsigned char *)word,alphabet,usenullsymbol,nullsymbol,code,codenot,alphabetsize,maxwordlength);
  printf("Size of hgraph/0 = %u\n",Biddy_CountNodes(hgraph));

  /* DEBUGGING */
  /*
  Biddy_WriteBddview("begin.bddview",hgraph,"BEGIN",NULL);
  */

  for (i=1; i<=dist; i++) {
    hgraph = oneSubstitutionFull(hgraph,vartable,usenullsymbol,nullsymbol,alphabetsize,maxwordlength);
    printf("Size of hgraph/%u = %u\n",i,Biddy_CountNodes(hgraph));
  }

  /* DEBUGGING */
  /*
  Biddy_WriteBddview("after.bddview",hgraph,"AFTER",NULL);
  */

  result = Biddy_Intersect(dictionary,hgraph);
  return result;
}

/*******************************************************************************
\brief Function subgraphLcs creates a subdictionary of all words which
        can be obtained by a given number of insertions and deletions.

### Description
### Side effects
### More info
*******************************************************************************/

Biddy_Edge
subgraphLcs(Biddy_String word, unsigned int dist, Biddy_Edge dictionary,
  unsigned char alphabet[], VarTable *vartable, Biddy_Boolean usenullsymbol,
  unsigned char nullsymbol, Biddy_Edge code[], Biddy_Edge codenot[],
  unsigned int alphabetsize, unsigned int maxwordlength)
{
  Biddy_Edge lcsgraph,igraph,dgraph,result;
  unsigned int i;

  /* DEBUGGING */
  /*
  printf("\nLCS FOR WORD %s\n",word);
  */

  if (!usenullsymbol) {
    printf("Without null symbol LCS is not possible\n");
    return(dictionary);
  }

  lcsgraph = encodeOneWord((unsigned char *)word,alphabet,usenullsymbol,nullsymbol,code,codenot,alphabetsize,maxwordlength);
  printf("Size of lcsgraph/0 = %u\n",Biddy_CountNodes(lcsgraph));

  /* DEBUGGING */
  /*
  Biddy_WriteBddview("begin.bddview",lcsgraph,"BEGIN",NULL);
  */

  for (i=1; i<=dist; i++) {
    igraph = oneInsertionFull(lcsgraph,alphabet,vartable,nullsymbol,alphabetsize,maxwordlength);
    printf("Size of igraph/%u = %u\n",i,Biddy_CountNodes(igraph));
    dgraph = oneDeletion(lcsgraph,alphabet,vartable,nullsymbol,alphabetsize,maxwordlength);
    printf("Size of dgraph/%u = %u\n",i,Biddy_CountNodes(dgraph));
    lcsgraph = Biddy_Union(igraph,dgraph);
    printf("Size of lcsgraph/%u = %u\n",i,Biddy_CountNodes(lcsgraph));
  }

  /* DEBUGGING */
  /*
  Biddy_WriteBddview("after.bddview",lcsgraph,"AFTER",NULL);
  */

  result = Biddy_Intersect(dictionary,lcsgraph);
  return result;
}

/*******************************************************************************
\brief Function subgraphLevenshtein creates a subdictionary of all words which
        can be obtained by a given number of substitutions, insertions, and
        deletions.

### Description
### Side effects
### More info
*******************************************************************************/

/* DIRECTCALCULATION is slower and is only used for testing */
#define NODIRECTCALCULATION

Biddy_Edge
subgraphLevenshtein(Biddy_String word, unsigned int dist,
  Biddy_Edge dictionary, unsigned char alphabet[], VarTable *vartable,
  Biddy_Boolean usenullsymbol, unsigned char nullsymbol,
  Biddy_Edge code[], Biddy_Edge codenot[],
  unsigned int alphabetsize, unsigned int maxwordlength)
{
  Biddy_Edge lgraph,hgraph,igraph,dgraph,result;
  unsigned int wordlength;
  Biddy_Boolean exist;
  unsigned char nonnullsymbol;
  unsigned int i,j;

  /* DEBUGGING */
  /*
  printf("\nLEVENSHTEIN FOR WORD %s\n",word);
  */

  if (!usenullsymbol) {
    printf("Without null symbol Levenshtein is not implemented\n");
    return(dictionary);
  }

  wordlength = (unsigned int) strlen(word);
  lgraph = encodeOneWord((unsigned char *)word,alphabet,usenullsymbol,nullsymbol,code,codenot,alphabetsize,maxwordlength);

  /* DEBUGGING */
  /*
  Biddy_WriteBddview("begin.bddview",lgraph,"BEGIN",NULL);
  */

#ifdef DIRECTCALCULATION

  printf("Preparing direct calculation for word \"%s\" ...\n",word);

  printf("Size of lgraph/0 = %u\n",Biddy_CountNodes(lgraph));
  /* reportSystem(); */ /* PROFILING */

  for (i=1; i<=dist; i++) {
    if (i > wordlength) {
      igraph = oneInsertionFull(lgraph,alphabet,vartable,nullsymbol,alphabetsize,maxwordlength);
      lgraph = igraph;
    } else {
      hgraph = oneSubstitutionFull(lgraph,vartable,usenullsymbol,nullsymbol,alphabetsize,maxwordlength);
      printf("hgraph/%u = %u, ",i,Biddy_CountNodes(hgraph));
      /* reportSystem(); */ /* PROFILING */
      igraph = oneInsertionFull(lgraph,alphabet,vartable,nullsymbol,alphabetsize,maxwordlength);
      printf("igraph/%u = %u, ",i,Biddy_CountNodes(igraph));
      /* reportSystem(); */ /* PROFILING */
      dgraph = oneDeletion(lgraph,alphabet,vartable,nullsymbol,alphabetsize,maxwordlength);
      printf("dgraph/%u = %u, ",i,Biddy_CountNodes(dgraph));
      /* reportSystem(); */ /* PROFILING */
      lgraph = Biddy_Union(lgraph,hgraph);
      lgraph = Biddy_Union(lgraph,igraph);
      lgraph = Biddy_Union(lgraph,dgraph);
    }

    MARK(lgraph);
    SWEEP();

    printf("lgraph/%u = %u",i,Biddy_CountNodes(lgraph));
    /* reportSystem(); */ /* PROFILING */

    /* TESTING */
    /*
    if (lgraph == igraph) printf(", IGRAPH FINAL");
    */

    printf("\n");
  }

#else

  i = 0;
  exist = TRUE;
  while (exist) {
    exist = FALSE;
    for (j=0; !exist && j<wordlength; j++) {
      if (word[j] == alphabet[i]) exist = TRUE;
    }
    if (exist) i++;
  }
  nonnullsymbol = alphabet[i];

  printf("Preparing template for word \"%s\", nonnullsymbol = \"%c\" ...\n",word,nonnullsymbol);

  printf("Size of lgraph/0 = %u\n",Biddy_CountNodes(lgraph));
  /* reportSystem(); */ /* PROFILING */

  for (i=1; i<=dist; i++) {
    if (i > wordlength) {
      igraph = oneInsertion(lgraph,alphabet,vartable,nonnullsymbol,nullsymbol,alphabetsize,maxwordlength);
      lgraph = igraph;
    } else {
      hgraph = oneSubstitution(lgraph,vartable,nonnullsymbol,usenullsymbol,nullsymbol,alphabetsize,maxwordlength);
      printf("hgraph/%u = %u, ",i,Biddy_CountNodes(hgraph));
      /* reportSystem(); */ /* PROFILING */
      igraph = oneInsertion(lgraph,alphabet,vartable,nonnullsymbol,nullsymbol,alphabetsize,maxwordlength);
      printf("igraph/%u = %u, ",i,Biddy_CountNodes(igraph));
      /* reportSystem(); */ /* PROFILING */
      dgraph = oneDeletion(lgraph,alphabet,vartable,nullsymbol,alphabetsize,maxwordlength);
      printf("dgraph/%u = %u, ",i,Biddy_CountNodes(dgraph));
      /* reportSystem(); */ /* PROFILING */
      lgraph = Biddy_Union(lgraph,hgraph);
      lgraph = Biddy_Union(lgraph,igraph);
      lgraph = Biddy_Union(lgraph,dgraph);
    }

    MARK(lgraph);
    SWEEP();

    printf("lgraph/%u = %u",i,Biddy_CountNodes(lgraph));
    /* reportSystem(); */ /* PROFILING */

    /* TESTING */
    /*
    if (lgraph == igraph) printf(", IGRAPH FINAL");
    */

    printf("\n");
  }

  /* DEBUGGING */
  /*
  Biddy_WriteBddview("lgraph.bddview",lgraph,"LGRAPH",NULL);
  */

  MARK(lgraph);
  SWEEP();
  lgraph = extendNonNullSymbol(lgraph,alphabet,vartable,nonnullsymbol,usenullsymbol,nullsymbol,alphabetsize,maxwordlength);
  MARK(lgraph);
  SWEEP();

#endif

  printf("Final size of lgraph/%u = %u\n",dist,Biddy_CountNodes(lgraph));
  printf("Final lgraph represents %.0f words\n",Biddy_CountMinterms(lgraph,maxwordlength*alphabetsize));

  /* DEBUGGING */
  /*
  Biddy_WriteBddview("after.bddview",lgraph,"AFTER",NULL);
  */

  printf("Calculating result...\n");

  result = Biddy_Intersect(dictionary,lgraph);
  MARK(result);
  SWEEP();

  return result;
}

/*******************************************************************************
\brief Function readLevenshteinTemplate.

### Description
### Side effects
### More info
*******************************************************************************/

Biddy_String
readLevenshteinTemplate(unsigned int wordlength, unsigned int dist,
  unsigned int alphabetsize, unsigned int maxwordlength)
{
  char name[255];
  Biddy_String bddnamelist,bddname;

  if (Biddy_GetManagerType() == BIDDYTYPEOBDD) {
    sprintf(name,"bddscout/DICTIONARY/levenshtein-template-OBDD-%u-%u/levenshtein-template-OBDD-%u-%u-%u-%u.bddview",
            alphabetsize,maxwordlength,alphabetsize,maxwordlength,wordlength,dist);
  } else if (Biddy_GetManagerType() == BIDDYTYPEOBDDC) {
    sprintf(name,"bddscout/DICTIONARY/levenshtein-template-OBDDC-%u-%u/levenshtein-template-OBDDC-%u-%u-%u-%u.bddview",
            alphabetsize,maxwordlength,alphabetsize,maxwordlength,wordlength,dist);
  } else if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
    sprintf(name,"bddscout/DICTIONARY/levenshtein-template-ZBDD-%u-%u/levenshtein-template-ZBDD-%u-%u-%u-%u.bddview",
            alphabetsize,maxwordlength,alphabetsize,maxwordlength,wordlength,dist);
  } else if (Biddy_GetManagerType() == BIDDYTYPEZBDDC) {
    sprintf(name,"bddscout/DICTIONARY/levenshtein-template-ZBDDC-%u-%u/levenshtein-template-ZBDDC-%u-%u-%u-%u.bddview",
            alphabetsize,maxwordlength,alphabetsize,maxwordlength,wordlength,dist);
  } else if (Biddy_GetManagerType() == BIDDYTYPETZBDD) {
    sprintf(name,"bddscout/DICTIONARY/levenshtein-template-TZBDD-%u-%u/levenshtein-template-TZBDD-%u-%u-%u-%u.bddview",
            alphabetsize,maxwordlength,alphabetsize,maxwordlength,wordlength,dist);
  } else if (Biddy_GetManagerType() == BIDDYTYPETZBDDC) {
    sprintf(name,"bddscout/DICTIONARY/levenshtein-template-TZBDDC-%u-%u/levenshtein-template-TZBDDC-%u-%u-%u-%u.bddview",
            alphabetsize,maxwordlength,alphabetsize,maxwordlength,wordlength,dist);
  }

  bddnamelist = Biddy_ReadBddview(name,NULL);
  if (bddnamelist) {
    bddname = strdup(strtok(bddnamelist," "));
  } else {
    bddname = NULL;
  }

  free(bddnamelist);

  return bddname;
}

/*******************************************************************************
\brief Function useLevenshteinTemplate.

### Description
### Side effects
    The idea of this function is not finished, yet. It is working for
    words with all different letters. It is not working if the same letter
    is present in the word twice.
### More info
*******************************************************************************/

#define NOTASCII

Biddy_Edge
useLevenshteinTemplate(Biddy_String word, unsigned int dist,
  Biddy_String *templatetable, unsigned char alphabet[],
  VarTable *vartable, unsigned char nullsymbol,
  unsigned int alphabetsize, unsigned int maxwordlength)
{
  Biddy_Edge tgraph;
  unsigned int tgraph_idx;
  unsigned int wordlength;
  Biddy_String oldword,newword,extraword;
  unsigned char nonnullsymbol,newnonnullsymbol;
  Biddy_Boolean exist;
  Biddy_Edge *nextcode;
  unsigned int i,j,k;

  nextcode = (Biddy_Edge *) calloc(alphabetsize*maxwordlength+1,sizeof(Biddy_Edge));

  wordlength = (unsigned int) strlen(word);

#ifdef ASCII
  oldword = createPatternAscii(alphabet,wordlength);
  newword = strdup(word);
  nonnullsymbol = 'A' + alphabet[wordlength];
#else
  oldword = createPattern(alphabet,wordlength);
  newword = strdup(word);
  nonnullsymbol = alphabet[wordlength];
#endif

  oldword = (Biddy_String) realloc(oldword,(2*(wordlength+1)+1)*sizeof(char));
  oldword[2*(wordlength+1)] = 0;

  newword = (Biddy_String) realloc(newword,(2*(wordlength+1)+1)*sizeof(char));
  newword[2*(wordlength+1)] = 0;

  oldword[wordlength] = nonnullsymbol;
  i = wordlength+1;
  for (j=0; j<wordlength; j++) {
    exist = FALSE;
    for (k=0; !exist && (k<i); k++) {
      if (oldword[k] == word[j]) exist = TRUE;
    }
    if (!exist) oldword[i++] = word[j];
  }
  while (i <= 2*(wordlength+1)) oldword[i++] = 0;

  i = wordlength;
  for (j=0; j<=wordlength; j++) {
    exist = FALSE;
    for (k=0; !exist && (k<wordlength); k++) {
      if (newword[k] == oldword[j]) exist = TRUE;
    }
    if (!exist) newword[i++] = oldword[j];
  }
  while (i <= 2*(wordlength+1)) newword[i++] = 0;

  if (strlen(newword) < strlen(oldword)) {
    printf("INTERNAL ERROR: WRONG strlen\n");
    exit(1);
  }

  if (strlen(newword) == strlen(oldword)) {
    extraword = strdup("");
  } else if (strlen(newword) > strlen(oldword)) {
    extraword = strdup(&newword[strlen(oldword)]);
    newword[strlen(oldword)] = 0;
  }

  newnonnullsymbol = newword[wordlength];

  printf("(\"%s\" -> \"%s\", newnonnullsymbol = \"%c\", extraword = \"%s\")...\n",oldword,newword,newnonnullsymbol,extraword);

  tgraph = Biddy_GetConstantZero();
  Biddy_FindFormula(templatetable[(wordlength-1)*maxwordlength+(dist-1)],&tgraph_idx,&tgraph);

  wordlength = (unsigned int) strlen(oldword);

  /* DEBUGGING */
  /**/
  {
  unsigned char *tmpword = (unsigned char *) calloc(maxwordlength+1,sizeof(unsigned char));
  unsigned int wordnum = 0;
  Biddy_String *wordlist = (Biddy_String *) malloc(sizeof(Biddy_String));
  Biddy_String *result = NULL;
  double num;
  num = Biddy_CountMinterms(tgraph,0);
  if (num > 1024) {
    printf("\nThere are %.0f patterns - to many to report them!\n",num);
  } else {
    for (i = 0; i < maxwordlength; i++) {
      tmpword[i] = '*';
    }
    wordlist[0] = NULL;
    writeTemplate(tgraph,vartable,tmpword,&wordnum,&wordlist);
    if (wordnum == 1) {
      printf("\nThere is %u pattern:\n",wordnum);
    } else {
      printf("\nThere are %u patterns:\n",wordnum);
    }
    result = wordlist;
    while (result[0]) {
      printf("%s\n",result[0]);
      free(result[0]);
      result++;
    }
  }
  printf("\n");
  free(wordlist);
  free(tmpword);
  }
  /**/

  Biddy_ResetVariablesValue();
  for (i=0; i<wordlength; i++) {
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].x == newword[i]) {
        /* DEBUGGING */
        /*
        printf("rename \"%c%u\" -> \"%s\"\n",oldword[i],vartable[j].y,Biddy_GetVariableName(j));
        */
        /* Biddy_Replace is controlled by variable's values (which are edges!). */
        /* for OBDDs and TZBDDs control values must be variables, but for ZBDDs they must be elements! */
        nextcode[alphabetsize*(vartable[j].y-1)+getLetterIndex(alphabet,alphabetsize,oldword[i])] =
          ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) ?
            Biddy_GetElementEdge(j) : Biddy_GetVariableEdge(j);
      }
    }
  }
  for (i=1; i<(alphabetsize*maxwordlength+1); i++) {
    if (vartable[i].x != nullsymbol) {
      if (nextcode[alphabetsize*(vartable[i].y-1)+getLetterIndex(alphabet,alphabetsize,vartable[i].x)]) {
        Biddy_SetVariableValue(i,nextcode[alphabetsize*(vartable[i].y-1)+getLetterIndex(alphabet,alphabetsize,vartable[i].x)]);
        /* DEBUGGING */
        /*
        printf("rename \"%s\"...",Biddy_GetVariableName(i));
        Biddy_PrintfBDD(Biddy_GetVariableValue(i));
        */
      }
    }
  }

  /* DEBUGGING */
  /*
  Biddy_WriteBddview("replacebefore.bddview",tgraph,"BEFORE",NULL);
  */

  tgraph = Biddy_Replace(tgraph);

  /* DEBUGGING */
  /*
  Biddy_WriteBddview("replacemiddle.bddview",tgraph,"AFTER",NULL);
  */

  for (i=0; i < strlen(extraword); i++) {
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].x == extraword[i]) {
        tgraph = Biddy_Diff(tgraph,Biddy_GetVariableEdge(j));
      }
    }
  }

  /* DEBUGGING */
  /*
  Biddy_WriteBddview("replaceafter.bddview",tgraph,"AFTER",NULL);
  */

  /* DEBUGGING */
  /**/
  {
  unsigned char *tmpword = (unsigned char *) calloc(maxwordlength+1,sizeof(unsigned char));
  unsigned int wordnum = 0;
  Biddy_String *wordlist = (Biddy_String *) malloc(sizeof(Biddy_String));
  Biddy_String *result = NULL;
  double num;
  num = Biddy_CountMinterms(tgraph,0);
  if (num > 1024) {
    printf("\nThere are %.0f patterns - to many to report them!\n",num);
  } else {
    for (i = 0; i < maxwordlength; i++) {
      tmpword[i] = '*';
    }
    wordlist[0] = NULL;
    writeTemplate(tgraph,vartable,tmpword,&wordnum,&wordlist);
    if (wordnum == 1) {
      printf("\nThere is %u pattern:\n",wordnum);
    } else {
      printf("\nThere are %u patterns:\n",wordnum);
    }
    result = wordlist;
    while (result[0]) {
      printf("%s\n",result[0]);
      free(result[0]);
      result++;
    }
  }
  printf("\n");
  free(wordlist);
  free(tmpword);
  }
  /**/

  free(nextcode);
  free(oldword);
  free(newword);

  return tgraph;
}
