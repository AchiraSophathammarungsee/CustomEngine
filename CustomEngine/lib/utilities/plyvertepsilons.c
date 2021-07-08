/****************************************************************************\

  Copyright 1995 The University of North Carolina at Chapel Hill.
  All Rights Reserved.

  Permission to use, copy, modify and distribute this software and its
  documentation for educational, research and non-profit purposes,
  without fee, and without a written agreement is hereby granted,
  provided that the above copyright notice and the following three
  paragraphs appear in all copies.

  IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL BE
  LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
  CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE
  USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY
  OF NORTH CAROLINA HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
  DAMAGES.


  Permission to use, copy, modify and distribute this software and its
  documentation for educational, research and non-profit purposes,
  without fee, and without a written agreement is hereby granted,
  provided that the above copyright notice and the following three
  paragraphs appear in all copies.

  THE UNIVERSITY OF NORTH CAROLINA SPECIFICALLY DISCLAIM ANY
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
  PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
  NORTH CAROLINA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
  UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

  The author may be contacted via:

  US Mail:             Jonathan Cohen
                       Department of Computer Science
                       Sitterson Hall, CB #3175
                       University of N. Carolina
                       Chapel Hill, NC 27599-3175

  Phone:               (919)962-1749

  EMail:               cohenj@cs.unc.edu

\****************************************************************************/
/*****************************************************************************\
  plyvertepsilons.c
  --
  Description : Assign per-vertex epsilon values according to some gradient

                Jonathan Cohen - January 1996
  ----------------------------------------------------------------------------
  $Source: /afs/cs.unc.edu/project/walk/playpen/mmr/src/ply/RCS/plyvertepsilons.c,v $
  $Revision: 1.1 $
  $Date: 1997/07/31 19:31:59 $
  $Author: hudson $
  $Locker: hudson $
\*****************************************************************************/


/*----------------------------- Local Includes -----------------------------*/

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <ply.h>
#include <values.h>

/*----------------------------- Local Constants -----------------------------*/

#define LO      0
#define HI       1

#define X         0
#define Y         1
#define Z         2

#define TRUE  1
#define FALSE 0
/*------------------------------ Local Macros -------------------------------*/

#define SQ_DIST3(a, b)          (((a)[0]-(b)[0])*((a)[0]-(b)[0]) +      \
                                 ((a)[1]-(b)[1])*((a)[1]-(b)[1]) +      \
                                 ((a)[2]-(b)[2])*((a)[2]-(b)[2]))

#define DOTPROD3(a, b)		 ((a)[0]*(b)[0] + (a)[1]*(b)[1] + (a)[2]*(b)[2])

#define MAX(x,y) ((x)>(y) ? (x) : (y))
#define MIN(x,y) ((x)<(y) ? (x) : (y))

/*------------------------------- Local Types -------------------------------*/

/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  double  coord[3];            /* coordinates of vertex */
  float epsilon;
  void *other_props;       /* other properties */
} Vertex;



/*------------------------ Local Function Prototypes ------------------------*/

void read_file();
void write_file();
void compute_bbox_diagonal();
void compute_vertex_epsilons();

/*------------------------------ Local Globals ------------------------------*/

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", PLY_FLOAT, PLY_DOUBLE, offsetof(Vertex,coord[X]), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_DOUBLE, offsetof(Vertex,coord[Y]), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_DOUBLE, offsetof(Vertex,coord[Z]), 0, 0, 0, 0},
  {"epsilon", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,epsilon), 0, 0, 0, 0},
};


/*** the PLY object ***/
static int nverts;
static Vertex **vlist;
static PlyOtherElems *other_elements = NULL;
static PlyOtherProp *vert_other;
static int nelems;
static char **elist;
static int num_comments;
static char **comments;
static int num_obj_info;
static char **obj_info;
static int file_type;

static int has_x, has_y, has_z;
static int has_epsilons;

static double bbox_diagonal;

static double gradient_direction[3] = {1.0, 0.0, 0.0};

static double min_epsilon = 1.0/64.0;
static double max_epsilon = 1.0;

/*---------------------------------Functions-------------------------------- */






/******************************************************************************
Main program.
******************************************************************************/

int main(int argc, char *argv[])
{
    read_file();
    compute_bbox_diagonal();
    compute_vertex_epsilons();    
    write_file();
    return 0;
}




/*****************************************************************************\
 @ compute_bbox_diagonal()
 -----------------------------------------------------------------------------
 description : 
 input       : 
 output      : 
 notes       :
\*****************************************************************************/
void compute_bbox_diagonal()
{
    int       i, j, k;
    Vertex   *vert;
    double  bbox[2][3];
    
    bbox[HI][X] = bbox[HI][Y] = bbox[HI][Z] = -MAXDOUBLE;
    bbox[LO][X] = bbox[LO][Y] = bbox[LO][Z] = MAXDOUBLE;
    for (i=0; i<nverts; i++)
    {
        vert = vlist[i];
        for (j=0; j<3; j++)
	    {
		bbox[HI][j] = MAX(bbox[HI][j], vert->coord[j]);
		bbox[LO][j] = MIN(bbox[LO][j], vert->coord[j]);
	    }
    }

    bbox_diagonal = sqrt(SQ_DIST3(bbox[HI], bbox[LO]));
    
    return;
} /** End of compute_bbox_diagonal() **/


void compute_vertex_epsilons()
{
/*
  x                                             64x
2^0  2^1  2^2  2^3  2^4  2^5  2^6
log2(64x/x) == 6
*/
    int i;
    double min_distance, max_distance, distance;
    double range, t, logscale, logscale_t;

    /* find minimum and maximum vertex with respect to the gradient direction */
    for (i=0, min_distance = MAXDOUBLE, max_distance = -MAXDOUBLE; 
            i<nverts; i++)
    {
        distance = DOTPROD3(vlist[i]->coord, gradient_direction);
        min_distance = MIN(distance, min_distance);
        max_distance = MAX(distance, max_distance);
    }


    range = max_distance - min_distance;
    logscale = log(max_epsilon/min_epsilon)/log(2.0);
    
    for (i=0; i<nverts; i++)
    {
        distance = DOTPROD3(vlist[i]->coord, gradient_direction);
        t = (distance - min_distance)/range;
        logscale_t = t*logscale;
        vlist[i]->epsilon = min_epsilon * pow(2.0, logscale_t) * bbox_diagonal * 0.01;
    }
}


/******************************************************************************
Read in the PLY file from standard in.
******************************************************************************/

void read_file()
{
  int i,j;
  PlyFile *ply;
  int nprops;
  int num_elems;
  PlyProperty **plist;
  char *elem_name;
  float version;
  
  /*** Read in the original PLY object ***/


  ply  = ply_read (stdin, &nelems, &elist);
  ply_get_info (ply, &version, &file_type);

  for (i = 0; i < nelems; i++) {

    /* get the description of the first element */
    elem_name = elist[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

    if (equal_strings ("vertex", elem_name)) {

      /* create a vertex list to hold all the vertices */
      vlist = (Vertex **) calloc (sizeof (Vertex *), num_elems);
      nverts = num_elems;

      /* set up for getting vertex elements */
      /* verify which properties these vertices have */
      has_x = has_y = has_z = has_epsilons = FALSE;
      
      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("x", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[0]);  /* x */
	      has_x = TRUE;
	  }
	  else if (equal_strings("y", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[1]);  /* y */
	      has_y = TRUE;
	  }
	  else if (equal_strings("z", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[2]);  /* z */
	      has_z = TRUE;
	  }
	  else if (equal_strings("epsilon", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[3]);
	      has_epsilons = TRUE;
	  }
      }
      vert_other = ply_get_other_properties (ply, elem_name,
					     offsetof(Vertex,other_props));

      /* test for necessary properties */
      if ((!has_x) || (!has_y) || (!has_z))
      {
	  fprintf(stderr, "Vertices must have x, y, and z coordinates\n");
	  exit(-1);
      }
      if (has_epsilons)
      {
              fprintf(stderr, "Vertices already have epsilons\n");
              exit(-1);
       }

      
      /* grab all the vertex elements */
      for (j = 0; j < num_elems; j++) {
        vlist[j] = (Vertex *) calloc (sizeof (Vertex), 1);
        ply_get_element (ply, (void *) vlist[j]);
      }
    }
    else
      other_elements = ply_get_other_element (ply, elem_name, num_elems);
  }

  comments = ply_get_comments (ply, &num_comments);
  obj_info = ply_get_obj_info (ply, &num_obj_info);

  ply_close (ply);
}


/******************************************************************************
Write out the PLY file to standard out.
******************************************************************************/

void write_file()
{
  int i,j;
  PlyFile *ply;
  static char *known_elements[] = {"vertex"};
  
  /*** Write out the final PLY object ***/


  ply = ply_write (stdout, 1, known_elements, file_type);


  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_property (ply, "vertex", &vert_props[3]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  
  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  /* set up and write the vertex elements */
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < nverts; i++)
      ply_put_element (ply, (void *) vlist[i]);
  
  ply_put_other_elements (ply);
  
  /* close the PLY file */
  ply_close (ply);
}

/*****************************************************************************\
  $Log: plyvertepsilons.c,v $
  Revision 1.1  1997/07/31 19:31:59  hudson
  Initial revision

  Revision 1.3  1997/04/10 21:24:50  cohenj
  Fixed syntax error

  Revision 1.2  1997/04/10 20:34:20  cohenj
  Modified call to ply_write to take correct arguments

  Revision 1.1  1996/04/08 17:12:03  cohenj
  Initial revision

 * Revision 1.3  1995/10/16  16:29:41  cohenj
 * Cleaned up.
 *
 * Revision 1.2  95/09/30  06:11:10  cohenj
 * almost done writing revision 1 -- just need the code to update
 * the model after a vertex is removed and finalize the model after removing
 * all vertices in the queue.
 * 
 * Revision 1.1  95/09/15  16:28:21  cohenj
 * Initial revision
 * 
 * Revision 1.1  1995/08/30  20:57:04  cohenj
 * Initial revision
 *
 * Revision 1.1  1995/08/30  20:57:04  cohenj
 * Initial revision
 *
\*****************************************************************************/
