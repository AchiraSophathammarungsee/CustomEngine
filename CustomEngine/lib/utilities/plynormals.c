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

Compute vertex normals.

Originally by Greg Turk, August 1994
Sharp edge detection with vertex splitting added by
  Jonathan Cohen, March 1996.

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


#define FALSE 0
#define TRUE  1
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))


/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  float x,y,z;
  float nx,ny,nz;
  int   nfaces;
  int  *faces;
  int   nedges;
  int  *edges;
  void *other_props;       /* other properties */
  int   count;
} Vertex;

typedef struct Face {
  int id;
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
  unsigned char nedges;
  int *edges;
  float nx, ny, nz;
  void *other_props;       /* other properties */
} Face;


typedef int Group[2];
typedef struct Edge 
{
    int  id;
    int  vert1, vert2;
    int  nfaces;
    int *faces;
    int *face_groups;
    int  ngroups;
    Group *groups;
} Edge;

typedef int EdgeTuple[3];
typedef struct FacePair
{
    int ids[2];
    float dot;
} FacePair;

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
  {"nx", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,nx), 0, 0, 0, 0},
  {"ny", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,ny), 0, 0, 0, 0},
  {"nz", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,nz), 0, 0, 0, 0},
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

static int flip_sign = 0;       /* flip the sign of the normals? */
static int split_verts = 0;     /* split vertices based on sharp edges? */
static float split_edge_angle = 90.0;  /* minimum angle between normals
					  adjacent to a sharp edge */

static Edge **edgelist;
static int    nedges;

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
        case 'f':
          flip_sign = 1;
          break;
        case 's':
	  split_verts = 1;
	  break;
        case 'a':
	  ++argv;
	  split_verts = 1;
	  split_edge_angle = atof (*argv);
	  argc -= 1;
	  break;  
        default:
          usage (progname);
          exit (-1);
          break;
      }
  }

  read_file();

  compute_face_normals();
  
  if (split_verts == 1)
      split();
  
  compute_vertex_normals();

  write_file();

  return 0;
}


/******************************************************************************
Print out usage information.
******************************************************************************/

usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] <in.ply >out.ply\n", progname);
  fprintf (stderr, "       -f            : flip sign of normals\n");
  fprintf (stderr, "       -s            : split verts to assign multiple normals across sharp edges\n");
  fprintf (stderr, "       -a <degrees>  : minimum angle between normals of sharp edge\n");
}


compute_face_normals()
{
    int i;
    Face *face;
    int  *verts;
    float x0, y0, z0;
    float x1, y1, z1;
    float x, y, z;
    float len, recip;
    
  for (i = 0; i < nfaces; i++) {

    face = flist[i];
    verts = face->verts;

    /* determine vectors parallel to two edges of face */

    x0 = vlist[verts[1]]->x - vlist[verts[0]]->x;
    y0 = vlist[verts[1]]->y - vlist[verts[0]]->y;
    z0 = vlist[verts[1]]->z - vlist[verts[0]]->z;

    x1 = vlist[verts[face->nverts-1]]->x - vlist[verts[0]]->x;
    y1 = vlist[verts[face->nverts-1]]->y - vlist[verts[0]]->y;
    z1 = vlist[verts[face->nverts-1]]->z - vlist[verts[0]]->z;


    /* find cross-product between these vectors */
    x = y0 * z1 - z0 * y1;
    y = z0 * x1 - x0 * z1;
    z = x0 * y1 - y0 * x1;

    /* normalize this vector */
    len = x*x + y*y + z*z;
    if (len == 0) {
      x = y = z = 0;
    }
    else {
      recip = 1 / sqrt (len);
      x *= recip;
      y *= recip;
      z *= recip;
    }

    face->nx = x;
    face->ny = y;
    face->nz = z;
  }
}



/******************************************************************************
Compute normals at the vertices.
******************************************************************************/

compute_vertex_normals()
{
  int i,j;
  Face *face;
  Vertex *vert;
  int *verts;
  float len, recip, dot;
  
  /* zero out all normal information at vertices */

  for (i = 0; i < nverts; i++) {
    vlist[i]->nx = 0;
    vlist[i]->ny = 0;
    vlist[i]->nz = 0;
    vlist[i]->count = 0;
  }

  /* add normal of each face to each vertex adjacent to the face */

  for (i = 0; i < nfaces; i++) {

    face = flist[i];
    verts = face->verts;
    
    /* add this normal to each vertex that is adjacent to face */
    for (j = 0; j < face->nverts; j++) {
      vlist[verts[j]]->nx += face->nx;
      vlist[verts[j]]->ny += face->ny;
      vlist[verts[j]]->nz += face->nz;
    }
  }

  /* normalize all the normals at the vertices */

  for (i = 0; i < nverts; i++) {
    vert = vlist[i];
    len = vert->nx * vert->nx + vert->ny * vert->ny + vert->nz * vert->nz;
    if (len == 0) {
      vert->nx = 0;
      vert->ny = 0;
      vert->nz = 0;
    }
    else {
      if (flip_sign)
        recip = -1 / sqrt (len);
      else
        recip = 1 / sqrt (len);
      vert->nx *= recip;
      vert->ny *= recip;
      vert->nz *= recip;
    }
  }

  /* verify all of the faces' vertex normals agree in direction with their
     face normals */
  for (i=0; i<nfaces; i++)
  {
      face = flist[i];
      for (j=0; j<face->nverts; j++)
      {
	  vert = vlist[face->verts[j]];
	  dot = face->nx*vert->nx + face->ny*vert->ny + face->nz*vert->nz;
	  if (((dot < 0) && (!(flip_sign))) || ((dot > 0) && (flip_sign)))
	      vert->count++;
      }
  }
  for (i=0; i<nverts; i++)
  {
      vert = vlist[i];
      if (vert->count)
	  fprintf(stderr,
	   "Warning: normal of vertex %d disagrees with %d face normals\n",
		  i, vert->count);
  }
}


int compare_edge_tuples(const void *tup1, const void *tup2)
{
    const EdgeTuple *tuple1, *tuple2;
    int       retval;
    
    tuple1 = tup1;
    tuple2 = tup2;

    retval =
	(((*tuple1)[0] < (*tuple2)[0]) ? -1 :
	 (((*tuple1)[0] > (*tuple2)[0]) ? 1 :
	  (((*tuple1)[1] < (*tuple2)[1]) ? -1 :
	   (((*tuple1)[1] > (*tuple2)[1]) ? 1 :
	    0))));

    return retval;
}


/* a good bit of this is borrowed from Amitabh Varshney's form_edges() function */
void do_index()
{
    int i, j;
    int face_edge_count;
    EdgeTuple *edge_tuples;
    int ntuples, current;
    int *xfer_function;
    Face *face;
    Edge *edge, *new_edge1, *new_edge2, *first_edge;
    Vertex *vert;
    unsigned char *edge_used, *face_used;
    int *sorted_edges, *sorted_faces;
    int edge_index;
    int nedges_used, nfaces_used;
    
    /* count face edges -- faces are closed, so nverts == nedges */
    for (i=0, face_edge_count = 0; i<nfaces; i++)
    {
	flist[i]->nedges = flist[i]->nverts;
	ALLOCN(flist[i]->edges, int, flist[i]->nedges);
	face_edge_count += flist[i]->nedges;
    }
    
    /* generate edge tuples -- at least twice as many as final edges */
    ALLOCN(edge_tuples, EdgeTuple, face_edge_count);
    for (i=0, ntuples=0; i<nfaces; i++)
    {
	face = flist[i];
	for (j=0; j<face->nedges; j++)
	{
	    edge_tuples[ntuples][0] =
		MIN(face->verts[j], face->verts[(j+1)%face->nverts]);
	    edge_tuples[ntuples][1] =
		MAX(face->verts[j], face->verts[(j+1)%face->nverts]);
	    edge_tuples[ntuples][2] = ntuples;
	    face->edges[j] = ntuples;
	    ntuples++;
	}
    }

    /* sort the edge tuples */
    qsort((void *)edge_tuples, ntuples, sizeof(EdgeTuple), compare_edge_tuples);

    /* set up a transfer function from edge tuple number to final edge number */
    ALLOCN(xfer_function, int, ntuples);
    for (i=0, current=0, nedges=0; i<ntuples; i++)
    {
	if ((edge_tuples[i][0] != edge_tuples[current][0]) ||
	    (edge_tuples[i][1] != edge_tuples[current][1]))
	{
	    current=i;
	    nedges++;
	}
	xfer_function[edge_tuples[i][2]] = nedges;
    }
    nedges++;
    FREE(edge_tuples);

    /* set up edges, including edge->face and face->edge indices */
    ALLOCN(edgelist, Edge *, nedges);
    for (i=0; i<nedges; i++)
    {
	ALLOCN(edgelist[i], Edge, 1);
	edgelist[i]->vert1 = edgelist[i]->vert2 = -1;
	edgelist[i]->nfaces = 0;
	edgelist[i]->faces = NULL;
	edgelist[i]->id = i;
    }

    for (i=0; i<nfaces; i++)
    {
	face = flist[i];
	for (j=0; j<face->nedges; j++)
	{
	    face->edges[j] = xfer_function[face->edges[j]];
	    edge = edgelist[face->edges[j]];
	    if (edge->nfaces == 0)
	    {
		edge->vert1 = face->verts[j];
		edge->vert2 = face->verts[(j+1)%(face->nverts)];
	    }
	    edge->nfaces++;
	}
    }
    FREE(xfer_function);

    /* allocate each edge's face list */
    for (i=0; i<nedges; i++)
    {
	edge = edgelist[i];
	ALLOCN(edge->faces, int, edge->nfaces);
	edge->nfaces = 0;
    }

    /* assign each edge's face list */
    for (i=0; i<nfaces; i++)
    {
	face = flist[i];
	for (j=0; j<face->nedges; j++)
	{
	    edge = edgelist[face->edges[j]];
	    edge->faces[edge->nfaces++] = i;
	}
    }

    
    
    /* the edges and faces are all set up, so now fix up the vertices */

    /* count vertex edges and faces */
    for (i=0; i<nverts; i++)
	vlist[i]->nfaces = vlist[i]->nedges = 0;
    for (i=0; i<nedges; i++)
    {
	vlist[edgelist[i]->vert1]->nedges++;
	vlist[edgelist[i]->vert2]->nedges++;
    }
    for (i=0; i<nfaces; i++)
	for (j=0; j<flist[i]->nverts; j++)
	    vlist[flist[i]->verts[j]]->nfaces++;

    /* fill in the vertices' edges and faces in arbitrary order */
    for (i=0; i<nverts; i++)
    {
	ALLOCN(vlist[i]->faces, int, vlist[i]->nfaces);
	ALLOCN(vlist[i]->edges, int, vlist[i]->nedges);
	vlist[i]->nfaces = vlist[i]->nedges = 0;
    }
    for (i=0; i<nedges; i++)
    {
	edge = edgelist[i];

	vert = vlist[edge->vert1];
	vert->edges[vert->nedges++] = i;

	vert = vlist[edge->vert2];
	vert->edges[vert->nedges++] = i;
    }
    for (i=0; i<nfaces; i++)
    {
	face = flist[i];
	for (j=0; j<face->nverts; j++)
	{
	    vert = vlist[face->verts[j]];
	    vert->faces[vert->nfaces++] = i;
	}
    }
}

int compare_face_pairs(const void *p1, const void *p2)
{
    const FacePair *pair1, *pair2;
    int             retval;
    
    pair1 = p1;
    pair2 = p2;

    /* sort in order of increasing angle, so decreasing dot product */
    retval =
	((pair1->dot > pair2->dot) ? -1 :
	 ((pair1->dot < pair2->dot) ? 1 :
	  0));

    return retval;
}

find_face_pairs()
{
    int       i, j, k;
    float     threshold;
    Edge     *edge;
    FacePair *pairs, *pair;
    int       npairs;
    Face     *face1, *face2;
    int       group;
    
    threshold = cos(split_edge_angle * M_PI / 180.0);
    for (i=0; i<nedges; i++)
    {
	edge = edgelist[i];
	ALLOCN(edge->face_groups, int, edge->nfaces);
	
	if (edge->nfaces > 1)
	{
	    ALLOCN(pairs, FacePair, (edge->nfaces * (edge->nfaces-1)) / 2);
	}
	else
	    pairs = NULL;

	/* set up all possible pairs of faces around this edge */
	npairs = 0;
	for (j=0; j<edge->nfaces - 1; j++)
	    for (k=j+1; k<edge->nfaces; k++)
	    {
		pair = &(pairs[npairs]);
		pair->ids[0] = j;
		pair->ids[1] = k;
		face1 = flist[edge->faces[j]];
		face2 = flist[edge->faces[k]];
		pair->dot =
		    face1->nx * face2->nx +
			face1->ny * face2->ny +
			    face1->nz * face2->nz;
		npairs++;
	    }

	qsort((void *)pairs, npairs, sizeof(FacePair), compare_face_pairs);
	
	/* now choose pairs in a greedy fashion using the dot product and the
	   threshold */

	for (j=0; j<edge->nfaces; j++)
	    edge->face_groups[j] = -1;

	/* pair up faces within the threshold */
	edge->ngroups = 0;
	for (j=0; j<npairs; j++)
	{
	    pair = &(pairs[j]);

	    if (pair->dot < threshold)
		break;
	    
	    if ((edge->face_groups[pair->ids[0]] == -1) &&
		(edge->face_groups[pair->ids[1]] == -1))
	    {
		edge->face_groups[pair->ids[0]] = edge->ngroups;
		edge->face_groups[pair->ids[1]] = edge->ngroups;
		edge->ngroups++;
	    }
	}

	/* put any remaining faces in their own individual groups */
	for (j=0; j<edge->nfaces; j++)
	{
	    if (edge->face_groups[j] == -1)
		edge->face_groups[j] = edge->ngroups++;
	}
	FREE(pairs);

	/* now organize group information */
	ALLOCN(edge->groups, Group, edge->ngroups);
	for (j=0; j<edge->ngroups; j++)
	    edge->groups[j][0] = edge->groups[j][1] = -1;
	for (j=0; j<edge->nfaces; j++)
	{
	    group = edge->face_groups[j];
	    if (edge->groups[group][0] == -1)
		edge->groups[group][0] = edge->faces[j];
	    else if (edge->groups[group][1] == -1)
		edge->groups[group][1] = edge->faces[j];
	    else
	    {
		fprintf(stderr, "error: too many faces in group\n");
		exit(-1);
	    }
	}
    }
} /* End of find_face_pairs */


do_splits()
{
    int         i, j, k, m;
    Vertex     *vert, *new_vert, **new_vlist;
    Face       *face;
    Edge       *edge, *new_edge;
    char       *face_used;
    int         nfaces_used;
    int         new_nverts;
    int         new_face_id, group;
    int         split_count, orig_nverts;

    split_count = 0;
    orig_nverts = nverts;
    
    ALLOCN(face_used, char, nfaces);
    for (i=0; i<orig_nverts; i++)
    {
	vert = vlist[i];
	if (!vert->nfaces)
	{
	    fprintf(stderr, "plynormals(): skipping vert with no faces.\n");
	    continue;
	}
	
	for (j=0; j<vert->nfaces; j++)
	    face_used[vert->faces[j]] = FALSE;
	nfaces_used = 0;

	ALLOCN(new_vlist, Vertex *, vert->nfaces);
	new_nverts = 0;
	
	while (nfaces_used < vert->nfaces)
	{
	    ALLOCN(new_vert, Vertex, 1);
	    new_vlist[new_nverts++] = new_vert;
	    ALLOCN(new_vert->faces, int, vert->nfaces);
	    new_vert->nfaces = 0;
	    
	    /* look for a face with a neighbor on only one of its two edges
	       connected to this vertex (or on neither edge -- solo face) */
	    for (j=0, face = NULL; ((j<vert->nedges) && (face == NULL)); j++)
	    {
		edge = edgelist[vert->edges[j]];
		for (k=0; ((k<edge->ngroups) && (face == NULL)); k++)
		{
		    if ((edge->groups[k][1] == -1) &&
			(face_used[edge->groups[k][0]] == FALSE))
			face = flist[edge->groups[k][0]];
		}
	    }
	    
	    /* if no such face exists, all the the sheets connected to this vertex
	       are complete cycles, so just choose an unused face */
	    if (face == NULL)
		for (j=0; ((j<vert->nfaces) && (face == NULL)); j++)
		    if (face_used[vert->faces[j]] == FALSE)
		    {
			face = flist[vert->faces[j]];
			edge = NULL;  /* any edge will do when we start
					 walking */
		    }
	    if (face == NULL)
	    {
		fprintf(stderr, "do_splits: Couldn't find unused face\n");
		exit(-1);
	    }

	    new_vert->faces[new_vert->nfaces++] = face->id;
	    face_used[face->id] = TRUE;
	    nfaces_used++;
	    
	    /* now walk the faces of this sheet */
	    while (face != NULL)
	    {
		/* find the other edge connected to this face and vertex */
		for (j=0; j<vert->nedges; j++)
		{
		    new_edge = edgelist[vert->edges[j]];
		    if (new_edge == edge)
			continue;
		    for (k=0; k<new_edge->nfaces; k++)
		    {
			if (new_edge->faces[k] == face->id)
			{
			    group = new_edge->face_groups[k];
			    if (new_edge->groups[group][0] == face->id)
				new_face_id = new_edge->groups[group][1];
			    else if (new_edge->groups[group][1] == face->id)
				new_face_id = new_edge->groups[group][0];
			    else
			    {
				fprintf(stderr,
				  "do_splits: couldn't find next face\n");
				exit(-1);
			    }
			    edge = new_edge;
			    if ((new_face_id == -1) ||
				(face_used[new_face_id] == TRUE))
			    {
				face = NULL;
				break;
			    }
			    else
				face = flist[new_face_id];
			    new_vert->faces[new_vert->nfaces++] = face->id;
			    face_used[face->id] = TRUE;
			    nfaces_used++;
			    break;
 			}
		    }
		    if (edge == new_edge)
			break;
		}
	    } /* loop over a vertex sheet's faces */    
	}  /* loop over a vertex's sheets */

	/* now move this set of new vertices into the main vlist */
	
	if (new_nverts < 1)
	{
	    fprintf(stderr, "do_split: invalid new_nverts\n");
	    exit(-1);
	}

	/* If there's only one new_vert, it's identical to the original, so
	   just throw it away and move on (common case) */
	if (new_nverts == 1)
	{
	    FREE(new_vlist[0]->faces);
	    FREE(new_vlist[0]);
	    FREE(new_vlist);
	    continue;
	}

	split_count++;

	/* copy all the extra info from the original vert into all the new
	   vertices, then throw away the original */
	for (j=0; j<new_nverts; j++)
	{
	    new_vlist[j]->x = vert->x;
	    new_vlist[j]->y = vert->y;
	    new_vlist[j]->z = vert->z;
	    new_vlist[j]->other_props = vert->other_props;
	    REALLOCN(new_vlist[j]->faces, int, vert->nfaces,
		     new_vlist[j]->nfaces);
	}
	FREE(vert->faces);
	FREE(vert->edges);
	FREE(vert);

	/* fill in the place of the original vert with one of the new ones */
	vlist[i] = new_vlist[--new_nverts];

	/* now tack all the new vertices onto the end of the main vertex list,
	   updating all adjacent faces' vertex ids to point to these new
	   vertices */
	REALLOCN(vlist, Vertex *, nverts, nverts+new_nverts);
	for (j=0; j<new_nverts; j++)
	{
	    vlist[nverts] = new_vlist[j];
	    for (k=0; k<vlist[nverts]->nfaces; k++)
	    {
		face = flist[vlist[nverts]->faces[k]];
		for (m=0; m<face->nverts; m++)
		    if (face->verts[m] == i)
			face->verts[m] = nverts;
	    }
	    nverts++;
	}
	
	FREE(new_vlist);
	
    }  /* loop over vertices */

    fprintf(stderr, "plynormals: Split %d vertices into %d vertices\n",
	    split_count, nverts-orig_nverts+split_count);
}



split()
{
    do_index();

    find_face_pairs();

    do_splits();
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
  int get_nx,get_ny,get_nz;

  /*** Read in the original PLY object ***/


  ply  = ply_read (stdin, &nelems, &elist);
  ply_get_info (ply, &version, &file_type);

  for (i = 0; i < nelems; i++) {

    /* get the description of the first element */
    elem_name = elist[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

    if (equal_strings ("vertex", elem_name)) {

      /* see if vertex holds any normal information */
      get_nx = get_ny = get_nz = 0;
      for (j = 0; j < nprops; j++) {
        if (equal_strings ("nx", plist[j]->name)) get_nx = 1;
        if (equal_strings ("ny", plist[j]->name)) get_ny = 1;
        if (equal_strings ("nz", plist[j]->name)) get_nz = 1;
      }

      /* create a vertex list to hold all the vertices */
      vlist = (Vertex **) malloc (sizeof (Vertex *) * num_elems);
      nverts = num_elems;

      /* set up for getting vertex elements */

      ply_get_property (ply, elem_name, &vert_props[0]);
      ply_get_property (ply, elem_name, &vert_props[1]);
      ply_get_property (ply, elem_name, &vert_props[2]);
      if (get_nx) ply_get_property (ply, elem_name, &vert_props[3]);
      if (get_ny) ply_get_property (ply, elem_name, &vert_props[4]);
      if (get_nz) ply_get_property (ply, elem_name, &vert_props[5]);
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
	flist[j]->id = j;
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
  char *known_elements[] = {"vertex", "face"};
  int   num_known_elements = 2;
  /*** Write out the final PLY object ***/


  ply = ply_write (stdout, num_known_elements, known_elements, file_type);


  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_property (ply, "vertex", &vert_props[3]);
  ply_describe_property (ply, "vertex", &vert_props[4]);
  ply_describe_property (ply, "vertex", &vert_props[5]);
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