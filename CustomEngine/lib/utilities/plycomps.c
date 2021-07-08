/*

Determine the connected components of a PLY object, and maybe write out
only selected components of the object.

Greg Turk, August 1994

---------------------------------------------------------------

Copyright (c) 1994 The Board of Trustees of The Leland Stanford
Junior University.  All rights reserved.   
  
Permission to use, copy, modify and distribute this software and its   
documentation for any purpose is hereby granted without fee, provided   
that the above copyright notice and this permission notice appear in   
all copies of this software and that you do not sell the software.   
  
THE SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,   
EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY   
WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.   

*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ply.h>


/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  float x,y,z;                  /* coordinate of vertex */
  int index;                    /* index of vertex in vertex list */
  int nfaces;                   /* number of faces in face list */
  int max_faces;                /* maximum number of faces in list */
  struct Face **faces;          /* face list */
  struct Comp *comp;            /* pointer to component description */
  struct Vertex *next;          /* pointer for vertex queue */
  void *other_props;            /* other properties */
} Vertex;

typedef struct Face {
  unsigned char nverts;         /* number of vertex indices in list */
  Vertex **verts;               /* vertex index list */
  struct Comp *comp;            /* pointer to component description */
  void *other_props;            /* other properties */
} Face;


PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
};

PlyProperty face_props[] = { /* list of property information for a vertex */
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
};

char *known_elements[] = {"vertex", "face"};
int   num_known_elements = 2;

/*** the PLY object ***/

int nverts,nfaces;
Vertex **vlist;
Face **flist;
PlyOtherElems *other_elements = NULL;
PlyOtherProp *vert_other,*face_other;
int nelems;
char **elist;
int num_comments;
char **comments;
int num_obj_info;
char **obj_info;
int file_type;

#define UNTOUCHED -1
#define ON_QUEUE  -2


/* queue used to assign components */
static Vertex *queue_start = NULL;
static Vertex *queue_end = NULL;

/* the connected components */

typedef struct Comp {
  int comp_num;         /* component number stored at vertices */
  int vcount;           /* how many vertices in this component */
  int fcount;           /* how many faces in this component */
} Comp;

static int num_comps;           /* number of components */
static Comp **comp_list;        /* list of components */
static int comp_max = 100;      /* maximum number in list */

static int not_silent = 1;      /* print out info about components? */
static int top_n = 10;          /* don't print out more components than this */


/******************************************************************************
Transform a PLY file.
******************************************************************************/

main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;
  int less = -1;
  int more = -1;
  int all  = -1;
  char *filename;

  progname = argv[0];

  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
        case 't':
          top_n = atoi (*++argv);
          argc -= 1;
          break;
        case 's':
          not_silent = 1 - not_silent;
          break;
        case 'm':
          more = atoi (*++argv);
          filename = strdup (*++argv);
          argc -= 2;
          break;
        case 'l':
          less = atoi (*++argv);
          filename = strdup (*++argv);
          argc -= 2;
          break;
        case 'a':
	  all = 1;
	  filename = strdup(*++argv);
	  argc -= 1;
	  break;
        default:
          usage (progname);
          exit (-1);
          break;
      }
  }

  read_file();
  index_verts();
  ints_to_ptrs();
  find_components();

  /* maybe write all components that have at least this number of vertices */
  if (more > -1)
    write_more (filename, more);

  /* maybe write all components that have at most this number of vertices */
  if (less > -1)
    write_less (filename, less);

  if (all > -1)
      write_all(filename);
}


/******************************************************************************
Print out usage information about this program.
******************************************************************************/

usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] <in.ply\n", progname);
  fprintf (stderr, "       -s (silent mode)\n");
  fprintf (stderr, "       -m num filename\n");
  fprintf (stderr, "          (writes out all components with >= num verts)\n");
  fprintf (stderr, "       -l num filename\n");
  fprintf (stderr, "          (writes out all components with <= num verts)\n");
  fprintf (stderr, "       -t num (max num of components printed)\n");
  fprintf (stderr, "          (default = 10)\n");
  fprintf (stderr, "       -a filename\n");
  fprintf (stderr, "          (writes out each component to a seperate file)\n");
  fprintf (stderr, "          (filename should contain \"%d\" where component\n");
  fprintf (stderr, "           number should be inserted)\n");
}


/******************************************************************************
Find the connected components of a collection of polygons.
******************************************************************************/

find_components()
{
  int i,j,k;
  Face *face;
  Vertex *vert,*vert2;
  int compare_components();

  /* create pointers from the vertices to the faces */

  for (i = 0; i < nfaces; i++) {

    face = flist[i];

    /* make pointers from the vertices to this face */

    for (j = 0; j < face->nverts; j++) {
      vert = face->verts[j];
      /* make sure there is enough room for the new face pointer */
      if (vert->nfaces >= vert->max_faces) {
        vert->max_faces += 3;
        vert->faces = (Face **) 
                      realloc (vert->faces, sizeof (Face *) * vert->max_faces);
      }
      /* add the face to this vertice's list of faces */
      vert->faces[vert->nfaces] = face;
      vert->nfaces++;
    }
  }

  /* label all vertices as initially untouched */

  for (i = 0; i < nverts; i++)
    vlist[i]->comp = (Comp *) UNTOUCHED;

  /* initialize the component count list */
  comp_list = (Comp **) malloc (sizeof (Comp *) * comp_max);

  /* examine each vertex to see what component it belongs to */

  num_comps = 0;

  for (i = 0; i < nverts; i++) {

    vert = vlist[i];

    /* don't touch it if we've already assigned it a component number */
    if (vert->comp != (Comp *) UNTOUCHED)
      continue;

    /* initialize the info for this component */
    comp_list[num_comps] = (Comp *) malloc (sizeof (Comp));
    comp_list[num_comps]->comp_num = num_comps;
    comp_list[num_comps]->vcount = 0;
    comp_list[num_comps]->fcount = 0;

    /* place this vertex on the queue */
    on_queue (vert);

    /* process the queue until it is empty */
    while (queue_start)
      process_queue (num_comps);

    /* if we get here we've got a new component */
    num_comps++;
    if (num_comps >= comp_max) {
      comp_max *= 2;
      comp_list = (Comp **) realloc (comp_list, sizeof (Comp *) * comp_max);
    }
  }

  /* count the faces in each component */

  for (i = 0; i < nfaces; i++) {
    flist[i]->comp = flist[i]->verts[0]->comp;
    flist[i]->comp->fcount++;
  }

  /* sort the list of components by number of vertices */

  qsort (comp_list, num_comps, sizeof (Comp *), compare_components);

  /* print out info about components */

  if (not_silent) {
    fprintf (stderr, "\n");

    for (i = 0; i < num_comps; i++) {
      /*
      fprintf (stderr, "comp %d : %d verts, %d faces\n",
              comp_list[i]->comp_num,
              comp_list[i]->vcount,
              comp_list[i]->fcount);
      */
      fprintf (stderr, "%d verts, %d faces\n",
               comp_list[i]->vcount, comp_list[i]->fcount);
      if (i+1 >= top_n)
        break;
    }
    if (num_comps > top_n)
      fprintf (stderr, "...\n");

    fprintf (stderr, "(%d components)\n", num_comps);
    fprintf (stderr, "\n");
  }
}


/******************************************************************************
Compare two component entries for quicksort.
******************************************************************************/

compare_components(Comp **c1, Comp **c2)
{
  if ((*c1)->vcount < (*c2)->vcount)
    return (1);
  else if ((*c1)->vcount > (*c2)->vcount)
    return (-1);
  else
    return (0);
}


/******************************************************************************
Process one vertex on the queue.
******************************************************************************/

process_queue(int num_comps)
{
  int i,j;
  Vertex *vert,*vert2;
  Face *face;

  /* pop one vertex off the queue */

  vert = queue_start;
  queue_start = vert->next;

  /* store the vertex's component number */
  vert->comp = comp_list[num_comps];

  /* count this new component */
  comp_list[num_comps]->vcount++;

  /* place this vertex's neighbors on the queue */

  for (i = 0; i < vert->nfaces; i++) {
    face = vert->faces[i];
    for (j = 0; j < face->nverts; j++) {
      vert2 = face->verts[j];
      if (vert2->comp == (Comp *) UNTOUCHED)
        on_queue (vert2);
    }
  }
}


/******************************************************************************
Place a vertex on the queue.
******************************************************************************/

on_queue(Vertex *vert)
{
  vert->comp = (Comp *) ON_QUEUE;
  vert->next = NULL;

  if (queue_start == NULL) {
    queue_start = vert;
    queue_end = vert;
  }
  else {
    queue_end->next = vert;
    queue_end = vert;
  }
}


/******************************************************************************
Index the vertices.
******************************************************************************/

index_verts()
{
  int i;
  Vertex *vert;

  for (i = 0; i < nverts; i++) {
    vert = vlist[i];
    vert->index = i;
    vert->nfaces = 0;
    vert->max_faces = 3;
    vert->faces = (Face **) malloc (sizeof (Face *) * vert->max_faces);
  }
}


/******************************************************************************
Change the vertex indices from integers to pointers.
******************************************************************************/

ints_to_ptrs()
{
  int i,j;
  Vertex **verts;

  for (i = 0; i < nfaces; i++) {
    verts = flist[i]->verts;
    for (j = 0; j < flist[i]->nverts; j++)
      verts[j] = vlist[(int) verts[j]];
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

      ply_get_property (ply, elem_name, &vert_props[0]);
      ply_get_property (ply, elem_name, &vert_props[1]);
      ply_get_property (ply, elem_name, &vert_props[2]);
      vert_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Vertex,other_props));

      /* grab all the vertex elements */
      for (j = 0; j < num_elems; j++) {
        vlist[j] = (Vertex *) malloc (sizeof (Vertex));
        ply_get_element (ply, (void *) vlist[j]);
      }
    }
    else if (equal_strings ("face", elem_name)) {

      /* create a list to hold all the face elements */
      flist = (Face **) malloc (sizeof (Face *) * num_elems);
      nfaces = num_elems;

      /* set up for getting face elements */

      ply_get_property (ply, elem_name, &face_props[0]);
      face_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Face,other_props));

      /* grab all the face elements */
      for (j = 0; j < num_elems; j++) {
        flist[j] = (Face *) malloc (sizeof (Face));
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
Write out those components that contain at least a given number of vertices.

Entry:
  filename - name of file to write to
  more     - minimum number of vertices to have component be written
******************************************************************************/

write_more(char *filename, int more)
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;
  int vsum,fsum;
  float version;
  Vertex **verts;
  int vcount;

  ply = ply_open_for_writing(filename, num_known_elements, known_elements,
			     file_type, &version);

  /* count vertices and faces that will be written */
  vsum = 0;
  fsum = 0;
  for (i = 0; i < num_comps; i++) {
    if (comp_list[i]->vcount >= more) {
      vsum += comp_list[i]->vcount;
      fsum += comp_list[i]->fcount;
    }
  }

  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", vsum);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  ply_element_count (ply, "face", fsum);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));

  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  /* change the vertex indices from pointers to indices */

  vcount = 0;
  for (i = 0; i < nverts; i++)
    if (vlist[i]->comp->vcount >= more)
      vlist[i]->index = vcount++;

  for (i = 0; i < nfaces; i++) {
    verts = flist[i]->verts;
    for (j = 0; j < flist[i]->nverts; j++)
      verts[j] = (Vertex *) verts[j]->index;
  }

  /* set up and write the vertex elements */
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < nverts; i++)
    if (vlist[i]->comp->vcount >= more)
      ply_put_element (ply, (void *) vlist[i]);

  /* set up and write the face elements */
  ply_put_element_setup (ply, "face");
  for (i = 0; i < nfaces; i++)
    if (flist[i]->comp->vcount >= more)
      ply_put_element (ply, (void *) flist[i]);

  ply_put_other_elements (ply);

  /* close the PLY file */
  ply_close (ply);
}


/******************************************************************************
Write out those components that contain at most a given number of vertices.

Entry:
  filename - name of file to write to
  less     - maximum number of vertices to have component be written
******************************************************************************/

write_less(char *filename, int less)
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;
  int vsum,fsum;
  float version;
  Vertex **verts;
  int vcount;

  ply = ply_open_for_writing(filename, num_known_elements, known_elements,
			     file_type, &version);

  /* count vertices and faces that will be written */
  vsum = 0;
  fsum = 0;
  for (i = 0; i < num_comps; i++) {
    if (comp_list[i]->vcount <= less) {
      vsum += comp_list[i]->vcount;
      fsum += comp_list[i]->fcount;
    }
  }

  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", vsum);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  ply_element_count (ply, "face", fsum);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));

  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  /* change the vertex indices from pointers to indices */

  vcount = 0;
  for (i = 0; i < nverts; i++)
    if (vlist[i]->comp->vcount <= less)
      vlist[i]->index = vcount++;

  for (i = 0; i < nfaces; i++) {
    verts = flist[i]->verts;
    for (j = 0; j < flist[i]->nverts; j++)
      verts[j] = (Vertex *) verts[j]->index;
  }

  /* set up and write the vertex elements */
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < nverts; i++)
    if (vlist[i]->comp->vcount <= less)
      ply_put_element (ply, (void *) vlist[i]);

  /* set up and write the face elements */
  ply_put_element_setup (ply, "face");
  for (i = 0; i < nfaces; i++)
    if (flist[i]->comp->vcount <= less)
      ply_put_element (ply, (void *) flist[i]);

  ply_put_other_elements (ply);

  /* close the PLY file */
  ply_close (ply);
}

write_all(char *filename)
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;
  float version;
  Vertex **verts;
  int vcount;
  char comp_filename[256];
  int  comp_num, vert_num;
  Comp *comp;
  
  for (comp_num=0; comp_num < num_comps; comp_num++)
      comp_list[comp_num]->vcount=0;

  for (vert_num=0; vert_num < nverts; vert_num++)
      vlist[vert_num]->index = vlist[vert_num]->comp->vcount++;
  
  for (i = 0; i < nfaces; i++) {
      verts = flist[i]->verts;
      for (j = 0; j < flist[i]->nverts; j++)
	  verts[j] = (Vertex *) verts[j]->index;
  }


  
  for (comp_num = 0; comp_num < num_comps; comp_num++)
  {
      sprintf(comp_filename, filename, comp_num);
      comp = comp_list[comp_num];
      
      ply = ply_open_for_writing(comp_filename, num_known_elements, known_elements,
				 file_type, &version);
      
      /* describe what properties go into the vertex and face elements */
      
      ply_element_count (ply, "vertex", comp->vcount);
      ply_describe_property (ply, "vertex", &vert_props[0]);
      ply_describe_property (ply, "vertex", &vert_props[1]);
      ply_describe_property (ply, "vertex", &vert_props[2]);
      ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));
      
      ply_element_count (ply, "face", comp->fcount);
      ply_describe_property (ply, "face", &face_props[0]);
      ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));
      
      ply_describe_other_elements (ply, other_elements);
      
      for (i = 0; i < num_comments; i++)
	  ply_put_comment (ply, comments[i]);
      
      for (i = 0; i < num_obj_info; i++)
	  ply_put_obj_info (ply, obj_info[i]);
      
      ply_header_complete (ply);
      
      /* set up and write the vertex elements */
      ply_put_element_setup (ply, "vertex");
      for (i = 0; i < nverts; i++)
      if (vlist[i]->comp == comp)
	ply_put_element (ply, (void *) vlist[i]);
      
      /* set up and write the face elements */
      ply_put_element_setup (ply, "face");
      for (i = 0; i < nfaces; i++)
      if (flist[i]->comp == comp)
	ply_put_element (ply, (void *) flist[i]);
      
      ply_put_other_elements (ply);
      
      /* close the PLY file */
      ply_close (ply);
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
  char **obj_info;

  /*** Write out the transformed PLY object ***/


  ply = ply_write(stdout, num_known_elements, known_elements, file_type);


  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  ply_element_count (ply, "face", nfaces);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));

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