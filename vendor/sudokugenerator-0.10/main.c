/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 *
 * Copyright (C) 2003    Carla Gomes   <gomes@cs.cornell.edu>
 *       	    	 Henry Kautz   <kautz@cs.washington.edu>
 *          	         Yongshao Ruan <ruan@cs.washington.edu>
 * Copyright (C) 2008    Carles Mateu  <carlesm@diei.udl.cat>
 * 
 * main.c is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * main.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with main.c.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "generator.h"



int order = 0;			/* global variable for order of ls */
int bsi_x;			/* block size for sudoku */
int bsi_y;			/* block size for sudoku */

int **squarecolor;		/* squarecolor[i][j] == color assigned to that square if any
				   == -1 if no color assigned */
int **squarefixed;		/* squarefixed[i][j] == 1 iff that square is colored,
				   == 0 iff color not yet determined */

int ***incidencecube;		/* incidencecube[i][j][k] ==
				   1 iff squarecolor[i][j] must be color k
				   0 iff squarecolor[i][j] *must not* be color k
				   -1 iff squarecolor[i][j] might or might not end up being k */


int *alldiffcheck;
int **violated_cells;

int
error (char *message)
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



int
main (int argc, char *argv[])
{
  char command[MAX_NAME_LENGTH];
  char infile[MAX_NAME_LENGTH];
  char outfile[MAX_NAME_LENGTH];
  char model[MAX_NAME_LENGTH];
  int imodel = -1;
  int numhole;
  int numrow;
  unsigned long seed;
  int shuffles;
  int shufflesintersudokus;
  FILE *fp;

  if (argc <= 1)
    {
      printf ("Usage : %s new {empty | regular} OUTFILE ORDER\n", argv[0]);

      printf ("Usage : %s newsudoku  OUTFILE ORDER [block_row_size]\n",
	      argv[0]);
      printf
	("Usage : %s shuffle INFILE NUMBEROFSUDOKUS OUTFILE SHUFFLESINTERSUDOKUS{ SEED }\n",
	 argv[0]);

      printf
	("Usage : %s poke {random | singlybalanced } INFILE NUMHOLE OUTFILE { SEED }\n",
	 argv[0]);
      printf ("Usage : %s poke rectangle INFILE NUMROW OUTFILE\n", argv[0]);
      printf
	("Usage : %s poke {groupbalanced | doublybalanced|fullybalanced} INFILE NUMHOLE SHUFFLES OUTFILE { SEED }\n",
	 argv[0]);
      exit (-1);
    }


  fp = fopen ("record", "a");
  if (fp == NULL)
    error ("Open file failure");
  fseek (fp, 0, 2);

  sscanf (argv[1], "%s", command);

  if (strcmp (command, "poke") == 0)
    {
      if (argc < 3)
	error ("Bad arguments");
      sscanf (argv[2], "%s", model);


      if ((strcmp (model, "random") == 0) ||
	  (strcmp (model, "singlybalanced") == 0))
	{
	  if (argc < 6)
	    error ("Bad arguments");
	  sscanf (argv[3], "%s", infile);
	  sscanf (argv[4], "%d", &numhole);

	  sscanf (argv[5], "%s", outfile);
	  if (argc > 6)
	    sscanf (argv[6], "%lu", &seed);
	  else
	    seed = getseed ();
	  if (strcmp (model, "random") == 0)
	    imodel = RANDOM;
	  else if (strcmp (model, "singlybalanced") == 0)
	    imodel = BALANCED;

	  poke (infile, numhole, seed, outfile, imodel);
	  printf
	    ("%s poke: infile: %s  order: %d  holes: %d  outfile: %s  seed: %lu\n",
	     model, infile, order, numhole, outfile, seed);
	  fprintf (fp,
		   "%s poke: infile: %s  order: %d  holes: %d  outfile: %s  seed: %lu\n",
		   model, infile, order, numhole, outfile, seed);
	}
      else if (strcmp (model, "rectangle") == 0)
	{
	  if (argc < 6)
	    error ("Bad arguments");
	  sscanf (argv[3], "%s", infile);
	  sscanf (argv[4], "%d", &numhole);


	  numrow = numhole;
	  sscanf (argv[5], "%s", outfile);
	  poke_rectangle (infile, numrow, outfile, seed);
	  printf
	    ("rectangle poke: infile: %s  order: %d  rows: %d  numholes: %d  outfile: %s\n",
	     infile, order, numrow, numrow * order, outfile);
	  fprintf (fp,
		   "rectangle poke: infile: %s  order: %d  rows: %d  numholes: %d  outfile: %s\n",
		   infile, order, numrow, numrow * order, outfile);
	}
      else
	{
	  if (argc < 7)
	    error ("Bad arguments");
	  sscanf (argv[2], "%s", model);
	  sscanf (argv[3], "%s", infile);
	  sscanf (argv[4], "%d", &numhole);
	  sscanf (argv[5], "%d", &shuffles);
	  sscanf (argv[6], "%s", outfile);
	  if (argc > 7)
	    sscanf (argv[7], "%lu", &seed);
	  else
	    seed = getseed ();

	  if (strcmp (model, "groupbalanced") == 0)
	    imodel = GROUPBALANCED;
	  else if (strcmp (model, "doublybalanced") == 0)
	    imodel = DOUBLYBALANCED;
	  else
	    imodel = FULLYBALANCED;

	  pokesudoku (infile, numhole, shuffles, seed, outfile, imodel);

	}

    }
  else if (strcmp (command, "newsudoku") == 0)
    {
      if (argc < 4)
	error ("Bad arguments to new");
      sscanf (argv[2], "%s", outfile);
      sscanf (argv[3], "%d", &order);
      if (argc == 4)
	{
	  bsi_x = (int) sqrt (order);
	  bsi_y = bsi_x;
	}
      if (argc == 5)
	{
	  sscanf (argv[4], "%d", &bsi_x);
	  bsi_y = (int) order / bsi_x;
	}
      new_sudoku (outfile);

    }
  else if (strcmp (command, "shuffle") == 0)
    {
      if (argc < 6)
	error ("Bad arguments");
      sscanf (argv[2], "%s", infile);
      sscanf (argv[3], "%d", &shuffles);
      sscanf (argv[4], "%s", outfile);
      sscanf (argv[5], "%d", &shufflesintersudokus);
      if (argc > 6)
	sscanf (argv[6], "%lu", &seed);
      else
	seed = getseed ();
      shuffle (infile, outfile, shuffles, seed, shufflesintersudokus);
      fprintf (fp,
	       "shuffle: infile: %s  outfile: %s  shuffles: %d  seed: %lu\n",
	       infile, outfile, shuffles, seed);
      printf ("shuffle: infile: %s  outfile: %s  shuffles: %d  seed: %lu\n",
	      infile, outfile, shuffles, seed);

    }
  else
    error ("Unknown command");

  //fclose(fp);

  exit (0);
}
