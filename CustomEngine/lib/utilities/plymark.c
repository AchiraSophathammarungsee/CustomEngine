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

Set a "marked" field on certain vertices or faces.  These can be interpreted
by other programs, like ply2pphigs, to highlight features or for other
purposes.
   
Jon Cohen
*/

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <ply.h>

#define UNKNOWN        -1
#define FALSE           0
#define TRUE            1


/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  unsigned char marked;
  void *other_props;       /* other properties */
} Vertex;

typedef struct Face {
  unsigned char marked;
  void *other_props;       /* other properties */
} Face;

char *elem_names[] = { /* list of the kinds of elements in the user's object */
  "vertex", "face"
};

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"marked", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,marked), 0, 0, 0, 0},
};

PlyProperty face_props[] = { /* list of property information for a vertex */
  {"marked", PLY_UCHAR, PLY_UCHAR, offsetof(Face,marked), 0, 0, 0, 0},
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

int has_vmarked, has_fmarked;
int write_vmarked, write_fmarked;
char **marked_verts, **marked_faces;
int num_marked_verts, num_marked_faces;

/******************************************************************************
Main program.
******************************************************************************/

main(int argc, char *argv[])
{
    init_defaults();
    get_options(argc, argv);    
    read_file();
    map_defaults();
    mark();
    write_file();
}

init_defaults()
{
    write_vmarked = UNKNOWN;
    write_fmarked = UNKNOWN;
    num_marked_verts = num_marked_faces = 0;
}

map_defaults()
{
    if ((write_vmarked == UNKNOWN) && (has_vmarked))
	write_vmarked = TRUE;
    if ((write_fmarked == UNKNOWN) && (has_fmarked))
	write_fmarked = TRUE;
}

get_options(int argc, char *argv[])
{
    char *s;
    char *progname;
    int   dummy;
    
    progname = argv[0];

    while (--argc > 0 && (*++argv)[0]=='-')
    {
	for (s = argv[0]+1; *s; s++)
	    switch (*s)
	    {
	    case 'v':
		++argv;
		marked_verts = argv;
		for (num_marked_verts = 0;
		     ((*argv) && (sscanf((*argv), "%d\n", &dummy) == 1));
		     num_marked_verts++)
		    ++argv;
		write_vmarked = TRUE;
		--argv;
		argc -= num_marked_verts;
		break;
	    case 'f':
		++argv;
		marked_faces = argv;
		for (num_marked_faces = 0;
		     ((*argv) && (sscanf((*argv), "%d\n", &dummy) == 1));
		     num_marked_faces++)
		    ++argv;
		write_fmarked = TRUE;
		--argv;
		argc -= num_marked_faces;
		break;
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
  fprintf(stderr, "usage: %s [flags]  <in.ply   >out.ply\n", progname);
  fprintf(stderr, "  -- optional flags -- \n");
  fprintf(stderr, "    -v <id> <id> ...    : vertices to mark\n");
  fprintf(stderr, "    -f <id> <id> ...    : faces to mark\n");
}

mark()
{
    int i, id;

    if ((write_vmarked) && (!has_vmarked))
	for (i=0; i<nverts; i++)
	    vlist[i]->marked = FALSE;
    if ((write_fmarked) && (!has_fmarked))
	for (i=0; i<nfaces; i++)
	    flist[i]->marked = FALSE;
    
    for (i=0; i<num_marked_verts; i++)
    {
	if ((sscanf(marked_verts[i], "%d", &id) != 1) ||
	    (id < 0) || (id >= nverts))
	{
	    fprintf(stderr, "Invalid vertex id\n");
	    exit(-1);
	}
	vlist[id]->marked = TRUE;
    }

    for (i=0; i<num_marked_faces; i++)
    {
	if ((sscanf(marked_faces[i], "%d", &id) != 1) ||
	    (id < 0) || (id >= nfaces))
	{
	    fprintf(stderr, "Invalid face id\n");
	    exit(-1);
	}
	flist[id]->marked = TRUE;
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
      vlist = (Vertex **) calloc (sizeof (Vertex *), num_elems);
      nverts = num_elems;

      /* set up for getting vertex elements */
      /* verify which properties these vertices have */
      has_vmarked = FALSE;
      
      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("marked", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[0]);  /* marked */
	      has_vmarked = TRUE;
	  }
      }
      vert_other = ply_get_other_properties (ply, elem_name,
					     offsetof(Vertex,other_props));

      /* grab all the vertex elements */
      for (j = 0; j < num_elems; j++) {
        vlist[j] = (Vertex *) calloc (sizeof (Vertex), 1);
        ply_get_element (ply, (void *) vlist[j]);
      }
    }
    else if (equal_strings ("face", elem_name)) {

      /* create a list to hold all the face elements */
      flist = (Face **) calloc (sizeof (Face *), num_elems);
      nfaces = num_elems;

      /* set up for getting face elements */
      /* verify which properties these vertices have */
      has_fmarked = FALSE;

      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("marked", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &face_props[0]);  /* marked */
	      has_fmarked = TRUE;
	  }
      }
      face_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Face,other_props));

      /* grab all the face elements */
      for (j = 0; j < num_elems; j++) {
        flist[j] = (Face *) calloc (sizeof (Face), 1);
        ply_get_element (ply, (void *) flist[j]);
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

  /*** Write out the final PLY object ***/


  ply = ply_write (stdout, 2, elem_names, file_type);


  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", nverts);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));
  if (write_vmarked == TRUE)
      ply_describe_property (ply, "vertex", &vert_props[0]);

  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));
  ply_element_count (ply, "face", nfaces);
  if (write_fmarked == TRUE)
      ply_describe_property (ply, "face", &face_props[0]);

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
  
  /* set up and write the face elements */
  ply_put_element_setup (ply, "face");
  for (i = 0; i < nfaces; i++)
      ply_put_element (ply, (void *) flist[i]);
  
  ply_put_other_elements (ply);
  
  /* close the PLY file */
  ply_close (ply);
}

