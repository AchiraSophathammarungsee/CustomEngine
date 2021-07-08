/*

Simplify a polygon model by collapsing multiple points together.  This
is basically Jarek Rossignac's method of simplifying polygon models.

This code borrows heavily from "plyshared".

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
#include <strings.h>
#include <ply.h>


/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  float x,y,z;             /* SUM of coordinates that contributed */
  int count;               /* number of vertices that contributed */
  int a,b,c;               /* integer indices used in hash */
  int index;
  struct Vertex *shared;
  struct Vertex *next;
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

/* tolerance and tolerance modes */
#define ABSOLUTE_DISTANCE          0
#define PERCENT_BBOX_DIAG_DISTANCE 1

static float tolerance = 0.5; /* half a percent of the bbox diag length */
static int tolerance_mode = PERCENT_BBOX_DIAG_DISTANCE;

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

/* hash table for near neighbor searches */

#define PR1  17
#define PR2 101
#define TABLE_SIZE1  5003
#define TABLE_SIZE2 17003
#define TABLE_SIZE3 53003

typedef struct Hash_Table {	/* uniform spatial subdivision, with hash */
  int npoints;			/* number of points placed in table */
  Vertex **verts;		/* array of hash cells */
  int num_entries;		/* number of array elements in verts */
  float scale;			/* size of cell */
} Hash_Table;


Hash_Table *init_table(int, float);
void add_to_hash (Vertex *, Hash_Table *, float);



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
        case 'd':
          tolerance = atof (*++argv);
	  tolerance_mode = ABSOLUTE_DISTANCE;
          argc -= 1;
          break;
        case 'p':
          tolerance = atof (*++argv);
	  tolerance_mode = PERCENT_BBOX_DIAG_DISTANCE;
          argc -= 1;
          break;
        default:
          usage (progname);
          exit (-1);
          break;
      }
  }

  read_file();

  if (tolerance_mode == PERCENT_BBOX_DIAG_DISTANCE)
     compute_absolute_tolerance();
  
  crunch_vertices();

  write_file();
}


compute_absolute_tolerance()
{
    int     i;
    float   minx, miny, minz;
    float   maxx, maxy, maxz;
    float   diag_length;
    Vertex *vert;
    
    minx = maxx = vlist[0]->x;
    miny = maxy = vlist[0]->y;
    minz = maxz = vlist[0]->z;
    
    for (i=1; i<nverts; i++)
    {
	vert = vlist[i];
	minx = MIN(minx, vert->x);
	miny = MIN(miny, vert->y);
	minz = MIN(minz, vert->z);
	maxx = MAX(maxx, vert->x);
	maxy = MAX(maxy, vert->y);
	maxz = MAX(maxz, vert->z);
    }

    diag_length =
	sqrt( ((maxx-minx) * (maxx-minx)) +
	      ((maxy-miny) * (maxy-miny)) +
	      ((maxz-minz) * (maxz-minz)) );

    tolerance = diag_length * tolerance / 100.0;
    tolerance_mode = ABSOLUTE_DISTANCE;
}





/******************************************************************************
Print out usage information.
******************************************************************************/

usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] <in.ply >out.ply\n", progname);
  fprintf (stderr, "       -d distance\n", tolerance);
  fprintf (stderr, "       -p percent bbox diagonal length distance"
	   " (default %g%%)\n", tolerance);
}


/******************************************************************************
Figure out which vertices should be collapsed into one.
******************************************************************************/

crunch_vertices()
{
  int i,j,k;
  int jj;
  Hash_Table *table;
  float squared_dist;
  Vertex *vert;
  Face *face;
  float recip;

  table = init_table (nverts, tolerance);

  squared_dist = tolerance * tolerance;

  /* place all vertices in the hash table, and in the process */
  /* learn which ones should be collapsed */

  for (i = 0; i < nverts; i++) {
    vlist[i]->count = 1;
    add_to_hash (vlist[i], table, squared_dist);
  }

  /* compute average of all coordinates that contributed to */
  /* the vertices placed in the hash table */

  for (i = 0; i < nverts; i++) {
    vert = vlist[i];
    if (vert->shared == vert) {
      recip = 1.0 / vert->count;
      vert->x *= recip;
      vert->y *= recip;
      vert->z *= recip;
    }
  }

  /* fix up the faces to point to the collapsed vertices */

  for (i = 0; i < nfaces; i++) {

    face = flist[i];
    
    /* change all indices to pointers to the collapsed vertices */
    for (j = 0; j < face->nverts; j++)
      face->verts[j] = (int) (vlist[face->verts[j]]->shared);

    /* collapse adjacent vertices in a face that are the same */
    for (j = face->nverts-1; j >= 0; j--) {
      jj = (j+1) % face->nverts;
      if (face->verts[j] == face->verts[jj]) {
        for (k = j+1; k < face->nverts - 1; k++)
          face->verts[k] = face->verts[k+1];
        face->nverts--;
      }
    }

    /* remove any faces with less than three vertices by setting */
    /* its vertex count to zero */

    if (face->nverts < 3)
      face->nverts = 0;

  }
}


/******************************************************************************
Add a vertex to it's hash table.

Entry:
  vert    - vertex to add
  table   - hash table to add to
  sq_dist - squared value of distance tolerance
******************************************************************************/

void add_to_hash(Vertex *vert, Hash_Table *table, float sq_dist)
{
  int index;
  int a,b,c;
  int aa,bb,cc;
  float scale;
  Vertex *ptr;
  float dx,dy,dz;
  float sq;
  float min_dist;
  Vertex *min_ptr;

  /* determine which cell the position lies within */

  scale = table->scale;
  a = floor (vert->x * scale);
  b = floor (vert->y * scale);
  c = floor (vert->z * scale);
  index = (a * PR1 + b * PR2 + c) % table->num_entries;
  if (index < 0)
    index += table->num_entries;

  /* examine all points hashed to this cell, looking for */
  /* a vertex to collapse with */

  for (ptr = table->verts[index]; ptr != NULL; ptr = ptr->next)
    if (a == ptr->a && b == ptr->b && c == ptr->c) {
      /* add to sums of coordinates (that later will be averaged) */
      ptr->x += vert->x;
      ptr->y += vert->y;
      ptr->z += vert->z;
      ptr->count++;
      vert->shared = ptr;
      return;
    }

  /* no match if we get here, so add new hash table entry */

  vert->next = table->verts[index];
  table->verts[index] = vert;
  vert->shared = vert;  /* self-reference as close match */
  vert->a = a;
  vert->b = b;
  vert->c = c;
}


/******************************************************************************
Initialize a uniform spatial subdivision table.  This structure divides
3-space into cubical cells and deposits points into their appropriate
cells.  It uses hashing to make the table a one-dimensional array.

Entry:
  nverts - number of vertices that will be placed in the table
  size   - size of a cell

Exit:
  returns pointer to hash table
******************************************************************************/

Hash_Table *init_table(int nverts, float size)
{
  int i;
  int index;
  int a,b,c;
  Hash_Table *table;
  float scale;

  /* allocate new hash table */

  table = (Hash_Table *) malloc (sizeof (Hash_Table));

  if (nverts < TABLE_SIZE1)
    table->num_entries = TABLE_SIZE1;
  else if (nverts < TABLE_SIZE2)
    table->num_entries = TABLE_SIZE2;
  else
    table->num_entries = TABLE_SIZE3;

  table->verts = (Vertex **) malloc (sizeof (Vertex *) * table->num_entries);

  /* set all table elements to NULL */
  for (i = 0; i < table->num_entries; i++)
    table->verts[i] = NULL;

  /* place each point in table */

  scale = 1 / size;
  table->scale = scale;

  return (table);
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
Write out the PLY file to standard out.
******************************************************************************/

write_file()
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;
  int vert_count;
  int face_count;

  /*** Write out the final PLY object ***/


  ply = ply_write (stdout, 2, elem_names, file_type);

  /* count the vertices that are in the hash table */

  vert_count = 0;
  for (i = 0; i < nverts; i++)
    if (vlist[i]->shared == vlist[i]) {
      vlist[i]->index = vert_count;
      vert_count++;
    }

  /* count the faces that have not been collapsed */

  face_count = 0;
  for (i = 0; i < nfaces; i++)
    if (flist[i]->nverts != 0)
      face_count++;

  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", vert_count);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  ply_element_count (ply, "face", face_count);
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
    if (vlist[i]->shared == vlist[i])
      ply_put_element (ply, (void *) vlist[i]);

  /* set up and write the face elements */

  ply_put_element_setup (ply, "face");

  for (i = 0; i < nfaces; i++) {
    if (flist[i]->nverts == 0)
      continue;
    for (j = 0; j < flist[i]->nverts; j++)
      flist[i]->verts[j] = ((Vertex *) flist[i]->verts[j])->index;
    ply_put_element (ply, (void *) flist[i]);
  }

  ply_put_other_elements (ply);

  /* close the PLY file */
  ply_close (ply);
}

