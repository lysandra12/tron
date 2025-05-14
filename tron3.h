#include <stdio.h>	
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>     
#include <time.h>
#include <string.h>
#include "winsuport2.h"		
#include "semafor.h"
#include "memoria.h"
#include <sys/time.h>
#include <unistd.h>

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
struct EstatJoc {
    int fi_usu;   // final usuari?
    int opo_vius; // num oponents vius
};


int df[] = {-1, 0, 1, 0};	/* moviments de les 4 direccions possibles */
int dc[] = {0, -1, 0, 1};	/* dalt, esquerra, baix, dreta */


void esborrar_posicions(pos p_pos[], int n_pos);

void inicialitza_joc(struct EstatJoc* joc);

void mou_usuari(struct EstatJoc* joc);