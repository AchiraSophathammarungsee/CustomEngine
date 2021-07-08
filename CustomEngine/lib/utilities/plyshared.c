/*

Turn a PLY object with un-shared vertices into one with faces that
share their vertices.

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
  float x,y,z;
  unsigned char r, g, b;
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

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
  {"red", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,r), 0, 0, 0, 0},
  {"green", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,g), 0, 0, 0, 0},
  {"blue", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,b), 0, 0, 0, 0},
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

static float tolerance = 0.0;   /* what constitutes "near" */

int    has_vertex_colors;

/* hash table for near neighbor searches */

#define PR1  17
#define PR2 101
#define TABLE_SIZE1  5003
#define TABLE_SIZE2 17003
#define TABLE_SIZE3 53003

/* note that all the table sizes are less then sqrt(MAXUNSIGNEDLONG) */


typedef struct Hash_Table {	/* uniform spatial subdivision, with hash */
  int npoints;			/* number of points placed in table */
  Vertex **verts;		/* array of hash cells */
  int num_entries;		/* number of array elements in verts */
  double scale;			/* size of cell */
} Hash_Table;


Hash_Table *init_table(int, float);
void add_to_hash (Vertex *, Hash_Table *, float);

#define TRUE  (1)
#define FALSE (0)

#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#define MAX(a,b) (((a)>(b)) ? (a) : (b))

float minx, miny, minz;


/******************************************************************************
Main program.
******************************************************************************/

int main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;

  progname = argv[0];

  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
        case 't':
          tolerance = atof (*++argv);
          argc -= 1;
          break;
        default:
          usage (progname);
          exit (-1);
          break;
      }
  }

  read_file();

  share_vertices();

  write_file();

  return 0;
}


/******************************************************************************
Print out usage information.
******************************************************************************/

usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] <in.ply >out.ply\n", progname);
  fprintf (stderr, "       -t tolerance (default = %g)\n", tolerance);
}


/******************************************************************************
Figure out which vertices are close enough to share.
******************************************************************************/

share_vertices()
{
  int i,j,k;
  int jj;
  Hash_Table *table;
  float squared_dist;
  Vertex *vert;
  Face *face;

  table = init_table (nverts, tolerance);

  squared_dist = tolerance * tolerance;

  minx = vlist[0]->x;
  miny = vlist[0]->y;
  minz = vlist[0]->z;
  for (i=1; i < nverts; i++)
  {
      minx = MIN(minx, vlist[i]->x);
      miny = MIN(miny, vlist[i]->y);
      minz = MIN(minz, vlist[i]->z);
  }
  
      
  
  /* place all vertices in the hash table, and in the process */
  /* learn which ones should be shared */

  for (i = 0; i < nverts; i++)
    add_to_hash (vlist[i], table, squared_dist);

  /* fix up the faces to point to the shared vertices */

  for (i = 0; i < nfaces; i++) {

    face = flist[i];
    
    /* change all indices to pointers to the shared vertices */
    for (j = 0; j < face->nverts; j++)
      face->verts[j] = (int) (vlist[face->verts[j]]->shared);

    /* collapse adjacent vertices that are the same */
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
  unsigned long a,b,c;
  unsigned long aa,bb,cc;
  double scale;
  Vertex *ptr;
  float dx,dy,dz;
  float sq;
  float min_dist;
  Vertex *min_ptr;

  /* determine which cell the position lies within */

  scale = table->scale;

  aa = (int)
      floor(fmod(fmod(vert->x, table->num_entries)+table->num_entries,
		 table->num_entries) * scale)
	  % table->num_entries;
  bb = (int)
      floor(fmod(fmod(vert->y, table->num_entries)+table->num_entries,
		 table->num_entries) * scale)
	  % table->num_entries;
  cc = (int)
      floor(fmod(fmod(vert->z, table->num_entries)+table->num_entries,
		 table->num_entries) * scale)
	  % table->num_entries;
  
  
  /* examine vertices in table to see if we're very close */

  min_dist = 1e20;
  min_ptr = NULL;

  /* look at nine cells, centered at cell containing location */

  for (a = (aa+table->num_entries-1)%table->num_entries;
       a != (aa+2)%table->num_entries;
       a = (a+1)%table->num_entries)
  for (b = (bb+table->num_entries-1)%table->num_entries;
       b != (bb+2)%table->num_entries;
       b = (b+1)%table->num_entries)
  for (c = (cc+table->num_entries-1)%table->num_entries;
       c != (cc+2)%table->num_entries;
       c = (c+1)%table->num_entries)

  {
      
    /* compute position in hash table */
    index =
	( ((a * PR1)%table->num_entries) +
	  ((b * PR2)%table->num_entries) + c)
	    % table->num_entries;

    /* examine all points hashed to this cell */
    for (ptr = table->verts[index]; ptr != NULL; ptr = ptr->next) {

      /* distance (squared) to this point */
      dx = ptr->x - vert->x;
      dy = ptr->y - vert->y;
      dz = ptr->z - vert->z;
      sq = dx*dx + dy*dy + dz*dz;

      /* maybe we've found new closest point */
      if (sq <= min_dist) {
        min_dist = sq;
        min_ptr = ptr;
      }
    }
  }

  /* If we found a match, have new vertex point to the matching vertex. */
  /* If we didn't find a match, add new vertex to the table. */

  if ((min_ptr && min_dist <= sq_dist)   /* geometric match */
      && ((!has_vertex_colors) ||
	  ((vert->r == min_ptr->r) &&    /* color match */
	   (vert->g == min_ptr->g) &&        /* equality test for now */
	   (vert->b == min_ptr->b))))
  {
      vert->shared = min_ptr;
  }
  else {          /* no match */
    index =
	( ((aa * PR1)%table->num_entries) +
	  ((bb * PR2)%table->num_entries) + cc) % table->num_entries;

    vert->next = table->verts[index];
    table->verts[index] = vert;
    vert->shared = vert;  /* self-reference as close match */
  }
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
  double scale;

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

  size = MAX(size, 1E-10);
  
  scale = 1.0 / size;
  table->scale = fmod(scale, (double)table->num_entries);

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
  int   has_x, has_y, has_z;
  int   has_r, has_g, has_b;

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
      has_x = has_y = has_z = has_r = has_g = has_b = FALSE;
      
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
	  else if (equal_strings("red", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[3]);  /* r */
	      has_r = TRUE;
	  }
	  else if (equal_strings("green", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[4]);  /* g */
	      has_g = TRUE;
	  }
	  else if (equal_strings("blue", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[5]);  /* b */
	      has_b = TRUE;
	  }
      }
      vert_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Vertex,other_props));

      if (!((has_x) && (has_y) && (has_z)))
      {
	  fprintf(stderr,
		  "plyshared: Vertices must have x, y, and z.  Exiting...\n");
	  exit(1);
      }
      if ((has_r) && (has_g) && (has_b))
	  has_vertex_colors = TRUE;
      else if ((!has_r) && (!has_g) && (!has_b))
	  has_vertex_colors = FALSE;
      else
      {
	  fprintf(stderr,
		  "plyshared: Vertices have only partial colors.  Exiting...\n");
	  exit(1);
      }
      
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
  char *known_elements[] = {"vertex", "face"};
  
  /*** Write out the final PLY object ***/


  ply = ply_write (stdout, 2, known_elements, file_type);

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
  if (has_vertex_colors)
  {
      ply_describe_property (ply, "vertex", &vert_props[3]);
      ply_describe_property (ply, "vertex", &vert_props[4]);
      ply_describe_property (ply, "vertex", &vert_props[5]);
  }
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