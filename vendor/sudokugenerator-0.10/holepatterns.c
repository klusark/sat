

#include "generator.h"



typedef struct
{
  int i, j, ip, jp;
} stSwitch;



#define SAMEGROUPX(i,ip)  (((i)/bsi_y) == ((ip)/bsi_y))
#define SAMEGROUPY(j,jp)   (((j)/bsi_x) == ((jp)/bsi_x))
#define VALIDSWITCH(i,j,ip,jp) ( (holepattern[(i)][(j)] != -1) && (holepattern[(ip)][(jp)] != -1) && (holepattern[(i)][(jp)] == -1) && (holepattern[(ip)][(j)] == -1) && ((i) != (ip)) && ((j) != (jp)) )



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
	    error ("The latin square is not valid...");
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
	    error ("The latin square is not valid...");
	}
    }
  free (markvector);
  return 1;
}

/******************************************************************************/

void
poke (char *infile, int numhole, unsigned long seed, char *plsname, int model)
{
  srandom (seed);
  read_pls_in_squarecolor (infile);
  test_validity_squarecolor ();

  switch (model)
    {
    case RANDOM:
      generate_random_holes (numhole);
      break;
    case BALANCED:
      generate_balanced_holes (numhole);
    }
  write_pls (plsname);
}



/******************************************************************************/

int
generate_random_holes (int numhole)
{
  int i, j;
  int *posvector, npos;
  int selectedpos;
  int wherepos;

  if (numhole > order * order)
    error ("Number of holes exceed order square");

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
	error ("??");
      squarefixed[selectedpos % order][selectedpos / order] = 0;
      posvector[wherepos] = posvector[--npos];
    }

  return (0);
}



int
generate_balanced_holes (int numhole)
{
  int n, i, j, k, i1, j1;
  int num_color, min_num_color;
  int *posvector, npos;
  int *propagatedcolors;
  int selectedpos;
  int wherepos;

  if (numhole > order * order)
    error ("Number of holes exceed order square");

  posvector = (int *) malloc (sizeof (int) * order * order);
  propagatedcolors = (int *) malloc (sizeof (int) * order * order);
  for (i = 0; i < order * order; i++)
    posvector[i] = 0;

  for (i = 0; i < order; i++)
    for (j = 0; j < order; j++)
      {
	squarefixed[i][j] = 1;
	propagatedcolors[i * order + j] = 0;
      }

  alloc_incidencecube ();
  init_incidencecube ();
  for (n = 0; n < numhole; n++)
    {
      /* Find out the fewest/most number of propagated color of non-empty squares */
      min_num_color = 2 * order;
      for (i = 0; i < order; i++)
	for (j = 0; j < order; j++)
	  {
	    if (squarefixed[i][j] == 0)
	      continue;
	    num_color = propagatedcolors[i * order + j];
	    if (min_num_color > num_color)
	      min_num_color = num_color;
	  }

      /* Find out the non-empty squares with the fewest/most number of propagated color */
      npos = 0;
      for (i = 0; i < order; i++)
	for (j = 0; j < order; j++)
	  {
	    if (squarefixed[i][j] == 0)
	      continue;
	    num_color = propagatedcolors[i * order + j];
	    if (num_color == min_num_color)
	      {
		posvector[npos++] = i * order + j;
	      }
	  }

      wherepos = random () % npos;
      selectedpos = posvector[wherepos];
      i = selectedpos / order;
      j = selectedpos % order;
      k = squarecolor[i][j];

      if (squarefixed[i][j] != 1)
	error ("Balanced generate holes???");
      squarefixed[i][j] = 0;

      // propoage to the squares in the same column
      for (i1 = 0; i1 < order; i1++)
	propagatedcolors[i1 * order + j]++;
      // propoage to the squares in the same row
      for (j1 = 0; j1 < order; j1++)
	propagatedcolors[i * order + j1]++;

    }

  return (0);
}



/******************************************************************************/

void
poke_rectangle (char *infile, int numcols, char *plsname, unsigned long seed)
{
  srandom (seed);
  read_pls_in_squarecolor (infile);
  test_validity_squarecolor ();
  generate_rectangle_holes_balanced (numcols);
  write_pls (plsname);
}



/******************************************************************************/

int
generate_rectangle_holes_balanced (int numcols)
{
  int i, j, J, nc;

  if (numcols > order)
    error ("Number of rows exceed order");

  J = -1;
  for (nc = 0; nc < numcols; nc++)
    {
      J = (J + 1) % bsi_y;
      j = random () % bsi_x;
      while (squarefixed[0][(J * bsi_x) + j] != 1)
	j = random () % bsi_x;
      for (i = 0; i < order; i++)
	{
	  squarefixed[i][j] = 0;
	}
    }
  return (0);
}




void
pokesudoku (char *infile, int numhole, int shuffles, unsigned long seed,
	    char *plsname, int model)
{
  srandom (seed);
  read_pls_in_squarecolor (infile);
  test_validity_squarecolor ();

  switch (model)
    {
    case GROUPBALANCED:
      generate_blockbalanced_holes (numhole);
      break;
    case DOUBLYBALANCED:
      generate_doublybalanced_holes (numhole, shuffles);
      break;
    case FULLYBALANCED:
      generate_fullybalanced_holes_region (numhole);
    }
  write_pls (plsname);
}

void
generate_blockbalanced_holes (int numhole)
{
  int i, j, hgroup, gx, gy, ngx, ngy, h;
  int holepattern[100][100];

  hgroup = numhole / order;
  ngx = order / bsi_x;
  ngy = order / bsi_y;

  for (i = 0; i < order; i++)
    for (j = 0; j < order; j++)
      holepattern[i][j] = -1;


  for (gx = 0; gx < ngx; gx++)
    for (gy = 0; gy < ngy; gy++)
      {
	h = 0;
	while (h < hgroup)
	  {
	    i = (random () % bsi_y);
	    j = (random () % bsi_x);
	    if (holepattern[(gy * bsi_y) + i][(gx * bsi_x) + j] == -1)
	      {
		holepattern[(gy * bsi_y) + i][(gx * bsi_x) + j] = 0;
		h++;
	      }
	  }
      }

  /*  Apply the resulting  hole pattern */
  for (i = 0; i < order; i++)
    for (j = 0; j < order; j++)
      squarefixed[i][j] = (holepattern[i][j] == -1);
}

void
generate_doublybalanced_holes (int numhole, int shuffles)
{
  int holepattern[100][100];
  int targetholes[100] = { 0 };
  int i, j, ir, num, numbers, tnumber;
  int ip, jp, s, s2, sh;
  stSwitch *pSwitch1, *pSwitch2;
  stSwitch ssw, nsw, ssw2;
  int numswitches, ti, top, gx, gy, ngx, ngy;
  int turn, newnum, ok, t;
  int rest;

  for (i = 0; i < order; i++)
    {
      ir = (i / bsi_y) + ((i % bsi_y) * bsi_x);	// Original row of the LS(order)
      for (j = 0; j < order; j++)
	{
	  holepattern[i][j] = ((ir + j) % order);
	  /*  printf( " %i", holepattern[i][j] ); */
	}
      /*    printf( "\n" ); */
    }
/*   printf( "\n" ); */

  numbers = numhole / order;
  for (num = 0; num < numbers;)
    {
      tnumber = random () % order;
      if (!targetholes[tnumber])
	{
	  targetholes[tnumber] = order;
	  num++;
	}
    }
  rest = numhole % order;
  if (rest)
    {
      tnumber = random () % order;
      while (targetholes[tnumber])
	tnumber = random () % order;
      targetholes[tnumber] = rest;
    }
  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  if (!targetholes[holepattern[i][j]])
	    holepattern[i][j] = -1;
	  else
	    targetholes[holepattern[i][j]]--;
/*         printf( " %+i", holepattern[i][j] );  */
	}
/*     printf( "\n" ); */
    }



  /*  Now, we have to shuffle the hole pattern  */

  /*   Set up the initial list of valid switches */


  pSwitch1 =
    (stSwitch *) malloc (((numhole * (numhole - 1)) / 2) * sizeof (stSwitch));
  pSwitch2 =
    (stSwitch *) malloc (((numhole * (numhole - 1)) / 2) * sizeof (stSwitch));

  numswitches = 0;

  for (i = 0; i < order; i++)
    {
      for (j = 0; j < order; j++)
	{
	  for (ip = i + 1; ip < order; ip++)
	    {
	      for (jp = 0; jp < order; jp++)
		{
		  if (SAMEGROUPX (i, ip) || SAMEGROUPY (j, jp))
		    {
		      if (VALIDSWITCH (i, j, ip, jp))
			{
			  pSwitch1[numswitches].i = i;
			  pSwitch1[numswitches].j = j;
			  pSwitch1[numswitches].ip = ip;
			  pSwitch1[numswitches++].jp = jp;
			}
		    }
		}
	    }
	}
    }





  /*  Perform "shuffles" switches in the hole pattern  */

  turn = 0;
  for (sh = 0; sh < shuffles; sh++)
    {
      /*   printf( " nsw: %i\n", numswitches );  */

      s = (random () % numswitches);
      if (!turn)
	ssw = pSwitch1[s];
      else
	ssw = pSwitch2[s];

      /*   printf( "ssw: (%i,%i) (%i,%i)\n", ssw.i, ssw.j, ssw.ip, ssw.jp ); */

      newnum = 0;
      for (s2 = 0; s2 < numswitches; s2++)
	{
	  if (!turn)
	    {
	      ssw2 = pSwitch1[s2];
	    }
	  else
	    ssw2 = pSwitch2[s2];

	  if (!(((ssw2.i == ssw.i) && (ssw2.j == ssw.j)) ||
		((ssw2.ip == ssw.i) && (ssw2.jp == ssw.j)) ||
		((ssw2.i == ssw.ip) && (ssw2.j == ssw.jp)) ||
		((ssw2.ip == ssw.ip) && (ssw2.jp == ssw.jp))) &&
	      !(((ssw2.i == ssw.i) && (ssw2.jp == ssw.jp)) ||
		((ssw2.ip == ssw.i) && (ssw2.j == ssw.jp)) ||
		((ssw2.i == ssw.ip) && (ssw2.jp == ssw.j)) ||
		((ssw2.ip == ssw.ip) && (ssw2.j == ssw.j))))
	    {
	      if (!turn)
		pSwitch2[newnum++] = ssw2;
	      else
		pSwitch1[newnum++] = ssw2;
	    }
	}

      /*  Perform switch  */
      holepattern[ssw.i][ssw.jp] = holepattern[ssw.i][ssw.j];
      holepattern[ssw.ip][ssw.j] = holepattern[ssw.ip][ssw.jp];
      holepattern[ssw.i][ssw.j] = -1;
      holepattern[ssw.ip][ssw.jp] = -1;

      /*   Add new switches with (i,jp) to the switch list  */

      gy = ssw.i / bsi_y;
      gx = ssw.jp / bsi_x;
      top = (gy + 1) * bsi_y;

      for (i = gy * bsi_y; i < top; i++)
	for (j = 0; j < order; j++)
	  {
	    if (VALIDSWITCH (ssw.i, ssw.jp, i, j))
	      {
		nsw.i = ssw.i;
		nsw.j = ssw.jp;
		nsw.ip = i;
		nsw.jp = j;

		if (!turn)
		  pSwitch2[newnum++] = nsw;
		else
		  pSwitch1[newnum++] = nsw;
	      }
	  }

      ti = gy * bsi_y;
      top = (gx + 1) * bsi_x;

      for (i = 0; i < ti; i++)
	for (j = gx * bsi_x; j < top; j++)
	  {
	    if (VALIDSWITCH (ssw.i, ssw.jp, i, j))
	      {
		nsw.i = ssw.i;
		nsw.j = ssw.jp;
		nsw.ip = i;
		nsw.jp = j;

		if (!turn)
		  pSwitch2[newnum++] = nsw;
		else
		  pSwitch1[newnum++] = nsw;
	      }
	  }


      for (i = ti + bsi_y; i < order; i++)
	for (j = gx * bsi_x; j < top; j++)
	  {
	    if (VALIDSWITCH (ssw.i, ssw.jp, i, j))
	      {
		nsw.i = ssw.i;
		nsw.j = ssw.jp;
		nsw.ip = i;
		nsw.jp = j;

		if (!turn)
		  pSwitch2[newnum++] = nsw;
		else
		  pSwitch1[newnum++] = nsw;
	      }
	  }


      /*   Add new switches with (ip,j) to the switch list  */

      gy = ssw.ip / bsi_y;
      gx = ssw.j / bsi_x;
      top = (gy + 1) * bsi_y;

      for (i = gy * bsi_y; i < top; i++)
	for (j = 0; j < order; j++)
	  {
	    if (((i != ssw.i) || (j != ssw.jp))
		&& VALIDSWITCH (ssw.ip, ssw.j, i, j))
	      {
		nsw.i = ssw.ip;
		nsw.j = ssw.j;
		nsw.ip = i;
		nsw.jp = j;

		if (!turn)
		  pSwitch2[newnum++] = nsw;
		else
		  pSwitch1[newnum++] = nsw;
	      }
	  }

      ti = gy * bsi_y;
      top = (gx + 1) * bsi_x;

      for (i = 0; i < ti; i++)
	for (j = gx * bsi_x; j < top; j++)
	  {
	    if (((i != ssw.i) || (j != ssw.jp))
		&& VALIDSWITCH (ssw.ip, ssw.j, i, j))
	      {
		nsw.i = ssw.ip;
		nsw.j = ssw.j;
		nsw.ip = i;
		nsw.jp = j;

		if (!turn)
		  pSwitch2[newnum++] = nsw;
		else
		  pSwitch1[newnum++] = nsw;
	      }
	  }


      for (i = ti + bsi_y; i < order; i++)
	for (j = gx * bsi_x; j < top; j++)
	  {
	    if (((i != ssw.i) || (j != ssw.jp))
		&& VALIDSWITCH (ssw.ip, ssw.j, i, j))
	      {
		nsw.i = ssw.ip;
		nsw.j = ssw.j;
		nsw.ip = i;
		nsw.jp = j;

		if (!turn)
		  pSwitch2[newnum++] = nsw;
		else
		  pSwitch1[newnum++] = nsw;
	      }
	  }


      turn = !turn;
      numswitches = newnum;


      /*  for (  i = 0; i < order ; i++ )
         {
         for ( j = 0; j < order ; j++  ) 
         {
         printf( " %+i", holepattern[i][j] );
         }  
         printf( "\n" );
         }
         printf( "\n" );  */
    }

  ok = 1;
  for (i = 0; ok && (i < order); i++)
    {
      for (t = 0, j = 0; j < order; j++)
	if (holepattern[i][j] != -1)
	  t++;
      if (t != numbers)
	ok = 0;
    }

  for (j = 0; ok && (j < order); j++)
    {
      for (t = 0, i = 0; i < order; i++)
	if (holepattern[i][j] != -1)
	  t++;
      if (t != numbers)
	ok = 0;
    }

  ngx = order / bsi_x;
  ngy = order / bsi_y;

  for (gx = 0; ok && (gx < ngx); gx++)
    for (gy = 0; ok && (gy < ngy); gy++)
      {
	for (t = 0, i = 0; (i < bsi_y); i++)
	  {
	    for (j = 0; (j < bsi_x); j++)
	      if (holepattern[(gy * bsi_y) + i][(gx * bsi_x) + j] != -1)
		t++;
	  }
	if (t != (numbers))
	  ok = 0;
      }
/*     if ( !ok ) { printf( "Eppss !! No valid full-balanced hole pattern !!!\n" );  exit(-1);  }  */

  /*  Apply the resulting  hole pattern */
  for (i = 0; i < order; i++)
    for (j = 0; j < order; j++)
      squarefixed[i][j] = (holepattern[i][j] == -1);

  free (pSwitch1);
  free (pSwitch2);
}





/*  Generate a hole pattern for a selected region */
/*  the hole pattern will be applied to the region with region column gy and region row gx */
/*  It finally updates the array square_fixed to reflect the generated hole pattern
    of the selected region  */
/*   regionsize is the x or y size of the region 
     We assume square regions for this method, so  
     regionsize should be  sqrt(order) = bsi_x = bsi_y  */

/*  numholes is the number of holes to put in the region we are
   going to puch holes
   The number should be a multiple of regionsize, to have a perfectly
    balanced hole pattern inside every region
    if not, try to have some regions with only one more hole than the others  */

/*  suffles should be, at least, equal to regionsize^3 */

void
generate_fullybalanced_holes (int gx, int gy, int regionsize, int numhole,
			      int shuffles)
{
  int holepattern[100][100];
  int targetholes[100] = { 0 };
  int targetspecrow[100] = { 0 };
  int i, j, num, numbers, tnumber, row;
  int ip, jp, s, s2, sh;
  stSwitch *pSwitch1, *pSwitch2;
  stSwitch ssw, nsw, ssw2;
  int numswitches;
  int turn, newnum;
  int rest, specialcell;

  for (i = 0; i < regionsize; i++)
    {
      for (j = 0; j < regionsize; j++)
	{
	  holepattern[i][j] = ((i + j) % regionsize);
	}
    }


  numbers = numhole / regionsize;
  for (num = 0; num < numbers;)
    {
      tnumber = random () % regionsize;
      if (!targetholes[tnumber])
	{
	  targetholes[tnumber] = regionsize;
	  num++;
	}
    }
  rest = numhole % regionsize;
  if (rest)
    {
      tnumber = random () % regionsize;
      while (targetholes[tnumber])
	tnumber = random () % regionsize;
      targetholes[tnumber] = rest;
      specialcell = tnumber;
    }
  else
    specialcell = -1;

  for (i = 0; i < regionsize; i++)
    {
      for (j = 0; j < regionsize; j++)
	{
	  if (holepattern[i][j] != specialcell)
	    {
	      if (!targetholes[holepattern[i][j]])
		holepattern[i][j] = -1;
	      else
		targetholes[holepattern[i][j]]--;
	    }
	}
    }

  if (rest)
    {
      for (row = 0; row < (regionsize - rest); row++)
	{
	  i = random () % regionsize;
	  while (targetspecrow[i])
	    i = random () % regionsize;
	  targetspecrow[i] = 1;
	  j =
	    ((specialcell - i) >
	     -1 ? (specialcell - i) : (regionsize + specialcell - i));
	  if (holepattern[i][j] == specialcell)
	    holepattern[i][j] = -1;
	  else
	    {
	      fprintf (stderr, " Error in region balanced !!\n");
	      exit (-1);
	    }
	}
    }


  if (numhole > 1)
    {
      /*  Now, we have to shuffle the hole pattern  */
      /*   Set up the initial list of valid switches */
      pSwitch1 =
	(stSwitch *) malloc (((numhole * (numhole - 1)) / 2) *
			     sizeof (stSwitch));
      pSwitch2 =
	(stSwitch *) malloc (((numhole * (numhole - 1)) / 2) *
			     sizeof (stSwitch));

      numswitches = 0;

      for (i = 0; i < regionsize; i++)
	{
	  for (j = 0; j < regionsize; j++)
	    {
	      for (ip = i + 1; ip < regionsize; ip++)
		{
		  for (jp = 0; jp < regionsize; jp++)
		    {
		      if (VALIDSWITCH (i, j, ip, jp))
			{
			  pSwitch1[numswitches].i = i;
			  pSwitch1[numswitches].j = j;
			  pSwitch1[numswitches].ip = ip;
			  pSwitch1[numswitches++].jp = jp;
			}
		    }
		}
	    }
	}

      /*  printf( "%d\n", numswitches ); */

      /*  Perform "shuffles" switches in the hole pattern  */

      turn = 0;
      for (sh = 0; sh < shuffles; sh++)
	{
	  /*   printf( " nsw: %i\n", numswitches );  */

	  s = (random () % numswitches);
	  if (!turn)
	    ssw = pSwitch1[s];
	  else
	    ssw = pSwitch2[s];

	  /*   printf( "ssw: (%i,%i) (%i,%i)\n", ssw.i, ssw.j, ssw.ip, ssw.jp ); */

	  newnum = 0;
	  for (s2 = 0; s2 < numswitches; s2++)
	    {
	      if (!turn)
		{
		  ssw2 = pSwitch1[s2];
		}
	      else
		ssw2 = pSwitch2[s2];

	      if (!(((ssw2.i == ssw.i) && (ssw2.j == ssw.j)) ||
		    ((ssw2.ip == ssw.i) && (ssw2.jp == ssw.j)) ||
		    ((ssw2.i == ssw.ip) && (ssw2.j == ssw.jp)) ||
		    ((ssw2.ip == ssw.ip) && (ssw2.jp == ssw.jp))) &&
		  !(((ssw2.i == ssw.i) && (ssw2.jp == ssw.jp)) ||
		    ((ssw2.ip == ssw.i) && (ssw2.j == ssw.jp)) ||
		    ((ssw2.i == ssw.ip) && (ssw2.jp == ssw.j)) ||
		    ((ssw2.ip == ssw.ip) && (ssw2.j == ssw.j))))
		{
		  if (!turn)
		    pSwitch2[newnum++] = ssw2;
		  else
		    pSwitch1[newnum++] = ssw2;
		}
	    }

	  /*  Perform switch  */
	  holepattern[ssw.i][ssw.jp] = holepattern[ssw.i][ssw.j];
	  holepattern[ssw.ip][ssw.j] = holepattern[ssw.ip][ssw.jp];
	  holepattern[ssw.i][ssw.j] = -1;
	  holepattern[ssw.ip][ssw.jp] = -1;

	  /*   Add new switches with (i,jp) to the switch list  */

	  for (i = 0; i < regionsize; i++)
	    for (j = 0; j < regionsize; j++)
	      {
		if (VALIDSWITCH (ssw.i, ssw.jp, i, j))
		  {
		    nsw.i = ssw.i;
		    nsw.j = ssw.jp;
		    nsw.ip = i;
		    nsw.jp = j;

		    if (!turn)
		      pSwitch2[newnum++] = nsw;
		    else
		      pSwitch1[newnum++] = nsw;
		  }
	      }

	  /*   Add new switches with (ip,j) to the switch list  */
	  for (i = 0; i < regionsize; i++)
	    for (j = 0; j < regionsize; j++)
	      {
		if (((i != ssw.i) || (j != ssw.jp))
		    && VALIDSWITCH (ssw.ip, ssw.j, i, j))
		  {
		    nsw.i = ssw.ip;
		    nsw.j = ssw.j;
		    nsw.ip = i;
		    nsw.jp = j;

		    if (!turn)
		      pSwitch2[newnum++] = nsw;
		    else
		      pSwitch1[newnum++] = nsw;
		  }
	      }

	  turn = !turn;
	  numswitches = newnum;
	}



      free (pSwitch1);
      free (pSwitch2);
    }
  /*  Apply the resulting  hole pattern */
  /*  Now, holepattern contains the new hole pattern
     for the region, being holepattern[i][j] == -1 if
     the subcell  i,j of the region does not have a hole,
     and != -1 if it has hole  */

  for (i = 0; i < regionsize; i++)
    for (j = 0; j < regionsize; j++)
      squarefixed[(bsi_x * gx) + i][(bsi_y * gy) + j] =
	(holepattern[i][j] == -1);
}


void
generate_fullybalanced_holes_region (int numhole)
{

  int nRegion, nRegionX, nRegionY;
  int holesRegionI, holesRegionR;
  int j, i;
  int orderRegion, nh, numbers, rest, flag;
  int regionpattern[100][100];

  nRegionX = (order / bsi_x);
  nRegionY = (order / bsi_y);
  nRegion = nRegionX * nRegionY;
  holesRegionI = numhole / nRegion;
  holesRegionR = numhole % nRegion;

  if (holesRegionI + 1 > bsi_x * bsi_y)
    error ("Number of holes per region exceeds region size");


//*****
//*****

  orderRegion = sqrt (order);
  nh = holesRegionR / orderRegion;

  for (i = 0; i < orderRegion; i++)
    {
      for (j = 0; j < orderRegion; j++)
	{
	  regionpattern[i][j] = ((i + j) % orderRegion);
	}
    }

  numbers = holesRegionR / orderRegion;
  rest = holesRegionR % orderRegion;

  for (i = 0; i < orderRegion; i++)
    {
      for (j = 0; j < orderRegion; j++)
	{
	  if (regionpattern[i][j] < numbers)
	    regionpattern[i][j] = -1;
	  if ((regionpattern[i][j] == numbers) && rest > 0)
	    {
	      regionpattern[i][j] = -1;
	      rest--;
	    }
	}
    }


//****
//****

  for (i = 0; i < nRegionX; i++)
    {
      for (j = 0; j < nRegionY; j++)
	{
	  flag = (regionpattern[i][j] == -1) ? 1 : 0;

	  generate_fullybalanced_holes (i, j, bsi_x,
					holesRegionI + flag,
					bsi_x * bsi_x * bsi_x);
	}
    }


}
