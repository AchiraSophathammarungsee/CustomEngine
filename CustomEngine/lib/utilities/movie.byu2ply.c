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

Convert a MOVIE.BYU file to a PLY file

Jonathan Cohen, September 1995

*/

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <ply.h>


/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  float x,y,z;
  void *other_props;       /* other properties */
} Vertex;

typedef struct Face {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
  void *other_props;       /* other properties */
} Face;

char *elem_names[] = { /* list of the kinds of elements in the user's object */
  "vertex", "face"
};

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
};

PlyProperty face_props[] = { /* list of property information for a vertex */
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
};


/*** the PLY object ***/

static int nverts,nfaces;
static Vertex **vlist;
static Face **flist;
static PlyOtherElems *other_elements = NULL;
static PlyOtherProp *vert_other,*face_other;
static int nelems;
static char **elist;
static int num_comments;
static char **comments;
static int num_obj_info;
static char **obj_info;
static int file_type;

/******************************************************************************
Main program.
******************************************************************************/

main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;

  progname = argv[0];

  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
        default:
          usage (progname);
          exit (-1);
          break;
      }
  }

  read_file();
  write_file();
}


/******************************************************************************
Print out usage information.
******************************************************************************/

usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] <in.movie.byu >out.ply\n", progname);
}


/******************************************************************************
Read in the MOVIE.BYU file from standard in.
******************************************************************************/

read_file()
{
  int   i, j;
  int   num_parts, total_edges, first_face, last_face;
  int   indices[100];

  if (scanf("%d", &num_parts) != 1)
  {
      fprintf(stderr, "Couldn't read number of parts\n");
      exit(1);
  }
  
  if (num_parts != 1)
  {
      fprintf(stderr, "MOVIE.BYU file has multiple parts - not supported\n");
      exit(1);
  }

  if (scanf("%d", &nverts) != 1)
  {
      fprintf(stderr, "Couldn't read number of vertices\n");
      exit(1);
  }

  if (nverts <= 0)
  {
      fprintf(stderr, "Invalid number of vertices\n");
      exit(1);
  }
  
  if (scanf("%d", &nfaces) != 1)
  {
      fprintf(stderr, "Couldn't read number of faces\n");
      exit(1);
  }
  if (nfaces <= 0)
  {
      fprintf(stderr, "Invalid number of faces\n");
      exit(1);
  }

  if (scanf("%d", &total_edges) != 1)
  {
      fprintf(stderr, "Couldn't read number of edges\n");
      exit(1);
  }
  if (total_edges <= 0)
  {
      fprintf(stderr, "Invalid number of edges\n");
      exit(1);
  }

  if (scanf("%d", &first_face) != 1)
  {
      fprintf(stderr, "Couldn't read first face\n");
      exit(1);
  }
  if (first_face != 1)
  {
      fprintf(stderr, "part doesn't start with face 1\n");
      exit(1);
  }

  if (scanf("%d", &last_face) != 1)
  {
      fprintf(stderr, "Couldn't read last face\n");
      exit(1);
  }
  if (last_face != nfaces)
  {
      fprintf(stderr, "part doesn't finish with last face\n");
      exit(1);
  }
  
      
        
  ALLOCN(vlist, Vertex *, nverts);
  for (i=0;i<nverts;i++)
  {
      ALLOCN(vlist[i], Vertex, 1);
      scanf("%f %f %f", &(vlist[i]->x), &(vlist[i]->y), &(vlist[i]->z));
  }
  
  ALLOCN(flist, Face *, nfaces);
  
  for (i=0;i< nfaces; i++)
  {
      ALLOCN(flist[i], Face, 1);
      for (j=0; j<100; j++)
      {
	  if (scanf("%d", &(indices[j])) != 1)
	  {
	      fprintf(stderr, "couldn't read vertex index\n");
	      exit(1);
	  }
	  if (indices[j] < 0)
	  {
	      indices[j] = -indices[j];
	      j++;
	      break;
	  }
      }
      if (j >= 100)
      {
	  fprintf(stderr, "too many vertex indices on face\n");
	  exit(1);
      }
      flist[i]->nverts = (unsigned char)j;
      ALLOCN(flist[i]->verts, int, flist[i]->nverts);
      for (j=0; j<flist[i]->nverts; j++)
	  flist[i]->verts[j] = indices[j] - 1;
  }
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

  /*** Write out the final PLY object ***/

  nelems = 2;
  file_type = PLY_BINARY_NATIVE;
  
  ply = ply_write (stdout, nelems, elem_names, file_type);


  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
#if 0
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));
#endif
  
  ply_element_count (ply, "face", nfaces);
  ply_describe_property (ply, "face", &face_props[0]);
#if 0
  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));

  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);
#endif
  
  ply_header_complete (ply);

  /* set up and write the vertex elements */
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < nverts; i++)
    ply_put_element (ply, (void *) vlist[i]);

  /* set up and write the face elements */
  ply_put_element_setup (ply, "face");
  for (i = 0; i < nfaces; i++)
    ply_put_element (ply, (void *) flist[i]);

  ply_put_other_elements (ply);

  /* close the PLY file */
  ply_close (ply);
}

