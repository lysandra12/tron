/*****************************************************************************/
/*									     */
/*				     tron2.c				     */
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
/*     $ gcc -c memoria.c -o memoria.o*/
/*     $ gcc -c semafor.c -o semafor.o */
/*	   $ gcc tron2.c winsuport.o memoria.o semafor.o -o tron2 -lcurses	     */
/*	   $ ./tron2 num_opo variabilitat fitxer_log [min_retard max_retard]		 */
/*									                                                         */
/*  Codis de retorn:						  	     */
/*     El programa retorna algun dels seguents codis al SO:		     */
/*	0  ==>  funcionament normal					     */
/*	1  ==>  numero d'arguments incorrecte 				     */
/*	2  ==>  no s'ha pogut crear el camp de joc (no pot iniciar CURSES)   */
/*	3  ==>  no hi ha prou memoria per crear les estructures dinamiques   */
/*									     */
/*****************************************************************************/

#include <stdio.h>	
#include <stdlib.h>
#include <unistd.h>
#include "winsuport.h"		
#include <sys/wait.h>     
#include <time.h>
#include "semafor.h"
#include <string.h>
#include "memoria.h"

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

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */

int varia;		/* valor de variabilitat dels oponents [0..9] */
int max_retard, min_retard;		/* valor del retard de moviment, en mil.lisegons */
int num_oponents;

tron usu;   	   		/* informacio de l'usuari */
pos *p_usu;			/* taula de posicions que van recorrent */
int n_usu = 0;  /* numero d'entrades en les taules de pos. */

#define max_trons 10
tron opo[max_trons];    // suposem màxim 10 oponents
pos *p_opo[max_trons];   // taules de posicions per a cada oponent
int n_opo[max_trons];        // nombre de posicions per a cada oponent

FILE *f;
int sem_tauler , sem_fitxer, sem_estat_joc;

//borrar char *p_tauler;
//borrar int id_tauler;

struct EstatJoc {
    int fi_usu;
    int opo_vius;
};

/* funcio per esborrar totes les posicions anteriors, sigui de l'usuari o */
/* de l'oponent */
void esborrar_posicions(pos p_pos[], int n_pos)
{
  int i;
  
  for (i=n_pos-1; i>=0; i--)		/* de l'ultima cap a la primera */
  {
    waitS(sem_tauler);
    win_escricar(p_pos[i].f,p_pos[i].c,' ',NO_INV);
    signalS(sem_tauler);
    //borrar p_tauler[p_pos[i].f* n_col +p_pos[i].c] = ' ';	/* esborra una pos. */
    win_retard(10);		/* un petit retard per simular el joc real */
  }
}

/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(struct EstatJoc* joc)
{
  //borrar:
  // llenar matriz de espacios
  //borrar memset(p_tauler, ' ',n_fil*n_col);

  // Llenar los bordes con 'x'
  // for (int i = 0; i < n_fil; i++) {
  //     for (int j = 0; j < n_col; j++) {
  //         //  superior 
  //         if (i == 0) p_tauler[i * n_col + j] = 'x';
  //         //  inferior 
  //         else if (i == n_fil - 2) p_tauler[i * n_col + j] = 'x';
  //         //  izquierdo 
  //         else if (j == 0) p_tauler[i * n_col + j] = 'x';
  //         //  derecho 
  //         else if (j == n_col - 1) p_tauler[i * n_col + j] = 'x';
  //     }
  // }

  // Inicializamos las variables fi_usu y opo_vius
  joc->fi_usu = 0;
  joc->opo_vius = num_oponents;

  char strin[45];

  usu.f = (n_fil-1)/2;
  usu.c = (n_col)/4;		/* fixa posicio i direccio inicial usuari */
  usu.d = 3;

  win_escricar(usu.f,usu.c,'0',INVERS);	
  //borrar p_tauler[usu.f * n_col + usu.c] = 0;/* escriu la primer posicio usuari */
  p_usu[n_usu].f = usu.f;		/* memoritza posicio inicial */
  p_usu[n_usu].c = usu.c;
  n_usu++;

  // Inicialitza tots els oponents disponibles 
  for (int index = 0; index < num_oponents; index++) { 
    opo[index].f = (n_fil/(index+2)) ;
    opo[index].c = (n_col*3)/4;		
    opo[index].d = 1;
    
    win_escricar(opo[index].f, opo[index].c, '1' + index, INVERS);
    //borrar [opo[index].f * n_col +opo[index].c] = index + 1;	/* escriu la primer posicio oponent */ 
    
    p_opo[index][n_opo[index]].f = opo[index].f;	
    p_opo[index][n_opo[index]].c = opo[index].c;
    n_opo[index]++;	

    fprintf(f, "tron inizialitzat %d!\n", index);
  }

  sprintf(strin,"Tecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  win_escristr(strin);
}


/* funcio per moure un oponent una posicio; retorna 1 si l'oponent xoca */
/* contra alguna cosa, 0 altrament					*/
int mou_oponent(int index, struct EstatJoc* joc)
{
  char cars;
  tron seg;
  int k, vk, nd, vd[3];
  int canvi = 0;
  int retorn = 0;
  srand(getpid());		/* inicialitza numeros aleatoris */
 
while (joc->fi_usu==0 ) {  
  seg.f = opo[index].f + df[opo[index].d]; /* calcular seguent posicio */
  seg.c = opo[index].c + dc[opo[index].d]; 

  waitS(sem_tauler);
  //borrar cars = p_tauler[seg.f * n_col + seg.c];	/* calcula caracter seguent posicio */
  cars = win_quincar(seg.f,seg.c);	/* calcula caracter seguent posicio */
  signalS(sem_tauler);

  if (cars != ' ')			/* si seguent posicio ocupada */
     canvi = 1;		/* anotar que s'ha de produir un canvi de direccio */
  else
    if (varia > 0)	/* si hi ha variabilitat */
    { k = rand() % 10;		/* prova un numero aleatori del 0 al 9 */
      if (k < varia) canvi = 1;	/* possible canvi de direccio */
    }
  
  if (canvi){		/* si s'ha de canviar de direccio */
  
    nd = 0;
    for (k=-1; k<=1; k++)	/* provar direccio actual i dir. veines */
    {
      vk = (opo[index].d + k) % 4;		/* nova direccio */
      if (vk < 0){
        vk += 4;		/* corregeix negatius */
      }
        seg.f = opo[index].f + df[vk]; /* corregeix negatius */
        seg.c = opo[index].c + dc[vk]; /* calcular posicio en la nova dir.*/
        
        waitS(sem_tauler);  // consultar tauler
        //borrar cars = p_tauler[seg.f* n_col + seg.c]; 
        cars = win_quincar(seg.f,seg.c);	/* calcula caracter seguent posicio */
        signalS(sem_tauler);

      
      if (cars == ' ')
      { vd[nd] = vk;			/* memoritza com a direccio possible */
        nd++;				/* anota una direccio possible mes */
      }
    }
    if (nd == 0){			/* si no pot continuar, */
  	retorn = 1;		/* xoc: ha perdut l'oponent! */
    } else { 

      if (nd == 1) opo[index].d = vd[0];/* si nomes pot en una direccio */
      else opo[index].d = vd[rand() % nd];	/* segueix una dir. aleatoria */

    }
  }

    if (retorn==0){
      opo[index].f += df[opo[index].d]; // actualitzar posició
      opo[index].c += dc[opo[index].d];

      waitS(sem_tauler); // escriure tauler
        win_escricar(opo[index].f, opo[index].c, '1' + index, INVERS); 
        //borrar p_tauler[opo[index].f * n_col +opo[index].c] = index + 1; 
      signalS(sem_tauler);

      waitS(sem_fitxer);  // log
      fprintf(f, "tron %d: %d-%d\n", index, opo[index].f, opo[index].c);
      signalS(sem_fitxer);

      // actualitzar posició
      p_opo[index][n_opo[index]].f = opo[index].f;
      p_opo[index][n_opo[index]].c = opo[index].c;
      n_opo[index]++;
      
      win_retard(rand() % ((max_retard - min_retard + 1) + min_retard));
    }else {

      //waitS(sem_tauler); // ha xocat
      esborrar_posicions(p_opo[index], n_opo[index]);
      //signalS(sem_tauler);

      waitS(sem_fitxer); // log
      fprintf(f, "tron %d ha xocat\n", index);
      signalS(sem_fitxer);

      waitS(sem_estat_joc); // actualitzar estat
      joc->opo_vius -= 1;
      signalS(sem_estat_joc);

      exit(0);
    }

  }
  waitS(sem_estat_joc);
  joc->opo_vius -= 1;
  signalS(sem_estat_joc);
  exit(0);

}

void mou_usuari(struct EstatJoc* joc) {  // nou capçalera sense paràmetres //new
  
  int tec;
  tron seg;
  char c;
  int retorn =0;

  while (joc->fi_usu==0 && joc->opo_vius>0) {   // bucle independent //new
    
    waitS(sem_tauler);
    tec = win_gettec();
    signalS(sem_tauler);

    // direcció moviment
    if (tec != 0) {
      if (tec == TEC_RETURN){
        //waitS(sem_tauler);
        esborrar_posicions(p_usu, n_usu);
        //signalS(sem_tauler);
        waitS(sem_estat_joc);
        joc->fi_usu=-1;
        signalS(sem_estat_joc);
        exit(0);
      }
      else if ((tec == TEC_DRETA) && (usu.d != 1)) usu.d = 3;
      else if ((tec == TEC_ESQUER) && (usu.d != 3)) usu.d = 1;
      else if ((tec == TEC_AVALL) && (usu.d != 0)) usu.d = 2;
      else if ((tec == TEC_AMUNT) && (usu.d != 2)) usu.d = 0;
    }

    // calc seguent posició
    seg.f = usu.f + df[usu.d];
    seg.c = usu.c + dc[usu.d];

    waitS(sem_tauler); // consultar tauler
    c = win_quincar(seg.f,seg.c);	/* calcular caracter seguent posicio */
    signalS(sem_tauler);


    if (c == ' ') { //moviment
      usu.f = seg.f;
      usu.c = seg.c;
      
      waitS(sem_tauler); // escriure tauler
      win_escricar(usu.f, usu.c, '0', INVERS);
      //p_tauler[usu.f* n_col + usu.c] = 0;
      signalS(sem_tauler);

      p_usu[n_usu].f = usu.f; // actualitzar posicio
      p_usu[n_usu].c = usu.c;
      n_usu++;

    }else{ // ha xocat
      //waitS(sem_tauler);
      esborrar_posicions(p_usu, n_usu);
      //signalS(sem_tauler);
      waitS(sem_estat_joc);
      joc->fi_usu=1;
      signalS(sem_estat_joc);

      exit(0);
    }

    win_retard(min_retard);
  }

  exit(0);
}

/* programa principal				    */
int main(int n_args, const char *ll_args[])
{
  int retwin;		/* variables locals */
  const char *log_file;

  srand(time(NULL));		/* inicialitza numeros aleatoris */

  if ((n_args != 4) && (n_args != 6))
  {	fprintf(stderr,"Comanda: ./tron0 num_oponents variabilitat nom_log [min_retard max_retard] \n");
  	fprintf(stderr,"         on \'variabilitat\' indica la frequencia de canvi de direccio\n");
  	fprintf(stderr,"         de l'oponent: de 0 a 3 (0- gens variable, 3- molt variable),\n");
  	fprintf(stderr,"         i \'retard\' es el numero de mil.lisegons que s'espera entre dos\n");
  	fprintf(stderr,"         moviments de cada jugador (minim 10, maxim 1000, 100 per defecte).\n");
  	exit(1);
  }

  log_file = ll_args[3];
  f = fopen(log_file, "w");  
  setbuf(f, NULL); 

  num_oponents = atoi(ll_args[1]);	/* obtenir parametre de num_oponents */

  varia = atoi(ll_args[2]);	/* obtenir parametre de variabilitat */
  if (varia < 0) varia = 0;	/* verificar limits */
  if (varia > 3) varia = 3;

  if (n_args == 6)		/* si s'ha especificat parametre de retard */
  {	min_retard = atoi(ll_args[4]);	
  	max_retard = atoi(ll_args[5]);	

  	if (min_retard < 10) min_retard = 10;	/* verificar limits */
  	if (max_retard > 1000) min_retard = 1000;
  }
  else min_retard = 100; max_retard = 400;		/* altrament, fixar retard per defecte */

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
  if (!p_usu)	/* si no hi ha prou memoria per als vectors de pos. */
  { 
    win_fi();				/* tanca les curses */
    if (p_usu) free(p_usu);
    /* allibera el que hagi pogut obtenir */
    fprintf(stderr,"Error en alocatacion de memoria dinamica.\n");
    exit(3);
  }

  for (int i = 0; i < num_oponents; i++) {
    p_opo[i] = (pos *) calloc(n_fil * n_col / 2, sizeof(pos));
    n_opo[i] = 0; 

    if (!p_opo[i]){
      for (i; i >= 0; i--){
        free(p_opo[i]);
      }
      fprintf(stderr,"Error en alocatacion de memoria dinamica.\n");
      exit(3);
    }
  }

  // semafors
  sem_tauler = ini_sem(1);
  sem_fitxer = ini_sem(1);
  sem_estat_joc = ini_sem(1);

  /* crear zona mem. compartida */
  //id_tauler = ini_mem(n_fil*n_col*sizeof(char)); 
  int id_estat_joc = ini_mem(sizeof(struct EstatJoc));

  // Paso 2: Mapear la memoria compartida
  struct EstatJoc* joc = (struct EstatJoc*) map_mem(id_estat_joc);
  //p_tauler = (char *)map_mem(id_tauler);

  inicialitza_joc(joc);
  
  // crear processos
  if (fork() == 0) mou_usuari(joc);   // procés per usuari

  for (int i = 0; i < num_oponents; i++)    // processos oponents
    if(fork() == 0) mou_oponent(i, joc);
  

 
  do			/********** bucle principal del joc **********/
  {
    wait(NULL);
  } while (joc->fi_usu==0 && joc->opo_vius>0);

  win_fi();
  fprintf(f, "win_fi");
  // esperar a que acabin els processos
  for (int i = 0; joc->opo_vius + (i < joc->fi_usu!=0 ? 1:0); i++) 
    wait(NULL);

  // RESULTATS
  if (joc->fi_usu==-1) printf("S'ha aturat el joc amb tecla RETURN!\n\n");
  else { printf("Ha guanyat %s\n\n", !joc->fi_usu ? "l'usuari":"l'ordinador"); }
  
  // eliminar semafors i memoria
  elim_sem(sem_fitxer);
  elim_sem(sem_tauler);
  elim_sem(sem_estat_joc);
  
  //elim_mem(id_tauler);
  elim_mem(id_estat_joc);
  free(p_usu);
  for (int i = 0; i < num_oponents; i++) 
    free(p_opo[i]);

  return(0);
}
