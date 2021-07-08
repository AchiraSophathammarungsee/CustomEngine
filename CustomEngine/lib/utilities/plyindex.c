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

Convert a shared vertex model containing faces with vertex indices to a fully
indexed model, containing fully indexed vertices, edges, and faces.

Fails if the model contains non-manifold vertices or edges.

Jon Cohen, July 1995
*/

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <values.h>
#include <ply.h>

#define FALSE 0
#define TRUE  1


double LAmag,LAsum; int LAi,LAj,LAk;
#define VEC_ASN_OP(a,op,b,m){for(LAi=0; LAi<m; LAi++) a[LAi] op b[LAi]; }

#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#define MIN(a,b) (((a)<(b)) ? (a) : (b))



/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  int id;
  unsigned char nfaces;
  int *faces;
  unsigned char nedges;
  int *edges;
  void *other_props;       /* other properties */
} Vertex;

typedef struct Face {
  int id;
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
  unsigned char nedges;
  int *edges;
  void *other_props;       /* other properties */
} Face;

typedef struct Edge {
  int id;
  int vert1;
  int vert2;
  int face1;
  int face2;
  void *other_props;       /* other properties */
} Edge;

typedef int EdgeTuple[3];

char *elem_names[] = { /* list of the kinds of elements in the user's object */
  "vertex", "face", "edge"
};

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"face_indices", PLY_INT, PLY_INT, offsetof(Vertex,faces),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,nfaces)},
  {"edge_indices", PLY_INT, PLY_INT, offsetof(Vertex,edges),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,nedges)},
};

PlyProperty face_props[] = { /* list of property information for a face */
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
  {"edge_indices", PLY_INT, PLY_INT, offsetof(Face,edges),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nedges)},
};

PlyProperty edge_props[] = { /* list of property information for an edge */
  {"vert1", PLY_INT, PLY_INT, offsetof(Edge,vert1),0,0,0,0},
  {"vert2", PLY_INT, PLY_INT, offsetof(Edge,vert2),0,0,0,0},
  {"face1", PLY_INT, PLY_INT, offsetof(Edge,face1),0,0,0,0},
  {"face2", PLY_INT, PLY_INT, offsetof(Edge,face2),0,0,0,0},
};


/*** the PLY object ***/

static int nverts,nfaces,nedges;
static Vertex **vlist;
static Face **flist;
static Edge **edgelist;
static PlyOtherElems *other_elements = NULL;
static PlyOtherProp *vert_other,*face_other,*edge_other;
static int nelems;
static char **elist;
static int num_comments;
static char **comments;
static int num_obj_info;
static char **obj_info;
static int file_type;

unsigned char has_vedges, has_vfaces, has_fverts, has_fedges;
unsigned char has_vert1, has_vert2, has_face1, has_face2;


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
#if 0
      case 'f':
          flip_sign = 1;
          break;
#endif
      default:
          usage (progname);
          exit (-1);
          break;
      }
  }

  read_file();

  ply_index();

  write_file();

  return 0;
}


/******************************************************************************
Print out usage information.
******************************************************************************/

usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] <in.ply >out.ply\n", progname);
}


/* sorts 2D tuples[m][] through tuples[n][].
   assumes that:
   (a) the key is tuples[i][0] and tuples[i][1] in that order,
       m <= i <= n
   (b) tuples[n+1][0] == tuples[n+1][1] == HUGE;
*/
void quick_sort_tuples(EdgeTuple *tuples, int m, int n)
{
  int                   i, j;
  int       		key[2];
  int        		temp_tuple[3];
  int			k = 3;

  if (m < n)
  { i = m;
    j = n + 1;

    VEC_ASN_OP(key, =, tuples[m], 2);

    do
    {  do { i++; } while 
       ((tuples[i][0] <key[0]) || 
       ((tuples[i][0]==key[0])&&(tuples[i][1] <key[1])));
       do {j--; } while
       ((tuples[j][0] >key[0]) || 
       ((tuples[j][0]==key[0])&&(tuples[j][1] >key[1])));
       if (i < j)
       { /* swap i and j-th elements */
	 VEC_ASN_OP(temp_tuple, =, tuples[i], k);
	 VEC_ASN_OP(tuples[i],  =, tuples[j], k);
	 VEC_ASN_OP(tuples[j], =, temp_tuple, k);
       }
    }
    while (i < j);

    /* swap m and j-th elements */
    VEC_ASN_OP(temp_tuple, =, tuples[m], k);
    VEC_ASN_OP(tuples[m],  =, tuples[j], k);
    VEC_ASN_OP(tuples[j], =, temp_tuple, k);

    quick_sort_tuples(tuples, m, j-1);
    quick_sort_tuples(tuples, j+1, n);
  }

}

/* a good bit of this is borrowed from Amitabh Varshney's form_edges() function */
ply_index()
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
    
    
    /* generate edge tuples -- twice as many as final edges */
    ALLOCN(edge_tuples, EdgeTuple, face_edge_count+1);
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
    edge_tuples[ntuples][0] = edge_tuples[ntuples][1] = MAXINT;
    edge_tuples[ntuples][2] = ntuples;

    /* sort the edge tuples */
    quick_sort_tuples(edge_tuples, 0, ntuples-1);

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
	else if ((i-current) > 1)
	{
	    fprintf(stderr, "More than two faces on an edge (%d, %d)\n",
		    edge_tuples[i][0], edge_tuples[i][1]);
	    exit(-1);
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
	edgelist[i]->vert1 = edgelist[i]->vert2 =
	    edgelist[i]->face1 = edgelist[i]->face2 = -1;
	edgelist[i]->id = i;
    }
    
    for (i=0; i<nfaces; i++)
    {
	face = flist[i];
	for (j=0; j<face->nedges; j++)
	{
	    face->edges[j] = xfer_function[face->edges[j]];
	    edge = edgelist[face->edges[j]];
	    if (edge->face1 == -1)
	    {
		edge->vert1 = face->verts[j];
		edge->vert2 = face->verts[(j+1)%(face->nverts)];
		edge->face1 = i;
	    }
	    else if (edge->face2 == -1)
		edge->face2 = i;
	    else
	    {
		fprintf(stderr,
			"More than 2 faces on an edge -- shouldn't reach here\n");
		exit(-1);
	    }
	}
    }
    FREE(xfer_function);

    /* the edges and faces are all set up, so now fix up the edges */

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
    

    /* now sort the faces and edges around each vertex */
    /* this doesn't currently guarantee clockwise or counterclockwise, or even
       that the ordering is the same or the opposite of the face orientation.
       if there are seperate components (i.e. the vertex borders multiple
       holes), each component is sorted, but there is no guarantee that the
       components come in any particular order */

    ALLOCN(edge_used, unsigned char, nedges);
    ALLOCN(face_used, unsigned char, nfaces);
    for (i=0; i<nverts; i++)
    {
	vert = vlist[i];

	ALLOCN(sorted_edges, int, vert->nedges);
	ALLOCN(sorted_faces, int, vert->nfaces);
	for (j=0; j<vert->nedges; j++)
	{
	    sorted_edges[j] = -1;
	    edge_used[vert->edges[j]] = FALSE;
	}
	
	for (j=0; j<vert->nfaces; j++)
	{
	    sorted_faces[j] = -1;
	    face_used[vert->faces[j]] = FALSE;
	}
	
	nedges_used = 0;
	nfaces_used = 0;

	while ((nedges_used < vert->nedges) || (nfaces_used < vert->nfaces))
	{
	    /* look for an unused boundary edge */
	    for (j=0, edge = NULL; j<vert->nedges; j++)
	    {
		if ((edgelist[vert->edges[j]]->face2 == -1) &&
		    (edge_used[vert->edges[j]] == FALSE))
		{
		    edge = edgelist[vert->edges[j]];
		    break;
		}
	    }
	    
	    /* if no boundary edges, this vertex must be completely
	       surrounded, so just take any edge */
	    if (edge == NULL)
		edge = edgelist[vert->edges[0]];
	    
	    face = flist[edge->face1];
	    
	    sorted_edges[nedges_used++] = edge->id;
	    edge_used[edge->id] = TRUE;
	    
	    sorted_faces[nfaces_used++] = face->id;
	    face_used[face->id] = TRUE;

	    first_edge = edge;
	    
	    while (face != NULL)
	    {
		
		/* find index of current edge on current face */
		for (edge_index=0; edge_index<face->nedges; edge_index++)
		    if (face->edges[edge_index] == edge->id)
			break;
		if (edge_index == face->nedges)
		{
		    fprintf(stderr, "Couldn't find edge on face\n");
		    exit(-1);
		}
		
		/* find next edge */
		new_edge1 =
		    edgelist[face->edges[(edge_index+1)%(face->nedges)]];
		new_edge2 =
		    edgelist[face->edges[(edge_index + face->nedges - 1) %
					 (face->nedges)]];
		if ((new_edge1->vert1 == vert->id) ||
		    (new_edge1->vert2 == vert->id))
		    edge = new_edge1;
		else if ((new_edge2->vert1 == vert->id) ||
			 (new_edge2->vert2 == vert->id))
		    edge = new_edge2;
		else
		{
		    fprintf(stderr, "Couldn't find next edge\n");
		    exit(-1);
		}
		
		/* add edge to sorted list
		   (if it's not the end of a full cycle)*/
		if (edge_used[edge->id] == FALSE)
		{
		    sorted_edges[nedges_used++] = edge->id;
		    edge_used[edge->id] = TRUE;
		}
		else if (edge == first_edge)
		{
		    face = NULL;
		    break;
		}
		else
		{
		    fprintf(stderr, "Edge found twice??\n");
		    exit(-1);
		}
		
		/* find next face */
		if (edge->face1 == face->id)
		    face = (edge->face2 == -1) ? NULL: flist[edge->face2];
		else if (edge->face2 == face->id)
		    face = flist[edge->face1];
		else
		{
		    fprintf(stderr, "Couldn't find face on edge\n");
		    exit(-1);
		}
		
		/* add face to sorted list (if there is a next face) */
		if (face)
		{
		    if (face_used[face->id] == TRUE)
		    {
			fprintf(stderr, "Face found twice??\n");
			exit(-1);
		    }
		    sorted_faces[nfaces_used++] = face->id;
		    face_used[face->id] = TRUE;
		}
	    }
	}
	FREE(vert->edges);
	FREE(vert->faces);
	vert->edges = sorted_edges;
	vert->faces = sorted_faces;
    }
    FREE(face_used);
    FREE(edge_used);
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
      ALLOCN(vlist, Vertex*, num_elems);
      nverts = num_elems;
	
      /* set up for getting vertex elements */
      has_vfaces = has_vedges = FALSE;
      
      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("face_indices", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[0]);
	      has_vfaces = TRUE;
	  }
	  else if (equal_strings("edge_indices", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[1]);
	      has_vedges = TRUE;
	  }
      }
      vert_other = ply_get_other_properties (ply, elem_name,
					     offsetof(Vertex,other_props));

      /* test for necessary properties */
      if ((has_vfaces) || (has_vedges))
      {
	  fprintf(stderr, "Vertices already have face or edge indices\n");
	  exit(-1);
      }
      
      /* grab all the vertex elements */
      for (j = 0; j < num_elems; j++) {
        ALLOCN(vlist[j], Vertex, 1);
        ply_get_element (ply, (void *) vlist[j]);
	vlist[j]->id = j;
      }
    }
    else if (equal_strings ("face", elem_name)) {

      /* create a list to hold all the face elements */
      ALLOCN(flist, Face *, num_elems);
      nfaces = num_elems;

      /* set up for getting face elements */
      has_fverts = has_fedges = FALSE;

      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("vertex_indices", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &face_props[0]);
	      has_fverts = TRUE;
	  }
	  else if (equal_strings("edge_indices", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &face_props[1]);
	      has_fedges = TRUE;
	  }
      }
      face_other = ply_get_other_properties (ply, elem_name,
					     offsetof(Face,other_props));

      /* test for necessary properties */
      if (!has_fverts)
      {
	  fprintf(stderr,"Faces must have vertex indices\n");
	  exit(-1);
      }
      if (has_fedges)
      {
	  fprintf(stderr,"Faces already have edge indices\n");
	  exit(-1);
      }
      
      /* grab all the face elements */
      for (j = 0; j < num_elems; j++) {
        ALLOCN(flist[j], Face, 1);
        ply_get_element (ply, (void *) flist[j]);
	flist[j]->id = j;
      }
    }
    else if (equal_strings ("edge", elem_name)) {

	fprintf(stderr, "Plyfile already has edges\n");
	exit(-1);
	
#if 0
      /* create a list to hold all the face elements */
      ALLOCN(edgelist, Edge *, num_elems);
      nedges = num_elems;

      /* set up for getting face elements */
      has_vert1 = has_vert2 = has_face1 = has_face2 = FALSE;

      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("vert1", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &edge_props[0]);
	      has_vert1 = TRUE;
	  }
	  else if (equal_strings("vert2", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &edge_props[1]);
	      has_vert2 = TRUE;
	  }
	  else if (equal_strings("face1", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &edge_props[2]);
	      has_face1 = TRUE;
	  }
	  else if (equal_strings("face2", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &edge_props[3]);
	      has_face2 = TRUE;
	  }
      }
      edge_other = ply_get_other_properties (ply, elem_name,
					     offsetof(Edge,other_props));

      /* test for necessary properties */

      /* grab all the edge elements */
      for (j = 0; j < num_elems; j++) {
        ALLOCN(edgelist[j], Edge, 1);
        ply_get_element (ply, (void *) edgelist[j]);
      }
#endif
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


  nelems = 3;
  ply = ply_write (stdout, nelems, elem_names, file_type);


  /* describe what properties go into the vertex and face elements */

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  ply_element_count (ply, "face", nfaces);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_property (ply, "face", &face_props[1]);
  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));

  ply_element_count (ply, "edge", nedges);
  ply_describe_property (ply, "edge", &edge_props[0]);
  ply_describe_property (ply, "edge", &edge_props[1]);
  ply_describe_property (ply, "edge", &edge_props[2]);
  ply_describe_property (ply, "edge", &edge_props[3]);
  
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

  /* set up and write the edge elements */
  ply_put_element_setup(ply, "edge");
  for (i = 0; i < nedges; i++)
    ply_put_element (ply, (void *) edgelist[i]);

  ply_put_other_elements (ply);

  /* close the PLY file */
  ply_close (ply);
}

