#include "tron5.h"

int sem_estat_joc, sem_fitxer, sem_tauler;
pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;
pthread_t thread_rastre, thread_rastre_inv;

int index_tron; // num tron
pos *p_opo;   // taula de posicions per a cada oponent
int n_opo = 0;    // nombre de posicions per a cada oponent
int max_retard, min_retard;

// funcio per rebre missatges i crear threads en consequencia
void *msg_listener(void *arg) {
    
    int bustia = *((int *)arg);  
    char msg[16];
    char *token;
    int f, c;

    receiveM(bustia, msg);  

    // per passar a segur rastre
    rastre_args *args1 = malloc(sizeof(rastre_args));
    rastre_args *args2 = malloc(sizeof(rastre_args));

    // agafar fila i columna del msg
    token = strtok(msg, ","); // fila
    if (token != NULL) {
        f = atoi(token);  
        token = strtok(NULL, ","); // columna
        if (token != NULL) c = atoi(token);
    }

    args1->index = buscar_pos(p_opo, n_opo, f, c);

    args1->dir = -1; // thread enrere
    if (pthread_create(&thread_rastre_inv, NULL, seguir_rastre, (void*) args1) != 0) {
    fprintf(stderr, "Error creant el thread del jugador\n");
    exit(1);
    }      

    args2->index = args1->index;
    args2->dir = 1; // thread endevant
    if (pthread_create(&thread_rastre, NULL, seguir_rastre, (void*) args2) != 0) {
    fprintf(stderr, "Error creant el thread del jugador\n");
    exit(1);
    }   

    return NULL;
}

// buscar el index de la posicio (pos) en el array 
int buscar_pos(pos *array, int n, int f, int c) {
    
    for (int i = 0; i < n; i++) 
        if (array[i].f == f && array[i].c == c) 
            return i; 

    return -1; // no trobat
}

// esborra les posicions per les quals ha pasat un torn
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

int main(int n_args, const char *ll_args[]){

  // agafar params
  int id_estat_joc = atoi(ll_args[1]);
  index_tron = atoi(ll_args[2]);
  int id_win = atoi(ll_args[3]);
  char log_file[50];
  strcpy(log_file, ll_args[4]);
  int n_col = atoi(ll_args[5]);
  int n_fil = atoi(ll_args[6]);
  int varia = atoi(ll_args[7]);
  max_retard = atoi(ll_args[8]);
  min_retard = atoi(ll_args[9]);
  sem_estat_joc = atoi(ll_args[10]);
  sem_fitxer = atoi(ll_args[11]);
  sem_tauler = atoi(ll_args[12]);
  int id_bustia = atoi(ll_args[13]);
  int id_sem_bustia = atoi(ll_args[14]);
  char msg[3];

  FILE *f = fopen(log_file, "a");  

  // mapejar memoria
  EstatJoc* joc = (EstatJoc*) map_mem(id_estat_joc);
  void *p_win = map_mem(id_win);
  
  // setejar finestra
  win_set(p_win, n_fil, n_col); 
  pthread_mutex_init(&mutex, NULL); /* inicialitza el semafor */

  char cars;
  tron seg;
  int k, vk, nd, vd[3];
  int canvi = 0;
  int retorn = 0;
  tron opo;    
  srand(getpid());		/* inicialitza numeros aleatoris */

  // reservar memoria
  p_opo = calloc(n_fil*n_col/2, sizeof(pos));	
  if (!p_opo)	/* si no hi ha prou memoria per als vectors de pos. */
  { 
    win_fi();				/* tanca les curses */
    if (p_opo) free(p_opo);
    /* allibera el que hagi pogut obtenir */
    fprintf(stderr,"Error en alocatacion de memoria dinamica.\n");
    exit(3);
  }

  //inicializar tron
  opo.f = (n_fil/(index_tron+2)) ;
  opo.c = (n_col*3)/4;		
  opo.d = 1;
  
  waitS(sem_tauler);
    win_escricar(opo.f, opo.c, '1' + index_tron, INVERS);
    win_update();
  signalS(sem_tauler);
  
  p_opo[n_opo].f = opo.f;	// guardar posicio
  p_opo[n_opo].c = opo.c;
  n_opo++;

  waitS(sem_fitxer);
    fprintf(f, "tron inizialitzat %d!\n", index_tron);
    fflush(f);
  signalS(sem_fitxer);

  // thread rebre msg
  pthread_t thread_msg;
  if (pthread_create(&thread_msg, NULL, msg_listener, (void*) &id_bustia) != 0) {
    fprintf(stderr, "Error creant el thread msg\n");
    exit(1);
  }  


// logica principal tron
while (joc->fi_usu==0 ) {  

  seg.f = opo.f + df[opo.d]; /* calcular seguent posicio */
  seg.c = opo.c + dc[opo.d]; 

  waitS(sem_tauler);
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
      vk = (opo.d + k) % 4;		/* nova direccio */
      if (vk < 0){
        vk += 4;		/* corregeix negatius */
      }
        seg.f = opo.f + df[vk]; /* corregeix negatius */
        seg.c = opo.c + dc[vk]; /* calcular posicio en la nova dir.*/
        
        waitS(sem_tauler);  // consultar tauler
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

      if (nd == 1) opo.d = vd[0];/* si nomes pot en una direccio */
      else opo.d = vd[rand() % nd];	/* segueix una dir. aleatoria */

    }
  }

    if (retorn==0){
      // posicio actual
      opo.f += df[opo.d]; 
      opo.c += dc[opo.d];

      waitS(sem_tauler); // escriure tauler
        win_escricar(opo.f, opo.c, '1' + index_tron, INVERS);
        win_update();
      signalS(sem_tauler);

      waitS(sem_fitxer);  // log
        fprintf(f, "tron %d: %d-%d\n", index_tron, opo.f, opo.c);
        fflush(f);
      signalS(sem_fitxer);

      // actualitzar posició
      pthread_mutex_lock(&mutex);
        p_opo[n_opo].f = opo.f;
        p_opo[n_opo].c = opo.c;
        n_opo++;
      pthread_mutex_unlock(&mutex);
      
      win_retard(rand() % ((max_retard - min_retard + 1) + min_retard));
    }else {
      esborrar_posicions(p_opo, n_opo);

      waitS(sem_fitxer); // log
        fprintf(f, "tron %d ha xocat\n", index_tron+1);
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

  // eliminar semafors, memoria
  pthread_mutex_destroy(&mutex); /* destrueix el semafor */
  elim_mem(id_estat_joc);
  elim_mem(id_win);
  elim_mem(id_sem_bustia);

}

// segueix el rastre del tron xocat des de la posicio i direccio indicada
void *seguir_rastre(void *arg){

  // args
  rastre_args *args = (rastre_args*) arg;
  int index_arr = args->index;
  int dir = args->dir;
  
  pos actual;
  int fi=0, i=0, total;


  for (int i = 0; !fi; i++) {
      pthread_mutex_lock(&mutex);
      total = n_opo;
      int idx = index_arr + i * dir; // calcular seguent

      if (idx < 0 || idx >= total) {
          pthread_mutex_unlock(&mutex);
          fi=1;  // fuera de rango
      }

      actual = p_opo[idx]; // seguent
      pthread_mutex_unlock(&mutex);

      //actualitzar pantalla
      win_escricar(actual.f, actual.c, '1' + index_tron, NO_INV);
      win_update();
      win_retard(rand() % ((max_retard - min_retard + 1) + min_retard));
  }

  free(args);
}