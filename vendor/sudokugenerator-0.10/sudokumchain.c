
#include "generator.h"



char linebuf[MAX_LINE_LENGTH + 1];
char stringbuf[MAX_LINE_LENGTH + 1];






int **oldsquarecolor;		/* squarecolor[i][j] == color assigned to that square if any */



void
init_alldiffcheck (void)
{
  int i;
  for (i = 0; i < order; i++)
    {
      alldiffcheck[i] = 0;
    }

}

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
	    }
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
		      error ("incidence ??????");
		    incidencecube[i1][j][k] = 0;
		  }
	      for (j1 = 0; j1 < order; j1++)
		if (j1 != j)
		  {
		    if (incidencecube[i][j1][k] == 1)
		      error ("incidence ??????");
		    incidencecube[i][j1][k] = 0;
		  }
	      for (k1 = 0; k1 < order; k1++)
		if (k1 != k)
		  {
		    if (incidencecube[i][j][k1] == 1)
		      error ("incidence ??????");
		    incidencecube[i][j][k1] = 0;
		  }
	    }
	}
    }
  return (0);
}

/******************************************************************************/



int
alloc_incidencecube (void)
{
  int i, j;
  incidencecube = (int ***) malloc (sizeof (int **) * order);
  for (i = 0; i < order; i++)
    {
      incidencecube[i] = (int **) malloc (sizeof (int *) * order);
      for (j = 0; j < order; j++)
	{
	  incidencecube[i][j] = (int *) malloc (sizeof (int) * order);
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




/* Output the regular completely solved latin square */
void
new_sudoku (char *outfile)
{
  int i, j, I;

  alloc_squarecolor ();
  alloc_squarefixed ();
  for (I = 0; I < bsi_x; I++)
    {
      for (i = 0; i < bsi_y; i++)
	{
	  for (j = 0; j < order; j++)
	    {
	      squarecolor[i + I * bsi_y][j] =
		((I + i * bsi_x) % order + j) % order;
	      squarefixed[i + I * bsi_y][j] = 1;
	    }
	}
    }
  alloc_incidencecube ();
  init_incidencecube ();
  write_pls (outfile);
}




int
write_pls (char *outfile)
{
  char plsfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j;

  strcpy (plsfilename, outfile);
  strcat (plsfilename, ".pls");

  fp = fopen (plsfilename, "w");

  fprintf (fp, "order %d\t%d\n", order, bsi_x);
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

void
print_pls ()
{
  int i, j;

  printf ("order %d\n", order);
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (squarefixed[i][j] == 1)
	    printf ("%4d", squarecolor[i][j]);
	  else
	    printf ("%4d", -1);
	}
      printf ("\n");
    }

}

void
test_validity_squarecolor_2 (void)
{
  int i, j, k;

  for (i = 0; i < order; i++)
    for (j = 0; j < order; j++)
      for (k = j + 1; k < order; k++)
	if (squarecolor[i][j] == squarecolor[i][k])
	  error ("Check proper latin square failed!");
  for (i = 0; i < order; i++)
    for (j = 0; j < order; j++)
      for (k = i + 1; k < order; k++)
	if (squarecolor[i][j] == squarecolor[k][j])
	  error ("Check proper latin square failed!");
}










void
shuffle (char *infile, char *outfile, int shuffles, unsigned long seed,
	 int shufflesintersudokus)
{
  int CounterProper = 0;
  int CounterSudokus = 0;
  int CounterSudokusPrinted = 0;
  int threshold = 20;
  //int X_improper_cell, Y_improper_cell, Z_improper_cell;
  int col, col_prime, symb, s, symb_prime, r, r_prime, l, row;
  int i;
//  int k, z, y;
  int tmp = 0;
  int srr1, srr2, srr, src;	//Sudok random row, col  and sym
//  printf"test");
  //initialize thge seed 


  // read square color from infile and test its validity
  read_pls_in_squarecolor (infile);
  test_validity_squarecolor_2 ();

  // initialize incidence cube according to square color
  alloc_incidencecube ();
  ls_ic_proper ();
  alloc_oldsquarecolor ();
  //
  alloc_alldiffcheck ();
  alloc_violated_cells ();

  srand (seed);
  if (is_sudoku () == 0)
    error ("Not sudoku input file\n");
  //while ( CounterProper < shuffles) 
  while ((!is_sudoku ()) || (CounterSudokusPrinted < shuffles))
    {

      tmp = violated_rows_cols ();
      if (tmp > 0)
	{

	  if (CounterProper > threshold)
	    {
	      CounterProper = 0;
	      read_pls_in_squarecolor (outfile);
	      ls_ic_proper ();

	      //recoversquarecolor();
	      r_prime = rand () % order;	// Fila 1 a

	      r = rand () % order;	// Fila 2
	      while (r_prime == r)
		{
		  r = rand () % order;
		}
	      l = rand () % order;	//Columna
	      while (l <= 1)
		{
		  l = rand () % order;
		}
	      s = rand () % order;
	    }
	  else
	    {

	      srr1 = rand () % tmp;
	      srr2 = rand () % tmp;
	      while (srr2 == srr1)
		{
		  srr2 = rand () % tmp;
		}
	      src = rand () % 2;
	      if (src == 0)
		{
		  src = violated_cells[srr1][1];
		  srr = violated_cells[srr1][0];
		}
	      else
		{
		  src = violated_cells[srr2][1];
		  srr = violated_cells[srr2][0];
		}
	      srr1 = violated_cells[srr1][0];
	      srr2 = violated_cells[srr2][0];
	      r_prime = srr1;	// Fila 1 a
	      r = srr2;		// Fila 2
	      l = rand () % order;	//Columna
	      while (l <= 1)
		{
		  l = rand () % order;
		}
	      s = squarecolor[srr][src];
	    }
	}
      else
	{
//      CounterProper=0;
	  r_prime = rand () % order;	// Fila 1 a

	  r = rand () % order;	// Fila 2
	  while (r_prime == r)
	    {
	      r = rand () % order;
	    }
	  l = rand () % order;	//Columna
	  while (l <= 1)
	    {
	      l = rand () % order;
	    }
	  s = rand () % order;	// Simbol dels 2 possibles que canviem
	}



/*
srr1: first row (r_prime)
srr2: second row (r)
src: column (l)
squarecolor[srr][src]: symbol (s) 
*/


      //initialize proper arguments
      col_prime = -1;
      symb_prime = -1;
      col = -1;
      row = -1;

      symb = -1;
      //first subsequence of the second method dealing with row r_prime and r
      for (i = 1; i <= l; i++)
	{

	  if (i == 1)
	    {
	      col = Find_col (r_prime, s, -1);
	      symb = Find_symb (r, col);
	      if (symb == -1)
		error ("error dealing with improper symbols");
	      incidencecube[r_prime][col][symb]++;
	      incidencecube[r_prime][col][s]--;
	      incidencecube[r][col][symb]--;
	      incidencecube[r][col][s]++;
	    }
	  else if (i == l)
	    {
	      col_prime = Find_col (r_prime, symb, col);
	      if (col_prime != -1)
		{
		  row = Find_row (s, col_prime);
		  if (row == -1)
		    error ("error dealing with improper rows");
		  incidencecube[r_prime][col_prime][s]++;
		  incidencecube[r_prime][col_prime][symb]--;
		  incidencecube[row][col_prime][s]--;
		  incidencecube[row][col_prime][symb]++;
		  col = col_prime;
		  r_prime = row;
		}
	    }
	  else
	    {
	      col_prime = Find_col (r_prime, symb, col);
	      if (col_prime != -1)
		{

		  symb_prime = Find_symb (r, col_prime);
		  if (symb_prime == -1)
		    error ("error dealing with improper symbols");
		  incidencecube[r_prime][col_prime][symb_prime]++;
		  incidencecube[r_prime][col_prime][symb]--;
		  incidencecube[r][col_prime][symb_prime]--;
		  incidencecube[r][col_prime][symb]++;
		}
	      else
		{

		  i = l + 1;	//a proper square was found in 
		}
	      symb = symb_prime;
	      col = col_prime;

	    }
	}


      //swap between r and a third row 
      if (r_prime == row)
	{

	  col_prime = Find_col (row, symb, col);
	  while (col_prime != -1)
	    {
	      symb_prime = Find_symb (r, col_prime);
	      incidencecube[row][col_prime][symb_prime]++;
	      incidencecube[row][col_prime][symb]--;
	      incidencecube[r][col_prime][symb_prime]--;
	      incidencecube[r][col_prime][symb]++;
	      symb = symb_prime;
	      col = col_prime;
	      col_prime = Find_col (row, symb, col);
	    }
	}
      // set square color according to incidence cube
      ic_ls_proper ();

      // test the validity of square color
      test_validity_squarecolor_2 ();
      //count

      if (is_sudoku ())
	{
	  CounterSudokus++;
	  if (CounterSudokus % shufflesintersudokus == 0)
	    {
	      CounterSudokusPrinted++;
	      print_pls ();
	    }
//        clonesquarecolor();
	  write_pls (outfile);
	}

      CounterProper++;
//   printf("CP: %d %d\n",CounterProper,CounterSudokus);
    }

  //write square color to outfile

}

int
violated_rows_cols ()
{
  int I, J, i, j, count;
  count = 0;
  for (I = 0; I < bsi_x; I++)
    {
      for (J = 0; J < bsi_y; J++)
	{
	  init_alldiffcheck ();
	  for (i = 0; i < bsi_y; i++)
	    {
	      for (j = 0; j < bsi_x; j++)
		{
		  alldiffcheck[squarecolor[i + I * bsi_y][j + J * bsi_x]]++;
		}
	    }
	  for (i = 0; i < bsi_y; i++)
	    {
	      for (j = 0; j < bsi_x; j++)
		{
		  if (alldiffcheck[squarecolor[i + I * bsi_y][j + J * bsi_x]]
		      > 1)
		    {
		      violated_cells[count][0] = i + I * bsi_y;
		      violated_cells[count][1] = j + J * bsi_x;
		      count++;
		    }
		}
	    }
	}
    }
  return count;
}

int
is_sudoku ()
{
  int I, J, i, j;
  if (order != (bsi_x * bsi_y))
    return 0;
  else
    {
      for (I = 0; I < bsi_x; I++)
	{
	  for (J = 0; J < bsi_y; J++)
	    {
	      init_alldiffcheck ();
	      for (i = 0; i < bsi_y; i++)
		{
		  for (j = 0; j < bsi_x; j++)
		    {
		      alldiffcheck[squarecolor[i + I * bsi_y][j + J * bsi_x]]
			= 1;
//                      printf("x y: %d %d %d\n",i+I*bsi_y,j+J*bsi_x,squarecolor[i+I*bsi_y][j+J*bsi_x]);
		    }
		}
	      i = 0;
	      while (i < order)
		{
		  if (alldiffcheck[i] == 0)
		    {
		      return 0;
		    }
		  i++;
		}
	    }
	}

      return 1;
    }
}

/**************************************************************************************************/


int
Find_col (int row, int symbol, int column)
{
  int new_col = 0;

  //find the porper column
  while (new_col < order)
    {

      if (new_col != column && incidencecube[row][new_col][symbol] == 1)
	{
	  return new_col;
	}
      new_col++;
    }
  return -1;

}


/***************************************************************************************************/

int
Find_symb (int row, int column)
{
  int new_symb = 0;

  //find proper symbol
  while (new_symb < order)
    {
      if (incidencecube[row][column][new_symb] == 1)
	{
	  return new_symb;
	}
      new_symb++;
    }
  return -1;
}


/*******************************************************************************************************/

int
Find_row (int symbol, int column)
{
  int new_row = 0;


  //find porper row
  while (new_row < order)
    {
      if (incidencecube[new_row][column][symbol] == 1)
	{
	  return new_row;
	}
      new_row++;
    }
  return -1;
}



int
read_pls_in_squarecolor (char *infile)
{
  char plsfilename[MAX_NAME_LENGTH];
  FILE *fp;
  int i, j;


  strcpy (plsfilename, infile);
  strcat (plsfilename, ".pls");

  fp = fopen (plsfilename, "r");
  if (fp == NULL)
    error ("Read_pls_in_squarecolor failed to open file");

  fgets (linebuf, MAX_LINE_LENGTH, fp);
  i = sscanf (linebuf, "%s %d %d", stringbuf, &order, &bsi_x);
  if (i == 3)
    {
      bsi_y = (int) order / bsi_x;
    }
  else
    {
      bsi_x = (int) sqrt (order);
      bsi_y = bsi_x;
    }

  while (strcmp (stringbuf, "order") != 0)
    {
      fgets (linebuf, MAX_LINE_LENGTH, fp);
      i = sscanf (linebuf, "%s %d %d", stringbuf, &order, &bsi_x);
      if (i == 3)
	{
	  bsi_y = (int) order / bsi_x;
	}
      else
	{
	  bsi_x = (int) sqrt (order);
	  bsi_y = bsi_x;
	}
    }

  alloc_squarecolor ();
  alloc_squarefixed ();

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
  fclose (fp);
  return (0);
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

int
alloc_oldsquarecolor (void)
{
  int i;
  oldsquarecolor = (int **) malloc (sizeof (int *) * order);
  for (i = 0; i < order; i++)
    {
      oldsquarecolor[i] = (int *) malloc (sizeof (int) * order);
    }
  return (0);
}

int
alloc_violated_cells (void)
{
  int i;
  violated_cells = (int **) malloc (sizeof (int *) * ((order * order)));
  for (i = 0; i < ((order * order)); i++)
    {
      violated_cells[i] = (int *) malloc (sizeof (int) * 2);
    }
  return (0);
}


int
alloc_alldiffcheck (void)
{
  alldiffcheck = (int *) malloc (sizeof (int) * order);
  return (0);
}
