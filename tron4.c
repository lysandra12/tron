/*****************************************************************************/
/*									     */
/*				     tron4.c				     */
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
/*     aleatoriament s un parametre del programa (veure Arguments).     */
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
/*	   $ gcc -c winsuport2.c -o winsuport.o			     	     */
/*     $ gcc -c memoria.c -o memoria.o*/
/*     $ gcc -c semafor.c -o semafor.o */
/*	   $ gcc tron4.c winsuport.o memoria.o semafor.o -o tron3 -lcurses	     */
/*	   $ ./tron3 num_opo variabilitat fitxer_log [min_retard max_retard]		 */
/*									                                                         */
/*  Codis de retorn:						  	     */
/*     El programa retorna algun dels seguents codis al SO:		     */
/*	0  ==>  funcionament normal					     */
/*	1  ==>  numero d'arguments incorrecte 				     */
/*	2  ==>  no s'ha pogut crear el camp de joc (no pot iniciar CURSES)   */
/*	3  ==>  no hi ha prou memoria per crear les estructures dinamiques   */
/*									     */
/*****************************************************************************/
#include "tron.h"

/* variables globals */

int n_fil, n_col;		/* dimensions del camp de joc */

int varia;		/* valor de variabilitat dels oponents [0..9] */
int max_retard, min_retard;		/* valor del retard de moviment, en mil.lisegons */
int num_oponents;

tron usu;   	  /* informacio de l'usuari */
pos *p_usu;			/* taula de posicions que van recorrent */
int n_usu = 0;  /* numero d'entrades en les taules de pos. */

int sem_tauler, sem_fitxer, sem_estat_joc;

FILE *f;

/* funcio per esborrar totes les posicions anteriors, sigui de l'usuari o */
/* de l'oponent */
void esborrar_posicions(pos p_pos[], int n_pos)
{
  int i;
  
  for (i=n_pos-1; i>=0; i--)		/* de l'ultima cap a la primera */
  {
    waitS(sem_tauler);
      win_escricar(p_pos[i].f,p_pos[i].c,' ',NO_INV);
      win_update();
    signalS(sem_tauler);
    win_retard(10);		/* un petit retard per simular el joc real */
  }
}

/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
void inicialitza_joc(struct EstatJoc* joc)
{
  // Inicializamos las variables fi_usu y opo_vius
  joc->fi_usu = 0;
  joc->opo_vius = num_oponents;

  char strin[45];

  usu.f = (n_fil-1)/2;
  usu.c = (n_col)/4;		/* fixa posicio i direccio inicial usuari */
  usu.d = 3;

  waitS(sem_tauler);
    win_escricar(usu.f,usu.c,'0',INVERS);	
  signalS(sem_tauler);

  p_usu[n_usu].f = usu.f;		/* memoritza posicio inicial */
  p_usu[n_usu].c = usu.c;
  n_usu++;

  // info
  sprintf(strin,"Tecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
		TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
  win_escristr(strin);
  win_update();
}

void* mou_usuari(void* arg) {

  struct EstatJoc* joc = (struct EstatJoc*) arg;
  
  int tec;
  tron seg;
  char c;
  int retorn =0;

  while (joc->fi_usu==0 && joc->opo_vius>0) {
    
    waitS(sem_tauler);
      tec = win_gettec();
    signalS(sem_tauler);

    // direcció moviment
    if (tec != 0) {

      if (tec == TEC_RETURN){
        esborrar_posicions(p_usu, n_usu);
        waitS(sem_estat_joc);
          joc->fi_usu=-1;
        signalS(sem_estat_joc);
        pthread_exit(0);
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
        win_update();
      signalS(sem_tauler);

      p_usu[n_usu].f = usu.f; // actualitzar posicio
      p_usu[n_usu].c = usu.c;
      n_usu++;

    }else{ // ha xocat
      esborrar_posicions(p_usu, n_usu);
      waitS(sem_estat_joc);
        joc->fi_usu=1;
      signalS(sem_estat_joc);

      pthread_exit(0);
    }

    win_retard(min_retard);
  }

  return NULL;
}

int main(int n_args, const char *ll_args[])
{
  int retwin;		/* variables locals */
  const char *log_file;
  char a0[50], a1[50], a2[50],a4[50],a5[50], a6[50], a7[50], a8[50], a9[50], a10[50], a11[50];
  srand(time(NULL));		/* inicialitza numeros aleatoris */

  pthread_t thread_usu;


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
  else min_retard = 150; max_retard = 500;		/* altrament, fixar retard per defecte */

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

  /* demana memoria dinamica */

  // usuari
  p_usu = calloc(n_fil*n_col/2, sizeof(pos));	
  if (!p_usu)	/* si no hi ha prou memoria per als vectors de pos. */
  { 
    win_fi();				/* tanca les curses */
    if (p_usu) free(p_usu);
    /* allibera el que hagi pogut obtenir */
    fprintf(stderr,"Error en alocatacion de memoria dinamica.\n");
    exit(3);
  }
  
  sem_tauler = ini_sem(1);
  sem_fitxer = ini_sem(1);
  sem_estat_joc = ini_sem(1);
  
  // crear zona mem. compartida 
  int id_estat_joc = ini_mem(sizeof(struct EstatJoc));
  int id_win = ini_mem(retwin);

  // mapear memoria compartida
  struct EstatJoc* joc = (struct EstatJoc*) map_mem(id_estat_joc);
  void *p_win = map_mem(id_win); // puntero m.c. info ventana

  win_set(p_win, n_fil, n_col); // asignar pantalla a tablero
  inicialitza_joc(joc);
  
  // thread usuari
  if (pthread_create(&thread_usu, NULL, mou_usuari, (void*)joc) != 0) {
  fprintf(stderr, "Error creant el thread del jugador\n");
  exit(1);
  }   

  sprintf(a0,"%i",id_estat_joc);
  sprintf(a2,"%i",id_win);
  sprintf(a4,"%i",n_col);
  sprintf(a5,"%i",n_fil);
  sprintf(a6,"%i",varia);
  sprintf(a7,"%i",max_retard);
  sprintf(a8,"%i",min_retard);
  sprintf(a9,"%i",sem_estat_joc);
  sprintf(a10,"%i",sem_fitxer);
  sprintf(a11,"%i",sem_tauler);

  // processos oponents
  for (int i = 0; i < num_oponents; i++){
    sprintf(a1,"%i",i);
  	if(fork() == 0) execlp("./oponent3", "oponent3", a0, a1, a2, log_file, a4, a5, a6, a7, a8, a9, a10, a11, (char *)0);
  }   

  // cronometre
  struct timeval inici, actual; 
  gettimeofday(&inici, NULL); // temps actual
  int s, ms, min, resta_s;

/********** bucle principal del joc **********/
while (joc->fi_usu==0 && joc->opo_vius>0) {
    
    gettimeofday(&actual, NULL); // actualitzar tems actual
    s = (actual.tv_sec - inici.tv_sec);
    ms = (actual.tv_usec - inici.tv_usec) / 1000;
    if (ms < 0) {
        ms += 1000;
        s--;
    }

    // conversio unitats
    min = s / 60;
    resta_s = s % 60;

    // imprimir
    char msg[40];
    sprintf(msg, "Temps: %02d:%02d", min, resta_s);
    waitS(sem_tauler);
        win_escristr(msg);
        win_update();
    signalS(sem_tauler);

    usleep(100000); // 0.1 s
  }

  win_fi();

  // esperar finalizacion procesos
  for (int i = 0; i < num_oponents; i++) wait(NULL);

  // RESULTATS
  if (joc->fi_usu==-1) printf("S'ha aturat el joc amb tecla RETURN!\n");
  else { printf("Ha guanyat %s !!\n", !joc->fi_usu ? "l'usuari":"l'ordinador"); }
  printf( "Temps emprat: %02d:%02d\n\n", min, resta_s);

  // eliminar semafors i memoria
  elim_sem(sem_fitxer);
  elim_sem(sem_tauler);
  elim_sem(sem_estat_joc);
  
  elim_mem(id_estat_joc);
  elim_mem(id_win);
  free(p_usu);

  return(0);
}
