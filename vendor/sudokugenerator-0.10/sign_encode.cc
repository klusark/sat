#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <vector>

using namespace std;

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

char linebuf[MAX_LINE_LENGTH + 1];

int first = 1;


/******************************************************************************/
int sub_order = 0;		/* global variable for order of ls, for sudoku */

int order = 0;			/* global variable for order of ls */
int **squarecolor;		/* squarecolor[i][j] == color assigned to that square if any
				   == -1 if no color assigned */
int **squarefixed;		/* squarefixed[i][j] == 1 iff that square is colored,
				   == 0 iff color not yet determined */

/******************************************************************************/

int ***incidencecube;		/* incidencecube[i][j][k] ==
				   1 iff squarecolor[i][j] must be color k
				   0 iff squarecolor[i][j] *must not* be color k
				   -1 iff squarecolor[i][j] might or might not end up being k */
int ***mapcolregvar;
int ***mapvar;			/* mapvar[i][j][k] == the boolean variable that represents
				   the proposition that square[i,j] has color k
				   if that proposition has not been determined true or false yet.
				   If that proposition has been determined, then == -1. */
int **mapregvar;
int ***maprowcolorvar;
int ***mapcolcolorvar;
int nvar = -1;			/* number of boolean variables in the sat encoding */
int nregvar = -1;		/* number of boolean variables in the regsat encoding */
int ncls = -1;			/* number of clauses in the sat encoding */
int nregcls = -1;		/* number of clauses in the sat encoding */
int *varval;			/* varval[i] holds value of variable i from .sol file */
int maxColors, maxdualtvalues;

/******************************************************************************/

/* ADD FORWARD DECLARATIONS OF ALL FUNCTIONS HERE! */


int satmvrdencode_pls (char *outfile, int minflag);
int regsatencode_dual_pls (char *outfile, int minflag);
int encode_signsat_pls (char *outfile, int minflag);
int encode_nbsignsat_pls (char *outfile, int minflag);
int encode_hybsignsat_pls (char *outfile, int minflag);

int satperminfoencode_pls (char *, int);
int satencode_pls (char *, int);
int sudoku_satencode_pls (char *, int minflag, int column_order);
int bcspencode_pls (char *, int);
int bcspencode2_pls (char *, int);
int satmvencode_pls (char *, int);
int satpuremvencode_pls (char *, int);
int satencode_pls_asregcnf (char *outfile, int minflag);
int regsatencode_pls (char *, int);
int regsatencode2_pls (char *, int);
int satpermredencode_pls (char *outfile, int minflag);
int regsatencode_noheavy_pls (char *, int);
int regsatencodeinclassic_noheavy_pls (char *, int);
int regsatencode_noheavy_pls_regmv (char *outfile, int minflag);
int regsatencode_noheavy_pls_special (char *, int);
int nunfixed (void);
int init_incidencecube (void);
int forward_checking_incidencecube (void);
int poke (const char *, int, unsigned long, char *);
int write_pls (const char *);
int generate_holes (int);
int test_validity_squarecolor (void);
int read_ols_in_squarecolor (char *);
int read_pls_in_squarecolor (const char *);
int read_sudoku_pls_in_squarecolor (char *infile);
int read_varval (char *);
int alloc_varval (void);
int read_regvarval (char *);
int read_regvarval_noheavy (char *);
int alloc_regvarval (void);
int alloc_mapdualregvar (void);
int alloc_incidencecube (void);
int alloc_squarecolor (void);
int alloc_squarefixed (void);
int alloc_mapvar (void);
int init_mapvar (void);
int alloc_mapregvar (void);
int init_mapregvar (void);
void new_empty (char *);
void new_regular (const char *);
void new_qcp (char *, int, unsigned long);
int ourerror (const char *);
unsigned long getseed (void);
void shuffle (const char *, const char *, int, unsigned long);

int init_mapdualregvar (void);


/******************************************************************************/


int
main (int argc, char *argv[])
{
  int i;
  char command[MAX_NAME_LENGTH];
  char infile[MAX_NAME_LENGTH];
  char outfile[MAX_NAME_LENGTH];
  int numhole;
  unsigned long seed;
  int shuffles;
  char szInitString[100], szlsname[100], szInsSAT2D[100], szInsSAT3D[100],
    szInsSATDual[100];
  float rRatioIni, rInc, ratio, noise;
  int var_classes, var_class, samplesvar_class, sample;
  int firstIdinstance, numInstances;
  int sols, bBothSATencodings, bBothRegSATencodings;
  FILE *fp2dSATsamples, *fp3dSATsamples, *fp2dRegSATsamples,
    *fpDualRegSATsamples, *fpaux;
  char szInsREGSAT2D[100], szInsREGSATDual[100], szInsBSAT2D[100],
    szInsBSAT3D[100];


  FILE *fp;

  if (argc <= 1)
    {
      printf ("Usage : %s poke INFILE NUMHOLE OUTFILE { SEED }\n", argv[0]);
      printf ("Usage : %s pls2satPermRedundant INFILE { minimal }\n",
	      argv[0]);
      printf ("Usage : %s pls2sat+perminfo INFILE { minimal }\n", argv[0]);
      printf ("Usage : %s pls2sat INFILE { minimal }\n", argv[0]);
      printf ("Usage : %s sudokupls2sat INFILE  { minimal }\n", argv[0]);
      printf ("Usage : %s pls2bcsp INFILE { minimal }\n", argv[0]);
      printf ("Usage : %s pls2bcsp2 INFILE { minimal }\n", argv[0]);
      printf ("Usage : %s pls2sat INFILE { minimal+amo }\n", argv[0]);
      printf ("Usage : %s pls2satmv INFILE { minimal }\n", argv[0]);
      printf ("Usage : %s pls2satmvrd INFILE { minimal }\n", argv[0]);
      printf ("Usage : %s pls2satpuremv INFILE { minimal }\n", argv[0]);
      printf ("Usage : %s pls2regsat INFILE { encoding }\n", argv[0]);
      printf ("Usage : %s pls2reginclassic INFILE { encoding }\n", argv[0]);
      printf ("Usage : %s pls2regmv INFILE { encoding }\n", argv[0]);
      printf ("Usage : %s pls2signsat INFILE { encoding }\n", argv[0]);
      printf ("Usage : %s pls2nbsignsat INFILE { encoding }\n", argv[0]);
      printf ("Usage : %s pls2hybsignsat INFILE { encoding }\n", argv[0]);
      printf ("Usage : %s ols2pls INFILE\n", argv[0]);
      printf ("Usage : %s forward INFILE { OUTFILE }\n", argv[0]);
      printf ("Usage : %s testsol INFILE { OUTFILE }\n", argv[0]);
      printf ("Usage : %s testregsol INFILE { OUTFILE }\n", argv[0]);
      printf ("Usage : %s new {empty | regular} OUTFILE ORDER\n", argv[0]);
      printf ("Usage : %s new qcp OUTFILE ORDER HOLES {SEED}\n", argv[0]);
      printf ("Usage : %s shuffle INFILE SHUFFLES OUTFILE { SEED }\n",
	      argv[0]);
      exit (-1);
    }

  fp = fopen ("record", "a");
  if (fp == NULL)
    ourerror ("Open file failure");
  fseek (fp, 0, 2);

  sscanf (argv[1], "%s", command);

  if (strcmp (command, "poke") == 0)
    {
      if (argc < 5)
	ourerror ("Bad arguments");
      sscanf (argv[2], "%s", infile);
      sscanf (argv[3], "%d", &numhole);
      sscanf (argv[4], "%s", outfile);
      if (argc > 5)
	sscanf (argv[5], "%lu", &seed);
      else
	seed = getseed ();
      poke (infile, numhole, seed, outfile);
      printf
	("genpls: infile: %s  order: %d  holes: %d  outfile: %s  seed: %lu\n",
	 infile, order, numhole, outfile, seed);
      fprintf (fp,
	       "genpls: infile: %s  order: %d  holes: %d  outfile: %s  seed: %lu\n",
	       infile, order, numhole, outfile, seed);

    }
  else if (strcmp (command, "pls2sat+perminfo") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapvar ();
      init_mapvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_bin_bb: %d  fwd_bin_bb_frac: %f\n", command,
	      orig_holes * order - nvar,
	      ((double) (orig_holes * order - nvar)) / (orig_holes * order));


      if ((argc >= 4 && strcmp (argv[3], "minimal") == 0))
	{
	  satperminfoencode_pls (infile, 1);
	}
      else if ((argc >= 4 && strcmp (argv[3], "minimal+amo") == 0))
	{
	  satperminfoencode_pls (infile, 2);
	}
      else
	{
	  satperminfoencode_pls (infile, 0);
	}



      printf ("%s: clauses: %d\n", command, ncls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nvar);

    }
  else if (strcmp (command, "pls2sat") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapvar ();
      init_mapvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_bin_bb: %d  fwd_bin_bb_frac: %f\n", command,
	      orig_holes * order - nvar,
	      ((double) (orig_holes * order - nvar)) / (orig_holes * order));


      if ((argc >= 4 && strcmp (argv[3], "minimal") == 0))
	{
	  satencode_pls (infile, 1);
	}
      else if ((argc >= 4 && strcmp (argv[3], "minimal+amo") == 0))
	{
	  satencode_pls (infile, 2);
	}
      else
	{
	  satencode_pls (infile, 0);
	}



      printf ("%s: clauses: %d\n", command, ncls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nvar);

    }
  else if (strcmp (command, "sudokupls2sat") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_sudoku_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapvar ();
      init_mapvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_bin_bb: %d  fwd_bin_bb_frac: %f\n", command,
	      orig_holes * order - nvar,
	      ((double) (orig_holes * order - nvar)) / (orig_holes * order));



      //int sub_order = atoi(argv[3]);





      if ((argc >= 4 && strcmp (argv[3], "minimal") == 0))
	{
	  sudoku_satencode_pls (infile, 1, sub_order);
	}
      else if ((argc >= 5 && strcmp (argv[3], "minimal+amo") == 0))
	{
	  sudoku_satencode_pls (infile, 2, sub_order);
	}
      else
	{

	  sudoku_satencode_pls (infile, 0, sub_order);

	}



      printf ("%s: clauses: %d\n", command, ncls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nvar);




    }
  else if (strcmp (command, "pls2bcsp") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapvar ();
      init_mapvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_bin_bb: %d  fwd_bin_bb_frac: %f\n", command,
	      orig_holes * order - nvar,
	      ((double) (orig_holes * order - nvar)) / (orig_holes * order));


      if ((argc >= 4 && strcmp (argv[3], "minimal") == 0))
	{
	  bcspencode_pls (infile, 1);
	}
      else if ((argc >= 4 && strcmp (argv[3], "minimal+amo") == 0))
	{
	  bcspencode_pls (infile, 2);
	}
      else
	{
	  bcspencode_pls (infile, 0);
	}



      printf ("%s: clauses: %d\n", command, ncls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nvar);

    }
  else if (strcmp (command, "pls2bcsp2") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapvar ();
      init_mapvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_bin_bb: %d  fwd_bin_bb_frac: %f\n", command,
	      orig_holes * order - nvar,
	      ((double) (orig_holes * order - nvar)) / (orig_holes * order));


      if ((argc >= 4 && strcmp (argv[3], "minimal") == 0))
	{
	  bcspencode2_pls (infile, 1);
	}
      else if ((argc >= 4 && strcmp (argv[3], "minimal+amo") == 0))
	{
	  bcspencode2_pls (infile, 2);
	}
      else
	{
	  bcspencode2_pls (infile, 0);
	}



      printf ("%s: clauses: %d\n", command, ncls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nvar);

    }
  else if (strcmp (command, "pls2satPermRedundant") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapvar ();
      init_mapvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_bin_bb: %d  fwd_bin_bb_frac: %f\n", command,
	      orig_holes * order - nvar,
	      ((double) (orig_holes * order - nvar)) / (orig_holes * order));



      satpermredencode_pls (infile,
			    (argc >= 4 && strcmp (argv[3], "minimal") == 0));


      printf ("%s: clauses: %d\n", command, ncls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nvar);

    }
  else if (strcmp (command, "pls2satpuremv") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapvar ();
      init_mapvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_bin_bb: %d  fwd_bin_bb_frac: %f\n", command,
	      orig_holes * order - nvar,
	      ((double) (orig_holes * order - nvar)) / (orig_holes * order));



      satpuremvencode_pls (infile,
			   (argc >= 4 && strcmp (argv[3], "minimal") == 0));




      printf ("%s: clauses: %d\n", command, ncls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nvar);

    }
  else if (strcmp (command, "pls2satmv") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapvar ();
      init_mapvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_bin_bb: %d  fwd_bin_bb_frac: %f\n", command,
	      orig_holes * order - nvar,
	      ((double) (orig_holes * order - nvar)) / (orig_holes * order));



      satmvencode_pls (infile,
		       (argc >= 4 && strcmp (argv[3], "minimal") == 0));




      printf ("%s: clauses: %d\n", command, ncls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nvar);

    }
  else if (strcmp (command, "pls2satmvrd") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapvar ();
      init_mapvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_bin_bb: %d  fwd_bin_bb_frac: %f\n", command,
	      orig_holes * order - nvar,
	      ((double) (orig_holes * order - nvar)) / (orig_holes * order));



      satmvrdencode_pls (infile,
			 (argc >= 4 && strcmp (argv[3], "minimal") == 0));




      printf ("%s: clauses: %d\n", command, ncls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nvar);

    }
  else if (strcmp (command, "pls2regsat") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapregvar ();
      init_mapregvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  regsat_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nregvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_reg_bb: %d  fwd_reg_bb_frac: %f\n", command,
	      orig_holes - nregvar,
	      ((double) (orig_holes - nregvar)) / (orig_holes));


      if (argc >= 4)
	{
	  if (strcmp (argv[3], "dual") == 0)
	    {
	      alloc_mapdualregvar ();
	      init_mapdualregvar ();
	      printf (" reg vars after adding dual vars: %i\n", nregvar);
	      regsatencode_dual_pls (infile, 1);
	    }
	  else
	    {
	      if (strcmp (argv[3], "3Daux") == 0)
		{		/* regsatencode_3daux_pls(infile, 1 ); */
		}
	      else
		regsatencode_noheavy_pls (infile, 1);
	    }
	}
      else
	regsatencode_noheavy_pls (infile, 1);


      printf ("%s: clauses: %d\n", command, nregcls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  regsat_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nregvar);

    }
  else if (strcmp (command, "pls2reginclassic") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapregvar ();
      init_mapregvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  regsat_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nregvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_reg_bb: %d  fwd_reg_bb_frac: %f\n", command,
	      orig_holes - nregvar,
	      ((double) (orig_holes - nregvar)) / (orig_holes));

      regsatencodeinclassic_noheavy_pls (infile, 1);

      printf ("%s: clauses: %d\n", command, nregcls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  regsat_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nregvar);

    }
  else if (strcmp (command, "pls2regmv") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapregvar ();
      init_mapregvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  regsat_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nregvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_reg_bb: %d  fwd_reg_bb_frac: %f\n", command,
	      orig_holes - nregvar,
	      ((double) (orig_holes - nregvar)) / (orig_holes));

      regsatencode_noheavy_pls_regmv (infile, 1);

      printf ("%s: clauses: %d\n", command, nregcls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  regsat_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nregvar);

    }
  else if (strcmp (command, "pls2signsat") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapregvar ();
      init_mapregvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  signsat_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nregvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_reg_bb: %d  fwd_reg_bb_frac: %f\n", command,
	      orig_holes - nregvar,
	      ((double) (orig_holes - nregvar)) / (orig_holes));


      if (argc >= 4)
	{
	  if (strcmp (argv[3], "dual") == 0)
	    {
	      alloc_mapdualregvar ();
	      init_mapdualregvar ();
	      printf (" sign vars after adding dual vars: %i\n", nregvar);
	      regsatencode_dual_pls (infile, 1);
	    }
	  else
	    {
	      if (strcmp (argv[3], "3Daux") == 0)
		{		/* regsatencode_3daux_pls(infile, 1 ); */
		}
	      else
		encode_signsat_pls (infile, 1);
	    }
	}
      else
	encode_signsat_pls (infile, 1);


      printf ("%s: clauses: %d\n", command, nregcls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  regsat_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nregvar);

    }
  else if (strcmp (command, "pls2nbsignsat") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapregvar ();
      init_mapregvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  signsat_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nregvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_reg_bb: %d  fwd_reg_bb_frac: %f\n", command,
	      orig_holes - nregvar,
	      ((double) (orig_holes - nregvar)) / (orig_holes));


      if (argc >= 4)
	{
	  if (strcmp (argv[3], "dual") == 0)
	    {
	      alloc_mapdualregvar ();
	      init_mapdualregvar ();
	      printf (" sign vars after adding dual vars: %i\n", nregvar);
	      regsatencode_dual_pls (infile, 1);
	    }
	  else
	    {
	      if (strcmp (argv[3], "3Daux") == 0)
		{		/* regsatencode_3daux_pls(infile, 1 ); */
		}
	      else
		encode_nbsignsat_pls (infile, 1);
	    }
	}
      else
	encode_nbsignsat_pls (infile, 1);


      printf ("%s: clauses: %d\n", command, nregcls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  regsat_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nregvar);

    }
  else if (strcmp (command, "pls2hybsignsat") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapregvar ();
      init_mapregvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  signsat_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nregvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_reg_bb: %d  fwd_reg_bb_frac: %f\n", command,
	      orig_holes - nregvar,
	      ((double) (orig_holes - nregvar)) / (orig_holes));


      if (argc >= 4)
	{
	  if (strcmp (argv[3], "dual") == 0)
	    {
	      alloc_mapdualregvar ();
	      init_mapdualregvar ();
	      printf (" sign vars after adding dual vars: %i\n", nregvar);
	      regsatencode_dual_pls (infile, 1);
	    }
	  else
	    {
	      if (strcmp (argv[3], "3Daux") == 0)
		{		/* regsatencode_3daux_pls(infile, 1 ); */
		}
	      else
		encode_hybsignsat_pls (infile, 1);
	    }
	}
      else
	encode_hybsignsat_pls (infile, 1);


      printf ("%s: clauses: %d\n", command, nregcls);
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  regsat_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nregvar);

    }
  else if (strcmp (command, "multisample") == 0)
    {
      int orig_holes, final_holes;

      sscanf (argv[2], "%i", &order);
      sscanf (argv[3], "%s", szInitString);
      sscanf (argv[4], "%f", &rRatioIni);
      sscanf (argv[5], "%f", &rInc);
      sscanf (argv[6], "%i", &var_classes);

      sscanf (argv[7], "%i", &firstIdinstance);
      sscanf (argv[8], "%i", &numInstances);
      sscanf (argv[9], "%i", &sols);
      sscanf (argv[10], "%f", &noise);
      sscanf (argv[11], "%i", &bBothSATencodings);
      sscanf (argv[12], "%i", &bBothRegSATencodings);

      fp2dSATsamples = fopen (argv[13], "w");
      /*   if ( bBothSATencodings )
         {  */
      fp3dSATsamples = fopen (argv[14], "w");
      fp2dRegSATsamples = fopen (argv[15], "w");
      /*    if ( bBothRegSATencodings )  */
      fpDualRegSATsamples = fopen (argv[16], "w");
      /*  }
         else
         {
         fp2dRegSATsamples = fopen( argv[12], "w" );
         if ( bBothRegSATencodings )
         fpDualRegSATsamples = fopen( argv[13], "w" );
         }   */

      fprintf (fp2dRegSATsamples, " %i \n\n", var_classes);
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  fprintf (fp2dRegSATsamples, " %s%i-2d-%2.2f- %i 0\n", szInitString,
		   order, ratio, numInstances);
	}
      fprintf (fp2dRegSATsamples, "\n\n");
      fprintf (fpDualRegSATsamples, " %i \n\n", var_classes);
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  fprintf (fpDualRegSATsamples, " %s%i-dual-%2.2f- %i 0\n",
		   szInitString, order, ratio, numInstances);
	}
      fprintf (fpDualRegSATsamples, "\n\n");


      srandom (getseed ());
      /*   alloc_incidencecube();  */
      alloc_mapvar ();
      alloc_mapregvar ();
      alloc_mapdualregvar ();

      first = 1;
      new_regular ("temp");
      first = 0;
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  numhole = (float) (((float) pow (order, 1.55)) * ratio);
	  printf (" class: %i, ratio: %f\n\n", var_class, ratio);

	  for (sample = firstIdinstance;
	       sample <= (firstIdinstance + numInstances - 1); sample++)
	    {
	      shuffle ("temp", "temp2", 1000, -1);
	      printf ("Shuffle...\n");
	      sprintf (szlsname, "%s%i-%2.2f-%i", szInitString, order, ratio,
		       sample);
	      sprintf (szInsSAT2D, "%s%i-2d-%2.2f-%i", szInitString, order,
		       ratio, sample);
	      sprintf (szInsSAT3D, "%s%i-3d-%2.2f-%i", szInitString, order,
		       ratio, sample);
	      sprintf (szInsSATDual, "%s%i-dual-%2.2f-%i", szInitString,
		       order, ratio, sample);

	      poke ("temp2", numhole, -1, szlsname);
	      printf ("Poke...\n");
	      /*  printf("genpls: infile: %s  order: %d  holes: %d  outfile: %s  seed: %lu\n",
	         infile, order, numhole, outfile, seed);   */

	      read_pls_in_squarecolor (szlsname);

	      init_incidencecube ();
	      orig_holes = nunfixed ();
	      if (forward_checking_incidencecube ())
		ourerror ("incidence ??????");
	      final_holes = nunfixed ();
	      printf
		(" original num of holes: %i     final num of holes: %i\n",
		 orig_holes, final_holes);
	      init_mapvar ();
	      init_mapregvar ();

	      if (nvar == 0)
		{
		  sprintf (szInsREGSAT2D, "%s.regcnf", szInsSAT2D);
		  sprintf (szInsREGSATDual, "%s.regcnf", szInsSATDual);
		  sprintf (szInsBSAT2D, "%s.cnf", szInsSAT2D);
		  sprintf (szInsBSAT3D, "%s.cnf", szInsSAT3D);

		  fpaux = fopen (szInsREGSAT2D, "w");
		  fprintf (fpaux,
			   "c void formula\n p regcnf 2 1 1\n\n-1 1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsREGSATDual, "w");
		  fprintf (fpaux,
			   "c void formula\n p regcnf 2 1 1\n\n-1 1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsBSAT2D, "w");
		  fprintf (fpaux, "c void formula\n p cnf 1 1\n\n-1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsBSAT3D, "w");
		  fprintf (fpaux, "c void formula\n p cnf 1 1\n\n-1 0\n");
		  fclose (fpaux);
		}
	      else
		{
		  satencode_pls (szInsSAT2D, 1);
		  satencode_pls (szInsSAT3D, 0);
		  regsatencode_noheavy_pls (szInsSAT2D, 1);
		  init_mapdualregvar ();
		  regsatencode_dual_pls (szInsSATDual, 1);
		}

	      fprintf (fp2dSATsamples,
		       "walksat -best -noise %i 100 -numsol %i -tries %i -cutoff 2100000000 < %s.cnf > %s.cnf.resw%1.2f\n",
		       (int) (noise * 100.0), sols, sols, szInsSAT2D,
		       szInsSAT2D, noise);
	      fprintf (fp3dSATsamples,
		       "walksat -best -noise %i 100 -numsol %i -tries %i -cutoff 2100000000 < %s.cnf > %s.cnf.resw%1.2f\n",
		       (int) (noise * 100.0), sols, sols, szInsSAT3D,
		       szInsSAT3D, noise);
	      fprintf (fp2dRegSATsamples, " %i.regcnf", sample);
	      fprintf (fpDualRegSATsamples, " %i.regcnf", sample);


	      if (!(sample % 10))
		{
		  fprintf (fp2dRegSATsamples, "\n");
		  fprintf (fpDualRegSATsamples, "\n");
		}

	    }
	}

      fclose (fp2dSATsamples);
      /*   if ( bBothSATencodings )
         {  */
      fclose (fp3dSATsamples);
      fclose (fp2dRegSATsamples);
      /*    if ( bBothRegSATencodings )  */
      fclose (fpDualRegSATsamples);


    }
  else if (strcmp (command, "multisample2") == 0)
    {
      int orig_holes, final_holes;

      sscanf (argv[2], "%i", &order);
      sscanf (argv[3], "%s", szInitString);
      sscanf (argv[4], "%f", &rRatioIni);
      sscanf (argv[5], "%f", &rInc);
      sscanf (argv[6], "%i", &var_classes);

      sscanf (argv[7], "%i", &firstIdinstance);
      sscanf (argv[8], "%i", &numInstances);
      sscanf (argv[9], "%i", &sols);
      sscanf (argv[10], "%f", &noise);
      sscanf (argv[11], "%i", &bBothSATencodings);
      sscanf (argv[12], "%i", &bBothRegSATencodings);

      fp2dSATsamples = fopen (argv[13], "w");
      /*   if ( bBothSATencodings )
         {  */
      fp3dSATsamples = fopen (argv[14], "w");
      fp2dRegSATsamples = fopen (argv[15], "w");
      /*    if ( bBothRegSATencodings )  */
      fpDualRegSATsamples = fopen (argv[16], "w");
      /*  }
         else
         {
         fp2dRegSATsamples = fopen( argv[12], "w" );
         if ( bBothRegSATencodings )
         fpDualRegSATsamples = fopen( argv[13], "w" );
         }   */

      fprintf (fp2dRegSATsamples, " %i \n\n", var_classes);
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  fprintf (fp2dRegSATsamples, " %s%i-2d-%2.2f- %i 0\n", szInitString,
		   order, ratio, numInstances);
	}
      fprintf (fp2dRegSATsamples, "\n\n");
      fprintf (fpDualRegSATsamples, " %i \n\n", var_classes);
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  fprintf (fpDualRegSATsamples, " %s%i-dual-%2.2f- %i 0\n",
		   szInitString, order, ratio, numInstances);
	}
      fprintf (fpDualRegSATsamples, "\n\n");

      srandom (getseed ());
      /*   alloc_incidencecube();  */
      alloc_mapvar ();
      alloc_mapregvar ();
      alloc_mapdualregvar ();

      first = 1;
      new_regular ("temp");
      first = 0;
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  numhole = (float) (((float) pow (order, 1.55)) * ratio);
	  printf (" class: %i, ratio: %f\n\n", var_class, ratio);

	  for (sample = firstIdinstance;
	       sample <= (firstIdinstance + numInstances - 1); sample++)
	    {
	      /* shuffle("temp", "temp2", 1000, -1 );  printf( "Shuffle...\n" ); */
	      sprintf (szlsname, "%s%i-%2.2f-%i", szInitString, order, ratio,
		       sample);
	      sprintf (szInsSAT2D, "%s%i-2d-%2.2f-%i", szInitString, order,
		       ratio, sample);
	      sprintf (szInsSAT3D, "%s%i-3d-%2.2f-%i", szInitString, order,
		       ratio, sample);
	      sprintf (szInsSATDual, "%s%i-dual-%2.2f-%i", szInitString,
		       order, ratio, sample);

	      /* poke( "temp2", numhole, -1, szlsname ); printf( "Poke...\n" ); */
	      /*  printf("genpls: infile: %s  order: %d  holes: %d  outfile: %s  seed: %lu\n",
	         infile, order, numhole, outfile, seed);   */

	      read_pls_in_squarecolor (szlsname);

	      init_incidencecube ();
	      orig_holes = nunfixed ();
	      if (forward_checking_incidencecube ())
		ourerror ("incidence ??????");
	      final_holes = nunfixed ();
	      printf
		(" original num of holes: %i     final num of holes: %i\n",
		 orig_holes, final_holes);
	      init_mapvar ();
	      init_mapregvar ();

	      if (nvar == 0)
		{
		  sprintf (szInsREGSAT2D, "%s.regcnf", szInsSAT2D);
		  sprintf (szInsREGSATDual, "%s.regcnf", szInsSATDual);
		  sprintf (szInsBSAT2D, "%s.cnf", szInsSAT2D);
		  sprintf (szInsBSAT3D, "%s.cnf", szInsSAT3D);

		  fpaux = fopen (szInsREGSAT2D, "w");
		  fprintf (fpaux,
			   "c void formula\n p regcnf 2 1 1\n\n-1 1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsREGSATDual, "w");
		  fprintf (fpaux,
			   "c void formula\n p regcnf 2 1 1\n\n-1 1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsBSAT2D, "w");
		  fprintf (fpaux, "c void formula\n p cnf 1 1\n\n-1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsBSAT3D, "w");
		  fprintf (fpaux, "c void formula\n p cnf 1 1\n\n-1 0\n");
		  fclose (fpaux);
		}
	      else
		{
		  satencode_pls (szInsSAT2D, 1);
		  satencode_pls (szInsSAT3D, 0);
		  regsatencode_noheavy_pls (szInsSAT2D, 1);
		  init_mapdualregvar ();
		  regsatencode_dual_pls (szInsSATDual, 1);
		}

	      fprintf (fp2dSATsamples,
		       "walksat -best -noise %i 100 -numsol %i -tries %i -cutoff 2100000000 < %s.cnf > %s.cnf.resw%1.2f\n",
		       (int) (noise * 100.0), sols, sols, szInsSAT2D,
		       szInsSAT2D, noise);
	      fprintf (fp3dSATsamples,
		       "walksat -best -noise %i 100 -numsol %i -tries %i -cutoff 2100000000 < %s.cnf > %s.cnf.resw%1.2f\n",
		       (int) (noise * 100.0), sols, sols, szInsSAT3D,
		       szInsSAT3D, noise);
	      fprintf (fp2dRegSATsamples, " %i.regcnf", sample);
	      fprintf (fpDualRegSATsamples, " %i.regcnf", sample);

	      if (!(sample % 10))
		{
		  fprintf (fp2dRegSATsamples, "\n");
		  fprintf (fpDualRegSATsamples, "\n");
		}

	    }
	}

      fclose (fp2dSATsamples);
      /*   if ( bBothSATencodings )
         {  */
      fclose (fp3dSATsamples);
      fclose (fp2dRegSATsamples);
      /*    if ( bBothRegSATencodings )  */
      fclose (fpDualRegSATsamples);


    }
  else if (strcmp (command, "multisample3") == 0)
    {
      int orig_holes, final_holes;

      sscanf (argv[2], "%i", &order);
      sscanf (argv[3], "%s", szInitString);
      sscanf (argv[4], "%f", &rRatioIni);
      sscanf (argv[5], "%f", &rInc);
      sscanf (argv[6], "%i", &var_classes);

      sscanf (argv[7], "%i", &firstIdinstance);
      sscanf (argv[8], "%i", &numInstances);
      sscanf (argv[9], "%i", &sols);
      sscanf (argv[10], "%f", &noise);
      sscanf (argv[11], "%i", &bBothSATencodings);
      sscanf (argv[12], "%i", &bBothRegSATencodings);

      fp2dSATsamples = fopen (argv[13], "w");
      /*   if ( bBothSATencodings )
         {  */
      fp3dSATsamples = fopen (argv[14], "w");
      fp2dRegSATsamples = fopen (argv[15], "w");
      /*    if ( bBothRegSATencodings )  */
      fpDualRegSATsamples = fopen (argv[16], "w");
      /*  }
         else
         {
         fp2dRegSATsamples = fopen( argv[12], "w" );
         if ( bBothRegSATencodings )
         fpDualRegSATsamples = fopen( argv[13], "w" );
         }   */

      fprintf (fp2dRegSATsamples, " %i \n\n", var_classes);
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  fprintf (fp2dRegSATsamples, " %s%i-2d-%2.2f- %i 0\n", szInitString,
		   order, ratio, numInstances);
	}
      fprintf (fp2dRegSATsamples, "\n\n");
      fprintf (fpDualRegSATsamples, " %i \n\n", var_classes);
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  fprintf (fpDualRegSATsamples, " %s%i-dual-%2.2f- %i 0\n",
		   szInitString, order, ratio, numInstances);
	}
      fprintf (fpDualRegSATsamples, "\n\n");

      srandom (getseed ());
      /*   alloc_incidencecube();  */
      alloc_mapvar ();
      alloc_mapregvar ();
      alloc_mapdualregvar ();

      first = 1;
      new_regular ("temp");
      first = 0;
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  numhole = (float) (((float) pow (order, 1.55)) * ratio);
	  printf (" var_class: %i, ratio: %f\n\n", var_class, ratio);

	  for (sample = firstIdinstance;
	       sample <= (firstIdinstance + numInstances - 1); sample++)
	    {
	      /* shuffle("temp", "temp2", 1000, -1 );  printf( "Shuffle...\n" ); */
	      sprintf (szlsname, "%s%i-%2.2f-%i", szInitString, order, ratio,
		       sample);
	      sprintf (szInsSAT2D, "%s%i-2d-%2.2f-%i", szInitString, order,
		       ratio, sample);
	      sprintf (szInsSAT3D, "%s%i-3d-%2.2f-%i", szInitString, order,
		       ratio, sample);
	      sprintf (szInsSATDual, "%s%i-dual-%2.2f-%i", szInitString,
		       order, ratio, sample);

	      /* poke( "temp2", numhole, -1, szlsname ); printf( "Poke...\n" ); */
	      /*  printf("genpls: infile: %s  order: %d  holes: %d  outfile: %s  seed: %lu\n",
	         infile, order, numhole, outfile, seed);   */

	      read_pls_in_squarecolor (szlsname);

	      init_incidencecube ();
	      orig_holes = nunfixed ();
	      /* if (forward_checking_incidencecube()) ourerror("incidence ??????");
	         final_holes = nunfixed();
	         printf( " original num of holes: %i     final num of holes: %i\n", orig_holes, final_holes );  */
	      init_mapvar ();
	      init_mapregvar ();	/*  _special();   */

	      if (nvar == 0)
		{
		  sprintf (szInsREGSAT2D, "%s.regcnf", szInsSAT2D);
		  sprintf (szInsREGSATDual, "%s.regcnf", szInsSATDual);
		  sprintf (szInsBSAT2D, "%s.cnf", szInsSAT2D);
		  sprintf (szInsBSAT3D, "%s.cnf", szInsSAT3D);

		  fpaux = fopen (szInsREGSAT2D, "w");
		  fprintf (fpaux,
			   "c void formula\n p regcnf 2 1 1\n\n-1 1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsREGSATDual, "w");
		  fprintf (fpaux,
			   "c void formula\n p regcnf 2 1 1\n\n-1 1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsBSAT2D, "w");
		  fprintf (fpaux, "c void formula\n p cnf 1 1\n\n-1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsBSAT3D, "w");
		  fprintf (fpaux, "c void formula\n p cnf 1 1\n\n-1 0\n");
		  fclose (fpaux);
		}
	      else
		{
		  /*   satencode_pls( szInsSAT2D,  1 ); 
		     satencode_pls( szInsSAT3D,  0 );  */
		  regsatencode_noheavy_pls (szInsSAT2D, 1);
		  /* init_mapdualregvar();
		     regsatencode_dual_pls( szInsSATDual, 1 );  */
		}

	      fprintf (fp2dSATsamples,
		       "walksat -best -noise %i 100 -numsol %i -tries %i -cutoff 2100000000 < %s.cnf > %s.cnf.resw%1.2f\n",
		       (int) (noise * 100.0), sols, sols, szInsSAT2D,
		       szInsSAT2D, noise);
	      fprintf (fp3dSATsamples,
		       "walksat -best -noise %i 100 -numsol %i -tries %i -cutoff 2100000000 < %s.cnf > %s.cnf.resw%1.2f\n",
		       (int) (noise * 100.0), sols, sols, szInsSAT3D,
		       szInsSAT3D, noise);
	      fprintf (fp2dRegSATsamples, " %i.regcnf", sample);
	      fprintf (fpDualRegSATsamples, " %i.regcnf", sample);

	      if (!(sample % 10))
		{
		  fprintf (fp2dRegSATsamples, "\n");
		  fprintf (fpDualRegSATsamples, "\n");
		}

	    }
	}

      fclose (fp2dSATsamples);
      /*   if ( bBothSATencodings )
         {  */
      fclose (fp3dSATsamples);
      fclose (fp2dRegSATsamples);
      /*    if ( bBothRegSATencodings )  */
      fclose (fpDualRegSATsamples);


    }

  else if (strcmp (command, "multisample4") == 0)
    {
      int orig_holes, final_holes;

      sscanf (argv[2], "%i", &order);
      sscanf (argv[3], "%s", szInitString);
      sscanf (argv[4], "%f", &rRatioIni);
      sscanf (argv[5], "%f", &rInc);
      sscanf (argv[6], "%i", &var_classes);

      sscanf (argv[7], "%i", &firstIdinstance);
      sscanf (argv[8], "%i", &numInstances);
      sscanf (argv[9], "%i", &sols);
      sscanf (argv[10], "%f", &noise);
      sscanf (argv[11], "%i", &bBothSATencodings);
      sscanf (argv[12], "%i", &bBothRegSATencodings);

      fp2dSATsamples = fopen (argv[13], "w");
      /*   if ( bBothSATencodings )
         {  */
      fp3dSATsamples = fopen (argv[14], "w");
      fp2dRegSATsamples = fopen (argv[15], "w");
      /*    if ( bBothRegSATencodings )  */
      fpDualRegSATsamples = fopen (argv[16], "w");
      /*  }
         else
         {
         fp2dRegSATsamples = fopen( argv[12], "w" );
         if ( bBothRegSATencodings )
         fpDualRegSATsamples = fopen( argv[13], "w" );
         }   */

      fprintf (fp2dRegSATsamples, " %i \n\n", var_classes);
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  fprintf (fp2dRegSATsamples, " %s%i-2d-%2.2f- %i 0\n", szInitString,
		   order, ratio, numInstances);
	}
      fprintf (fp2dRegSATsamples, "\n\n");
      fprintf (fpDualRegSATsamples, " %i \n\n", var_classes);
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  fprintf (fpDualRegSATsamples, " %s%i-dual-%2.2f- %i 0\n",
		   szInitString, order, ratio, numInstances);
	}
      fprintf (fpDualRegSATsamples, "\n\n");

      srandom (getseed ());
      /*   alloc_incidencecube();  */
      alloc_mapvar ();
      alloc_mapregvar ();
      alloc_mapdualregvar ();

      first = 1;
      new_regular ("temp");
      first = 0;
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  numhole = (float) (((float) pow (order, 1.55)) * ratio);
	  printf (" var_class: %i, ratio: %f\n\n", var_class, ratio);

	  for (sample = firstIdinstance;
	       sample <= (firstIdinstance + numInstances - 1); sample++)
	    {
	      shuffle ("temp", "temp2", 1000, -1);
	      printf ("Shuffle...\n");
	      sprintf (szlsname, "%s%i-%2.2f-%i", szInitString, order, ratio,
		       sample);
	      sprintf (szInsSAT2D, "%s%i-2d-%2.2f-%i", szInitString, order,
		       ratio, sample);
	      sprintf (szInsSAT3D, "%s%i-3d-%2.2f-%i", szInitString, order,
		       ratio, sample);
	      sprintf (szInsSATDual, "%s%i-dual-%2.2f-%i", szInitString,
		       order, ratio, sample);

	      poke ("temp2", numhole, -1, szlsname);
	      printf ("Poke...\n");
	      /*  printf("genpls: infile: %s  order: %d  holes: %d  outfile: %s  seed: %lu\n",
	         infile, order, numhole, outfile, seed);   */

	      read_pls_in_squarecolor (szlsname);

	      init_incidencecube ();
	      orig_holes = nunfixed ();
	      /* if (forward_checking_incidencecube()) ourerror("incidence ??????");
	         final_holes = nunfixed();
	         printf( " original num of holes: %i     final num of holes: %i\n", orig_holes, final_holes );  */
	      init_mapvar ();
	      init_mapregvar ();

	      if (nvar == 0)
		{
		  sprintf (szInsREGSAT2D, "%s.regcnf", szInsSAT2D);
		  sprintf (szInsREGSATDual, "%s.regcnf", szInsSATDual);
		  sprintf (szInsBSAT2D, "%s.cnf", szInsSAT2D);
		  sprintf (szInsBSAT3D, "%s.cnf", szInsSAT3D);

		  fpaux = fopen (szInsREGSAT2D, "w");
		  fprintf (fpaux,
			   "c void formula\n p regcnf 2 1 1\n\n-1 1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsREGSATDual, "w");
		  fprintf (fpaux,
			   "c void formula\n p regcnf 2 1 1\n\n-1 1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsBSAT2D, "w");
		  fprintf (fpaux, "c void formula\n p cnf 1 1\n\n-1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsBSAT3D, "w");
		  fprintf (fpaux, "c void formula\n p cnf 1 1\n\n-1 0\n");
		  fclose (fpaux);
		}
	      else
		{
		  satencode_pls (szInsSAT2D, 1);
		  /*   satencode_pls( szInsSAT3D,  0 );   */
		  regsatencode_noheavy_pls (szInsSAT2D, 1);
		  /*    init_mapdualregvar();
		     regsatencode_dual_pls( szInsSATDual, 1 );   */
		}

	      fprintf (fp2dSATsamples,
		       "walksat -best -noise %i 100 -numsol %i -tries %i -cutoff 2100000000 < %s.cnf > %s.cnf.resw%1.2f\n",
		       (int) (noise * 100.0), sols, sols, szInsSAT2D,
		       szInsSAT2D, noise);
	      fprintf (fp3dSATsamples,
		       "walksat -best -noise %i 100 -numsol %i -tries %i -cutoff 2100000000 < %s.cnf > %s.cnf.resw%1.2f\n",
		       (int) (noise * 100.0), sols, sols, szInsSAT3D,
		       szInsSAT3D, noise);
	      fprintf (fp2dRegSATsamples, " %i.regcnf", sample);
	      fprintf (fpDualRegSATsamples, " %i.regcnf", sample);

	      if (!(sample % 10))
		{
		  fprintf (fp2dRegSATsamples, "\n");
		  fprintf (fpDualRegSATsamples, "\n");
		}

	    }
	}

      fclose (fp2dSATsamples);
      /*   if ( bBothSATencodings )
         {  */
      fclose (fp3dSATsamples);
      fclose (fp2dRegSATsamples);
      /*    if ( bBothRegSATencodings )  */
      fclose (fpDualRegSATsamples);


    }

  else if (strcmp (command, "multisample5") == 0)
    {
      int orig_holes, final_holes;

      sscanf (argv[2], "%i", &order);
      sscanf (argv[3], "%s", szInitString);
      sscanf (argv[4], "%f", &rRatioIni);
      sscanf (argv[5], "%f", &rInc);
      sscanf (argv[6], "%i", &var_classes);

      sscanf (argv[7], "%i", &firstIdinstance);
      sscanf (argv[8], "%i", &numInstances);
      sscanf (argv[9], "%i", &sols);
      sscanf (argv[10], "%f", &noise);
      sscanf (argv[11], "%i", &bBothSATencodings);
      sscanf (argv[12], "%i", &bBothRegSATencodings);

      fp2dSATsamples = fopen (argv[13], "w");
      /*   if ( bBothSATencodings )
         {  */
      fp3dSATsamples = fopen (argv[14], "w");
      fp2dRegSATsamples = fopen (argv[15], "w");
      /*    if ( bBothRegSATencodings )  */
      fpDualRegSATsamples = fopen (argv[16], "w");
      /*  }
         else
         {
         fp2dRegSATsamples = fopen( argv[12], "w" );
         if ( bBothRegSATencodings )
         fpDualRegSATsamples = fopen( argv[13], "w" );
         }   */

      fprintf (fp2dRegSATsamples, " %i \n\n", var_classes);
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  fprintf (fp2dRegSATsamples, " %s%i-sat2d-%2.2f- %i 0\n",
		   szInitString, order, ratio, numInstances);
	}
      fprintf (fp2dRegSATsamples, "\n\n");
      fprintf (fpDualRegSATsamples, " %i \n\n", var_classes);
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  fprintf (fpDualRegSATsamples, " %s%i-dual-%2.2f- %i 0\n",
		   szInitString, order, ratio, numInstances);
	}
      fprintf (fpDualRegSATsamples, "\n\n");

      srandom (getseed ());
      /*   alloc_incidencecube();  */
      alloc_mapvar ();
      alloc_mapregvar ();
      alloc_mapdualregvar ();

      first = 1;
      new_regular ("temp");
      first = 0;
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  numhole = (float) (((float) pow (order, 1.55)) * ratio);
	  printf (" var_class: %i, ratio: %f\n\n", var_class, ratio);

	  for (sample = firstIdinstance;
	       sample <= (firstIdinstance + numInstances - 1); sample++)
	    {
	      /* shuffle("temp", "temp2", 1000, -1 );  printf( "Shuffle...\n" ); */
	      sprintf (szlsname, "%s%i-%2.2f-%i", szInitString, order, ratio,
		       sample);
	      sprintf (szInsSAT2D, "%s%i-sat2d-%2.2f-%i", szInitString, order,
		       ratio, sample);
	      sprintf (szInsSAT3D, "%s%i-3d-%2.2f-%i", szInitString, order,
		       ratio, sample);
	      sprintf (szInsSATDual, "%s%i-dual-%2.2f-%i", szInitString,
		       order, ratio, sample);

	      /* poke( "temp2", numhole, -1, szlsname ); printf( "Poke...\n" ); */
	      /*  printf("genpls: infile: %s  order: %d  holes: %d  outfile: %s  seed: %lu\n",
	         infile, order, numhole, outfile, seed);   */

	      read_pls_in_squarecolor (szlsname);

	      init_incidencecube ();
	      orig_holes = nunfixed ();
	      /* if (forward_checking_incidencecube()) ourerror("incidence ??????");
	         final_holes = nunfixed();
	         printf( " original num of holes: %i     final num of holes: %i\n", orig_holes, final_holes );  */
	      init_mapvar ();
	      init_mapregvar ();	/*  _special();   */

	      if (nvar == 0)
		{
		  sprintf (szInsREGSAT2D, "%s.regcnf", szInsSAT2D);
		  sprintf (szInsREGSATDual, "%s.regcnf", szInsSATDual);

		  sprintf (szInsBSAT2D, "%s.regcnf", szInsSAT2D);

		  sprintf (szInsBSAT3D, "%s.cnf", szInsSAT3D);

		  fpaux = fopen (szInsREGSAT2D, "w");
		  fprintf (fpaux,
			   "c void formula\n p regcnf 2 1 1\n\n-1 1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsREGSATDual, "w");
		  fprintf (fpaux,
			   "c void formula\n p regcnf 2 1 1\n\n-1 1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsBSAT2D, "w");
		  fprintf (fpaux,
			   "c void formula\n p regcnf 2 1 1\n\n-1 1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsBSAT3D, "w");
		  fprintf (fpaux, "c void formula\n p cnf 1 1\n\n-1 0\n");
		  fclose (fpaux);
		}
	      else
		{
		  satencode_pls_asregcnf (szInsSAT2D, 1);
		}

	      fprintf (fp2dSATsamples,
		       "walksat -best -noise %i 100 -numsol %i -tries %i -cutoff 2100000000 < %s.cnf > %s.cnf.resw%1.2f\n",
		       (int) (noise * 100.0), sols, sols, szInsSAT2D,
		       szInsSAT2D, noise);
	      fprintf (fp3dSATsamples,
		       "walksat -best -noise %i 100 -numsol %i -tries %i -cutoff 2100000000 < %s.cnf > %s.cnf.resw%1.2f\n",
		       (int) (noise * 100.0), sols, sols, szInsSAT3D,
		       szInsSAT3D, noise);
	      fprintf (fp2dRegSATsamples, " %i.regcnf", sample);
	      fprintf (fpDualRegSATsamples, " %i.regcnf", sample);

	      if (!(sample % 10))
		{
		  fprintf (fp2dRegSATsamples, "\n");
		  fprintf (fpDualRegSATsamples, "\n");
		}

	    }
	}

      fclose (fp2dSATsamples);
      /*   if ( bBothSATencodings )
         {  */
      fclose (fp3dSATsamples);
      fclose (fp2dRegSATsamples);
      /*    if ( bBothRegSATencodings )  */
      fclose (fpDualRegSATsamples);


    }

  else if (strcmp (command, "multisample6") == 0)
    {
      int orig_holes, final_holes;

      sscanf (argv[2], "%i", &order);
      sscanf (argv[3], "%s", szInitString);
      sscanf (argv[4], "%f", &rRatioIni);
      sscanf (argv[5], "%f", &rInc);
      sscanf (argv[6], "%i", &var_classes);

      sscanf (argv[7], "%i", &firstIdinstance);
      sscanf (argv[8], "%i", &numInstances);
      sscanf (argv[9], "%i", &sols);
      sscanf (argv[10], "%f", &noise);
      sscanf (argv[11], "%i", &bBothSATencodings);
      sscanf (argv[12], "%i", &bBothRegSATencodings);

      fp2dSATsamples = fopen (argv[13], "w");
      /*   if ( bBothSATencodings )
         {  */
      fp3dSATsamples = fopen (argv[14], "w");
      fp2dRegSATsamples = fopen (argv[15], "w");
      /*    if ( bBothRegSATencodings )  */
      fpDualRegSATsamples = fopen (argv[16], "w");
      /*  }
         else
         {
         fp2dRegSATsamples = fopen( argv[12], "w" );
         if ( bBothRegSATencodings )
         fpDualRegSATsamples = fopen( argv[13], "w" );
         }   */

      fprintf (fp2dRegSATsamples, " %i \n\n", var_classes);
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  fprintf (fp2dRegSATsamples, " %s%i-sat3d-%2.2f- %i 0\n",
		   szInitString, order, ratio, numInstances);
	}
      fprintf (fp2dRegSATsamples, "\n\n");
      fprintf (fpDualRegSATsamples, " %i \n\n", var_classes);
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  fprintf (fpDualRegSATsamples, " %s%i-dual-%2.2f- %i 0\n",
		   szInitString, order, ratio, numInstances);
	}
      fprintf (fpDualRegSATsamples, "\n\n");

      srandom (getseed ());
      /*   alloc_incidencecube();  */
      alloc_mapvar ();
      alloc_mapregvar ();
      alloc_mapdualregvar ();

      first = 1;
      new_regular ("temp");
      first = 0;
      for (var_class = 1, ratio = rRatioIni; var_class <= var_classes;
	   var_class++, ratio += rInc)
	{
	  numhole = (float) (((float) pow (order, 1.55)) * ratio);
	  printf (" var_class: %i, ratio: %f\n\n", var_class, ratio);

	  for (sample = firstIdinstance;
	       sample <= (firstIdinstance + numInstances - 1); sample++)
	    {
	      /* shuffle("temp", "temp2", 1000, -1 );  printf( "Shuffle...\n" ); */
	      sprintf (szlsname, "%s%i-%2.2f-%i", szInitString, order, ratio,
		       sample);
	      sprintf (szInsSAT2D, "%s%i-sat3d-%2.2f-%i", szInitString, order,
		       ratio, sample);
	      sprintf (szInsSAT3D, "%s%i-3d-%2.2f-%i", szInitString, order,
		       ratio, sample);
	      sprintf (szInsSATDual, "%s%i-dual-%2.2f-%i", szInitString,
		       order, ratio, sample);

	      /* poke( "temp2", numhole, -1, szlsname ); printf( "Poke...\n" ); */
	      /*  printf("genpls: infile: %s  order: %d  holes: %d  outfile: %s  seed: %lu\n",
	         infile, order, numhole, outfile, seed);   */

	      read_pls_in_squarecolor (szlsname);

	      init_incidencecube ();
	      orig_holes = nunfixed ();
	      /* if (forward_checking_incidencecube()) ourerror("incidence ??????");
	         final_holes = nunfixed();
	         printf( " original num of holes: %i     final num of holes: %i\n", orig_holes, final_holes );  */
	      init_mapvar ();
	      init_mapregvar ();	/*  _special();   */

	      if (nvar == 0)
		{
		  sprintf (szInsREGSAT2D, "%s.regcnf", szInsSAT2D);
		  sprintf (szInsREGSATDual, "%s.regcnf", szInsSATDual);

		  sprintf (szInsBSAT2D, "%s.regcnf", szInsSAT2D);

		  sprintf (szInsBSAT3D, "%s.cnf", szInsSAT3D);

		  fpaux = fopen (szInsREGSAT2D, "w");
		  fprintf (fpaux,
			   "c void formula\n p regcnf 2 1 1\n\n-1 1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsREGSATDual, "w");
		  fprintf (fpaux,
			   "c void formula\n p regcnf 2 1 1\n\n-1 1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsBSAT2D, "w");
		  fprintf (fpaux,
			   "c void formula\n p regcnf 2 1 1\n\n-1 1 0\n");
		  fclose (fpaux);
		  fpaux = fopen (szInsBSAT3D, "w");
		  fprintf (fpaux, "c void formula\n p cnf 1 1\n\n-1 0\n");
		  fclose (fpaux);
		}
	      else
		{
		  satencode_pls_asregcnf (szInsSAT2D, 0);
		}

	      fprintf (fp2dSATsamples,
		       "walksat -best -noise %i 100 -numsol %i -tries %i -cutoff 2100000000 < %s.cnf > %s.cnf.resw%1.2f\n",
		       (int) (noise * 100.0), sols, sols, szInsSAT2D,
		       szInsSAT2D, noise);
	      fprintf (fp3dSATsamples,
		       "walksat -best -noise %i 100 -numsol %i -tries %i -cutoff 2100000000 < %s.cnf > %s.cnf.resw%1.2f\n",
		       (int) (noise * 100.0), sols, sols, szInsSAT3D,
		       szInsSAT3D, noise);
	      fprintf (fp2dRegSATsamples, " %i.regcnf", sample);
	      fprintf (fpDualRegSATsamples, " %i.regcnf", sample);

	      if (!(sample % 10))
		{
		  fprintf (fp2dRegSATsamples, "\n");
		  fprintf (fpDualRegSATsamples, "\n");
		}

	    }
	}

      fclose (fp2dSATsamples);
      /*   if ( bBothSATencodings )
         {  */
      fclose (fp3dSATsamples);
      fclose (fp2dRegSATsamples);
      /*    if ( bBothRegSATencodings )  */
      fclose (fpDualRegSATsamples);


    }

  else if (strcmp (command, "forward") == 0)
    {
      int orig_holes, final_holes;
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      orig_holes = nunfixed ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      final_holes = nunfixed ();
      alloc_mapvar ();
      init_mapvar ();

      printf
	("%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	 command, infile, order, orig_holes, final_holes, nvar);
      printf ("%s: fwd_csp_bb: %d  fwd_csp_bb_frac: %f\n", command,
	      orig_holes - final_holes,
	      ((double) (orig_holes - final_holes)) / orig_holes);
      printf ("%s: fwd_bin_bb: %d  fwd_bin_bb_frac: %f\n", command,
	      orig_holes * order - nvar,
	      ((double) (orig_holes * order - nvar)) / (orig_holes * order));
      if (argc > 3)
	{
	  sscanf (argv[3], "%s", outfile);
	  write_pls (outfile);
	  printf ("%s: outfile: %s\n", command, outfile);
	}
      fprintf (fp,
	       "%s: infile: %s  order: %d  holes: %d  csp_vars: %d  binary_vars: %d\n",
	       command, infile, order, orig_holes, final_holes, nvar);

    }
  else if (strcmp (command, "testsol") == 0)
    {
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      alloc_mapvar ();
      init_mapvar ();
      fprintf (fp, "testsol: infile: %s  order: %d  binary_vars: %d\n",
	       infile, order, nvar);
      read_varval (infile);
      test_validity_squarecolor ();
      printf ("testsol: The solution is valid\n");
      if (argc > 3)
	{
	  sscanf (argv[3], "%s", outfile);
	  write_pls (outfile);
	}

    }
  else if (strcmp (command, "testregsol") == 0)
    {
      if (argc < 3)
	ourerror ("Bad arguments");

      sscanf (argv[2], "%s", infile);
      read_pls_in_squarecolor (infile);
      alloc_incidencecube ();
      init_incidencecube ();
      if (forward_checking_incidencecube ())
	ourerror ("incidence ??????");
      alloc_mapregvar ();
      init_mapregvar ();
      fprintf (fp, "testsol: infile: %s  order: %d  reg_vars: %d\n", infile,
	       order, nregvar);

      if (argc >= 4 && strcmp (argv[3], "noheavy") == 0)
	read_regvarval_noheavy (infile);
      else
	read_regvarval (infile);

      test_validity_squarecolor ();
      printf ("testsol: The solution is valid\n");
      if (argc > 4)
	{
	  sscanf (argv[4], "%s", outfile);
	  write_pls (outfile);
	}

    }
  else if (strcmp (command, "new") == 0)
    {
      if (argc < 5)
	ourerror ("Bad arguments to new");
      sscanf (argv[3], "%s", outfile);
      sscanf (argv[4], "%d", &order);
      if (strcmp (argv[2], "empty") == 0)
	new_empty (outfile);
      else if (strcmp (argv[2], "regular") == 0)
	new_regular (outfile);
      else if (strcmp (argv[2], "qcp") == 0)
	{
	  if (argc < 6)
	    ourerror ("Bad arguments");
	  sscanf (argv[5], "%d", &numhole);
	  if (argc > 6)
	    sscanf (argv[6], "%lu", &seed);
	  else
	    seed = getseed ();
	  new_qcp (outfile, numhole, seed);
	}
      else
	ourerror ("Bad argument to new");
      if (strcmp (argv[2], "qcp") == 0)
	{
	  fprintf (fp,
		   "new: type: %s  lsname: %s  order: %d  numhole: %d  seed: %d\n",
		   argv[2], outfile, order, numhole, seed);
	  printf
	    ("new: type: %s  lsname: %s  order: %d  numhole: %d  seed: %d\n",
	     argv[2], outfile, order, numhole, seed);
	}
      else
	{
	  fprintf (fp, "new: type: %s  lsname: %s  order: %d\n", argv[2],
		   outfile, order);
	  printf ("new: type: %s  lsname: %s  order: %d\n", argv[2], outfile,
		  order);
	}

    }
  else if (strcmp (command, "shuffle") == 0)
    {
      if (argc < 5)
	ourerror ("Bad arguments");
      sscanf (argv[2], "%s", infile);
      sscanf (argv[3], "%d", &shuffles);
      sscanf (argv[4], "%s", outfile);
      if (argc > 5)
	sscanf (argv[5], "%lu", &seed);
      else
	seed = getseed ();
      shuffle (infile, outfile, shuffles, seed);
      fprintf (fp,
	       "shuffle: infile: %s  outfile: %s  shuffles: %d  seed: %lu\n",
	       infile, outfile, shuffles, seed);
      printf ("shuffle: infile: %s  outfile: %s  shuffles: %d  seed: %lu\n",
	      infile, outfile, shuffles, seed);

    }
  else if (strcmp (command, "ols2pls") == 0)
    {
      if (argc < 3)
	ourerror ("Bad arguments");
      sscanf (argv[2], "%s", infile);
      read_ols_in_squarecolor (infile);
      write_pls (infile);
      fprintf (fp, "ols2pls: infile: %s  order: %d\n", infile, order);
      printf ("ols2pls: infile: %s  order: %d\n", infile, order);

    }
  else
    printf ("Unknown command: %s\n", command);


  fclose (fp);
/*
    if (strcmp(command, "sudokupls2sat") == 0) {
      vector<int> result;
      //run satz to solve
      char command[100];
      sprintf(command,"./%s %s.cnf > %s.statistics",satz_solver,infile,infile);
      fprintf(stdout,"%s\n",command);
     
      system(command);
      
      FILE* fp_in=fopen("satx.sol", "r");	 
      if (fp_in == NULL) {
	fprintf(stderr,"Can not open satx.sol file\n");
	exit(-2);
      };
      //fprintf(stdout,"hola\n");
      int var;
      
      for (i=0; i<nvar; i++) {
	fscanf(fp_in, "%d ", &var);
	if(var<0) {
	  result.push_back(0);
	}else {
	  result.push_back(1);
	}
      }	 
      fclose(fp_in);	
    
      //output the result
      for (int i = 0; i < order; i++) {
	for (int j = 0; j < order; j++) {
	  for (int k = 0; k < order; k++) {
	    if (incidencecube[i][j][k] == -1) {
	    if(result[mapvar[i][j][k]]) {
	      //fprintf(stdout,"var %d at %d,%d color %d\n",mapvar[i][j][k],i,j,k);
	      squarecolor[i][j] = k;
	      squarefixed[i][j] = 1;
	    }
	    }
	  }
	}
      }

      
      char file_name[MAX_NAME_LENGTH];
      sprintf(file_name,"res-%s",infile);
      write_pls(file_name);       
    }

*/

  exit (0);
}

/******************************************************************************/

int
sudoku_satencode_pls (char *outfile, int minflag, int row_order)
{



  char satfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k1;
  int i2, j2, k2;
  int flag = 0;
  int nlit;
  int lncls;			/* local version of ncls */


  strcpy (satfilename, outfile);
  strcat (satfilename, ".cnf");

  lncls = 0;




loop:

  fp = fopen (satfilename, "w");
  fprintf (fp, "p cnf %d %d\n", nvar, ncls);

  int column_order = (order / row_order);

  //additional constraints for sudoku
  for (int h1 = 0; h1 < column_order; h1++)
    {
      for (int h2 = 0; h2 < row_order; h2++)
	{


	  //detect which colors are already assigned into the submatrix.
	  vector < int >colors;
	  colors.resize (order);
	  for (int p = 0; p < order; p++)
	    {
	      colors[p] = 0;
	    }

	  for (k = 0; k < order; k++)
	    {
	      for (j = (h1 * row_order); j < ((h1 + 1) * row_order); j++)
		{
		  for (i = (h2 * column_order); i < ((h2 + 1) * column_order);
		       i++)
		    {
		      if (incidencecube[i][j][k] == 1)
			{
			  colors[k] = 1;
			}
		    }
		}
	    }


	  for (k = 0; k < order; k++)
	    {
	      //grab elements
	      vector < int >v;
	      for (j = (h1 * row_order); j < ((h1 + 1) * row_order); j++)
		{
		  for (i = (h2 * column_order); i < ((h2 + 1) * column_order);
		       i++)
		    {
		      if (incidencecube[i][j][k] == -1)
			{

			  //si el color ya esta en la submatriz
			  if (colors[k])
			    {
			      //fprintf(stdout,"color already  ");
			      fprintf (fp, "%8d 0\n", -(mapvar[i][j][k] + 1));
			      //fprintf(stdout,"** h1=[%d]h2=[%d]k=[%d]i=[%d]j=[%d] == %d\n",h1,h2,k,j,i,mapvar[i][j][k]);
			      lncls++;
			    }
			  else
			    {
			      v.push_back (mapvar[i][j][k]);
			      //fprintf(stdout,"-- h1=[%d]h2=[%d]k=[%d]i=[%d]j=[%d] == %d\n",h1,h2,k,j,i,mapvar[i][j][k]);
			    }

			}
		      else
			{
			  //fprintf(stdout,"   h1=[%d]h2=[%d]k=[%d]i=[%d]j=[%d] == %d\n",h1,h2,k,j,i,mapvar[i][j][k]);
			}
		    }
		}

	      //getchar();

	      //output mutual exclusion
	      if (v.size () > 0)
		{
		  for (int e1 = 0; e1 < (v.size () - 1); e1++)
		    {
		      for (int e2 = e1 + 1; e2 < v.size (); e2++)
			{

			  fprintf (fp, "%8d", -(v[e1] + 1));
			  fprintf (fp, "%8d", -(v[e2] + 1));
			  fprintf (fp, "%8d\n", 0);


			  //fprintf(stdout, "%8d", -(v[e1] ));
			  //fprintf(stdout, "%8d", -(v[e2] ));
			  //fprintf(stdout, "%8d\n", 0);
			  lncls++;
			}
		    }
		}
	    }
	}
    }


  if (minflag != 1)
    {
      /* Binary mutex constraint: no two colors assigned to the same square */
      for (i = 0; i < order; i++)
	{
	  for (j = 0; j < order; j++)
	    {
	      for (k1 = 0; k1 < order - 1; k1++)
		{
		  if (incidencecube[i][j][k1] == -1)
		    {
		      for (k2 = k1 + 1; k2 < order; k2++)
			{
			  if (incidencecube[i][j][k2] == -1)
			    {
			      fprintf (fp, "%8d", -(mapvar[i][j][k1] + 1));
			      fprintf (fp, "%8d", -(mapvar[i][j][k2] + 1));
			      fprintf (fp, "%8d\n", 0);
			      lncls++;
			    }
			}
		    }
		}
	    }
	}
    }


  /* Binary mutex constraint: a color can appear at most once in each row */
  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  for (i1 = 0; i1 < order - 1; i1++)
	    {
	      if (incidencecube[i1][j][k] == -1)
		{
		  for (i2 = i1 + 1; i2 < order; i2++)
		    {
		      if (incidencecube[i2][j][k] == -1)
			{
			  fprintf (fp, "%8d", -(mapvar[i1][j][k] + 1));
			  fprintf (fp, "%8d", -(mapvar[i2][j][k] + 1));
			  fprintf (fp, "%8d\n", 0);
			  lncls++;
			}
		    }
		}
	    }
	}
    }


  /* Binary mutex constraint: a color can appear at most once in each column */
  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  for (j1 = 0; j1 < order - 1; j1++)
	    {
	      if (incidencecube[i][j1][k] == -1)
		{
		  for (j2 = j1 + 1; j2 < order; j2++)
		    {
		      if (incidencecube[i][j2][k] == -1)
			{
			  fprintf (fp, "%8d", -(mapvar[i][j1][k] + 1));
			  fprintf (fp, "%8d", -(mapvar[i][j2][k] + 1));
			  fprintf (fp, "%8d\n", 0);
			  lncls++;
			}
		    }
		}
	    }
	}
    }

  /* long positive clause: some color must be assigned to each uncolored square */
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  nlit = 0;
	  for (k = 0; k < order; k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "%8d\n", 0);
	      lncls++;
	    }
	}
    }

  if (minflag == 0)
    {
      /* long positive clause: each color must appear at least once in each row */
      for (j = 0; j < order; j++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (i = 0; i < order; i++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%8d\n", 0);
		  lncls++;
		}
	    }
	}
      /* long positive clause: each color must appear at least once in each column */
      for (i = 0; i < order; i++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (j = 0; j < order; j++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%8d\n", 0);
		  lncls++;
		}
	    }
	}
    }

  fclose (fp);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      ncls = lncls;
      goto loop;
    }

  return (0);
}


int
satencode_pls (char *outfile, int minflag)
{
  char satfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k1;
  int i2, j2, k2;
  int flag = 0;
  int nlit;
  int lncls;			/* local version of ncls */

  strcpy (satfilename, outfile);
  strcat (satfilename, ".cnf");

  lncls = 0;



  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (satfilename, "w");
  fprintf (fp, "p cnf %d %d\n", nvar, ncls);

  if (minflag != 1)
    {
      /* Binary mutex constraint: no two colors assigned to the same square */
      for (i = 0; i < order; i++)
	{
	  for (j = 0; j < order; j++)
	    {
	      for (k1 = 0; k1 < order - 1; k1++)
		{
		  if (incidencecube[i][j][k1] == -1)
		    {
		      for (k2 = k1 + 1; k2 < order; k2++)
			{
			  if (incidencecube[i][j][k2] == -1)
			    {
			      fprintf (fp, "%8d", -(mapvar[i][j][k1] + 1));
			      fprintf (fp, "%8d", -(mapvar[i][j][k2] + 1));
			      fprintf (fp, "%8d\n", 0);
			      lncls++;
			    }
			}
		    }
		}
	    }
	}
    }

  /* Binary mutex constraint: a color can appear at most once in each row */
  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  for (i1 = 0; i1 < order - 1; i1++)
	    {
	      if (incidencecube[i1][j][k] == -1)
		{
		  for (i2 = i1 + 1; i2 < order; i2++)
		    {
		      if (incidencecube[i2][j][k] == -1)
			{
			  fprintf (fp, "%8d", -(mapvar[i1][j][k] + 1));
			  fprintf (fp, "%8d", -(mapvar[i2][j][k] + 1));
			  fprintf (fp, "%8d\n", 0);
			  lncls++;
			}
		    }
		}
	    }
	}
    }


  /* Binary mutex constraint: a color can appear at most once in each column */
  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  for (j1 = 0; j1 < order - 1; j1++)
	    {
	      if (incidencecube[i][j1][k] == -1)
		{
		  for (j2 = j1 + 1; j2 < order; j2++)
		    {
		      if (incidencecube[i][j2][k] == -1)
			{
			  fprintf (fp, "%8d", -(mapvar[i][j1][k] + 1));
			  fprintf (fp, "%8d", -(mapvar[i][j2][k] + 1));
			  fprintf (fp, "%8d\n", 0);
			  lncls++;
			}
		    }
		}
	    }
	}
    }

  /* long positive clause: some color must be assigned to each uncolored square */
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  nlit = 0;
	  for (k = 0; k < order; k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "%8d\n", 0);
	      lncls++;
	    }
	}
    }

  if (minflag == 0)
    {
      /* long positive clause: each color must appear at least once in each row */
      for (j = 0; j < order; j++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (i = 0; i < order; i++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%8d\n", 0);
		  lncls++;
		}
	    }
	}
      /* long positive clause: each color must appear at least once in each column */
      for (i = 0; i < order; i++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (j = 0; j < order; j++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%8d\n", 0);
		  lncls++;
		}
	    }
	}
    }

  fclose (fp);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      ncls = lncls;
      goto loop;
    }

  return (0);
}

int
satperminfoencode_pls (char *outfile, int minflag)
{
  char satfilename[MAX_NAME_LENGTH];
  char mvfilename[MAX_NAME_LENGTH];
  char permfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k1;
  int i2, j2, k2;
  int flag = 0;
  int nlit;
  int lncls;			/* local version of ncls */

  FILE *f_mv = NULL;
  FILE *f_perm = NULL;
  int mv_size = 0;
  int current_mv_length = 0;
  int flag_perm = 0;
  int count = 0;
  int *var = NULL;
  int perm_size = 0;

  strcpy (satfilename, outfile);
  strcat (satfilename, ".cnf");

  lncls = 0;


  var = (int *) malloc (sizeof (int) * order * order);



  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (satfilename, "w");
  fprintf (fp, "p cnf %d %d\n", nvar, ncls);
  //fprintf(stdout, "p cnf %d %d\n", nvar, ncls);

  if (minflag != 1)
    {
      /* Binary mutex constraint: no two colors assigned to the same square */
      for (i = 0; i < order; i++)
	{
	  for (j = 0; j < order; j++)
	    {
	      for (k1 = 0; k1 < order - 1; k1++)
		{
		  if (incidencecube[i][j][k1] == -1)
		    {
		      for (k2 = k1 + 1; k2 < order; k2++)
			{
			  if (incidencecube[i][j][k2] == -1)
			    {
			      fprintf (fp, "%8d", -(mapvar[i][j][k1] + 1));
			      fprintf (fp, "%8d", -(mapvar[i][j][k2] + 1));
			      fprintf (fp, "%8d\n", 0);
			      lncls++;
			    }
			}
		    }
		}
	    }
	}
    }

  /* Binary mutex constraint: a color can appear at most once in each row */
  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  for (i1 = 0; i1 < order - 1; i1++)
	    {
	      if (incidencecube[i1][j][k] == -1)
		{
		  for (i2 = i1 + 1; i2 < order; i2++)
		    {
		      if (incidencecube[i2][j][k] == -1)
			{
			  fprintf (fp, "%8d", -(mapvar[i1][j][k] + 1));
			  fprintf (fp, "%8d", -(mapvar[i2][j][k] + 1));
			  fprintf (fp, "%8d\n", 0);
			  lncls++;
			}
		    }
		}
	    }
	}
    }
  /* Binary mutex constraint: a color can appear at most once in each column */
  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  for (j1 = 0; j1 < order - 1; j1++)
	    {
	      if (incidencecube[i][j1][k] == -1)
		{
		  for (j2 = j1 + 1; j2 < order; j2++)
		    {
		      if (incidencecube[i][j2][k] == -1)
			{
			  fprintf (fp, "%8d", -(mapvar[i][j1][k] + 1));
			  fprintf (fp, "%8d", -(mapvar[i][j2][k] + 1));
			  fprintf (fp, "%8d\n", 0);
			  lncls++;
			}
		    }
		}
	    }
	}
    }

  /* long positive clause: some color must be assigned to each uncolored square */
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  nlit = 0;
	  for (k = 0; k < order; k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "%8d\n", 0);
	      lncls++;
	    }
	}
    }



  if (minflag == 0)
    {
      /* long positive clause: each color must appear at least once in each row */
      for (j = 0; j < order; j++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (i = 0; i < order; i++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%8d\n", 0);
		  lncls++;
		}
	    }
	}
      /* long positive clause: each color must appear at least once in each column */
      for (i = 0; i < order; i++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (j = 0; j < order; j++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%8d\n", 0);
		  lncls++;
		}
	    }
	}
    }


  strcpy (mvfilename, outfile);
  strcat (mvfilename, ".cnf.mv");
  f_mv = fopen (mvfilename, "w");

  strcpy (permfilename, outfile);
  strcat (permfilename, ".cnf.perm");
  f_perm = fopen (permfilename, "w");

  //escribiendo la cabecera.
  fprintf (f_mv, "MV %d\n", mv_size);

  mv_size = 0;
  /* long positive clause: some color must be assigned to each uncolored square */
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{

	  current_mv_length = 0;
	  //capturando la longitud de la multivaluada.
	  for (k = 0; k < order; k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  current_mv_length++;
		}
	    }
	  //escribiendo la multivaluada.
	  if (current_mv_length > 0)
	    {
	      fprintf (f_mv, "%d ", current_mv_length);
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {

		      fprintf (f_mv, "%d(%d) ", mapvar[i][j][k] + 1, k);

		    }
		}
	      fprintf (f_mv, "\n");


	      *(var + (i * order) + j) = mv_size;
	      mv_size++;
	    }

	}
    }

  fprintf (f_perm, "PERM %d\n", perm_size);

  for (i = 0; i < order; i++)
    {
      count = 0;

      //cuantas hay en la fila.
      for (j = 0; j < order; j++)
	{
	  flag_perm = 0;
	  for (k = 0; (k < order) && (flag_perm == 0); k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  flag_perm++;
		}
	    }
	  //variable en la permutacion.
	  if (flag_perm)
	    {
	      count++;
	    }
	}

      if (count > 0)
	{
	  fprintf (f_perm, "%d ", count);

	  for (j = 0; j < order; j++)
	    {
	      flag_perm = 0;
	      for (k = 0; (k < order) && (flag_perm == 0); k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      flag_perm++;
		    }
		}
	      //variable en la permutacion.
	      if (flag_perm)
		{
		  //escribir variable.
		  fprintf (f_perm, "%d ", (*(var + (i * order) + j)) + 1);
		}
	    }

	  fprintf (f_perm, "\n");
	  perm_size++;
	}

    }




  for (i = 0; i < order; i++)
    {
      count = 0;
      for (j = 0; j < order; j++)
	{
	  flag_perm = 0;
	  for (k = 0; (k < order) && (flag_perm == 0); k++)
	    {
	      if (incidencecube[j][i][k] == -1)
		{
		  flag_perm++;
		}
	    }
	  //variable en la permutacion.
	  if (flag_perm)
	    {
	      //escribir variable.
	      count++;
	    }
	}

      if (count > 0)
	{
	  fprintf (f_perm, "%d ", count);

	  for (j = 0; j < order; j++)
	    {
	      flag_perm = 0;
	      for (k = 0; (k < order) && (flag_perm == 0); k++)
		{
		  if (incidencecube[j][i][k] == -1)
		    {
		      flag_perm++;
		    }
		}
	      //variable en la permutacion.
	      if (flag_perm)
		{
		  //escribir variable.
		  fprintf (f_perm, "%d ", (*(var + (j * order) + i)) + 1);
		  count++;
		}
	    }


	  //fin de la permutacion.

	  fprintf (f_perm, "\n");
	  perm_size++;
	}
    }


  fclose (f_mv);
  fclose (f_perm);
  fclose (fp);










  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      ncls = lncls;
      goto loop;
    }

  return (0);
}



int
bcspencode_pls (char *outfile, int minflag)
{
  char satfilename[MAX_NAME_LENGTH];
  char permfilename[MAX_NAME_LENGTH];
  FILE *fp;
  FILE *fperm;

  int i, j, k;
  int i1, j1, k1;
  int i2, j2, k2;
  int flag = 0;
  int nlit;
  int lncls;			/* local version of ncls */

  int *var;
  int num_var = 0, num_color = 0, num_c = 0;

  strcpy (satfilename, outfile);
  strcat (satfilename, ".bcsp");

  strcpy (permfilename, outfile);
  strcat (permfilename, ".bcsp.perm");



  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

  fp = fopen (satfilename, "w");

  fperm = fopen (permfilename, "w");

  var = (int *) malloc (sizeof (int) * order * order);
  //poner a -1 todas las posiciones.
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  *(var + (i * order) + j) = -1;
	}
    }



  num_var = 0;

  fprintf (fp, "#\n");
  /* long positive clause: some color must be assigned to each uncolored square */
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  nlit = 0;

	  num_color = 0;
	  for (k = 0; k < order; k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  if (nlit == 0)
		    {
		      *(var + (i * order) + j) = num_var;
		      num_var++;
		      fprintf (fp, "%d ", *(var + (i * order) + j));
		    }
		  mapvar[i][j][k] = num_color;

		  num_color++;
		  fprintf (fp, "%d ", mapvar[i][j][k]);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "*\n", 0);
	      lncls++;
	    }
	}
    }
  fprintf (fp, "#\n");

  //escribir las permutaciones de fila
  fprintf (fperm, "ROW PERM\n");
  for (i = 0; i < order; i++)
    {
      fprintf (fperm, "PERM");
      for (j = 0; j < order; j++)
	{
	  if (*(var + (i * order) + j) != -1)
	    {
	      fprintf (fperm, " %d", *(var + (i * order) + j));
	    }
	}
      fprintf (fperm, "\n");
    }

  //escribir las permutaciones de columna
  fprintf (fperm, "COLUMN PERM\n");
  for (i = 0; i < order; i++)
    {
      fprintf (fperm, "PERM");
      for (j = 0; j < order; j++)
	{
	  if (*(var + (j * order) + i) != -1)
	    {
	      fprintf (fperm, " %d", *(var + (j * order) + i));
	    }
	}
      fprintf (fperm, "\n");
    }




  /* Binary mutex constraint: a color can appear at most once in each row */
  for (j = 0; j < order; j++)
    {
      for (i1 = 0; i1 < order - 1; i1++)
	{
	  for (i2 = i1 + 1; i2 < order; i2++)
	    {
	      nlit = 0;
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[j][i1][k] == -1)
		    {
		      if (incidencecube[j][i2][k] == -1)
			{
			  if (nlit == 0)
			    {
			      fprintf (fp, "c%d nogood %d %d\n", num_c,
				       *(var + (j * order) + i1),
				       *(var + (j * order) + i2));
			      nlit++;
			      num_c++;
			    }
			  fprintf (fp, "%d ", mapvar[j][i1][k]);
			  fprintf (fp, "%d ", mapvar[j][i2][k]);
			  fprintf (fp, "\n");
			  lncls++;
			}
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "*\n", 0);
		}
	    }
	}
    }

  /* Binary mutex constraint: a color can appear at most once in each column */
  for (i = 0; i < order; i++)
    {
      for (j1 = 0; j1 < order - 1; j1++)
	{
	  for (j2 = j1 + 1; j2 < order; j2++)
	    {
	      nlit = 0;
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[j1][i][k] == -1)
		    {
		      if (incidencecube[j2][i][k] == -1)
			{
			  if (nlit == 0)
			    {
			      fprintf (fp, "c%d nogood %d %d\n", num_c,
				       *(var + (j1 * order) + i),
				       *(var + (j2 * order) + i));
			      nlit++;
			      num_c++;
			    }
			  fprintf (fp, "%d ", mapvar[j1][i][k]);
			  fprintf (fp, "%d ", mapvar[j2][i][k]);
			  fprintf (fp, "\n");
			  lncls++;


			}
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "*\n", 0);
		}
	    }
	}
    }

  fprintf (fp, "#\n");
  fclose (fp);
  fclose (fperm);

  return (0);
}


int
bcspencode2_pls (char *outfile, int minflag)
{
  char satfilename[MAX_NAME_LENGTH];
  char permfilename[MAX_NAME_LENGTH];
  char mapfilename[MAX_NAME_LENGTH];

  FILE *fp;
  FILE *fperm;
  FILE *fmap;

  int perm_num = 0;;
  int num = 0;

  int i, j, k;
  int i1, j1, k1;
  int i2, j2, k2;
  int flag = 0;
  int nlit;
  int lncls;			/* local version of ncls */

  int *var;
  int num_var = 0, num_color = 0, num_c = 0;

  strcpy (satfilename, outfile);
  strcat (satfilename, ".bcsp");

  strcpy (permfilename, outfile);
  strcat (permfilename, ".bcsp.perm");

  strcpy (mapfilename, outfile);
  strcat (mapfilename, ".bcsp.cnf.map");

loop:

  fmap = fopen (mapfilename, "w");
  //escribimos el mapeo entre filas y columnas.
  fprintf (fmap, "BOOLEAN_VAR %d ORDER %d\n", nvar, order);


  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  for (k = 0; k < order; k++)
	    {
	      if (mapvar[i][j][k] != -1)
		{
		  fprintf (fmap, "%d %d %d %d \n", mapvar[i][j][k] + 1, i, j,
			   k);
		}
	    }
	}
    }

  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

  fp = fopen (satfilename, "w");

  fperm = fopen (permfilename, "w");

  var = (int *) malloc (sizeof (int) * order * order);
  //poner a -1 todas las posiciones.
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  *(var + (i * order) + j) = -1;
	}
    }

  num_var = 0;
  fprintf (fp, "#\n");
  /* long positive clause: some color must be assigned to each uncolored square */
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  nlit = 0;
	  num_color = 0;
	  for (k = 0; k < order; k++)
	    {

	      if (incidencecube[i][j][k] == -1)
		{

		  if (nlit == 0)
		    {
		      *(var + (i * order) + j) = num_var;
		      num_var++;
		      fprintf (fp, "%d ", (i * order) + j);
		    }


		  num_color++;
		  fprintf (fp, "%d ", k);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "*\n", 0);
	      lncls++;
	    }
	}
    }
  fprintf (fp, "#\n");

  fprintf (fperm, "PERM %d ORDER %d\n", perm_num, order);
  //escribir las permutaciones de fila
  for (i = 0; i < order; i++)
    {
      num = 0;
      for (j = 0; j < order; j++)
	{
	  if (*(var + (i * order) + j) != -1)
	    {
	      num++;
	    }
	}

      if (num > 0)
	{
	  fprintf (fperm, " %d", num);
	  for (j = 0; j < order; j++)
	    {
	      if (*(var + (i * order) + j) != -1)
		{
		  fprintf (fperm, " %d", (i * order) + j);
		}
	    }
	  fprintf (fperm, "\n");
	  perm_num++;
	}


    }

  //escribir las permutaciones de columna

  for (i = 0; i < order; i++)
    {
      num = 0;

      for (j = 0; j < order; j++)
	{
	  if (*(var + (j * order) + i) != -1)
	    {

	      num++;
	    }
	}

      if (num > 0)
	{
	  fprintf (fperm, " %d", num);
	  for (j = 0; j < order; j++)
	    {
	      if (*(var + (j * order) + i) != -1)
		{
		  fprintf (fperm, " %d", (j * order) + i);

		}

	    }
	  fprintf (fperm, "\n");
	  perm_num++;
	}


    }




  /* Binary mutex constraint: a color can appear at most once in each row */
  for (j = 0; j < order; j++)
    {
      for (i1 = 0; i1 < order - 1; i1++)
	{
	  for (i2 = i1 + 1; i2 < order; i2++)
	    {
	      nlit = 0;
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[j][i1][k] == -1)
		    {
		      if (incidencecube[j][i2][k] == -1)
			{
			  if (nlit == 0)
			    {
			      fprintf (fp, "c%d nogood %d %d\n", num_c,
				       (j * order) + i1, (j * order) + i2);
			      nlit++;
			      num_c++;
			    }
			  fprintf (fp, "%d ", k);
			  fprintf (fp, "%d ", k);
			  fprintf (fp, "\n");
			  lncls++;
			}
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "*\n", 0);
		}
	    }
	}
    }

  /* Binary mutex constraint: a color can appear at most once in each column */
  for (i = 0; i < order; i++)
    {
      for (j1 = 0; j1 < order - 1; j1++)
	{
	  for (j2 = j1 + 1; j2 < order; j2++)
	    {
	      nlit = 0;
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[j1][i][k] == -1)
		    {
		      if (incidencecube[j2][i][k] == -1)
			{
			  if (nlit == 0)
			    {
			      fprintf (fp, "c%d nogood %d %d\n", num_c,
				       (j1 * order) + i, (j2 * order) + i);
			      nlit++;
			      num_c++;
			    }
			  fprintf (fp, "%d ", k);
			  fprintf (fp, "%d ", k);
			  fprintf (fp, "\n");
			  lncls++;


			}
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "*\n", 0);
		}
	    }
	}
    }

  fprintf (fp, "#\n");
  fclose (fp);
  fclose (fperm);
  fclose (fmap);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      ncls = lncls;
      goto loop;
    }
  return (0);
}



int
satpermredencode_pls (char *outfile, int minflag)
{
  char satfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k1;
  int i2, j2, k2;
  int flag = 0;
  int nlit;
  int lncls;			/* local version of ncls */

  strcpy (satfilename, outfile);
  strcat (satfilename, ".cnf");

  lncls = 0;

  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (satfilename, "w");
  fprintf (fp, "p cnf %d %d\n", nvar, ncls);

  if (minflag != 1)
    {
      /* Binary mutex constraint: no two colors assigned to the same square */
      for (i = 0; i < order; i++)
	{
	  for (j = 0; j < order; j++)
	    {
	      for (k1 = 0; k1 < order - 1; k1++)
		{
		  if (incidencecube[i][j][k1] == -1)
		    {
		      for (k2 = k1 + 1; k2 < order; k2++)
			{
			  if (incidencecube[i][j][k2] == -1)
			    {
			      fprintf (fp, "%8d", -(mapvar[i][j][k1] + 1));
			      fprintf (fp, "%8d", -(mapvar[i][j][k2] + 1));
			      fprintf (fp, "%8d\n", 0);
			      lncls++;
			    }
			}
		    }
		}
	    }
	}
    }

  /* Binary mutex constraint: a color can appear at most once in each row */
  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  for (i1 = 0; i1 < order - 1; i1++)
	    {
	      if (incidencecube[i1][j][k] == -1)
		{
		  for (i2 = i1 + 1; i2 < order; i2++)
		    {
		      if (incidencecube[i2][j][k] == -1)
			{
			  fprintf (fp, "%8d", -(mapvar[i1][j][k] + 1));
			  fprintf (fp, "%8d", -(mapvar[i2][j][k] + 1));
			  fprintf (fp, "%8d\n", 0);
			  lncls++;

			  fprintf (fp, "%8d", -(mapvar[i1][j][k] + 1));
			  fprintf (fp, "%8d", -(mapvar[i2][j][k] + 1));
			  fprintf (fp, "%8d\n", 0);
			  lncls++;

			}
		    }
		}
	    }
	}
    }
  /* Binary mutex constraint: a color can appear at most once in each column */
  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  for (j1 = 0; j1 < order - 1; j1++)
	    {
	      if (incidencecube[i][j1][k] == -1)
		{
		  for (j2 = j1 + 1; j2 < order; j2++)
		    {
		      if (incidencecube[i][j2][k] == -1)
			{
			  fprintf (fp, "%8d", -(mapvar[i][j1][k] + 1));
			  fprintf (fp, "%8d", -(mapvar[i][j2][k] + 1));
			  fprintf (fp, "%8d\n", 0);
			  lncls++;
			  fprintf (fp, "%8d", -(mapvar[i][j1][k] + 1));
			  fprintf (fp, "%8d", -(mapvar[i][j2][k] + 1));
			  fprintf (fp, "%8d\n", 0);
			  lncls++;

			}
		    }
		}
	    }
	}
    }

  /* long positive clause: some color must be assigned to each uncolored square */
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  nlit = 0;
	  for (k = 0; k < order; k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "%8d\n", 0);
	      lncls++;
	    }
	}
    }

  if (minflag == 0)
    {
      /* long positive clause: each color must appear at least once in each row */
      for (j = 0; j < order; j++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (i = 0; i < order; i++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%8d\n", 0);
		  lncls++;
		}
	    }
	}
      /* long positive clause: each color must appear at least once in each column */
      for (i = 0; i < order; i++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (j = 0; j < order; j++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%8d\n", 0);
		  lncls++;
		}
	    }
	}
    }

  fclose (fp);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      ncls = lncls;
      goto loop;
    }

  return (0);
}



//escribe la descripcion multivaluada tanto de
//las mv como de la columna y fila.

int
satmvencode_pls (char *outfile, int minflag)
{
  char satfilename[MAX_NAME_LENGTH];
  char mvfilename[MAX_NAME_LENGTH];
  FILE *f_mv;
  FILE *fp;
  int i, j, k;
  int i1, j1, k1;
  int i2, j2, k2;
  int flag = 0;
  int nlit;
  int lncls;			/* local version of ncls */
  int q;
  int mv_size = 0;
  int current_mv_length = 0;

  strcpy (satfilename, outfile);
  strcat (satfilename, ".cnf");


  strcpy (mvfilename, outfile);
  strcat (mvfilename, ".cnf.mv");


  lncls = 0;

  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (satfilename, "w");

  fprintf (fp, "p cnf %d %d\n", nvar, ncls);

  if (!minflag)
    {


      /*
         // Binary mutex constraint: no two colors assigned to the same square
         for (i = 0; i < order; i++) {
         for (j = 0; j < order; j++) {
         for (k1 = 0; k1 < order -1 ; k1++) {
         if (incidencecube[i][j][k1] == -1) {
         for (k2 = k1 + 1; k2 < order; k2++) {
         if (incidencecube[i][j][k2] == -1) {
         fprintf(fp, "%8d", -(mapvar[i][j][k1] + 1));
         fprintf(fp, "%8d", -(mapvar[i][j][k2] + 1));
         fprintf(fp, "%8d\n", 0);
         lncls++;
         }
         }
         }
         }
         }
         }

       */
    }


  /*
     // Binary mutex constraint: a color can appear at most once in each row 
     for (j = 0; j < order; j++) {
     for (k = 0; k < order; k++) {
     for (i1 = 0; i1 < order - 1 ; i1++) {
     if (incidencecube[i1][j][k] == -1) {
     for (i2 = i1 + 1; i2 < order; i2++) {
     if (incidencecube[i2][j][k] == -1) {
     fprintf(fp, "%8d", -(mapvar[i1][j][k] + 1));
     fprintf(fp, "%8d", -(mapvar[i2][j][k] + 1));
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     }
     }
     }
   */

  /*
     // Binary mutex constraint: a color can appear at most once in each column 
     for (i = 0; i < order; i++) {
     for (k = 0; k < order; k++) {
     for (j1 = 0; j1 < order - 1 ; j1++) {
     if (incidencecube[i][j1][k] == -1) {
     for (j2 = j1 + 1; j2 < order; j2++) {
     if (incidencecube[i][j2][k] == -1) {
     fprintf(fp, "%8d", -(mapvar[i][j1][k] + 1));
     fprintf(fp, "%8d", -(mapvar[i][j2][k] + 1));
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     }
     }
     }
   */


  /* long positive clause: some color must be assigned to each uncolored square */
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  nlit = 0;
	  for (k = 0; k < order; k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "%8d\n", 0);
	      lncls++;
	    }
	}
    }

  if (!minflag)
    {
      /* long positive clause: each color must appear at least once in each row */
      for (j = 0; j < order; j++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (i = 0; i < order; i++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%8d\n", 0);
		  lncls++;
		}
	    }
	}
      /* long positive clause: each color must appear at least once in each column */
      for (i = 0; i < order; i++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (j = 0; j < order; j++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%8d\n", 0);
		  lncls++;
		}
	    }
	}
    }

  //escribiendo la cabecera.
  f_mv = fopen (mvfilename, "w");
  fprintf (f_mv, "MV %d\n", mv_size);

  /* long positive clause: some color must be assigned to each uncolored square */

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{

	  current_mv_length = 0;
	  //capturando la longitud de la multivaluada.
	  for (k = 0; k < order; k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  current_mv_length++;
		}
	    }

	  //escribiendo la multivaluada.
	  if (current_mv_length > 1)
	    {
	      fprintf (f_mv, "%d ", current_mv_length);
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {

		      fprintf (f_mv, "%d ", mapvar[i][j][k] + 1);

		    }
		}
	      fprintf (f_mv, "\n");
	      mv_size++;
	    }

	}
    }

  if (!minflag)
    {
      /* long positive clause: each color must appear at least once in each row */
      for (j = 0; j < order; j++)
	{
	  for (k = 0; k < order; k++)
	    {
	      current_mv_length = 0;
	      for (i = 0; i < order; i++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      current_mv_length++;
		    }
		}

	      if (current_mv_length > 1)
		{
		  fprintf (f_mv, "%d ", current_mv_length);
		  for (i = 0; i < order; i++)
		    {
		      if (incidencecube[i][j][k] == -1)
			{
			  fprintf (f_mv, "%d ", mapvar[i][j][k] + 1);

			}
		    }
		  fprintf (f_mv, "\n");
		  mv_size++;
		}


	    }
	}

      /* long positive clause: each color must appear at least once in each column */

      for (i = 0; i < order; i++)
	{
	  for (k = 0; k < order; k++)
	    {
	      current_mv_length = 0;
	      for (j = 0; j < order; j++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      current_mv_length++;
		    }
		}

	      if (current_mv_length > 1)
		{
		  fprintf (f_mv, "%d ", current_mv_length);

		  for (j = 0; j < order; j++)
		    {
		      if (incidencecube[i][j][k] == -1)
			{
			  fprintf (f_mv, "%d ", mapvar[i][j][k] + 1);
			}
		    }
		  fprintf (f_mv, "\n");
		  mv_size++;
		}

	    }
	}

    }


  fclose (fp);
  fclose (f_mv);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      ncls = lncls;
      goto loop;
    }

  return (0);
}

/******************************************************************************/

//Para minimal.
//--- Descripcion multivaludada de primales
//---AMO-2 y AMO-3
//Sin minimal
//--- Descripcion multivaludada de primales
//---AMO-2 y AMO-3
//---ALO-2 y ALO-3

//Faltaria mirar el dar la descripcion de todas las multivauadas,
//y luego conectarlas por ejemplo con equivalencias, aunque
//eso podria incluir el reductor, si se da cuenta que hay mv
//que comparten literales clasicos.


int
satmvrdencode_pls (char *outfile, int minflag)
{
  char satfilename[MAX_NAME_LENGTH];
  char mvfilename[MAX_NAME_LENGTH];
  FILE *f_mv;
  FILE *fp;
  int i, j, k;
  int i1, j1, k1;
  int i2, j2, k2;
  int flag = 0;
  int nlit;
  int lncls;			/* local version of ncls */
  int q;
  int mv_size = 0;
  int current_mv_length = 0;

  strcpy (satfilename, outfile);
  strcat (satfilename, ".cnf");


  strcpy (mvfilename, outfile);
  strcat (mvfilename, ".cnf.mv");


  lncls = 0;

  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (satfilename, "w");

  fprintf (fp, "p cnf %d %d\n", nvar, ncls);

  if (!minflag)
    {


      /*
         // Binary mutex constraint: no two colors assigned to the same square
         for (i = 0; i < order; i++) {
         for (j = 0; j < order; j++) {
         for (k1 = 0; k1 < order -1 ; k1++) {
         if (incidencecube[i][j][k1] == -1) {
         for (k2 = k1 + 1; k2 < order; k2++) {
         if (incidencecube[i][j][k2] == -1) {
         fprintf(fp, "%8d", -(mapvar[i][j][k1] + 1));
         fprintf(fp, "%8d", -(mapvar[i][j][k2] + 1));
         fprintf(fp, "%8d\n", 0);
         lncls++;
         }
         }
         }
         }
         }
         }
       */

    }



  // Binary mutex constraint: a color can appear at most once in each row 
  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  for (i1 = 0; i1 < order - 1; i1++)
	    {
	      if (incidencecube[i1][j][k] == -1)
		{
		  for (i2 = i1 + 1; i2 < order; i2++)
		    {
		      if (incidencecube[i2][j][k] == -1)
			{
			  fprintf (fp, "%8d", -(mapvar[i1][j][k] + 1));
			  fprintf (fp, "%8d", -(mapvar[i2][j][k] + 1));
			  fprintf (fp, "%8d\n", 0);
			  lncls++;
			}
		    }
		}
	    }
	}
    }



  // Binary mutex constraint: a color can appear at most once in each column 
  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  for (j1 = 0; j1 < order - 1; j1++)
	    {
	      if (incidencecube[i][j1][k] == -1)
		{
		  for (j2 = j1 + 1; j2 < order; j2++)
		    {
		      if (incidencecube[i][j2][k] == -1)
			{
			  fprintf (fp, "%8d", -(mapvar[i][j1][k] + 1));
			  fprintf (fp, "%8d", -(mapvar[i][j2][k] + 1));
			  fprintf (fp, "%8d\n", 0);
			  lncls++;
			}
		    }
		}
	    }
	}
    }



  /* long positive clause: some color must be assigned to each uncolored square */
  /*
     for (i = 0; i < order; i++) {
     for (j = 0; j < order; j++) {
     nlit = 0;
     for (k = 0; k < order ; k++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
   */


  if (!minflag)
    {

      /* long positive clause: each color must appear at least once in each row */

      for (j = 0; j < order; j++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (i = 0; i < order; i++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%8d\n", 0);
		  lncls++;
		}
	    }
	}

      /* long positive clause: each color must appear at least once in each column */

      for (i = 0; i < order; i++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (j = 0; j < order; j++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%8d\n", 0);
		  lncls++;
		}
	    }
	}

    }

  //escribiendo la cabecera.
  f_mv = fopen (mvfilename, "w");
  fprintf (f_mv, "MV %d\n", mv_size);

  /* long positive clause: some color must be assigned to each uncolored square */

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{

	  current_mv_length = 0;
	  //capturando la longitud de la multivaluada.
	  for (k = 0; k < order; k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  current_mv_length++;
		}
	    }

	  //escribiendo la multivaluada.
	  if (current_mv_length > 1)
	    {
	      fprintf (f_mv, "%d ", current_mv_length);
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {

		      fprintf (f_mv, "%d ", mapvar[i][j][k] + 1);

		    }
		}
	      fprintf (f_mv, "\n");
	      mv_size++;
	    }

	}
    }

  if (!minflag)
    {
      /* long positive clause: each color must appear at least once in each row */
      /*
         for (j = 0; j < order; j++) {
         for (k = 0; k < order; k++) {
         current_mv_length = 0;
         for (i = 0; i < order; i++) {
         if (incidencecube[i][j][k] == -1) {
         current_mv_length++;
         }
         }

         if(current_mv_length>1) {
         fprintf(f_mv,"%d ",current_mv_length);
         for (i = 0; i < order; i++) {
         if (incidencecube[i][j][k] == -1) {
         fprintf(f_mv, "%d ", mapvar[i][j][k] + 1);

         }
         }
         fprintf(f_mv, "\n");
         mv_size++;
         }


         }
         }
       */
      /* long positive clause: each color must appear at least once in each column */
      /*
         for (i = 0; i < order; i++) {
         for (k = 0; k < order ; k++) {
         current_mv_length = 0;
         for (j = 0; j < order; j++) {
         if (incidencecube[i][j][k] == -1) {
         current_mv_length++;
         }
         }

         if(current_mv_length>1) {
         fprintf(f_mv,"%d ",current_mv_length);

         for (j = 0; j < order; j++) {
         if (incidencecube[i][j][k] == -1) {
         fprintf(f_mv, "%d ", mapvar[i][j][k] + 1);
         }
         }
         fprintf(f_mv, "\n");
         mv_size++;
         }

         }
         }
       */
    }


  fclose (fp);
  fclose (f_mv);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      ncls = lncls;
      goto loop;
    }

  return (0);
}

//escribe la descripcion multivaluada tanto de
//las mv como de la columna y fila.

int
satpuremvencode_pls (char *outfile, int minflag)
{
  char satfilename[MAX_NAME_LENGTH];
  char mvfilename[MAX_NAME_LENGTH];
  FILE *f_mv;
  FILE *fp;
  int i, j, k;
  int i1, j1, k1;
  int i2, j2, k2;
  int flag = 0;
  int nlit;
  int lncls;			/* local version of ncls */
  int q;
  int mv_size = 0;
  int current_mv_length = 0;

  strcpy (satfilename, outfile);
  strcat (satfilename, ".cnf");


  strcpy (mvfilename, outfile);
  strcat (mvfilename, ".cnf.mv");


  lncls = 0;

  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (satfilename, "w");

  fprintf (fp, "p cnf %d %d\n", nvar, ncls);
  /*
     if (!minflag){
     // Binary mutex constraint: no two colors assigned to the same square 
     for (i = 0; i < order; i++) {
     for (j = 0; j < order; j++) {
     for (k1 = 0; k1 < order -1 ; k1++) {
     if (incidencecube[i][j][k1] == -1) {
     for (k2 = k1 + 1; k2 < order; k2++) {
     if (incidencecube[i][j][k2] == -1) {
     fprintf(fp, "%8d", -(mapvar[i][j][k1] + 1));
     fprintf(fp, "%8d", -(mapvar[i][j][k2] + 1));
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     }
     }
     }
     }
   */
  /*
     // Binary mutex constraint: a color can appear at most once in each row 
     for (j = 0; j < order; j++) {
     for (k = 0; k < order; k++) {
     for (i1 = 0; i1 < order - 1 ; i1++) {
     if (incidencecube[i1][j][k] == -1) {
     for (i2 = i1 + 1; i2 < order; i2++) {
     if (incidencecube[i2][j][k] == -1) {
     fprintf(fp, "%8d", -(mapvar[i1][j][k] + 1));
     fprintf(fp, "%8d", -(mapvar[i2][j][k] + 1));
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     }
     }
     }
   */

  /*
     // Binary mutex constraint: a color can appear at most once in each column 
     for (i = 0; i < order; i++) {
     for (k = 0; k < order; k++) {
     for (j1 = 0; j1 < order - 1 ; j1++) {
     if (incidencecube[i][j1][k] == -1) {
     for (j2 = j1 + 1; j2 < order; j2++) {
     if (incidencecube[i][j2][k] == -1) {
     fprintf(fp, "%8d", -(mapvar[i][j1][k] + 1));
     fprintf(fp, "%8d", -(mapvar[i][j2][k] + 1));
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     }
     }
     }

   */

  /* long positive clause: some color must be assigned to each uncolored square */
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  nlit = 0;
	  for (k = 0; k < order; k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "%8d\n", 0);
	      lncls++;
	    }
	}
    }

  if (!minflag)
    {
      /* long positive clause: each color must appear at least once in each row */
      for (j = 0; j < order; j++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (i = 0; i < order; i++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%8d\n", 0);
		  lncls++;
		}
	    }
	}
      /* long positive clause: each color must appear at least once in each column */
      for (i = 0; i < order; i++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (j = 0; j < order; j++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, "%8d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%8d\n", 0);
		  lncls++;
		}
	    }
	}
    }

  //escribiendo la cabecera.
  f_mv = fopen (mvfilename, "w");
  fprintf (f_mv, "MV %d\n", mv_size);

  /* long positive clause: some color must be assigned to each uncolored square */

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{

	  current_mv_length = 0;
	  //capturando la longitud de la multivaluada.
	  for (k = 0; k < order; k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  current_mv_length++;
		}
	    }

	  //escribiendo la multivaluada.
	  if (current_mv_length > 1)
	    {
	      fprintf (f_mv, "%d ", current_mv_length);
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {

		      fprintf (f_mv, "%d ", mapvar[i][j][k] + 1);

		    }
		}
	      fprintf (f_mv, "\n");
	      mv_size++;
	    }

	}
    }

  if (!minflag)
    {
      /* long positive clause: each color must appear at least once in each row */
      for (j = 0; j < order; j++)
	{
	  for (k = 0; k < order; k++)
	    {
	      current_mv_length = 0;
	      for (i = 0; i < order; i++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      current_mv_length++;
		    }
		}

	      if (current_mv_length > 1)
		{
		  fprintf (f_mv, "%d ", current_mv_length);
		  for (i = 0; i < order; i++)
		    {
		      if (incidencecube[i][j][k] == -1)
			{
			  fprintf (f_mv, "%d ", mapvar[i][j][k] + 1);

			}
		    }
		  fprintf (f_mv, "\n");
		  mv_size++;
		}


	    }
	}

      /* long positive clause: each color must appear at least once in each column */

      for (i = 0; i < order; i++)
	{
	  for (k = 0; k < order; k++)
	    {
	      current_mv_length = 0;
	      for (j = 0; j < order; j++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      current_mv_length++;
		    }
		}

	      if (current_mv_length > 1)
		{
		  fprintf (f_mv, "%d ", current_mv_length);

		  for (j = 0; j < order; j++)
		    {
		      if (incidencecube[i][j][k] == -1)
			{
			  fprintf (f_mv, "%d ", mapvar[i][j][k] + 1);
			}
		    }
		  fprintf (f_mv, "\n");
		  mv_size++;
		}

	    }
	}

    }


  fclose (fp);
  fclose (f_mv);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      ncls = lncls;
      goto loop;
    }

  return (0);
}

/******************************************************************************/




int
satencode_pls_asregcnf (char *outfile, int minflag)
{
  char satfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k1;
  int i2, j2, k2;
  int flag = 0;
  int nlit;
  int lncls;			/* local version of ncls */

  strcpy (satfilename, outfile);
  strcat (satfilename, ".regcnf");

  lncls = 0;

  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (satfilename, "w");
  fprintf (fp, "c cnf as regcnf\np regcnf 2 %d %d\n", nvar, ncls);


  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == 0)
		    {
		      fprintf (fp, " -1 %6d 0\n", (mapvar[i][j][k] + 1));
		      lncls++;
		    }
		}
	    }
	}
    }


  if (!minflag)
    {
      /* Binary mutex constraint: no two colors assigned to the same square */
      for (i = 0; i < order; i++)
	{
	  for (j = 0; j < order; j++)
	    {
	      for (k1 = 0; k1 < order - 1; k1++)
		{
		  if (incidencecube[i][j][k1] == -1)
		    {
		      for (k2 = k1 + 1; k2 < order; k2++)
			{
			  if (incidencecube[i][j][k2] == -1)
			    {
			      fprintf (fp, " -1 %6d", (mapvar[i][j][k1] + 1));
			      fprintf (fp, " -1 %6d", (mapvar[i][j][k2] + 1));
			      fprintf (fp, "%3d\n", 0);
			      lncls++;
			    }
			}
		    }
		}
	    }
	}
    }

  /* Binary mutex constraint: a color can appear at most once in each row */
  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  for (i1 = 0; i1 < order - 1; i1++)
	    {
	      if (incidencecube[i1][j][k] == -1)
		{
		  for (i2 = i1 + 1; i2 < order; i2++)
		    {
		      if (incidencecube[i2][j][k] == -1)
			{
			  fprintf (fp, " -1 %6d", (mapvar[i1][j][k] + 1));
			  fprintf (fp, " -1 %6d", (mapvar[i2][j][k] + 1));
			  fprintf (fp, "%3d\n", 0);
			  lncls++;
			}
		    }
		}
	    }
	}
    }
  /* Binary mutex constraint: a color can appear at most once in each column */
  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  for (j1 = 0; j1 < order - 1; j1++)
	    {
	      if (incidencecube[i][j1][k] == -1)
		{
		  for (j2 = j1 + 1; j2 < order; j2++)
		    {
		      if (incidencecube[i][j2][k] == -1)
			{
			  fprintf (fp, " -1 %6d", (mapvar[i][j1][k] + 1));
			  fprintf (fp, " -1 %6d", (mapvar[i][j2][k] + 1));
			  fprintf (fp, "%3d\n", 0);
			  lncls++;
			}
		    }
		}
	    }
	}
    }

  /* long positive clause: some color must be assigned to each uncolored square */
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  nlit = 0;
	  for (k = 0; k < order; k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  fprintf (fp, " 2 %6d", mapvar[i][j][k] + 1);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "%3d\n", 0);
	      lncls++;
	    }
	}
    }

  if (!minflag)
    {
      /* long positive clause: each color must appear at least once in each row */
      for (j = 0; j < order; j++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (i = 0; i < order; i++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, " 2 %4d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%3d\n", 0);
		  lncls++;
		}
	    }
	}
      /* long positive clause: each color must appear at least once in each column */
      for (i = 0; i < order; i++)
	{
	  for (k = 0; k < order; k++)
	    {
	      nlit = 0;
	      for (j = 0; j < order; j++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      fprintf (fp, " 2 %4d", mapvar[i][j][k] + 1);
		      nlit++;
		    }
		}
	      if (nlit > 0)
		{
		  fprintf (fp, "%3d\n", 0);
		  lncls++;
		}
	    }
	}
    }

  fclose (fp);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      ncls = lncls;
      goto loop;
    }

  return (0);
}

/******************************************************************************/






int
regsatencode_pls (char *outfile, int minflag)
{
  char regsatfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k1;
  int i2, j2, k2;
  int flag = 0;
  int nlit, colors, cu;
  int lnregcls;			/* local version of ncls */
  int leftcolor, rightcolor;
  int *row;

  strcpy (regsatfilename, outfile);
  strcat (regsatfilename, ".regcnf");

  lnregcls = 0;

  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (regsatfilename, "w");
  fprintf (fp,
	   "c lattin square problem encoded as regsat\np regcnf %d %d %d\n",
	   order, nregvar, nregcls);


  /* Binary mutex constraint: a color can appear at most once in each row */

  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  for (i1 = 0; i1 < order - 1; i1++)
	    {
	      if (incidencecube[i1][j][k] == -1)
		{
		  for (i2 = i1 + 1; i2 < order; i2++)
		    {
		      if (incidencecube[i2][j][k] == -1)
			{
			  if (k > 0)
			    fprintf (fp, " %8d %8d", -(k),
				     (mapregvar[i1][j] + 1));
			  if (k < (order - 1))
			    fprintf (fp, " %8d %8d", (k + 2),
				     (mapregvar[i1][j] + 1));
			  if (k > 0)
			    fprintf (fp, " %8d %8d", -(k),
				     (mapregvar[i2][j] + 1));
			  if (k < (order - 1))
			    fprintf (fp, " %8d %8d", (k + 2),
				     (mapregvar[i2][j] + 1));
			  fprintf (fp, " %8d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }
  /* Binary mutex constraint: a color can appear at most once in each column */

  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  for (j1 = 0; j1 < order - 1; j1++)
	    {
	      if (incidencecube[i][j1][k] == -1)
		{
		  for (j2 = j1 + 1; j2 < order; j2++)
		    {
		      if (incidencecube[i][j2][k] == -1)
			{
			  if (k > 0)
			    fprintf (fp, " %8d %8d", -(k),
				     (mapregvar[i][j1] + 1));
			  if (k < (order - 1))
			    fprintf (fp, " %8d %8d", (k + 2),
				     (mapregvar[i][j1] + 1));
			  if (k > 0)
			    fprintf (fp, " %8d %8d", -(k),
				     (mapregvar[i][j2] + 1));
			  if (k < (order - 1))
			    fprintf (fp, " %8d %8d", (k + 2),
				     (mapregvar[i][j2] + 1));
			  fprintf (fp, " %8d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }


  /* Ensure that the free cells do not receive a color yet used in its row or column */


  row = (int *) malloc (sizeof (int) * (order + 1));

  for (j = 0; j < order; j++)
    {

      for (colors = 1; colors <= order; colors++)
	row[colors] = 0;
      colors = 0;

      for (i1 = 0; i1 < order; i1++)
	{
	  if (squarefixed[i1][j] == 1)
	    {
	      row[squarecolor[i1][j] + 1] = 1;
	      colors++;
	    }
	}

      /* Now we put all the used colors one next the other in ascending order */

      if (colors)
	{
	  row[0] = colors;
	  for (cu = 1, colors = 1; colors <= order; colors++)
	    {
	      if (row[colors])
		row[cu++] = colors;
	    }


	  /*  Now we can generate the clauses that ensure that used colors are not used in
	     free cells of the row  */

	  for (i1 = 0; i1 < order; i1++)
	    {
	      if (squarefixed[i1][j] != 1)
		{
		  cu = 1;
		  leftcolor = row[cu++];
		  while (cu <= row[0])
		    {
		      while ((cu <= row[0]) && (row[cu] == (row[cu - 1] + 1)))
			cu++;
		      if (cu <= row[0])
			{
			  rightcolor = row[cu - 1];
			  if (leftcolor > 1)
			    fprintf (fp, " %8d %8d", -(leftcolor - 1),
				     (mapregvar[i1][j] + 1));
			  if (rightcolor < order)
			    fprintf (fp, " %8d %8d", (rightcolor + 1),
				     (mapregvar[i1][j] + 1));
			  fprintf (fp, " %8d\n", 0);
			  lnregcls++;
			  leftcolor = row[cu++];
			}
		    }
		  rightcolor = row[cu - 1];
		  if (leftcolor > 1)
		    fprintf (fp, " %8d %8d", -(leftcolor - 1),
			     (mapregvar[i1][j] + 1));
		  if (rightcolor < order)
		    fprintf (fp, " %8d %8d", (rightcolor + 1),
			     (mapregvar[i1][j] + 1));
		  fprintf (fp, " %8d\n", 0);
		  lnregcls++;
		}
	    }
	}
    }



  for (i = 0; i < order; i++)
    {

      for (colors = 1; colors <= order; colors++)
	row[colors] = 0;
      colors = 0;

      for (j1 = 0; j1 < order; j1++)
	{
	  if (squarefixed[i][j1] == 1)
	    {
	      row[squarecolor[i][j1] + 1] = 1;
	      colors++;
	    }
	}

      /* Now we put all the used colors one next the other in ascending order */

      if (colors)
	{
	  row[0] = colors;
	  for (cu = 1, colors = 1; colors <= order; colors++)
	    {
	      if (row[colors])
		row[cu++] = colors;
	    }


	  /*  Now we can generate the clauses that ensure that used colors
	     are not used in free cells of the column  */

	  for (j1 = 0; j1 < order; j1++)
	    {
	      if (squarefixed[i][j1] != 1)
		{
		  cu = 1;
		  leftcolor = row[cu++];
		  while (cu <= row[0])
		    {
		      while ((cu <= row[0]) && (row[cu] == (row[cu - 1] + 1)))
			cu++;
		      if (cu <= row[0])
			{
			  rightcolor = row[cu - 1];
			  if (leftcolor > 1)
			    fprintf (fp, " %8d %8d", -(leftcolor - 1),
				     (mapregvar[i][j1] + 1));
			  if (rightcolor < order)
			    fprintf (fp, " %8d %8d", (rightcolor + 1),
				     (mapregvar[i][j1] + 1));
			  fprintf (fp, " %8d\n", 0);
			  lnregcls++;
			  leftcolor = row[cu++];
			}
		    }
		  rightcolor = row[cu - 1];
		  if (leftcolor > 1)
		    fprintf (fp, " %8d %8d", -(leftcolor - 1),
			     (mapregvar[i][j1] + 1));
		  if (rightcolor < order)
		    fprintf (fp, " %8d %8d", (rightcolor + 1),
			     (mapregvar[i][j1] + 1));
		  fprintf (fp, " %8d\n", 0);
		  lnregcls++;
		}
	    }
	}
    }




  /*   This part will be done when regularSAT  + monosignedSAT is available 

     if (!minflag){
     long positive clause: each color must appear at least once in each row  
     for (j = 0; j < order; j++) {
     for (k = 0; k < order; k++) {
     nlit = 0;
     for (i = 0; i < order; i++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     long positive clause: each color must appear at least once in each column 
     for (i = 0; i < order; i++) {
     for (k = 0; k < order ; k++) {
     nlit = 0;
     for (j = 0; j < order; j++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     }

   */


  fclose (fp);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      nregcls = lnregcls;
      goto loop;
    }

  return (0);
}

/******************************************************************************/




int
regsatencode2_pls (char *outfile, int minflag)
{
  char regsatfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k1;
  int i2, j2, k2;
  int flag = 0;
  int nlit, colors, cu;
  int lnregcls;			/* local version of ncls */
  int leftcolor, rightcolor, tvalue;
  int *row;
  double rVar, rMean;
  int t, samples = 0;
  double rgSample[6000];

  strcpy (regsatfilename, outfile);
  strcat (regsatfilename, ".regcnf");

  lnregcls = 0;
  rMean = 0.0;
  rVar = 0.0;

  /* Let's map the available colors for every non-fixed square to a truth value */

  maxColors = 2;
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      tvalue = 0;
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      mapcolregvar[i][j][k + 1] = ++tvalue;
		    }
		}
	      mapcolregvar[i][j][0] = tvalue;	/* printf( "(%i %i %i)", i, j, tvalue );  */
	      if (maxColors < tvalue)
		maxColors = tvalue;
	      rMean += tvalue;
	      rgSample[samples++] = tvalue;
	    }
	}			/* printf( "\n" ); */
    }
  rMean /= ((double) samples);
  for (t = 0; t < samples; t++)
    {
      rVar += ((rgSample[t] - rMean) * (rgSample[t] - rMean));
    }
  printf ("\n----- mean available colors %lf, variance %lf, stddev  %lf\n\n",
	  rMean, rVar / ((double) samples), sqrt (rVar / ((double) samples)));


  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (regsatfilename, "w");
  fprintf (fp,
	   "c lattin square problem encoded as regsat\np regcnf %d %d %d\n",
	   maxColors, nregvar, nregcls);


  /* Binary mutex constraint: a color can appear at most once in each row */

  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  for (i1 = 0; i1 < order - 1; i1++)
	    {
	      if (incidencecube[i1][j][k] == -1)
		{
		  for (i2 = i1 + 1; i2 < order; i2++)
		    {
		      if (incidencecube[i2][j][k] == -1)
			{
			  k1 = mapcolregvar[i1][j][k + 1];
			  k2 = mapcolregvar[i2][j][k + 1];
			  if (k1 > 1)
			    fprintf (fp, " %8d %8d", -(k1 - 1),
				     (mapregvar[i1][j] + 1));
			  if ((mapcolregvar[i1][j][0] == maxColors)
			      || (k1 < mapcolregvar[i1][j][0]))
			    {
			      if (k1 < maxColors)
				fprintf (fp, " %8d %8d", (k1 + 1),
					 (mapregvar[i1][j] + 1));
			    }
			  if (k2 > 1)
			    fprintf (fp, " %8d %8d", -(k2 - 1),
				     (mapregvar[i2][j] + 1));
			  if ((mapcolregvar[i2][j][0] == maxColors)
			      || (k2 < mapcolregvar[i2][j][0]))
			    {
			      if (k2 < maxColors)
				fprintf (fp, " %8d %8d", (k2 + 1),
					 (mapregvar[i2][j] + 1));
			    }
			  fprintf (fp, " %8d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }


  /* Binary mutex constraint: a color can appear at most once in each column */

  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  for (j1 = 0; j1 < order - 1; j1++)
	    {
	      if (incidencecube[i][j1][k] == -1)
		{
		  for (j2 = j1 + 1; j2 < order; j2++)
		    {
		      if (incidencecube[i][j2][k] == -1)
			{
			  k1 = mapcolregvar[i][j1][k + 1];
			  k2 = mapcolregvar[i][j2][k + 1];
			  if (k1 > 1)
			    fprintf (fp, " %8d %8d", -(k1 - 1),
				     (mapregvar[i][j1] + 1));
			  if ((mapcolregvar[i][j1][0] == maxColors)
			      || (k1 < mapcolregvar[i][j1][0]))
			    {
			      if (k1 < maxColors)
				fprintf (fp, " %8d %8d", (k1 + 1),
					 (mapregvar[i][j1] + 1));
			    }
			  if (k2 > 1)
			    fprintf (fp, " %8d %8d", -(k2 - 1),
				     (mapregvar[i][j2] + 1));
			  if ((mapcolregvar[i][j2][0] == maxColors)
			      || (k2 < mapcolregvar[i][j2][0]))
			    {
			      if (k2 < maxColors)
				fprintf (fp, " %8d %8d", (k2 + 1),
					 (mapregvar[i][j2] + 1));
			    }
			  fprintf (fp, " %8d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }





  /*   This part will be done when regularSAT  + monosignedSAT is available 

     if (!minflag){
     long positive clause: each color must appear at least once in each row  
     for (j = 0; j < order; j++) {
     for (k = 0; k < order; k++) {
     nlit = 0;
     for (i = 0; i < order; i++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     long positive clause: each color must appear at least once in each column 
     for (i = 0; i < order; i++) {
     for (k = 0; k < order ; k++) {
     nlit = 0;
     for (j = 0; j < order; j++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     }

   */


  fclose (fp);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      nregcls = lnregcls;
      goto loop;
    }

  return (0);
}

/******************************************************************************/




int
regsatencode_noheavy_pls_special (char *outfile, int minflag)
{
  char regsatfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k11, k12;
  int i2, j2, k21, k22;
  int flag = 0;
  int nlit, colors, cu, nexttvalue, coc, rem;
  int lnregcls;			/* local version of ncls */
  int leftcolor, rightcolor, tvalue;
  int *row;

  strcpy (regsatfilename, outfile);
  strcat (regsatfilename, ".regcnf");

  lnregcls = 0;

  /* Let's map the available colors for every non-fixed square to a truth value */

  maxColors = 2;
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{

	  tvalue = order;
	  mapcolregvar[i][j][0] = tvalue;	/* printf( "(%i %i %i)", i, j, tvalue ); */
	  if (maxColors < tvalue)
	    maxColors = tvalue;

	}			/* printf( "\n" );  */
    }

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{

	  nexttvalue = 1;
	  coc = (maxColors / mapcolregvar[i][j][0]) - 1;
	  rem = maxColors % mapcolregvar[i][j][0];
	  for (k = 0; k < order; k++)
	    {

	      mapcolregvar[i][j][(k * 2) + 1] = nexttvalue;
	      if (rem)
		{
		  nexttvalue += (coc + 1);
		  rem--;
		}
	      else
		nexttvalue += coc;
	      mapcolregvar[i][j][(k * 2) + 2] = nexttvalue;
	      nexttvalue++;
	    }

	}			/* printf( "\n" ); */
    }


  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */


loop:

  fp = fopen (regsatfilename, "w");
  fprintf (fp,
	   "c lattin square problem encoded as regsat\np regcnf %d %d %d\n",
	   maxColors, nregvar, nregcls);


  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] == 1)
	    {
	      k = squarecolor[i][j] + 1;
	      if (k < maxColors)
		{
		  fprintf (fp, " %4d %5d 0\n", -(k), (mapregvar[i][j] + 1));
		  lnregcls++;
		}
	      if (k > 1)
		{
		  fprintf (fp, " %4d %5d 0\n", (k), (mapregvar[i][j] + 1));
		  lnregcls++;
		}
	    }
	}
    }


  /* Binary mutex constraint: a color can appear at most once in each row */

  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  for (i1 = 0; i1 < order - 1; i1++)
	    {
	      if (incidencecube[i1][j][k] == -1)
		{
		  for (i2 = i1 + 1; i2 < order; i2++)
		    {
		      if (incidencecube[i2][j][k] == -1)
			{
			  k11 = mapcolregvar[i1][j][(k * 2) + 1];
			  k12 = mapcolregvar[i1][j][(k * 2) + 2];
			  k21 = mapcolregvar[i2][j][(k * 2) + 1];
			  k22 = mapcolregvar[i2][j][(k * 2) + 2];
			  if (k11 > 1)
			    fprintf (fp, " %4d %5d", -(k11 - 1),
				     (mapregvar[i1][j] + 1));
			  if (k12 < maxColors)
			    fprintf (fp, " %4d %5d", (k12 + 1),
				     (mapregvar[i1][j] + 1));
			  if (k21 > 1)
			    fprintf (fp, " %4d %5d", -(k21 - 1),
				     (mapregvar[i2][j] + 1));
			  if (k22 < maxColors)
			    fprintf (fp, " %4d %5d", (k22 + 1),
				     (mapregvar[i2][j] + 1));
			  fprintf (fp, " %8d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }


  /* Binary mutex constraint: a color can appear at most once in each column */

  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  for (j1 = 0; j1 < order - 1; j1++)
	    {
	      if (incidencecube[i][j1][k] == -1)
		{
		  for (j2 = j1 + 1; j2 < order; j2++)
		    {
		      if (incidencecube[i][j2][k] == -1)
			{

			  k11 = mapcolregvar[i][j1][(k * 2) + 1];
			  k12 = mapcolregvar[i][j1][(k * 2) + 2];
			  k21 = mapcolregvar[i][j2][(k * 2) + 1];
			  k22 = mapcolregvar[i][j2][(k * 2) + 2];
			  if (k11 > 1)
			    fprintf (fp, " %4d %5d", -(k11 - 1),
				     (mapregvar[i][j1] + 1));
			  if (k12 < maxColors)
			    fprintf (fp, " %4d %5d", (k12 + 1),
				     (mapregvar[i][j1] + 1));
			  if (k21 > 1)
			    fprintf (fp, " %4d %5d", -(k21 - 1),
				     (mapregvar[i][j2] + 1));
			  if (k22 < maxColors)
			    fprintf (fp, " %4d %5d", (k22 + 1),
				     (mapregvar[i][j2] + 1));
			  fprintf (fp, " %8d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }





  /*   This part will be done when regularSAT  + monosignedSAT is available 

     if (!minflag){
     long positive clause: each color must appear at least once in each row  
     for (j = 0; j < order; j++) {
     for (k = 0; k < order; k++) {
     nlit = 0;
     for (i = 0; i < order; i++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     long positive clause: each color must appear at least once in each column 
     for (i = 0; i < order; i++) {
     for (k = 0; k < order ; k++) {
     nlit = 0;
     for (j = 0; j < order; j++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     }

   */


  fclose (fp);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      nregcls = lnregcls;
      goto loop;
    }

  return (0);
}







int
regsatencode_noheavy_pls (char *outfile, int minflag)
{
  char regsatfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k11, k12;
  int i2, j2, k21, k22;
  int flag = 0;
  int nlit, colors, cu, nexttvalue, coc, rem;
  int lnregcls;			/* local version of ncls */
  int leftcolor, rightcolor, tvalue;
  int *row;

  strcpy (regsatfilename, outfile);
  strcat (regsatfilename, ".regcnf");

  lnregcls = 0;

  /* Let's map the available colors for every non-fixed square to a truth value */

  maxColors = 2;
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      tvalue = 0;
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    ++tvalue;
		}
	      mapcolregvar[i][j][0] = tvalue;	/* printf( "(%i %i %i)", i, j, tvalue ); */
	      if (maxColors < tvalue)
		maxColors = tvalue;
	    }
	}			/* printf( "\n" );  */
    }

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      nexttvalue = 1;
	      coc = (maxColors / mapcolregvar[i][j][0]) - 1;
	      rem = maxColors % mapcolregvar[i][j][0];
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      mapcolregvar[i][j][(k * 2) + 1] = nexttvalue;
		      if (rem)
			{
			  nexttvalue += (coc + 1);
			  rem--;
			}
		      else
			nexttvalue += coc;
		      mapcolregvar[i][j][(k * 2) + 2] = nexttvalue;
		      nexttvalue++;
		    }
		}
	    }
	}			/* printf( "\n" ); */
    }


  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (regsatfilename, "w");
  fprintf (fp,
	   "c lattin square problem encoded as regsat\np regcnf %d %d %d\n",
	   maxColors, nregvar, nregcls);


  /* Binary mutex constraint: a color can appear at most once in each row */

  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  for (i1 = 0; i1 < order - 1; i1++)
	    {
	      if (incidencecube[i1][j][k] == -1)
		{
		  for (i2 = i1 + 1; i2 < order; i2++)
		    {
		      if (incidencecube[i2][j][k] == -1)
			{
			  k11 = mapcolregvar[i1][j][(k * 2) + 1];
			  k12 = mapcolregvar[i1][j][(k * 2) + 2];
			  k21 = mapcolregvar[i2][j][(k * 2) + 1];
			  k22 = mapcolregvar[i2][j][(k * 2) + 2];
			  if (k11 > 1)
			    fprintf (fp, " %4d %5d", -(k11 - 1),
				     (mapregvar[i1][j] + 1));
			  if (k12 < maxColors)
			    fprintf (fp, " %4d %5d", (k12 + 1),
				     (mapregvar[i1][j] + 1));
			  if (k21 > 1)
			    fprintf (fp, " %4d %5d", -(k21 - 1),
				     (mapregvar[i2][j] + 1));
			  if (k22 < maxColors)
			    fprintf (fp, " %4d %5d", (k22 + 1),
				     (mapregvar[i2][j] + 1));
			  fprintf (fp, " %8d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }


  /* Binary mutex constraint: a color can appear at most once in each column */

  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  for (j1 = 0; j1 < order - 1; j1++)
	    {
	      if (incidencecube[i][j1][k] == -1)
		{
		  for (j2 = j1 + 1; j2 < order; j2++)
		    {
		      if (incidencecube[i][j2][k] == -1)
			{
			  k11 = mapcolregvar[i][j1][(k * 2) + 1];
			  k12 = mapcolregvar[i][j1][(k * 2) + 2];
			  k21 = mapcolregvar[i][j2][(k * 2) + 1];
			  k22 = mapcolregvar[i][j2][(k * 2) + 2];
			  if (k11 > 1)
			    fprintf (fp, " %4d %5d", -(k11 - 1),
				     (mapregvar[i][j1] + 1));
			  if (k12 < maxColors)
			    fprintf (fp, " %4d %5d", (k12 + 1),
				     (mapregvar[i][j1] + 1));
			  if (k21 > 1)
			    fprintf (fp, " %4d %5d", -(k21 - 1),
				     (mapregvar[i][j2] + 1));
			  if (k22 < maxColors)
			    fprintf (fp, " %4d %5d", (k22 + 1),
				     (mapregvar[i][j2] + 1));
			  fprintf (fp, " %8d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }





  /*   This part will be done when regularSAT  + monosignedSAT is available 

     if (!minflag){
     long positive clause: each color must appear at least once in each row  
     for (j = 0; j < order; j++) {
     for (k = 0; k < order; k++) {
     nlit = 0;
     for (i = 0; i < order; i++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     long positive clause: each color must appear at least once in each column 
     for (i = 0; i < order; i++) {
     for (k = 0; k < order ; k++) {
     nlit = 0;
     for (j = 0; j < order; j++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     }

   */


  fclose (fp);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      nregcls = lnregcls;
      goto loop;
    }

  return (0);
}


int
regsatencodeinclassic_noheavy_pls (char *outfile, int minflag)
{
  char regsatfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k11, k12;
  int i2, j2, k21, k22;
  int flag = 0;
  int nlit, colors, cu, nexttvalue, coc, rem;
  int lnregcls;			/* local version of ncls */
  int leftcolor, rightcolor, tvalue;
  int *row;

  strcpy (regsatfilename, outfile);
  strcat (regsatfilename, ".reginclassic");

  lnregcls = 0;

  /* Let's map the available colors for every non-fixed square to a truth value */

  maxColors = 2;
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      tvalue = 0;
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    ++tvalue;
		}
	      mapcolregvar[i][j][0] = tvalue;	/* printf( "(%i %i %i)", i, j, tvalue ); */
	      if (maxColors < tvalue)
		maxColors = tvalue;
	    }
	}			/* printf( "\n" );  */
    }

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      nexttvalue = 1;
	      coc = (maxColors / mapcolregvar[i][j][0]) - 1;
	      rem = maxColors % mapcolregvar[i][j][0];
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      mapcolregvar[i][j][(k * 2) + 1] = nexttvalue;
		      if (rem)
			{
			  nexttvalue += (coc + 1);
			  rem--;
			}
		      else
			nexttvalue += coc;
		      mapcolregvar[i][j][(k * 2) + 2] = nexttvalue;
		      nexttvalue++;
		    }
		}
	    }
	}			/* printf( "\n" ); */
    }


  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */



loop:

  fp = fopen (regsatfilename, "w");
  fprintf (fp, "c lattin square problem encoded as regsat\np cnf %d %d\n",
	   maxColors * nregvar, nregcls);


  /* Binary mutex constraint: a color can appear at most once in each row */

  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  for (i1 = 0; i1 < order - 1; i1++)
	    {
	      if (incidencecube[i1][j][k] == -1)
		{
		  for (i2 = i1 + 1; i2 < order; i2++)
		    {
		      if (incidencecube[i2][j][k] == -1)
			{
			  k11 = mapcolregvar[i1][j][(k * 2) + 1];
			  k12 = mapcolregvar[i1][j][(k * 2) + 2];
			  k21 = mapcolregvar[i2][j][(k * 2) + 1];
			  k22 = mapcolregvar[i2][j][(k * 2) + 2];
			  if (k11 > 1)
			    fprintf (fp, " %d",
				     -((k11) +
				       ((mapregvar[i1][j]) * maxColors)));
			  if (k12 < maxColors)
			    fprintf (fp, " %d",
				     ((k12 + 1) +
				      ((mapregvar[i1][j]) * maxColors)));
			  if (k21 > 1)
			    fprintf (fp, " %d",
				     -((k21) +
				       ((mapregvar[i2][j]) * maxColors)));
			  if (k22 < maxColors)
			    fprintf (fp, " %d",
				     ((k22 + 1) +
				      ((mapregvar[i2][j]) * maxColors)));
			  fprintf (fp, " %d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }


  /* Binary mutex constraint: a color can appear at most once in each column */

  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  for (j1 = 0; j1 < order - 1; j1++)
	    {
	      if (incidencecube[i][j1][k] == -1)
		{
		  for (j2 = j1 + 1; j2 < order; j2++)
		    {
		      if (incidencecube[i][j2][k] == -1)
			{
			  k11 = mapcolregvar[i][j1][(k * 2) + 1];
			  k12 = mapcolregvar[i][j1][(k * 2) + 2];
			  k21 = mapcolregvar[i][j2][(k * 2) + 1];
			  k22 = mapcolregvar[i][j2][(k * 2) + 2];
			  if (k11 > 1)
			    fprintf (fp, " %d",
				     -((k11) +
				       ((mapregvar[i][j1]) * maxColors)));
			  if (k12 < maxColors)
			    fprintf (fp, " %d",
				     ((k12 + 1) +
				      ((mapregvar[i][j1]) * maxColors)));
			  if (k21 > 1)
			    fprintf (fp, " %d",
				     -((k21) +
				       ((mapregvar[i][j2]) * maxColors)));
			  if (k22 < maxColors)
			    fprintf (fp, " %d",
				     ((k22 + 1) +
				      ((mapregvar[i][j2]) * maxColors)));
			  fprintf (fp, " %d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }



  //ahora tenemos que anadir la parte que falta para que la emulacion clasica
  //funcione.

  for (i = 0; i < nregvar; i++)
    {
      for (j = 0; j < maxColors - 1; j++)
	{
	  fprintf (fp, "%d -%d 0\n", (j + (maxColors * i)) + 1,
		   (j + 1 + (maxColors * i)) + 1);
	  lnregcls++;
	}
    }



  /*   This part will be done when regularSAT  + monosignedSAT is available 

     if (!minflag){
     long positive clause: each color must appear at least once in each row  
     for (j = 0; j < order; j++) {
     for (k = 0; k < order; k++) {
     nlit = 0;
     for (i = 0; i < order; i++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     long positive clause: each color must appear at least once in each column 
     for (i = 0; i < order; i++) {
     for (k = 0; k < order ; k++) {
     nlit = 0;
     for (j = 0; j < order; j++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     }

   */


  fclose (fp);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      nregcls = lnregcls;
      goto loop;
    }

  return (0);
}

int
regsatencode_noheavy_pls_regmv (char *outfile, int minflag)
{
  char regsatfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k11, k12;
  int i2, j2, k21, k22;
  int flag = 0;
  int nlit, colors, cu, nexttvalue, coc, rem;
  int lnregcls;			/* local version of ncls */
  int leftcolor, rightcolor, tvalue;
  int *row;

  strcpy (regsatfilename, outfile);
  strcat (regsatfilename, ".regmv");

  lnregcls = 0;

  /* Let's map the available colors for every non-fixed square to a truth value */

  maxColors = 2;
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      tvalue = 0;
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    ++tvalue;
		}
	      mapcolregvar[i][j][0] = tvalue;	/* printf( "(%i %i %i)", i, j, tvalue ); */
	      if (maxColors < tvalue)
		maxColors = tvalue;
	    }
	}			/* printf( "\n" );  */
    }

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      nexttvalue = 1;
	      coc = (maxColors / mapcolregvar[i][j][0]) - 1;
	      rem = maxColors % mapcolregvar[i][j][0];
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      mapcolregvar[i][j][(k * 2) + 1] = nexttvalue;
		      if (rem)
			{
			  nexttvalue += (coc + 1);
			  rem--;
			}
		      else
			nexttvalue += coc;
		      mapcolregvar[i][j][(k * 2) + 2] = nexttvalue;
		      nexttvalue++;
		    }
		}
	    }
	}			/* printf( "\n" ); */
    }


  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (regsatfilename, "w");
  fprintf (fp, "p cnf %d %d %d\n", maxColors, nregvar, nregcls);


  /* Binary mutex constraint: a color can appear at most once in each row */

  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  for (i1 = 0; i1 < order - 1; i1++)
	    {
	      if (incidencecube[i1][j][k] == -1)
		{
		  for (i2 = i1 + 1; i2 < order; i2++)
		    {
		      if (incidencecube[i2][j][k] == -1)
			{
			  k11 = mapcolregvar[i1][j][(k * 2) + 1];
			  k12 = mapcolregvar[i1][j][(k * 2) + 2];
			  k21 = mapcolregvar[i2][j][(k * 2) + 1];
			  k22 = mapcolregvar[i2][j][(k * 2) + 2];
			  if (k11 > 1)
			    fprintf (fp, " -%4d:%5d", (k11 - 1),
				     (mapregvar[i1][j] + 1));
			  if (k12 < maxColors)
			    fprintf (fp, " +%4d:%5d", (k12 + 1),
				     (mapregvar[i1][j] + 1));
			  if (k21 > 1)
			    fprintf (fp, " -%4d:%5d", (k21 - 1),
				     (mapregvar[i2][j] + 1));
			  if (k22 < maxColors)
			    fprintf (fp, " +%4d:%5d", (k22 + 1),
				     (mapregvar[i2][j] + 1));
			  fprintf (fp, " %8d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }


  /* Binary mutex constraint: a color can appear at most once in each column */

  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  for (j1 = 0; j1 < order - 1; j1++)
	    {
	      if (incidencecube[i][j1][k] == -1)
		{
		  for (j2 = j1 + 1; j2 < order; j2++)
		    {
		      if (incidencecube[i][j2][k] == -1)
			{
			  k11 = mapcolregvar[i][j1][(k * 2) + 1];
			  k12 = mapcolregvar[i][j1][(k * 2) + 2];
			  k21 = mapcolregvar[i][j2][(k * 2) + 1];
			  k22 = mapcolregvar[i][j2][(k * 2) + 2];
			  if (k11 > 1)
			    fprintf (fp, " -%4d:%5d", (k11 - 1),
				     (mapregvar[i][j1] + 1));
			  if (k12 < maxColors)
			    fprintf (fp, " +%4d:%5d", (k12 + 1),
				     (mapregvar[i][j1] + 1));
			  if (k21 > 1)
			    fprintf (fp, " -%4d:%5d", (k21 - 1),
				     (mapregvar[i][j2] + 1));
			  if (k22 < maxColors)
			    fprintf (fp, " +%4d:%5d", (k22 + 1),
				     (mapregvar[i][j2] + 1));
			  fprintf (fp, " %8d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }





  /*   This part will be done when regularSAT  + monosignedSAT is available 

     if (!minflag){
     long positive clause: each color must appear at least once in each row  
     for (j = 0; j < order; j++) {
     for (k = 0; k < order; k++) {
     nlit = 0;
     for (i = 0; i < order; i++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     long positive clause: each color must appear at least once in each column 
     for (i = 0; i < order; i++) {
     for (k = 0; k < order ; k++) {
     nlit = 0;
     for (j = 0; j < order; j++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     }

   */


  fclose (fp);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      nregcls = lnregcls;
      goto loop;
    }

  return (0);
}


int
encode_signsat_pls (char *outfile, int minflag)
{
  char regsatfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k11, k12;
  int i2, j2, k21, k22;
  int flag = 0;
  int nlit, colors, cu, nexttvalue, coc, rem;
  int lnregcls;			/* local version of ncls */
  int leftcolor, rightcolor, tvalue;
  int *row;

  strcpy (regsatfilename, outfile);
  strcat (regsatfilename, ".signcnf");

  lnregcls = 0;

  /* Let's map the available colors for every non-fixed square to a truth value */

  maxColors = 2;
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      tvalue = 0;
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    ++tvalue;
		}
	      mapcolregvar[i][j][0] = tvalue;	/* printf( "(%i %i %i)", i, j, tvalue ); */
	      if (maxColors < tvalue)
		maxColors = tvalue;
	    }
	}			/* printf( "\n" );  */
    }

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      nexttvalue = 1;
	      coc = (maxColors / mapcolregvar[i][j][0]) - 1;
	      rem = maxColors % mapcolregvar[i][j][0];
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      mapcolregvar[i][j][k + 1] = nexttvalue++;
		    }
		}
	    }
	}			/* printf( "\n" ); */
    }


  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (regsatfilename, "w");
  fprintf (fp,
	   "c lattin square problem encoded as signedsat\np cnf %d %d %d\n",
	   maxColors, nregvar, nregcls);


  // Binary mutex constraint: a color can appear at most once in each row 
  /*
     for (j = 0; j < order; j++) {
     for (k = 0; k < order; k++) {
     for (i1 = 0; i1 < order - 1 ; i1++) {
     if (incidencecube[i1][j][k] == -1) {
     for (i2 = i1 + 1; i2 < order; i2++) {
     if (incidencecube[i2][j][k] == -1) {
     k11 =  mapcolregvar[i1][j][(k*2)+1];
     k12 =  mapcolregvar[i1][j][(k*2)+2];
     k21 =  mapcolregvar[i2][j][(k*2)+1];
     k22 =  mapcolregvar[i2][j][(k*2)+2];
     if (k11 > 1)      fprintf(fp, " %4d %5d", -(k11-1),   (mapregvar[i1][j] + 1) );
     if (k12 < maxColors)  fprintf(fp, " %4d %5d",  (k12+1), (mapregvar[i1][j] + 1) );
     if (k21 > 1)       fprintf(fp, " %4d %5d", -(k21-1),   (mapregvar[i2][j] + 1) );
     if (k22 < maxColors)  fprintf(fp, " %4d %5d",  (k22+1), (mapregvar[i2][j] + 1) );
     fprintf(fp, " %8d\n", 0);
     lnregcls++;
     }
     }
     }
     }
     }
     }
   */

  // Binary mutex constraint: a color can appear at most once in each column 
  /*
     for (i = 0; i < order; i++) {
     for (k = 0; k < order; k++) {
     for (j1 = 0; j1 < order - 1 ; j1++) {
     if (incidencecube[i][j1][k] == -1) {
     for (j2 = j1 + 1; j2 < order; j2++) {
     if (incidencecube[i][j2][k] == -1) {
     k11 =  mapcolregvar[i][j1][(k*2)+1];
     k12 =  mapcolregvar[i][j1][(k*2)+2];
     k21 =  mapcolregvar[i][j2][(k*2)+1];
     k22 =  mapcolregvar[i][j2][(k*2)+2];
     if (k11 > 1)      fprintf(fp, " %4d %5d", -(k11-1),   (mapregvar[i][j1] + 1) );
     if (k12 < maxColors)  fprintf(fp, " %4d %5d",  (k12+1), (mapregvar[i][j1] + 1) );
     if (k21 > 1)      fprintf(fp, " %4d %5d", -(k21-1),   (mapregvar[i][j2] + 1) );
     if (k22 < maxColors)  fprintf(fp, " %4d %5d",  (k22+1), (mapregvar[i][j2] + 1) );
     fprintf(fp, " %8d\n", 0);
     lnregcls++;
     }
     }
     }
     }
     }
     }
   */




  //   This part will be done when regularSAT  + monosignedSAT is available  

  // if (!minflag){
  //  long positive clause: each color must appear at least once in each row  

  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  nlit = 0;
	  for (i = 0; i < order; i++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  fprintf (fp, " =%d:%3d", mapcolregvar[i][j][k + 1],
			   mapregvar[i][j] + 1);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "%2d\n", 0);
	      lnregcls++;
	    }
	}
    }
  //long positive clause: each color must appear at least once in each column 
  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  nlit = 0;
	  for (j = 0; j < order; j++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  fprintf (fp, " =%d:%3d", mapcolregvar[i][j][k + 1],
			   mapregvar[i][j] + 1);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "%8d\n", 0);
	      lnregcls++;
	    }
	}
    }
  //  }




  fclose (fp);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      nregcls = lnregcls;
      goto loop;
    }

  return (0);
}

int
encode_nbsignsat_pls (char *outfile, int minflag)
{
  char regsatfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k11, k12;
  int i2, j2, k21, k22;
  int flag = 0;
  int nlit, colors, cu, nexttvalue, coc, rem;
  int lnregcls;			/* local version of ncls */
  int leftcolor, rightcolor, tvalue;
  int *row;

  strcpy (regsatfilename, outfile);
  strcat (regsatfilename, ".nbsigncnf");

  lnregcls = 0;

  /* Let's map the available colors for every non-fixed square to a truth value */

  maxColors = 2;
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      tvalue = 0;
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    ++tvalue;
		}
	      mapcolregvar[i][j][0] = tvalue;	/* printf( "(%i %i %i)", i, j, tvalue ); */
	      if (maxColors < tvalue)
		maxColors = tvalue;
	    }
	}			/* printf( "\n" );  */
    }

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      nexttvalue = 1;
	      coc = (maxColors / mapcolregvar[i][j][0]) - 1;
	      rem = maxColors % mapcolregvar[i][j][0];
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      mapcolregvar[i][j][k + 1] = nexttvalue++;
		    }
		}
	    }
	}			/* printf( "\n" ); */
    }


  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (regsatfilename, "w");
  fprintf (fp,
	   "c lattin square problem encoded as signedsat\nnb cnf %d %d %d\n",
	   nregvar, nregcls, maxColors);


  // Binary mutex constraint: a color can appear at most once in each row 
  /*
     for (j = 0; j < order; j++) {
     for (k = 0; k < order; k++) {
     for (i1 = 0; i1 < order - 1 ; i1++) {
     if (incidencecube[i1][j][k] == -1) {
     for (i2 = i1 + 1; i2 < order; i2++) {
     if (incidencecube[i2][j][k] == -1) {
     k11 =  mapcolregvar[i1][j][(k*2)+1];
     k12 =  mapcolregvar[i1][j][(k*2)+2];
     k21 =  mapcolregvar[i2][j][(k*2)+1];
     k22 =  mapcolregvar[i2][j][(k*2)+2];
     if (k11 > 1)      fprintf(fp, " %4d %5d", -(k11-1),   (mapregvar[i1][j] + 1) );
     if (k12 < maxColors)  fprintf(fp, " %4d %5d",  (k12+1), (mapregvar[i1][j] + 1) );
     if (k21 > 1)       fprintf(fp, " %4d %5d", -(k21-1),   (mapregvar[i2][j] + 1) );
     if (k22 < maxColors)  fprintf(fp, " %4d %5d",  (k22+1), (mapregvar[i2][j] + 1) );
     fprintf(fp, " %8d\n", 0);
     lnregcls++;
     }
     }
     }
     }
     }
     }
   */

  // Binary mutex constraint: a color can appear at most once in each column 
  /*
     for (i = 0; i < order; i++) {
     for (k = 0; k < order; k++) {
     for (j1 = 0; j1 < order - 1 ; j1++) {
     if (incidencecube[i][j1][k] == -1) {
     for (j2 = j1 + 1; j2 < order; j2++) {
     if (incidencecube[i][j2][k] == -1) {
     k11 =  mapcolregvar[i][j1][(k*2)+1];
     k12 =  mapcolregvar[i][j1][(k*2)+2];
     k21 =  mapcolregvar[i][j2][(k*2)+1];
     k22 =  mapcolregvar[i][j2][(k*2)+2];
     if (k11 > 1)      fprintf(fp, " %4d %5d", -(k11-1),   (mapregvar[i][j1] + 1) );
     if (k12 < maxColors)  fprintf(fp, " %4d %5d",  (k12+1), (mapregvar[i][j1] + 1) );
     if (k21 > 1)      fprintf(fp, " %4d %5d", -(k21-1),   (mapregvar[i][j2] + 1) );
     if (k22 < maxColors)  fprintf(fp, " %4d %5d",  (k22+1), (mapregvar[i][j2] + 1) );
     fprintf(fp, " %8d\n", 0);
     lnregcls++;
     }
     }
     }
     }
     }
     }
   */




  //   This part will be done when regularSAT  + monosignedSAT is available  

  // if (!minflag){
  //  long positive clause: each color must appear at least once in each row  

  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  nlit = 0;
	  for (i = 0; i < order; i++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  fprintf (fp, " %3d{%d}", mapregvar[i][j] + 1,
			   mapcolregvar[i][j][k + 1] - 1);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "%2d\n", 0);
	      lnregcls++;
	    }
	}
    }
  //long positive clause: each color must appear at least once in each column 
  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  nlit = 0;
	  for (j = 0; j < order; j++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  fprintf (fp, " %3d{%d}", mapregvar[i][j] + 1,
			   mapcolregvar[i][j][k + 1] - 1);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "%8d\n", 0);
	      lnregcls++;
	    }
	}
    }
  //  }




  fclose (fp);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      nregcls = lnregcls;
      goto loop;
    }

  return (0);
}


int
encode_hybsignsat_pls (char *outfile, int minflag)
{
  char regsatfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k11, k12;
  int i2, j2, k21, k22;
  int flag = 0;
  int nlit, colors, cu, nexttvalue, coc, rem;
  int lnregcls;			/* local version of ncls */
  int leftcolor, rightcolor, tvalue;
  int *row;

  strcpy (regsatfilename, outfile);
  strcat (regsatfilename, ".hybmv");

  lnregcls = 0;

  /* Let's map the available colors for every non-fixed square to a truth value */

  maxColors = 2;
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      tvalue = 0;
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    ++tvalue;
		}
	      mapcolregvar[i][j][0] = tvalue;	/* printf( "(%i %i %i)", i, j, tvalue ); */
	      if (maxColors < tvalue)
		maxColors = tvalue;
	    }
	}			/* printf( "\n" );  */
    }

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      nexttvalue = 1;
	      coc = (maxColors / mapcolregvar[i][j][0]) - 1;
	      rem = maxColors % mapcolregvar[i][j][0];
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      mapcolregvar[i][j][k + 1] = nexttvalue++;
		    }
		}
	    }
	}			/* printf( "\n" ); */
    }


  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (regsatfilename, "w");
  fprintf (fp,
	   "c lattin square problem encoded as signedsat\np cnf %d %d %d\n",
	   maxColors, nregvar, nregcls);


  // Binary mutex constraint: a color can appear at most once in each row 


  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  for (i1 = 0; i1 < order - 1; i1++)
	    {
	      if (incidencecube[i1][j][k] == -1)
		{
		  for (i2 = i1 + 1; i2 < order; i2++)
		    {
		      if (incidencecube[i2][j][k] == -1)
			{
			  k11 = mapcolregvar[i1][j][k + 1];
			  //    k12 =  mapcolregvar[i1][j][(k*2)+2];
			  k21 = mapcolregvar[i2][j][k + 1];
			  //  k22 =  mapcolregvar[i2][j][(k*2)+2];
			  if (k11 > 1)
			    fprintf (fp, " -%4d:%5d", (k11 - 1),
				     (mapregvar[i1][j] + 1));
			  if (k11 < maxColors)
			    fprintf (fp, " +%4d:%5d", (k11 + 1),
				     (mapregvar[i1][j] + 1));
			  if (k21 > 1)
			    fprintf (fp, " -%4d:%5d", (k21 - 1),
				     (mapregvar[i2][j] + 1));
			  if (k21 < maxColors)
			    fprintf (fp, "+ %4d:%5d", (k21 + 1),
				     (mapregvar[i2][j] + 1));
			  fprintf (fp, " %3d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }


  // Binary mutex constraint: a color can appear at most once in each column 

  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  for (j1 = 0; j1 < order - 1; j1++)
	    {
	      if (incidencecube[i][j1][k] == -1)
		{
		  for (j2 = j1 + 1; j2 < order; j2++)
		    {
		      if (incidencecube[i][j2][k] == -1)
			{
			  k11 = mapcolregvar[i][j1][k + 1];
			  //      k12 =  mapcolregvar[i][j1][(k*2)+2];
			  k21 = mapcolregvar[i][j2][k + 1];
			  //  k22 =  mapcolregvar[i][j2][(k*2)+2];
			  if (k11 > 1)
			    fprintf (fp, " -%4d:%5d", (k11 - 1),
				     (mapregvar[i][j1] + 1));
			  if (k11 < maxColors)
			    fprintf (fp, " +%4d:%5d", (k11 + 1),
				     (mapregvar[i][j1] + 1));
			  if (k21 > 1)
			    fprintf (fp, " -%4d:%5d", (k21 - 1),
				     (mapregvar[i][j2] + 1));
			  if (k21 < maxColors)
			    fprintf (fp, " +%4d:%5d", (k21 + 1),
				     (mapregvar[i][j2] + 1));
			  fprintf (fp, " %3d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }





  //   This part will be done when regularSAT  + monosignedSAT is available  

  // if (!minflag){
  //  long positive clause: each color must appear at least once in each row  

  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  nlit = 0;
	  for (i = 0; i < order; i++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  fprintf (fp, " =%d:%3d", mapcolregvar[i][j][k + 1],
			   mapregvar[i][j] + 1);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "%2d\n", 0);
	      lnregcls++;
	    }
	}
    }
  //long positive clause: each color must appear at least once in each column 
  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  nlit = 0;
	  for (j = 0; j < order; j++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  fprintf (fp, " =%d:%3d", mapcolregvar[i][j][k + 1],
			   mapregvar[i][j] + 1);
		  nlit++;
		}
	    }
	  if (nlit > 0)
	    {
	      fprintf (fp, "%8d\n", 0);
	      lnregcls++;
	    }
	}
    }
  //  }




  fclose (fp);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      nregcls = lnregcls;
      goto loop;
    }

  return (0);
}


/******************************************************************************/



int
regsatencode_dual_pls (char *outfile, int minflag)
{
  char regsatfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int i1, j1, k11, k12;
  int i2, j2, k21, k22;
  int flag = 0;
  int nlit, colors, cu, nexttvalue, coc, rem;
  int lnregcls;			/* local version of ncls */
  int leftcolor, rightcolor, tvalue;
  int *row;

  printf ("regularSAT dual encoding\n");

  strcpy (regsatfilename, outfile);
  strcat (regsatfilename, ".regcnf");

  lnregcls = 0;

  /* Let's map the available colors for every non-fixed square to a truth value */

  maxColors = maxdualtvalues;
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      tvalue = 0;
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    ++tvalue;
		}
	      mapcolregvar[i][j][0] = tvalue;	/* printf( "(%i %i %i)", i, j, tvalue ); */
	      if (maxColors < tvalue)
		maxColors = tvalue;
	    }
	}			/* printf( "\n" ); */
    }

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      nexttvalue = 1;
	      coc = (maxColors / mapcolregvar[i][j][0]) - 1;
	      rem = maxColors % mapcolregvar[i][j][0];
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      mapcolregvar[i][j][(k * 2) + 1] = nexttvalue;
		      if (rem)
			{
			  nexttvalue += (coc + 1);
			  rem--;
			}
		      else
			nexttvalue += coc;
		      mapcolregvar[i][j][(k * 2) + 2] = nexttvalue;
		      nexttvalue++;
		    }
		}
	    }
	}			/* printf( "\n" );  */
    }

  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  if (maprowcolorvar[i][k][0] != -1)
	    {
	      nexttvalue = 1;
	      coc = (maxColors / maprowcolorvar[i][k][1]) - 1;
	      rem = maxColors % maprowcolorvar[i][k][1];
	      for (j = 0; j < order; j++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      maprowcolorvar[i][k][(j * 2) + 2] = nexttvalue;
		      if (rem)
			{
			  nexttvalue += (coc + 1);
			  rem--;
			}
		      else
			nexttvalue += coc;
		      maprowcolorvar[i][k][(j * 2) + 3] = nexttvalue;
		      nexttvalue++;
		    }
		}
	    }
	}
    }

  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  if (mapcolcolorvar[j][k][0] != -1)
	    {
	      nexttvalue = 1;
	      coc = (maxColors / mapcolcolorvar[j][k][1]) - 1;
	      rem = maxColors % mapcolcolorvar[j][k][1];
	      for (i = 0; i < order; i++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      mapcolcolorvar[j][k][(i * 2) + 2] = nexttvalue;
		      if (rem)
			{
			  nexttvalue += (coc + 1);
			  rem--;
			}
		      else
			nexttvalue += coc;
		      mapcolcolorvar[j][k][(i * 2) + 3] = nexttvalue;
		      nexttvalue++;
		    }
		}
	    }
	}
    }


  /* We actually do this twice, since we don't know the number of
     clauses until we finish generating them the first time!
     ncls is set at the end of the first time through the loop. */

loop:

  fp = fopen (regsatfilename, "w");
  fprintf (fp,
	   "c lattin square problem encoded as regsat\np regcnf %d %d %d\n",
	   maxColors, nregvar, nregcls);


  /* Binary mutex constraint: a color can appear at most once in each row */

  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  for (i1 = 0; i1 < order - 1; i1++)
	    {
	      if (incidencecube[i1][j][k] == -1)
		{
		  for (i2 = i1 + 1; i2 < order; i2++)
		    {
		      if (incidencecube[i2][j][k] == -1)
			{
			  k11 = mapcolregvar[i1][j][(k * 2) + 1];
			  k12 = mapcolregvar[i1][j][(k * 2) + 2];
			  k21 = mapcolregvar[i2][j][(k * 2) + 1];
			  k22 = mapcolregvar[i2][j][(k * 2) + 2];
			  if (k11 > 1)
			    fprintf (fp, " %4d %5d", -(k11 - 1),
				     (mapregvar[i1][j] + 1));
			  if (k12 < maxColors)
			    fprintf (fp, " %4d %5d", (k12 + 1),
				     (mapregvar[i1][j] + 1));
			  if (k21 > 1)
			    fprintf (fp, " %4d %5d", -(k21 - 1),
				     (mapregvar[i2][j] + 1));
			  if (k22 < maxColors)
			    fprintf (fp, " %4d %5d", (k22 + 1),
				     (mapregvar[i2][j] + 1));
			  fprintf (fp, " %8d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }


  /* Binary mutex constraint: a color can appear at most once in each column */

  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  for (j1 = 0; j1 < order - 1; j1++)
	    {
	      if (incidencecube[i][j1][k] == -1)
		{
		  for (j2 = j1 + 1; j2 < order; j2++)
		    {
		      if (incidencecube[i][j2][k] == -1)
			{
			  k11 = mapcolregvar[i][j1][(k * 2) + 1];
			  k12 = mapcolregvar[i][j1][(k * 2) + 2];
			  k21 = mapcolregvar[i][j2][(k * 2) + 1];
			  k22 = mapcolregvar[i][j2][(k * 2) + 2];
			  if (k11 > 1)
			    fprintf (fp, " %4d %5d", -(k11 - 1),
				     (mapregvar[i][j1] + 1));
			  if (k12 < maxColors)
			    fprintf (fp, " %4d %5d", (k12 + 1),
				     (mapregvar[i][j1] + 1));
			  if (k21 > 1)
			    fprintf (fp, " %4d %5d", -(k21 - 1),
				     (mapregvar[i][j2] + 1));
			  if (k22 < maxColors)
			    fprintf (fp, " %4d %5d", (k22 + 1),
				     (mapregvar[i][j2] + 1));
			  fprintf (fp, " %8d\n", 0);
			  lnregcls++;
			}
		    }
		}
	    }
	}
    }



  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      k11 = mapcolregvar[i][j][(k * 2) + 1];
		      k12 = mapcolregvar[i][j][(k * 2) + 2];
		      i1 = mapcolcolorvar[j][k][(i * 2) + 2];
		      i2 = mapcolcolorvar[j][k][(i * 2) + 3];
		      j1 = maprowcolorvar[i][k][(j * 2) + 2];
		      j2 = maprowcolorvar[i][k][(j * 2) + 3];

		      /*  normal  =>   duales    */
		      if (i1 > 1)
			{
			  if (k11 > 1)
			    fprintf (fp, " %d %d", -(k11 - 1),
				     (mapregvar[i][j] + 1));
			  if (k12 < maxColors)
			    fprintf (fp, " %d %d", (k12 + 1),
				     (mapregvar[i][j] + 1));
			  fprintf (fp, " %d %d", (i1),
				   (mapcolcolorvar[j][k][0] + 1));
			  fprintf (fp, " %d\n", 0);
			  lnregcls++;
			}

		      if (i2 < maxColors)
			{
			  if (k11 > 1)
			    fprintf (fp, " %d %d", -(k11 - 1),
				     (mapregvar[i][j] + 1));
			  if (k12 < maxColors)
			    fprintf (fp, " %d %d", (k12 + 1),
				     (mapregvar[i][j] + 1));
			  fprintf (fp, " %d %d", -(i2),
				   (mapcolcolorvar[j][k][0] + 1));
			  fprintf (fp, " %d\n", 0);
			  lnregcls++;
			}

		      if (j1 > 1)
			{
			  if (k11 > 1)
			    fprintf (fp, " %d %d", -(k11 - 1),
				     (mapregvar[i][j] + 1));
			  if (k12 < maxColors)
			    fprintf (fp, " %d %d", (k12 + 1),
				     (mapregvar[i][j] + 1));
			  fprintf (fp, " %d %d", (j1),
				   (maprowcolorvar[i][k][0] + 1));
			  fprintf (fp, " %d\n", 0);
			  lnregcls++;
			}

		      if (j2 < maxColors)
			{
			  if (k11 > 1)
			    fprintf (fp, " %d %d", -(k11 - 1),
				     (mapregvar[i][j] + 1));
			  if (k12 < maxColors)
			    fprintf (fp, " %d %d", (k12 + 1),
				     (mapregvar[i][j] + 1));
			  fprintf (fp, " %d %d", -(j2),
				   (maprowcolorvar[i][k][0] + 1));
			  fprintf (fp, " %d\n", 0);
			  lnregcls++;
			}

		      /*  duales =>  normal   */

		      if (k11 > 1)
			{
			  if (i1 > 1)
			    fprintf (fp, " %d %d", -(i1 - 1),
				     (mapcolcolorvar[j][k][0] + 1));
			  if (i2 < maxColors)
			    fprintf (fp, " %d %d", (i2 + 1),
				     (mapcolcolorvar[j][k][0] + 1));
			  if (j1 > 1)
			    fprintf (fp, " %d %d", -(j1 - 1),
				     (maprowcolorvar[i][k][0] + 1));
			  if (j2 < maxColors)
			    fprintf (fp, " %d %d", (j2 + 1),
				     (maprowcolorvar[i][k][0] + 1));
			  fprintf (fp, " %d %d", (k11),
				   (mapregvar[i][j] + 1));
			  fprintf (fp, " %d\n", 0);
			  lnregcls++;
			}

		      if (k12 < maxColors)
			{
			  if (i1 > 1)
			    fprintf (fp, " %d %d", -(i1 - 1),
				     (mapcolcolorvar[j][k][0] + 1));
			  if (i2 < maxColors)
			    fprintf (fp, " %d %d", (i2 + 1),
				     (mapcolcolorvar[j][k][0] + 1));
			  if (j1 > 1)
			    fprintf (fp, " %d %d", -(j1 - 1),
				     (maprowcolorvar[i][k][0] + 1));
			  if (j2 < maxColors)
			    fprintf (fp, " %d %d", (j2 + 1),
				     (maprowcolorvar[i][k][0] + 1));
			  fprintf (fp, " %d %d", -(k12),
				   (mapregvar[i][j] + 1));
			  fprintf (fp, " %d\n", 0);
			  lnregcls++;
			}

		    }
		}
	    }
	}
    }


  /*   This part will be done when regularSAT  + monosignedSAT is available 

     if (!minflag){
     long positive clause: each color must appear at least once in each row  
     for (j = 0; j < order; j++) {
     for (k = 0; k < order; k++) {
     nlit = 0;
     for (i = 0; i < order; i++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     long positive clause: each color must appear at least once in each column 
     for (i = 0; i < order; i++) {
     for (k = 0; k < order ; k++) {
     nlit = 0;
     for (j = 0; j < order; j++) {
     if (incidencecube[i][j][k] == -1) {
     fprintf(fp, "%8d", mapvar[i][j][k] + 1);
     nlit++;
     }
     }
     if (nlit > 0) {
     fprintf(fp, "%8d\n", 0);
     lncls++;
     }
     }
     }
     }

   */


  fclose (fp);

  if (flag == 0)
    {
      /* First time through the loop finished -- remember the number of clauses */
      flag = 1;
      nregcls = lnregcls;
      goto loop;
    }

  return (0);
}



/******************************************************************************/



int
nunfixed (void)
{
  int i, j, k;
  int cnt;
  int nnunfixed = 0;

  /* Computes number of squares not yet assigned a definite color
     == number of holes */
  nvar = 0;
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  cnt = 0;
	  for (k = 0; k < order; k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		cnt++;
	    }
	  if (cnt != 0)
	    nnunfixed++;
	}
    }
  return (nnunfixed);
}


/******************************************************************************/


int
init_mapvar (void)
{
  int i, j, k;
  nvar = 0;
  /* Computes the number of binary variables and assigns the variables
     to positions in the incidencecube */

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      for (k = 0; k < order; k++)
		mapvar[i][j][k] = nvar++;
	    }
	  else
	    {
	      for (k = 0; k < order; k++)
		mapvar[i][j][k] = -1;
	    }
	}
    }




  return (0);
}


/******************************************************************************/


int
init_mapregvar (void)
{
  int i, j, k;
  nregvar = 0;
  /* Computes the number of CSP variables and assigns the variables
     to positions in the lattin square */

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    mapregvar[i][j] = nregvar++;
	  else
	    mapregvar[i][j] = -1;
	}
    }
  return (0);
}

int
init_mapregvar_special (void)
{
  int i, j, k;
  nregvar = 0;
  /* Computes the number of CSP variables and assigns the variables
     to positions in the lattin square */

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  mapregvar[i][j] = nregvar++;
	}
    }
  return (0);
}


int
init_mapdualregvar (void)
{
  int i, j, k;
  maxdualtvalues = 0;

  /* Computes the number of CSP variables and assigns the variables
     to positions in the lattin square */

  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  maprowcolorvar[i][k][0] = -1;
	  maprowcolorvar[i][k][1] = 0;
	  j = 0;
	  while ((j < order) && (incidencecube[i][j][k] != -1))
	    {
	      j++;
	    }
	  if (j < order)
	    {
	      maprowcolorvar[i][k][0] = nregvar++;
	      while (j < order)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      maprowcolorvar[i][k][1]++;
		    }
		  j++;
		}
	      if (maprowcolorvar[i][k][1] > maxdualtvalues)
		maxdualtvalues = maprowcolorvar[i][k][1];
	    }
	}
    }

  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  mapcolcolorvar[j][k][0] = -1;
	  mapcolcolorvar[j][k][1] = 0;
	  i = 0;
	  while ((i < order) && (incidencecube[i][j][k] != -1))
	    {
	      i++;
	    }
	  if (i < order)
	    {
	      mapcolcolorvar[j][k][0] = nregvar++;
	      while (i < order)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      mapcolcolorvar[j][k][1]++;
		    }
		  i++;
		}
	      if (mapcolcolorvar[j][k][1] > maxdualtvalues)
		maxdualtvalues = mapcolcolorvar[j][k][1];
	    }
	}
    }


  return (0);
}



/******************************************************************************/


int
init_incidencecube (void)
{
  int i, j, k;
  int i1, j1, k1;

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  for (k = 0; k < order; k++)
	    {
	      incidencecube[i][j][k] = -1;
	      mapcolregvar[i][j][k] = -1;
	    }
	  mapcolregvar[i][j][order] = -1;
	}
    }

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] == 1)
	    {
	      k = squarecolor[i][j];
	      incidencecube[i][j][k] = 1;
	      for (i1 = 0; i1 < order; i1++)
		if (i1 != i)
		  {
		    if (incidencecube[i1][j][k] == 1)
		      ourerror ("incidence ??????");
		    incidencecube[i1][j][k] = 0;
		  }
	      for (j1 = 0; j1 < order; j1++)
		if (j1 != j)
		  {
		    if (incidencecube[i][j1][k] == 1)
		      ourerror ("incidence ??????");
		    incidencecube[i][j1][k] = 0;
		  }
	      for (k1 = 0; k1 < order; k1++)
		if (k1 != k)
		  {
		    if (incidencecube[i][j][k1] == 1)
		      ourerror ("incidence ??????");
		    incidencecube[i][j][k1] = 0;
		  }
	    }
	}
    }
  return (0);
}

/******************************************************************************/



int
forward_checking_incidencecube (void)
{
  int i, j, k;
  int i1, j1, k1;
  int holecnt, holei, holej, holek;
  int flag;

loop:
  flag = 0;

  /* Look at each square to find ones that have only a single coloring option left */
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{

	  /* Count how many free options there are for coloring square[i,j] */
	  holecnt = 0;
	  for (k = 0; k < order; k++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  holecnt++;
		  holek = k;
		}
	    }
	  /* If there is a just single option left, then in fact square[i,j] must 
	     in fact be that color */
	  if (holecnt == 1)
	    {
	      /* set flag to remember that we made a change */
	      flag = 1;
	      k = holek;
	      incidencecube[i][j][k] = 1;
	      squarecolor[i][j] = k;
	      squarefixed[i][j] = 1;
	      /* Now that this square is colored, propagate this decision up and down */
	      for (i1 = 0; i1 < order; i1++)
		if (i1 != i)
		  {
		    if (incidencecube[i1][j][k] == 1)
		      return 1;
		    incidencecube[i1][j][k] = 0;
		  }
	      for (j1 = 0; j1 < order; j1++)
		if (j1 != j)
		  {
		    if (incidencecube[i][j1][k] == 1)
		      return 1;
		    incidencecube[i][j1][k] = 0;
		  }
	      /* This is actually unnecessary since this was the last option,
	         but it cannot hurt */
	      for (k1 = 0; k1 < order; k1++)
		if (k1 != k)
		  {
		    if (incidencecube[i][j][k1] == 1)
		      return 1;
		    incidencecube[i][j][k1] = 0;
		  }
	    }
	}
    }

  /* Repeat with the cube rotated 90 degrees.
     Now for each row and color, look for ones that have only a single column option left */
  for (j = 0; j < order; j++)
    {
      for (k = 0; k < order; k++)
	{
	  holecnt = 0;
	  for (i = 0; i < order; i++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  holecnt++;
		  holei = i;
		}
	    }
	  if (holecnt == 1)
	    {
	      flag = 1;
	      i = holei;
	      incidencecube[i][j][k] = 1;
	      squarecolor[i][j] = k;
	      squarefixed[i][j] = 1;
	      for (i1 = 0; i1 < order; i1++)
		if (i1 != i)
		  {
		    if (incidencecube[i1][j][k] == 1)
		      return 1;
		    incidencecube[i1][j][k] = 0;
		  }
	      for (j1 = 0; j1 < order; j1++)
		if (j1 != j)
		  {
		    if (incidencecube[i][j1][k] == 1)
		      return 1;
		    incidencecube[i][j1][k] = 0;
		  }
	      for (k1 = 0; k1 < order; k1++)
		if (k1 != k)
		  {
		    if (incidencecube[i][j][k1] == 1)
		      return 1;
		    incidencecube[i][j][k1] = 0;
		  }
	    }
	}
    }
  /* Repeat with the cube rotated another 90 degrees.
     Now for each column and color, look for ones that have only a single row option left */
  for (i = 0; i < order; i++)
    {
      for (k = 0; k < order; k++)
	{
	  holecnt = 0;
	  for (j = 0; j < order; j++)
	    {
	      if (incidencecube[i][j][k] == -1)
		{
		  holecnt++;
		  holej = j;
		}
	    }
	  if (holecnt == 1)
	    {
	      flag = 1;
	      j = holej;
	      incidencecube[i][j][k] = 1;
	      squarecolor[i][j] = k;
	      squarefixed[i][j] = 1;
	      for (i1 = 0; i1 < order; i1++)
		if (i1 != i)
		  {
		    if (incidencecube[i1][j][k] == 1)
		      return 1;
		    incidencecube[i1][j][k] = 0;
		  }
	      for (j1 = 0; j1 < order; j1++)
		if (j1 != j)
		  {
		    if (incidencecube[i][j1][k] == 1)
		      return 1;
		    incidencecube[i][j1][k] = 0;
		  }
	      for (k1 = 0; k1 < order; k1++)
		if (k1 != k)
		  {
		    if (incidencecube[i][j][k1] == 1)
		      return 1;
		    incidencecube[i][j][k1] = 0;
		  }
	    }
	}
    }

  /* If a change was made, start all over again.  This will still converge
     in polytime. */
  if (flag == 1)
    goto loop;

  return (0);
}

/******************************************************************************/

int
poke (const char *infile, int numhole, unsigned long seed, char *plsname)
{
  if (seed != -1)
    srandom (seed);
  read_pls_in_squarecolor (infile);
  test_validity_squarecolor ();
  generate_holes (numhole);
  write_pls (plsname);
}

/******************************************************************************/

int
write_pls (const char *outfile)
{
  char plsfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j;

  printf ("writting regular LS(%i)\n", order);

  strcpy (plsfilename, outfile);
  strcat (plsfilename, ".pls");

  fp = fopen (plsfilename, "w");

  fprintf (fp, "order %d\n", order);
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] == 1)
	    fprintf (fp, "%4d", squarecolor[i][j]);
	  else
	    fprintf (fp, "%4d", -1);
	}
      fprintf (fp, "\n");
    }

  fclose (fp);

  return (0);
}

/******************************************************************************/

int
generate_holes (int numhole)
{
  int i, j;
  int *posvector, npos;
  int selectedpos;
  int wherepos;

  if (numhole > order * order)
    ourerror ("Number of holes exceed order square");

  posvector = (int *) malloc (sizeof (int) * order * order);
  for (i = 0; i < order * order; i++)
    posvector[i] = i;
  npos = order * order;

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	squarefixed[i][j] = 1;
    }
  for (i = 0; i < numhole; i++)
    {
      wherepos = random () % npos;
      selectedpos = posvector[wherepos];
      if (squarefixed[selectedpos % order][selectedpos / order] != 1)
	ourerror ("??");
      squarefixed[selectedpos % order][selectedpos / order] = 0;
      posvector[wherepos] = posvector[--npos];
    }
/*
    for (i = 0; i < order; i++) {
        for (j = 0; j < order; j++) {
            if (squarefixed[i][j] == 1) printf("%3d", squarecolor[i][j]);
            else printf("   ");
        }
        printf("\n");
    }
*/
  return (0);
}

/******************************************************************************/

int
test_validity_squarecolor (void)
{
  int i, j;
  int color;
  int *markvector;

  markvector = (int *) malloc (sizeof (int) * order);
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	markvector[j] = 0;
      for (j = 0; j < order; j++)
	{
	  color = squarecolor[i][j];
	  if ((color >= 0) && (color <= order - 1))
	    markvector[color] = 1;
	}
      for (j = 0; j < order; j++)
	{
	  if (markvector[j] == 0)
	    ourerror ("The latin square is not valid...");
	}
      for (j = 0; j < order; j++)
	markvector[j] = 0;
      for (j = 0; j < order; j++)
	{
	  color = squarecolor[j][i];
	  if ((color >= 0) && (color <= order - 1))
	    markvector[color] = 1;
	}
      for (j = 0; j < order; j++)
	{
	  if (markvector[j] == 0)
	    ourerror ("The latin square is not valid...");
	}
    }
  free (markvector);
}

/******************************************************************************/

int
read_ols_in_squarecolor (char *olsname)
{
  char olsfilename[MAX_NAME_LENGTH];
  char stringbuf[MAX_LINE_LENGTH + 1];
  FILE *fp;
  int i, j;
  int n;

  strcpy (olsfilename, olsname);
  strcat (olsfilename, ".ols");

  fp = fopen (olsfilename, "r");
  if (fp == NULL)
    ourerror ("Read_ols_in_squarecolor failed to open file");

  fgets (linebuf, MAX_LINE_LENGTH, fp);
  sscanf (linebuf, "%s %d", stringbuf, &order);
  while (strcmp (stringbuf, "Order:") != 0)
    {
      fgets (linebuf, MAX_LINE_LENGTH, fp);
      sscanf (linebuf, "%s %d", stringbuf, &order);
    }
  fgets (linebuf, MAX_LINE_LENGTH, fp);
  sscanf (linebuf, "%s", stringbuf);
  while (strcmp (stringbuf, "New") != 0)
    {
      fgets (linebuf, MAX_LINE_LENGTH, fp);
      sscanf (linebuf, "%s", stringbuf);
    }

  alloc_squarecolor ();
  alloc_squarefixed ();

  fscanf (fp, "%s", stringbuf);
  for (i = 0; i < order; i++)
    {
      fscanf (fp, "%s", stringbuf);
      for (j = 0; j < order; j++)
	{
	  fscanf (fp, "%d", &squarecolor[i][j]);
	  fscanf (fp, "%s", stringbuf);
	}
    }
  //printf("File loaded\n");

  return (0);
}

/******************************************************************************/



int
read_sudoku_pls_in_squarecolor (char *infile)
{
  char plsfilename[MAX_NAME_LENGTH];
  char stringbuf[MAX_LINE_LENGTH + 1];
  FILE *fp;
  int i, j;
  int n;


  strcpy (plsfilename, infile);
  strcat (plsfilename, ".pls");

  fp = fopen (plsfilename, "r");
  if (fp == NULL)
    ourerror ("Read_pls_in_squarecolor failed to open file");

  fgets (linebuf, MAX_LINE_LENGTH, fp);
  sscanf (linebuf, "%s %d %d", stringbuf, &order, &sub_order);
  while (strcmp (stringbuf, "order") != 0)
    {
      fgets (linebuf, MAX_LINE_LENGTH, fp);
      sscanf (linebuf, "%s %d %d", stringbuf, &order, &sub_order);
    }

  if (first)
    {
      alloc_squarecolor ();
      alloc_squarefixed ();
    }

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  fscanf (fp, "%d", &squarecolor[i][j]);
	  if (squarecolor[i][j] != -1)
	    squarefixed[i][j] = 1;
	  else
	    squarefixed[i][j] = 0;
	}
    }
  //printf("PLS File loaded\n");

  return (0);
}


int
read_pls_in_squarecolor (const char *infile)
{
  char plsfilename[MAX_NAME_LENGTH];
  char stringbuf[MAX_LINE_LENGTH + 1];
  FILE *fp;
  int i, j;
  int n;


  strcpy (plsfilename, infile);
  strcat (plsfilename, ".pls");

  fp = fopen (plsfilename, "r");
  if (fp == NULL)
    ourerror ("Read_pls_in_squarecolor failed to open file");

  fgets (linebuf, MAX_LINE_LENGTH, fp);
  sscanf (linebuf, "%s %d", stringbuf, &order);
  while (strcmp (stringbuf, "order") != 0)
    {
      fgets (linebuf, MAX_LINE_LENGTH, fp);
      sscanf (linebuf, "%s %d", stringbuf, &order);
    }

  if (first)
    {
      alloc_squarecolor ();
      alloc_squarefixed ();
    }

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  fscanf (fp, "%d", &squarecolor[i][j]);
	  if (squarecolor[i][j] != -1)
	    squarefixed[i][j] = 1;
	  else
	    squarefixed[i][j] = 0;
	}
    }
  //printf("PLS File loaded\n");

  return (0);
}

/******************************************************************************/

int
read_varval (char *infile)
{
  char solfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j, k;
  int n;
  int cnt = 0;

  /*
     for (i = 0; i < order; i++) {
     for (j = 0; j < order; j++) {
     printf("%3d", squarecolor[i][j]);
     }
     printf("\n");
     }
   */
  strcpy (solfilename, infile);
  strcat (solfilename, ".sol");

  fp = fopen (solfilename, "r");
  if (fp == NULL)
    ourerror ("Read_varval failed to open file");

  alloc_varval ();

  // read variable value from the solution file
  for (i = 0; i < nvar; i++)
    {
      fscanf (fp, "%d", &n);
      if (n > 0)
	{
	  varval[n - 1] = 1;
	}
      else
	{
	  varval[-n - 1] = 0;
	}
    }

  for (n = 0; n < nvar; n++)
    if (varval[n] == 1)
      {
	cnt = 0;
	for (i = 0; i < order; i++)
	  {
	    for (j = 0; j < order; j++)
	      {
		for (k = 0; k < order; k++)
		  {
		    if (mapvar[i][j][k] == n)
		      {
			squarecolor[i][j] = k;
			squarefixed[i][j] = 1;
			cnt++;
		      }
		  }
	      }
	  }
      }

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  printf ("%3d", squarecolor[i][j]);
	}
      printf ("\n");
    }
  return (0);
}

/******************************************************************************/

int
alloc_varval (void)
{
  int i;
  varval = (int *) malloc (sizeof (int) * nvar);
  return (0);
}


/******************************************************************************/

int
read_regvarval (char *infile)
{
  char solfilename[MAX_NAME_LENGTH];
  char szStrTmp[400], szStr2[100];
  FILE *fp;
  int i, j, k;
  int n, tvalue;
  int cnt = 0;

  /*
     for (i = 0; i < order; i++) {
     for (j = 0; j < order; j++) {
     printf("%3d", squarecolor[i][j]);
     }
     printf("\n");
     }
   */


  maxColors = 2;
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      tvalue = 0;
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      mapcolregvar[i][j][k + 1] = ++tvalue;
		    }
		}
	      mapcolregvar[i][j][0] = tvalue;	/* printf( "(%i %i %i) [ ", i, j, tvalue ); */
	      if (maxColors < tvalue)
		maxColors = tvalue;
	      for (k = 1, tvalue = 1; k <= order; k++)
		{
		  if (mapcolregvar[i][j][k] != -1)
		    {
		      mapcolregvar[i][j][tvalue++] = k;	/* printf( " %i", k ); */
		    }
		}
	      /* printf( " ] " ); */
	    }
	}
      /* printf( "\n" ); */
    }




  strcpy (solfilename, infile);
  strcat (solfilename, ".regcnf.res");

  fp = fopen (solfilename, "r");
  if (fp == NULL)
    ourerror ("Read_regvarval failed to open file");

  alloc_regvarval ();

  fgets (szStrTmp, 399, fp);
  sscanf (szStrTmp, " ------ Asignacion %s", szStr2);
  while (strcmp (szStr2, "de") != 0)
    {
      fgets (szStrTmp, 399, fp);
      sscanf (szStrTmp, " ------ Asignacion %s", szStr2);
    }

  // read variable value from the solution file
  for (i = 0; i < nregvar; i++)
    {
      fscanf (fp, " - %*d : %d", &n);
      varval[i] = n;
    }

  for (n = 0; n < nregvar; n++)
    {
      for (i = 0; i < order; i++)
	{
	  for (j = 0; j < order; j++)
	    {

	      if (mapregvar[i][j] == n)
		{
		  if (varval[n] > mapcolregvar[i][j][0])
		    varval[n] = mapcolregvar[i][j][0];
		  squarecolor[i][j] = mapcolregvar[i][j][varval[n]] - 1;
		  squarefixed[i][j] = 1;
		  cnt++;
		}

	    }
	}
    }

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  printf ("%3d", squarecolor[i][j]);
	}
      printf ("\n");
    }
  return (0);
}


/******************************************************************************/

int
read_regvarval_noheavy (char *infile)
{
  char solfilename[MAX_NAME_LENGTH];
  char szStrTmp[400], szStr2[100];
  FILE *fp;
  int i, j, k;
  int n, tvalue;
  int cnt = 0;

  /*
     for (i = 0; i < order; i++) {
     for (j = 0; j < order; j++) {
     printf("%3d", squarecolor[i][j]);
     }
     printf("\n");
     }
   */


  maxColors = 2;
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] != 1)
	    {
	      tvalue = 0;
	      for (k = 0; k < order; k++)
		{
		  if (incidencecube[i][j][k] == -1)
		    {
		      mapcolregvar[i][j][k + 1] = ++tvalue;
		    }
		}
	      mapcolregvar[i][j][0] = tvalue;	/* printf( "(%i %i %i) [ ", i, j, tvalue ); */
	      if (maxColors < tvalue)
		maxColors = tvalue;
	      for (k = 1, tvalue = 1; k <= order; k++)
		{
		  if (mapcolregvar[i][j][k] != -1)
		    {
		      mapcolregvar[i][j][tvalue++] = k;	/* printf( " %i", k ); */
		    }
		}
	      /* printf( " ] " ); */
	    }
	}
      /* printf( "\n" ); */
    }




  strcpy (solfilename, infile);
  strcat (solfilename, ".regcnf.res");

  fp = fopen (solfilename, "r");
  if (fp == NULL)
    ourerror ("Read_regvarval failed to open file");

  alloc_regvarval ();

  fgets (szStrTmp, 399, fp);
  sscanf (szStrTmp, " ------ Asignacion %s", szStr2);
  while (strcmp (szStr2, "de") != 0)
    {
      fgets (szStrTmp, 399, fp);
      sscanf (szStrTmp, " ------ Asignacion %s", szStr2);
    }

  // read variable value from the solution file
  for (i = 0; i < nregvar; i++)
    {
      fscanf (fp, " - %*d : %d", &n);
      varval[i] = n;
    }

  for (n = 0; n < nregvar; n++)
    {
      for (i = 0; i < order; i++)
	{
	  for (j = 0; j < order; j++)
	    {

	      if (mapregvar[i][j] == n)
		{
		  if (varval[n] > mapcolregvar[i][j][0])
		    varval[n] = mapcolregvar[i][j][0];
		  squarecolor[i][j] = mapcolregvar[i][j][varval[n]] - 1;
		  squarefixed[i][j] = 1;
		  cnt++;
		}

	    }
	}
    }

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  printf ("%3d", squarecolor[i][j]);
	}
      printf ("\n");
    }
  return (0);
}




/******************************************************************************/

int
alloc_regvarval (void)
{
  int i;
  varval = (int *) malloc (sizeof (int) * nregvar);
  return (0);
}


int
alloc_mapvar (void)
{
  int i, j;
  mapvar = (int ***) malloc (sizeof (int **) * order);
  for (i = 0; i < order; i++)
    {
      mapvar[i] = (int **) malloc (sizeof (int *) * order);
      for (j = 0; j < order; j++)
	{
	  mapvar[i][j] = (int *) malloc (sizeof (int) * order);
	}
    }
  return (0);
}


int
alloc_mapregvar (void)
{
  int i, j;
  mapregvar = (int **) malloc (sizeof (int *) * order);
  for (i = 0; i < order; i++)
    {
      mapregvar[i] = (int *) malloc (sizeof (int) * order);
    }
  return (0);
}


int
alloc_mapdualregvar (void)
{
  int i, j;
  maprowcolorvar = (int ***) malloc (sizeof (int **) * order);
  mapcolcolorvar = (int ***) malloc (sizeof (int **) * order);
  for (i = 0; i < order; i++)
    {
      maprowcolorvar[i] = (int **) malloc (sizeof (int *) * order);
      mapcolcolorvar[i] = (int **) malloc (sizeof (int *) * order);
      for (j = 0; j < order; j++)
	{
	  maprowcolorvar[i][j] =
	    (int *) malloc (sizeof (int) * ((order * 2) + 2));
	  mapcolcolorvar[i][j] =
	    (int *) malloc (sizeof (int) * ((order * 2) + 2));
	}
    }
  return (0);
}



int
alloc_incidencecube (void)
{
  int i, j;
  incidencecube = (int ***) malloc (sizeof (int **) * order);
  mapcolregvar = (int ***) malloc (sizeof (int **) * order);
  for (i = 0; i < order; i++)
    {
      incidencecube[i] = (int **) malloc (sizeof (int *) * order);
      mapcolregvar[i] = (int **) malloc (sizeof (int *) * order);
      for (j = 0; j < order; j++)
	{
	  incidencecube[i][j] = (int *) malloc (sizeof (int) * order);
	  mapcolregvar[i][j] =
	    (int *) malloc (sizeof (int) * ((order * 2) + 1));
	}
    }
  return (0);
}

int
alloc_squarecolor (void)
{
  int i;
  squarecolor = (int **) malloc (sizeof (int *) * order);
  for (i = 0; i < order; i++)
    {
      squarecolor[i] = (int *) malloc (sizeof (int) * order);
    }
  return (0);
}

int
alloc_squarefixed (void)
{
  int i;
  squarefixed = (int **) malloc (sizeof (int *) * order);
  for (i = 0; i < order; i++)
    {
      squarefixed[i] = (int *) malloc (sizeof (int) * order);
    }
  return (0);
}

/******************************************************************************/

/* Output a completely empty latin square */
void
new_empty (char *plsname)
{
  int i, j;

  alloc_squarecolor ();
  alloc_squarefixed ();
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  squarecolor[i][j] = -1;
	  squarefixed[i][j] = 0;
	}
    }
  alloc_incidencecube ();
  init_incidencecube ();
  write_pls (plsname);
}

/******************************************************************************/

/* Output the regular completely solved latin square */
void
new_regular (const char *outfile)
{
  int i, j;

  alloc_squarecolor ();
  alloc_squarefixed ();
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  squarecolor[i][j] = (i + j) % order;
	  squarefixed[i][j] = 1;
	}
    }
  alloc_incidencecube ();
  init_incidencecube ();
  write_pls (outfile);
}


/******************************************************************************/

/* Output the partial latin square with number of holes */
void
new_qcp (char *outfile, int numhole, unsigned long seed)
{
  int i, j, k, l, num_uncolored_square;
  int ***colorvector, **available_color, *uncolored_square;
  int **saved_squarecolor, **saved_squarefixed;
  int selectedcolor, selectedsquare, wherepos;
  int failed = 1, attempts = 0;

  if (numhole > order * order)
    ourerror ("Number of holes exceed order square");

  srandom (seed);
  alloc_squarecolor ();
  alloc_squarefixed ();
  alloc_incidencecube ();

  // allocate colorvetor
  colorvector = (int ***) malloc (sizeof (int **) * order);
  for (i = 0; i < order; i++)
    {
      colorvector[i] = (int **) malloc (sizeof (int *) * order);
      for (j = 0; j < order; j++)
	{
	  colorvector[i][j] = (int *) malloc (sizeof (int) * order);
	}
    }
  // allocate available_color
  available_color = (int **) malloc (sizeof (int *) * order);
  for (i = 0; i < order; i++)
    available_color[i] = (int *) malloc (sizeof (int) * order);
  // allocate uncolored_square
  uncolored_square = (int *) malloc (sizeof (int) * order * order);
  // allocate saved_squarecolor
  saved_squarecolor = (int **) malloc (sizeof (int *) * order);
  for (i = 0; i < order; i++)
    saved_squarecolor[i] = (int *) malloc (sizeof (int) * order);
  // allocate saved_squarefixed
  saved_squarefixed = (int **) malloc (sizeof (int *) * order);
  for (i = 0; i < order; i++)
    saved_squarefixed[i] = (int *) malloc (sizeof (int) * order);

  while (failed && attempts++ < 1000)
    {
      failed = 0;
      // initialize
      for (i = 0, num_uncolored_square = 0; i < order; i++)
	for (j = 0; j < order; j++)
	  {
	    squarecolor[i][j] = -1;
	    squarefixed[i][j] = 0;
	    available_color[i][j] = order;	// Initially each square has all the color choices
	    uncolored_square[num_uncolored_square++] = i * order + j;
	    for (k = 0; k < order; k++)
	      colorvector[i][j][k] = k;	// Keep all the color choices
	  }

      while (num_uncolored_square > numhole)
	{
	  // pick a random uncolored square
	  wherepos = random () % num_uncolored_square;
	  selectedsquare = uncolored_square[wherepos];
	  i = selectedsquare / order;
	  j = selectedsquare % order;
	  uncolored_square[wherepos] =
	    uncolored_square[--num_uncolored_square];

	  // pick a random color for the square
	  wherepos = random () % available_color[i][j];
	  selectedcolor = colorvector[i][j][wherepos];
	  squarecolor[i][j] = selectedcolor;
	  squarefixed[i][j] = 1;

	  // propagate the effect of this choice

	  // On i-th row
	  for (k = 0; k < order && !failed; k++)
	    {
	      if (squarefixed[i][k])
		continue;	// omit these colored squared
	      for (l = 0; l < available_color[i][k]; l++)
		if (colorvector[i][k][l] == selectedcolor)
		  {
		    if (--available_color[i][k] == 0)
		      {
			failed = 1;
			break;
		      }
		    colorvector[i][k][l] =
		      colorvector[i][k][available_color[i][k]];
		    break;
		  }
	    }
	  if (failed)
	    break;
	  // On j-th colomn
	  for (k = 0; k < order && !failed; k++)
	    {
	      if (squarefixed[k][j])
		continue;
	      for (l = 0; l < available_color[k][j]; l++)
		if (colorvector[k][j][l] == selectedcolor)
		  {
		    if (--available_color[k][j] == 0)
		      {
			failed = 1;
			break;
		      }
		    colorvector[k][j][l] =
		      colorvector[k][j][available_color[k][j]];
		    break;
		  }
	    }
	  if (failed)
	    break;
	}

      if (failed)
	continue;

      init_incidencecube ();

      // save the state of squarecolor and squarefixed
      for (k = 0; k < order; k++)
	for (l = 0; l < order; l++)
	  {
	    saved_squarecolor[k][l] = squarecolor[k][l];
	    saved_squarefixed[k][l] = squarefixed[k][l];
	  }

      // perform full forward checking
      if (forward_checking_incidencecube ())
	{			// if detect inconsistency
	  failed = 1;
	  continue;
	}

      // restore the state of squarecolor and squarefixed
      for (k = 0; k < order; k++)
	for (l = 0; l < order; l++)
	  {
	    squarecolor[k][l] = saved_squarecolor[k][l];
	    squarefixed[k][l] = saved_squarefixed[k][l];
	  }
    }
  if (failed)
    ourerror ("Failed to generate a quasigroup completion problem\n");
  else
    write_pls (outfile);
}


/******************************************************************************/

int
ourerror (const char *message)
{
  printf ("%s\n", message);
  exit (1);
}

/******************************************************************************/

unsigned long
getseed (void)
{
  struct timeval tv;
  struct timezone tzp;
  gettimeofday (&tv, &tzp);
  return ((tv.tv_sec & 0177) * 1000000) + tv.tv_usec;
}

/******************************************************************************/
// test validity of incidence cube
void
test_validity_incidencecube (void)
{
  int i, j, k;
  int num_1 = 0;
  int num_neg1 = 0;

  for (k = 0; k < order; k++)
    {
      for (i = 0; i < order; i++)
	{
	  for (j = 0; j < order; j++)
	    {
	      if (incidencecube[i][j][k] == 1)
		num_1++;
	      if (incidencecube[i][j][k] == -1)
		num_neg1++;
	      if (incidencecube[i][j][k] < -1 || incidencecube[i][j][k] > 1)
		ourerror ("Check incidence cube error!");
	    }
	  if (((num_neg1 != 1) || (num_1 != 2))
	      && ((num_neg1 != 0) || (num_1 != 1)))
	    ourerror ("Check incidence cube error!");
	  /*
	     if (num_neg1 == 1){ 
	     if (num_1 != 2) ourerror("Check incidence cube error 1!");
	     }
	     else {
	     if(num_neg1 == 0){
	     if (num_1 != 1) ourerror("Check incidence cube error 2!");
	     }
	     else
	     ourerror("Check incidence cube error 3!");
	     }
	   */
	  num_1 = 0;
	  num_neg1 = 0;
	}
      for (j = 0; j < order; j++)
	{
	  for (i = 0; i < order; i++)
	    {
	      if (incidencecube[i][j][k] == 1)
		{
		  num_1++;
		}
	      if (incidencecube[i][j][k] == -1)
		{
		  num_neg1++;
		}
	      if (incidencecube[i][j][k] < -1 || incidencecube[i][j][k] > 1)
		ourerror ("Check incidence cube error!");
	    }
	  if (((num_neg1 != 1) || (num_1 != 2))
	      && ((num_neg1 != 0) || (num_1 != 1)))
	    ourerror ("Check incidence cube error!");
	  /*
	     if (num_neg1 == 1){ 
	     if (num_1 != 2) ourerror("Check incidence cube error 1!");
	     }
	     else {
	     if(num_neg1 == 0){
	     if (num_1 != 1) ourerror("Check incidence cube error 2!");
	     }
	     else
	     ourerror("Check incidence cube error 3!");
	     }
	   */
	  num_1 = 0;
	  num_neg1 = 0;
	}
    }
}

/******************************************************************************/
// set colors to latin square according to incidence cube
void
ic_ls_proper (void)
{
  int i, j, k;

  for (i = 0; i < order; i++)
    for (j = 0; j < order; j++)
      for (k = 0; k < order; k++)
	if (incidencecube[i][j][k] == 1)
	  squarecolor[i][j] = k;
}

/******************************************************************************/
// set incidence cube according to the color setting of latin square
void
ls_ic_proper (void)
{
  int i, j, k;

  for (i = 0; i < order; i++)
    for (j = 0; j < order; j++)
      for (k = 0; k < order; k++)
	if (squarecolor[i][j] == k)
	  incidencecube[i][j][k] = 1;
	else
	  incidencecube[i][j][k] = 0;
}

/******************************************************************************/
// check latin square colors
void
test_validity_squarecolor_2 (void)
{
  int i, j, k;

  for (i = 0; i < order; i++)
    for (j = 0; j < order; j++)
      for (k = j + 1; k < order; k++)
	if (squarecolor[i][j] == squarecolor[i][k])
	  ourerror ("Check proper latin square failed!");
  for (i = 0; i < order; i++)
    for (j = 0; j < order; j++)
      for (k = i + 1; k < order; k++)
	if (squarecolor[i][j] == squarecolor[k][j])
	  ourerror ("Check proper latin square failed!");
}

/******************************************************************************/
// input:  three integer poiters to store values of X, Y, Z of improper cell if any
// output: 1 if the move is proper
//         0 otherwise
int
move_proper_ls (int *X_improper_cell, int *Y_improper_cell,
		int *Z_improper_cell)
{
  int i, l = 0;
  int random_index, FlagProperLs;
  int x1, x2, y1, y2, z1, z2;
  int *ArrayAltValues = (int *) malloc (sizeof (int) * (order - 1));

  // select a cell
  x1 = random () % order;
  y1 = random () % order;
  z1 = squarecolor[x1][y1];

  // select a value
  for (i = 0; i < order; i++)
    if (i != z1)
      ArrayAltValues[l++] = i;

  random_index = random () % (order - 1);
  z2 = ArrayAltValues[random_index];

  // locate row where z2 appears in the same col of selected cell ie y1
  for (i = 0; i < order; i++)
    if (squarecolor[i][y1] == z2)
      {
	x2 = i;
	break;
      }

  // locate col where z2 appears in the same row of selected cell ie x1
  for (i = 0; i < order; i++)
    if (squarecolor[x1][i] == z2)
      {
	y2 = i;
	break;
      }

  // make 8 changes

  // new value
  incidencecube[x1][y1][z2]++;
  incidencecube[x1][y2][z2]--;
  incidencecube[x2][y1][z2]--;
  incidencecube[x2][y2][z2]++;

  // old value
  incidencecube[x1][y1][z1]--;
  incidencecube[x2][y1][z1]++;
  incidencecube[x1][y2][z1]++;
  if (incidencecube[x2][y2][z1] == 0)
    {
      FlagProperLs = 0;
      *X_improper_cell = x2;
      *Y_improper_cell = y2;
      *Z_improper_cell = z1;
    }
  else
    FlagProperLs = 1;

  incidencecube[x2][y2][z1]--;

  free (ArrayAltValues);
  return FlagProperLs;
}

/******************************************************************************/
// input:  X, Y, Z of improper cell
// output: 1 if the move is proper
//         0 otherwise
int
move_improper_ls (int *X_improper_cell, int *Y_improper_cell,
		  int *Z_improper_cell)
{
  int i, l = 0;
  int random_index, FlagProperLs;
  int x1, x2, y1, y2, z1, z2;
  int x_index = -1, y_index = -1, z_index = -1;
  int Xcells[2], Ycells[2], Zcells[2];

  // select a cell and a value
  x1 = *X_improper_cell;
  y1 = *Y_improper_cell;
  z1 = *Z_improper_cell;

  // identify the two +1 occurences of symbol z1 in the same col
  for (i = 0; i < order; i++)
    if (incidencecube[i][y1][z1] == 1)
      Xcells[++x_index] = i;

  if (x_index != 1)
    ourerror ("Error in move improper latin square 1!");

  x_index = random () % 2;
  x2 = Xcells[x_index];

  // identify the two +1 occurences of symbol z1 in the same row
  for (i = 0; i < order; i++)
    if (incidencecube[x1][i][z1] == 1)
      Ycells[++y_index] = i;

  if (y_index > 1)
    ourerror ("Error in move improper latin square 2!");

  y_index = random () % 2;
  y2 = Ycells[y_index];

  // identify the two +1 occurences of other symbols in the same row and col
  for (i = 0; i < order; i++)
    if (incidencecube[x1][y1][i] == 1)
      Zcells[++z_index] = i;

  if (z_index > 1)
    ourerror ("Error in move improper latin square 3!");

  z_index = random () % 2;
  z2 = Zcells[z_index];

  // make 8 changes
  // old value
  incidencecube[x1][y1][z1]++;
  incidencecube[x2][y1][z1]--;
  incidencecube[x1][y2][z1]--;
  incidencecube[x2][y2][z1]++;

  // new value
  incidencecube[x1][y1][z2]--;
  incidencecube[x1][y2][z2]++;
  incidencecube[x2][y1][z2]++;
  if (incidencecube[x2][y2][z2] == 1)
    FlagProperLs = 1;
  else
    {
      FlagProperLs = 0;
      *X_improper_cell = x2;
      *Y_improper_cell = y2;
      *Z_improper_cell = z2;
    }
  incidencecube[x2][y2][z2]--;

  return FlagProperLs;
}

/******************************************************************************/

void
print_incidencecube (void)
{
  int i, j, k;
  for (i = 0; i < order; i++)
    for (j = 0; j < order; j++)
      {
	for (k = 0; k < order; k++)
	  printf ("%d ", incidencecube[i][j][k]);
	printf ("\n");
      }
}

/******************************************************************************/

/* Input infile.pls, shuffle it, and output outfile.pls */
void
shuffle (const char *infile, const char *outfile, int shuffles,
	 unsigned long seed)
{
  int FlagProperLs = 1, CounterProper = 0;
  int X_improper_cell, Y_improper_cell, Z_improper_cell;

  if (seed != -1)
    srandom (seed);

  // read square color from infile and test its validity
  read_pls_in_squarecolor (infile);
  test_validity_squarecolor_2 ();

  // initialize incidence cube according to square color
  if (first)
    alloc_incidencecube ();
  ls_ic_proper ();

  while (CounterProper <= shuffles)
    {
      // test validity of inicidence cube
      test_validity_incidencecube ();
      //print_incidencecube();
      if (FlagProperLs == 1)
	{
	  // set square color according to incidence cube
	  ic_ls_proper ();

	  // test the validity of square color
	  test_validity_squarecolor_2 ();

	  // shuffle the latin square
	  FlagProperLs =
	    move_proper_ls (&X_improper_cell, &Y_improper_cell,
			    &Z_improper_cell);
	}
      else			// reshuffle the latin square if last shuffle is inproper
	FlagProperLs =
	  move_improper_ls (&X_improper_cell, &Y_improper_cell,
			    &Z_improper_cell);


      // increase CounterProper by one if FlagProperLs is TRUE
      CounterProper += FlagProperLs;
    }

  //write square color to outfile
  write_pls (outfile);
}
