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
/*

   Program to shade according to curvature.

   Jon Cohen - July 1995
   
*/

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <ply.h>

#define FALSE           0
#define TRUE            1

#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#define FP_EQ_EPS( a, b, c )  ((((a) - (b)) < (c)) && (((a) - (b)) > -(c)))

/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  float curvature;
  unsigned char red, green, blue;
  void *other_props;       /* other properties */
} Vertex;

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"curvature", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,curvature), 0, 0, 0, 0},
  {"red", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,red), 0, 0, 0, 0},
  {"green", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,green), 0, 0, 0, 0},
  {"blue", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,blue), 0, 0, 0, 0},
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

int has_z, has_red, has_green, has_blue;


/******************************************************************************
Main program.
******************************************************************************/

main(int argc, char *argv[])
{
    get_options(argc, argv);    
    read_file();
    curveshade();
    write_file();
}

get_options(int argc, char *argv[])
{
    char *s;
    char *progname;
    
    progname = argv[0];

    while (--argc > 0 && (*++argv)[0]=='-')
    {
	for (s = argv[0]+1; *s; s++)
	    switch (*s)
	    {
	    default:
		usage (progname);
		exit (-1);
		break;
	    }
    }
}

/******************************************************************************
Print out usage information.
******************************************************************************/

usage(char *progname)
{
  fprintf(stderr, "usage: %s [flags]  <in.ply   >out.ascarch\n", progname);
}


curveshade()
{
    int i;

    Vertex *vert;
    float   min, max, delta, intensity;
    float   min_intensity = 25.0;
    float   max_intensity = 255.0;
    float   scale_delta;
    int     int_intensity;
    
    /* set all vertex color intensities based on curvature */
    for (i=0; i<nverts; i++)
    {
	vert = vlist[i];

	vert->red = 255*(1.0 - vert->curvature);
	vert->green = 0.0;
	vert->blue = 255*vert->curvature;
    }
}



/******************************************************************************
Read in the PLY file from standard in.
******************************************************************************/

read_file()
{
  int i,j,k;
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
      vlist = (Vertex **) malloc (sizeof (Vertex *) * num_elems);
      nverts = num_elems;

      /* set up for getting vertex elements */
      /* verify which properties these vertices have */
      has_z = has_red = has_green = has_blue = FALSE;
      
      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("z", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[0]);  /* z */
	      has_z = TRUE;
	  }
	  else if (equal_strings("red", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[1]);  /* red */
	      has_red = TRUE;
	  }
	  else if (equal_strings("green", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[2]);  /* green */
	      has_green = TRUE;
	  }
	  else if (equal_strings("blue", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[3]);  /* blue */
	      has_blue = TRUE;
	  }
      }
      vert_other = ply_get_other_properties (ply, elem_name,
					     offsetof(Vertex,other_props));

      /* test for necessary properties */
      if (!has_z)
      {
	  fprintf(stderr, "Vertices don't have z\n");
	  exit(-1);
      }
      if ((has_red) || (has_green) || (has_blue))
      {
	  fprintf(stderr, "Vertices already have colors\n");
	  exit(-1);
      }
      
      
      /* grab all the vertex elements */
      for (j = 0; j < num_elems; j++) {
        vlist[j] = (Vertex *) malloc (sizeof (Vertex));
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

write_file()
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;
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

