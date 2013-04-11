#ifndef _SUDOKUGEN_H_
#define _SUDOKUGEN_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<math.h>


#ifndef NT
	#include <sys/times.h>
	#include <sys/time.h>
#endif

#ifdef NT
	#define random() rand()
	#define srandom(seed) srand(seed)
#endif

#ifndef CLK_TCK
	#define CLK_TCK 60
#endif

#define MAX_LINE_LENGTH 10000
#define MAX_NAME_LENGTH 10000

#define RANDOM 0
#define BALANCED 1

#define GROUPBALANCED 1
#define DOUBLYBALANCED 2
#define FULLYBALANCED 3

int error(char *);
unsigned long getseed(void);

void new_sudoku(char * );
int alloc_squarecolor(void);
int alloc_squarefixed(void);
int alloc_incidencecube(void);
int init_incidencecube(void);
int write_pls(char *);
void shuffle(char*, char*, int, unsigned long,int);
int Find_col(int, int, int);
int Find_symb(int, int);
int Find_row(int, int);
int is_sudoku(void);
int violated_rows_cols(void);
int read_pls_in_squarecolor(char *);
void test_validity_squarecolor_2(void);
int alloc_violated_cells(void);
void ls_ic_proper(void);
void ic_ls_proper(void) ;
int alloc_oldsquarecolor(void);
int alloc_violated_cells(void);
int alloc_alldiffcheck(void);
void init_alldiffcheck(void);



void poke(char *, int , unsigned long , char *, int );
int test_validity_squarecolor(void);
int generate_random_holes(int );
int generate_balanced_holes(int );


void  poke_rectangle(char *, int, char *, unsigned long);
int generate_rectangle_holes_balanced(int );
void pokesudoku(char *, int, int, unsigned long, char *, int);





void generate_blockbalanced_holes( int );
void generate_doublybalanced_holes( int , int  );
void generate_fullybalanced_holes_region (int );




extern int order;
extern int bsi_x; 
extern int bsi_y; 
extern int **squarecolor;  
extern int **squarefixed;  
extern int ***incidencecube;  
extern int *alldiffcheck;
extern int ** violated_cells;



#endif /* _SUDOKUGEN_H_ */
