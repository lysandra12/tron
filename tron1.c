/*****************************************************************************/
/*									     */
/*				     tron1.c				     */
/*									     */
/*  Programa inicial d'exemple per a les practiques 2 de FSO   	             */
/*     Es tracta del joc del tron: sobre un camp de joc rectangular, es      */
/*     mouen uns objectes que anomenarem 'trons' (amb o tancada). En aquesta */
/*     primera versio del joc, nomes hi ha un tron que controla l'usuari, i  */
/*     que representarem amb un '0', i un tron que controla l'ordinador, el  */
/*     qual es representara amb un '1'. Els trons son una especie de 'motos' */
/*     que quan es mouen deixen rastre (el caracter corresponent). L'usuari  */
/*     pot canviar la direccio de moviment del seu tron amb les tecles:      */
/*     'w' (adalt), 's' (abaix), 'd' (dreta) i 'a' (esquerra). El tron que   */
/*     controla l'ordinador es moura aleatoriament, canviant de direccio     */
/*     aleatoriament segons un parametre del programa (veure Arguments).     */
/*     El joc consisteix en que un tron intentara 'tancar' a l'altre tron.   */
/*     El primer tron que xoca contra un obstacle (sigui rastre seu o de     */
/*     l'altre tron), esborrara tot el seu rastre i perdra la partida.       */
/*									     */
/*  Arguments del programa:						     */
/*     per controlar la variabilitat del canvi de direccio, s'ha de propor-  */
/*     cionar com a primer argument un numero del 0 al 3, el qual indicara   */
/*     si els canvis s'han de produir molt frequentment (3 es el maxim) o    */
/*     poc frequentment, on 0 indica que nomes canviara per esquivar les     */
/*     parets.								     */
/*									     */
/*     A mes, es podra afegir un segon argument opcional per indicar el      */
/*     retard de moviment del menjacocos i dels fantasmes (en ms);           */
/*     el valor per defecte d'aquest parametre es 100 (1 decima de segon).   */
/*									     */
/*  Compilar i executar:					  	     */
/*     El programa invoca les funcions definides a "winsuport.c", les        */
/*     quals proporcionen una interficie senzilla per crear una finestra     */
/*     de text on es poden escriure caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses':				     */
/*									     */
/*	   $ gcc -c winsuport.c -o winsuport.o			     	     */
/*        gcc -c semafor.c -o semafor.o */
/*        gcc -c winsuport2.c -o winsuport2.o */
/*	   $ gcc tron1.c winsuport.o semafor.o -o tron1 -lcurses			     */
/*	   $ ./tron0 variabilitat [retard]				     */
/*									     */
/*  Codis de retorn:						  	     */
/*     El programa retorna algun dels seguents codis al SO:		     */
/*	0  ==>  funcionament normal					     */
/*	1  ==>  numero d'arguments incorrecte 				     */
/*	2  ==>  no s'ha pogut crear el camp de joc (no pot iniciar CURSES)   */
/*	3  ==>  no hi ha prou memoria per crear les estructures dinamiques   */
/*									     */
/*****************************************************************************/

#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <unistd.h>
#include "winsuport.h"		/* incloure definicions de funcions propies */
#include <sys/wait.h>     //new
#include <time.h>
#include "semafor.h"


				/* definir estructures d'informacio */
typedef struct {		/* per un tron (usuari o oponent) */
	int f;				/* posicio actual: fila */
	int c;				/* posicio actual: columna */
	int d;				/* direccio actual: [0..3] */
} tron;

typedef struct {		/* per una entrada de la taula de posicio */
	int f;
	int c;
} pos;

/* variables globals */

int n_fil, n_col;		/* dimensions del camp de joc */
int fi1 = 0, fi2 = 0;  // condicions final

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */

int varia;		/* valor de variabilitat dels oponents [0..9] */
int retard;		/* valor del retard de moviment, en mil.lisegons */

tron usu;   	   		/* informacio de l'usuari */
pos *p_usu;			/* taula de posicions que van recorrent */
int n_usu = 0;  /* numero d'entrades en les taules de pos. */

#define num_trons 4
tron opo[num_trons];    // suposem màxim 10 oponents
pos *p_opo[num_trons];   // taules de posicions per a cada oponent
int n_opo[num_trons];        // nombre de posicions per a cada oponent

#define min_retard 100
#define max_retard 300

FILE *f;
int sem_tabler, sem_fixer;

/* funcio per esborrar totes les posicions anteriors, sigui de l'usuari o */
/* de l'oponent */
void esborrar_posicions(pos p_pos[], int n_pos)
{
  int i;
  
  for (i=n_pos-1; i>=0; i--)		/* de l'ultima cap a la primera */
  {
    win_escricar(p_pos[i].f,p_pos[i].c,' ',NO_INV);	/* esborra una pos. */
    win_retard(10);		/* un petit retard per simular el joc real */
  }
}

/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(void)
{
  char strin[45];

  usu.f = (n_fil-1)/2;
  usu.c = (n_col)/4;		/* fixa posicio i direccio inicial usuari */
  usu.d = 3;
    waitS(sem_tabler);
  win_escricar(usu.f,usu.c,'0',INVERS);	/* escriu la primer posicio usuari */
   signalS(sem_tabler);
  p_usu[n_usu].f = usu.f;		/* memoritza posicio inicial */
  p_usu[n_usu].c = usu.c;
  n_usu++;

  // Inicialitza tots els oponents disponibles 
  for (int index = 0; index < num_trons; index++) { 
    opo[index].f = (n_fil/(index+2)) ;
    opo[index].c = (n_col*3)/4;		
    opo[index].d = 1;
    
    waitS(sem_tabler);
    win_escricar(opo[index].f, opo[index].c, '1' + index, INVERS);	/* escriu la primer posicio oponent */ 
    signalS(sem_tabler);
    
    p_opo[index][n_opo[index]].f = opo[index].f;	
    p_opo[index][n_opo[index]].c = opo[index].c;
    n_opo[index]++;	
    
    waitS(sem_fixer);
    fprintf(f, "tron inizialitzat %d!\n", index);
    signalS(sem_fixer);
  }

  sprintf(strin,"Tecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  win_escristr(strin);
}

void mou_oponent(int index) {  
  char c;
  tron seg;
  int k, vk, nd, vd[3];
  int varia = 1;
  int canvi = 0;
  srand(time(NULL));		/* inicialitza numeros aleatoris */

  while (!fi1 && !fi2) {   
    seg.f = opo[index].f + df[opo[index].d]; /* calcular seguent posicio */
    seg.c = opo[index].c + dc[opo[index].d]; 
    c = win_quincar(seg.f, seg.c);    /* calcula caracter seguent posicio */

    if (c != ' ') canvi = 1; /* si seguent posicio ocupada anotar que s'ha de produir un canvi de direccio*/
    else if ((varia > 0) && (rand() % 10 < varia)) canvi = 1; /* si hi ha variabilitat possible canvi de direccio */

    if (canvi) { /* si s'ha de canviar de direccio */
      nd = 0;
      for (k = -1; k <= 1; k++) { /* provar direccio actual i dir. veines */
        vk = (opo[index].d + k + 4) % 4; /* nova direccio */
        seg.f = opo[index].f + df[vk]; /* corregeix negatius */
        seg.c = opo[index].c + dc[vk]; /* calcular posicio en la nova dir.*/
        waitS(sem_tabler);
            c = win_quincar(seg.f, seg.c); /* calcula caracter seguent posicio */
        signalS(sem_tabler);

         if (c == ' ') vd[nd++] = vk; /* memoritza com a direccio possible */
      }
      if (nd == 0) { 	/* si no pot continuar, */
        waitS(sem_tabler);
        esborrar_posicions(p_opo[index], n_opo[index]);
        signalS(sem_tabler);

        waitS(sem_fixer);
          fprintf(f, "colision del tron %d.\n", index);
        signalS(sem_fixer);
         fi2 = 1;   //new
        return;
      }
      else{
        if (nd == 1)			/* si nomes pot en una direccio */
  	      opo[index].d = vd[0];			/* li assigna aquesta */
        else				/* altrament */
    	    opo[index].d = vd[rand() % 4];	/* segueix una dir. aleatoria */
      }
    }

    opo[index].f += df[opo[index].d];
    opo[index].c += dc[opo[index].d];
      waitS(sem_tabler);
    win_escricar(opo[index].f, opo[index].c, '1' + index, INVERS); // imprimeix número del tron //new
    signalS(sem_tabler);
    waitS(sem_fixer);
    fprintf(f, "nova posicio del tron %d: %d, %d\n", index, opo[index].f, opo[index].c);
    signalS(sem_fixer);
    p_opo[index][n_opo[index]].f = opo[index].f;
    p_opo[index][n_opo[index]].c = opo[index].c;
    n_opo[index]++;

    win_retard(rand() % (max_retard - min_retard + 1) + min_retard);
  }
}

void mou_usuari(void) {  // nou capçalera sense paràmetres //new
  int tec;
  tron seg;
  char c;

  while (!fi1 && !fi2) {   // bucle independent //new

    tec = win_gettec();

    // direcció moviment
    if (tec != 0) {
      if (tec == TEC_RETURN) exit(-1);
      else if ((tec == TEC_DRETA) && (usu.d != 1)) usu.d = 3;
      else if ((tec == TEC_ESQUER) && (usu.d != 3)) usu.d = 1;
      else if ((tec == TEC_AVALL) && (usu.d != 0)) usu.d = 2;
      else if ((tec == TEC_AMUNT) && (usu.d != 2)) usu.d = 0;
    }

    // calc seguent posició
    seg.f = usu.f + df[usu.d];
    seg.c = usu.c + dc[usu.d];
    c = win_quincar(seg.f, seg.c);
    
    // printf("usu:%d, %d", usu.f, usu.c);
    // printf("seg:%d, %d", seg.f, seg.c);
    // printf("Caràcter llegit: [%c] (ASCII: %d)\n", c, c);

    if (c != ' ') { // ha xocat
      esborrar_posicions(p_usu, n_usu);
      fi2 = 1;
    }

    //moviment 
    usu.f = seg.f;
    usu.c = seg.c;
    win_escricar(usu.f, usu.c, '0', INVERS);
    p_usu[n_usu].f = usu.f;
    p_usu[n_usu].c = usu.c;
    n_usu++;

    win_retard(retard);
  }
  exit(0);

}


/* programa principal				    */
int main(int n_args, const char *ll_args[])
{
  int retwin;		/* variables locals */
  int pid;
  const char *log_file;
  srand(time(NULL));		/* inicialitza numeros aleatoris */

  if ((n_args != 2) && (n_args != 4))
  {	fprintf(stderr,"Comanda: ./tron0 variabilitat [retard] nom_log\n");
  	fprintf(stderr,"         on \'variabilitat\' indica la frequencia de canvi de direccio\n");
  	fprintf(stderr,"         de l'oponent: de 0 a 3 (0- gens variable, 3- molt variable),\n");
  	fprintf(stderr,"         i \'retard\' es el numero de mil.lisegons que s'espera entre dos\n");
  	fprintf(stderr,"         moviments de cada jugador (minim 10, maxim 1000, 100 per defecte).\n");
  	exit(1);
  }

  log_file = ll_args[3];
  f = fopen(log_file, "w");  
  setbuf(f, NULL); 

  varia = atoi(ll_args[1]);	/* obtenir parametre de variabilitat */
  if (varia < 0) varia = 0;	/* verificar limits */
  if (varia > 3) varia = 3;

  if (n_args == 3)		/* si s'ha especificat parametre de retard */
  {	retard = atoi(ll_args[2]);	/* convertir-lo a enter */
  	if (retard < 10) retard = 10;	/* verificar limits */
  	if (retard > 1000) retard = 1000;
  }
  else retard = 100;		/* altrament, fixar retard per defecte */

  printf("Joc del Tron\n\tTecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  printf("prem una tecla per continuar:\n");
  getchar();

  n_fil = 0; n_col = 0;		/* demanarem dimensions de taulell maximes */
  retwin = win_ini(&n_fil,&n_col,'+',INVERS);	/* intenta crear taulell */

  if (retwin < 0)	/* si no pot crear l'entorn de joc amb les curses */
  { fprintf(stderr,"Error en la creacio del taulell de joc:\t");
    switch (retwin)
    {	case -1: fprintf(stderr,"camp de joc ja creat!\n"); break;
	case -2: fprintf(stderr,"no s'ha pogut inicialitzar l'entorn de curses!\n"); break;
	case -3: fprintf(stderr,"les mides del camp demanades son massa grans!\n"); break;
	case -4: fprintf(stderr,"no s'ha pogut crear la finestra!\n"); break;
     }
     exit(2);
  }

  p_usu = calloc(n_fil*n_col/2, sizeof(pos));	/* demana memoria dinamica */
  for (int i = 0; i < num_trons; i++) {
    p_opo[i] = (pos *) calloc(n_fil * n_col / 2, sizeof(pos));
    n_opo[i] = 0; 
  }
  if (!p_usu || !p_opo)	/* si no hi ha prou memoria per als vectors de pos. */
  { win_fi();				/* tanca les curses */
    if (p_usu) free(p_usu);
    if (p_opo) free(p_opo);	   /* allibera el que hagi pogut obtenir */
    fprintf(stderr,"Error en alocatacion de memoria dinamica.\n");
    exit(3);
  }

  sem_tabler = ini_sem(1);
  sem_fixer = ini_sem(1);

  inicialitza_joc();

  for(int i = 0; i < num_trons; i++){
    pid = fork();
    if(pid == 0){
      mou_oponent(i);
    }
  }

  if (fork() == 0) { mou_usuari(); exit(0); }   // procés per usuari //new
  do			/********** bucle principal del joc **********/
  {
    wait(NULL);
  } while (!fi1 || !fi2);

  if (fi1==-1) printf("S'ha aturat el joc amb tecla RETURN!\n\n");
  else { printf("Ha guanyat %s\n\n", fi2 ? "l'usuari":"l'ordinador"); }
  elim_sem(sem_fixer);
  elim_sem(sem_tabler);
  return(0);
}