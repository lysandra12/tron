#include "tron3.h"

int sem_estat_joc, sem_fitxer, sem_tauler;

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
  int index = atoi(ll_args[2]);
  int id_win = atoi(ll_args[3]);
  char log_file[50];
  strcpy(log_file, ll_args[4]);
  int n_col = atoi(ll_args[5]);
  int n_fil = atoi(ll_args[6]);
  int varia = atoi(ll_args[7]);
  int max_retard = atoi(ll_args[8]);
  int min_retard = atoi(ll_args[9]);
  sem_estat_joc = atoi(ll_args[10]);
  sem_fitxer = atoi(ll_args[11]);
  sem_tauler = atoi(ll_args[12]);

  FILE *f = fopen(log_file, "a");  

  // mapejar memoria
  struct EstatJoc* joc = (struct EstatJoc*) map_mem(id_estat_joc);
  void *p_win = map_mem(id_win);

  // setejar finestra
  win_set(p_win, n_fil, n_col); 

  char cars;
  tron seg;
  int k, vk, nd, vd[3];
  int canvi = 0;
  int retorn = 0;
  tron opo;    
  pos *p_opo;   // taula de posicions per a cada oponent
  int n_opo = 0;    // nombre de posicions per a cada oponent
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
  opo.f = (n_fil/(index+2)) ;
  opo.c = (n_col*3)/4;		
  opo.d = 1;
  
  waitS(sem_tauler);
    win_escricar(opo.f, opo.c, '1' + index, INVERS);
    win_update();
  signalS(sem_tauler);
  
  p_opo[n_opo].f = opo.f;	// guardar posicio
  p_opo[n_opo].c = opo.c;
  n_opo++;

  waitS(sem_fitxer);
    fprintf(f, "tron inizialitzat %d!\n", index);
    fflush(f);
  signalS(sem_fitxer);


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
        win_escricar(opo.f, opo.c, '1' + index, INVERS);
        win_update();
      signalS(sem_tauler);

      waitS(sem_fitxer);  // log
        fprintf(f, "tron %d: %d-%d\n", index, opo.f, opo.c);
        fflush(f);
      signalS(sem_fitxer);

      // actualitzar posició
      p_opo[n_opo].f = opo.f;
      p_opo[n_opo].c = opo.c;
      n_opo++;
      
      win_retard(rand() % ((max_retard - min_retard + 1) + min_retard));
    }else {

      esborrar_posicions(p_opo, n_opo);

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