#include <stdio.h>	
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>     
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include "winsuport2.h"		
#include "semafor.h"
#include "memoria.h"
#include "missatge.h"

#define MSG_SIZE 20
#define MAX_PROCS 10


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

// info rellevant sobre el estat
typedef struct {
    int fi_usu;   // final usuari?
    int opo_vius; // num oponents vius
}EstatJoc;

typedef struct { // seguir rastre
    int index;	// index posicio
    int dir;	// direccio moviment
} rastre_args;

int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */

void esborrar_posicions(pos p_pos[], int n_pos); // esborrar un tron

int buscar_pos(pos *array, int n, int f, int c); // buscar index posició especifica

void *seguir_rastre(void *arg); // seguir el rastre de un tron