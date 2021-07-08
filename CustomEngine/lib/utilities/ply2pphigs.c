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

Convert from a PLY file to a PPHIGS ascii archive file for viewing on
Pixel-Planes 5.

Jon Cohen
*/

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <values.h>

#include <ply.h>

#if 1
#define FRONTBACK
#endif

#define DEFAULT_RED    50
#define DEFAULT_GREEN 250
#define DEFAULT_BLUE   50

#define MARKED_RED    255
#define MARKED_GREEN    0
#define MARKED_BLUE     0

#define FRONT_RED      50
#define FRONT_GREEN   200
#define FRONT_BLUE    250

#define BACK_RED      250
#define BACK_GREEN    120
#define BACK_BLUE      50

#define DEFAULT_COLOR     0
#define GLOBAL_COLOR      1
#define RANDOM_COLORS     2
#define FACE_COLORS       3
#define VERTEX_COLORS     4
#define FRONTBACK_COLORS  5
#define PLYMATERIAL_COLOR 6

#define DEFAULT_NORMALS 0
#define FACE_NORMALS    1
#define VERTEX_NORMALS  2

#define FALSE           0
#define TRUE            1

#define SQ(a) ((a)*(a))

/* user's vertex and face definitions for a polygonal object */

typedef struct Vertex {
  float x,y,z;
  float nx, ny, nz;
  unsigned char red, green, blue;
  unsigned char marked;
  void *other_props;       /* other properties */
} Vertex;

typedef struct Face {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
  unsigned char red, green, blue;
  unsigned char marked;
  void *other_props;       /* other properties */
} Face;

typedef struct Material
{
    unsigned char ambient_red;
    unsigned char ambient_green;
    unsigned char ambient_blue;
    unsigned char diffuse_red;
    unsigned char diffuse_green;
    unsigned char diffuse_blue;
    unsigned char specular_red;
    unsigned char specular_green;
    unsigned char specular_blue;
    unsigned char specular_power;
    unsigned char emissive_red;
    unsigned char emissive_green;
    unsigned char emissive_blue;
    unsigned char opacity_red;
    unsigned char opacity_green;
    unsigned char opacity_blue;
} Material;


PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
  {"nx", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,nx), 0, 0, 0, 0},
  {"ny", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,ny), 0, 0, 0, 0},
  {"nz", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,nz), 0, 0, 0, 0},
  {"red", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,red), 0, 0, 0, 0},
  {"green", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,green), 0, 0, 0, 0},
  {"blue", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,blue), 0, 0, 0, 0},
  {"marked", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,marked), 0, 0, 0, 0},
};

PlyProperty face_props[] = { /* list of property information for a vertex */
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
     1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
  {"red", PLY_UCHAR, PLY_UCHAR, offsetof(Face,red), 0, 0, 0, 0},
  {"green", PLY_UCHAR, PLY_UCHAR, offsetof(Face,green), 0, 0, 0, 0},
  {"blue", PLY_UCHAR, PLY_UCHAR, offsetof(Face,blue), 0, 0, 0, 0},
  {"marked", PLY_UCHAR, PLY_UCHAR, offsetof(Face,marked), 0, 0, 0, 0},
};

PlyProperty material_props[] = { /* list of property information for a material */
  {"ambient_red", PLY_UCHAR, PLY_UCHAR, offsetof(Material,ambient_red),
   0, 0, 0, 0},
  {"ambient_green", PLY_UCHAR, PLY_UCHAR, offsetof(Material,ambient_green),
   0, 0, 0, 0},
  {"ambient_blue", PLY_UCHAR, PLY_UCHAR, offsetof(Material,ambient_blue),
   0, 0, 0, 0},
  {"diffuse_red", PLY_UCHAR, PLY_UCHAR, offsetof(Material,diffuse_red),
   0, 0, 0, 0},
  {"diffuse_green", PLY_UCHAR, PLY_UCHAR, offsetof(Material,diffuse_green),
   0, 0, 0, 0},
  {"diffuse_blue", PLY_UCHAR, PLY_UCHAR, offsetof(Material,diffuse_blue),
   0, 0, 0, 0},
  {"specular_red", PLY_UCHAR, PLY_UCHAR, offsetof(Material,specular_red),
   0, 0, 0, 0},
  {"specular_green", PLY_UCHAR, PLY_UCHAR, offsetof(Material,specular_green),
   0, 0, 0, 0},
  {"specular_blue", PLY_UCHAR, PLY_UCHAR, offsetof(Material,specular_blue),
   0, 0, 0, 0},
  {"specular_power", PLY_UCHAR, PLY_UCHAR, offsetof(Material,specular_power),
   0, 0, 0, 0},
  {"emissive_red", PLY_UCHAR, PLY_UCHAR, offsetof(Material,emissive_red),
   0, 0, 0, 0},
  {"emissive_green", PLY_UCHAR, PLY_UCHAR, offsetof(Material,emissive_green),
   0, 0, 0, 0},
  {"emissive_blue", PLY_UCHAR, PLY_UCHAR, offsetof(Material,emissive_blue),
   0, 0, 0, 0},
  {"opacity_red", PLY_UCHAR, PLY_UCHAR, offsetof(Material,opacity_red),
   0, 0, 0, 0},
  {"opacity_green", PLY_UCHAR, PLY_UCHAR, offsetof(Material,opacity_green),
   0, 0, 0, 0},
  {"opacity_blue", PLY_UCHAR, PLY_UCHAR, offsetof(Material,opacity_blue),
   0, 0, 0, 0},
};

/*** the PLY object ***/

static int nverts,nfaces,nmaterials;
static Vertex **vlist;
static Face **flist;
static Material **mlist;
static PlyOtherElems *other_elements = NULL;
static PlyOtherProp *vert_other,*face_other;
static int nelems;
static char **elist;
static int num_comments;
static char **comments;
static int num_obj_info;
static char **obj_info;
static int file_type;

int color_mode, normal_mode;

int has_x, has_y, has_z;
int has_nx, has_ny, has_nz;
int has_vred, has_vgreen, has_vblue;
int has_vmarked;

int has_vertex_indices;
int has_fred, has_fgreen, has_fblue;
int has_fmarked;

int has_material;
int has_material_props[16];

unsigned char polygon_red, polygon_green, polygon_blue;
char structure_name[28];

float *vert_min_lengths;

void write_structure_header(char *name);
void write_structure_end();
void write_polygon_color(int front_red, int front_green, int front_blue,
			 int back_red,  int back_green,  int back_blue);
void write_polygon_front_material_color(int diff_red, int diff_green,
					int diff_blue, int diff_alpha,
					int spec_red, int spec_green,
					int spec_blue, int spec_power);
void write_polygon(Face *face);
void write_vnormals_polygon(Face *face);
void write_vcolors_polygon(Face *face);
void write_fcolors_polygon(Face *face);
void write_fcolors_vnormals_polygon(Face *face);
void write_vcolors_vnormals_polygon(Face *face);
void write_random_color_polygon(Face *face);
void write_random_color_vnormals_polygon(Face *face);
void write_sphere_color(int red, int green, int blue);
void write_vertex(Vertex *vertex, float min_length);

#if 0
void write_tris(FILE *fp, TriType *tris, char *name, int color_code);
void write_special_verts(FILE *fp, char *name);
void write_aprx_tris(FILE *fp, AprxType **tris, int num_tris, char *name);
void write_line_color(FILE *fp, int red, int green, int blue);
void write_sphere(FILE *fp, Point center, double radius);
void write_line(FILE *fp, Point p1, Point p2);
void write_vertex(FILE *fp, VertType *vert);
void write_edge(FILE *fp, EdgeType *edge);
void write_selected_tris(FILE *fp, char *name);
void write_selected_edges(FILE *fp, char *name);
void write_selected_verts(FILE *fp, char *name);
#endif



/******************************************************************************
Main program.
******************************************************************************/

main(int argc, char *argv[])
{
    init_defaults();
    get_options(argc, argv);    
    read_file();
    map_defaults();
    write_file();
    return 0;
}

init_defaults()
{
    polygon_red   = DEFAULT_RED;
    polygon_green = DEFAULT_GREEN;
    polygon_blue  = DEFAULT_BLUE;
    color_mode    = DEFAULT_COLOR;
    normal_mode   = DEFAULT_NORMALS;
    strcpy(structure_name, "ply_object");
}

map_defaults()
{
    if (color_mode == DEFAULT_COLOR)
    {
	if (has_material)
	    color_mode = PLYMATERIAL_COLOR;
	else if ((has_vred) && (has_vgreen) && (has_vblue))
	    color_mode = VERTEX_COLORS;
	else if ((has_fred) && (has_fgreen) && (has_fblue))
	    color_mode = FACE_COLORS;
	else
	    color_mode = GLOBAL_COLOR;
    }

    if (normal_mode == DEFAULT_NORMALS)
    {
	if ((has_nx) && (has_ny) && (has_nz))
	    normal_mode = VERTEX_NORMALS;
	else
	    normal_mode = FACE_NORMALS;
    }
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
	    case 'c':
	    case 'C':
		++argv;
		if (equal_strings(*argv, "random"))
		{
		    color_mode = RANDOM_COLORS;
		    argc -= 1;
		}
		else if (equal_strings(*argv, "vertex"))
		{
		    color_mode = VERTEX_COLORS;
		    argc -= 1;
		}
		else if (equal_strings(*argv, "face"))
		{
		    color_mode = FACE_COLORS;
		    argc -= 1;
		}
		else if (equal_strings(*argv, "frontback"))
		{
		    color_mode = FRONTBACK_COLORS;
		    argc -= 1;
		}
		else
		{
		    color_mode = GLOBAL_COLOR;
		    polygon_red = atoi (*argv);
		    polygon_green = atoi (*++argv);
		    polygon_blue = atoi (*++argv);
		    argc -= 3;
		}
		break;
	    case 'n':
	    case 'N':
		++argv;
		if (equal_strings(*argv, "face"))
		{
		    normal_mode = FACE_NORMALS;
		    argc -= 1;
		}
		else if (equal_strings(*argv, "vertex"))
		{
		    normal_mode = VERTEX_NORMALS;
		    argc -= 1;
		}
		else
		{
		    usage(progname);
		    exit(-1);
		}
		break;
	    case 's':
	    case 'S':
		++argv;
		strncpy(structure_name, *argv, 27);
		argc -= 1;
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
  fprintf(stderr, "usage: %s [flags]  <in.ply   >out.ascarch\n", progname);
  fprintf(stderr, "  -- optional flags (default depends on model) -- \n");
  fprintf(stderr, "       -c vertex               : vertex colors\n");
  fprintf(stderr, "       -c face                 : face colors\n");
  fprintf(stderr, "       -c random               : random face colors\n");
  fprintf(stderr, "       -c frontback            : front red, back blue\n");
  fprintf(stderr, "       -c <red> <green> <blue> : global colors in [0-255]\n");
  fprintf(stderr, "       -n vertex               : vertex normals\n");
  fprintf(stderr, "       -n face                 : face normals\n");
  fprintf(stderr, "       -s <structure_name>     : name for PPHIGS structure\n");
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

  has_material = FALSE;
  
  for (i = 0; i < nelems; i++) {

    /* get the description of the first element */
    elem_name = elist[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

    if (equal_strings ("vertex", elem_name)) {

      /* create a vertex list to hold all the vertices */
      vlist = (Vertex **) malloc (sizeof (Vertex *) * num_elems);
      nverts = num_elems;

      /* set up for getting vertex elements */
      has_x = has_y = has_z = has_nx = has_ny = has_nz =
	  has_vred = has_vgreen = has_vblue = has_vmarked = FALSE;
      
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
	  else if (equal_strings("nx", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[3]);  /* nx */
	      has_nx = TRUE;
	  }
	  else if (equal_strings("ny", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[4]);  /* ny */
	      has_ny = TRUE;
	  }
	  else if (equal_strings("nz", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[5]);  /* nz */
	      has_nz = TRUE;
	  }
	  else if (equal_strings("red", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[6]);  /* red */
	      has_vred = TRUE;
	  }
	  else if (equal_strings("green", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[7]);  /* green */
	      has_vgreen = TRUE;
	  }
	  else if (equal_strings("blue", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[8]);  /* blue */
	      has_vblue = TRUE;
	  }
	  else if (equal_strings("marked", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[9]);  /* marked */
	      has_vmarked = TRUE;
	  }
      }
      vert_other = ply_get_other_properties (ply, elem_name,
					     offsetof(Vertex,other_props));

      /* test for necessary properties */
      if ((!has_x) || (!has_y) || (!has_z))
      {
	  fprintf(stderr, "Vertices don't have x, y, and z\n");
	  exit(-1);
      }
      if ((color_mode == VERTEX_COLORS) &&
	  (!(has_vred && has_vgreen && has_vblue)))
      {
	  fprintf(stderr, "Vertex colors not available\n");
	  exit(-1);
      }
      if ((normal_mode == VERTEX_NORMALS) &&
	  (!(has_nx && has_ny && has_nz)))
      {
	  fprintf(stderr, "Vertex normals not available\n");
	  exit(-1);
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
      /* verify which properties these vertices have */
      has_vertex_indices = has_fred = has_fgreen = has_fblue =
	  has_fmarked = FALSE;

      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("vertex_indices", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &face_props[0]);  /* vertex_indices */
	      has_vertex_indices = TRUE;
	  }
	  else if (equal_strings("red", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &face_props[1]);  /* red */
	      has_fred = TRUE;
	  }
	  else if (equal_strings("green", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &face_props[2]);  /* green */
	      has_fgreen = TRUE;
	  }
	  else if (equal_strings("blue", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &face_props[3]);  /* blue */
	      has_fblue = TRUE;
	  }
	  else if (equal_strings("marked", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &face_props[4]);  /* marked */
	      has_fmarked = TRUE;
	  }
      }
      face_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Face,other_props));

      /* test for necessary properties */
      if (!has_vertex_indices)
      {
	  fprintf(stderr, "Faces don't have vertex indices\n");
	  exit(-1);
      }
      if ((color_mode == FACE_COLORS) &&
	  ((!has_fred) || (!has_fgreen) || (!has_fblue)))
      {
	  fprintf(stderr, "Face colors not available\n");
	  exit(-1);
      }
      
      /* grab all the face elements */
      for (j = 0; j < num_elems; j++) {
        flist[j] = (Face *) malloc (sizeof (Face));
        ply_get_element (ply, (void *) flist[j]);
      }
    }
    else if (equal_strings ("material", elem_name))
    {
      has_material = TRUE;
      
      /* create a list to hold all the material elements */
      ALLOCN(mlist, Material *, num_elems);
      nmaterials = num_elems;

      for (k=0; k<16; k++)
	  has_material_props[k] = FALSE;

      for (j=0; j<nprops; j++)
      {
	  for (k=0; k<16; k++)
	  {
	      if (equal_strings(material_props[k].name, plist[j]->name))
	      {
		  ply_get_property (ply, elem_name, &material_props[k]);
		  has_material_props[k] = TRUE;
	      }
	  }
      }
      
      /* test for necessary properties */
      for (k=0; k<16; k++)
	  if (!has_material_props[k])
	  {
	      fprintf(stderr, "Materials don't have %s\n",
		      material_props[k].name);
	      exit(1);
	  }
      
      /* grab all the material elements */
      for (j = 0; j < num_elems; j++) {
	ALLOCN(mlist[j], Material, 1);
        ply_get_element (ply, (void *) mlist[j]);
      }
    }
    else
	other_elements = ply_get_other_element(ply, elem_name, num_elems);
  }
  
  ply_close (ply);
}




/******************************************************************************
Write out the PPHIGS ascii file to standard out.
******************************************************************************/

write_file()
{
    int i;
    
    write_structure_header(structure_name);

    switch(color_mode)
    {
    case GLOBAL_COLOR:
	write_polygon_color(polygon_red, polygon_green, polygon_blue,
			    polygon_red, polygon_green, polygon_blue);
	switch(normal_mode)
	{
	case FACE_NORMALS:
	    for (i=0; i<nfaces; i++)
		if ((!has_fmarked) || (flist[i]->marked == FALSE))
		    write_polygon(flist[i]);
	    break;
	case VERTEX_NORMALS:
	    for (i=0; i<nfaces; i++)
		if ((!has_fmarked) || (flist[i]->marked == FALSE))
		    write_vnormals_polygon(flist[i]);
	    break;
	default:
	    fprintf(stderr, "Invalid normal mode\n");
	    exit(-1);
	    break;
	}
	break;
    case PLYMATERIAL_COLOR:
	write_polygon_front_material_color
	    (mlist[0]->diffuse_red,
	     mlist[0]->diffuse_green,
	     mlist[0]->diffuse_blue,
	     255 - ((mlist[0]->opacity_red + mlist[0]->opacity_green +
		     mlist[0]->opacity_blue) / 3.0),
	     mlist[0]->specular_red,
	     mlist[0]->specular_green,
	     mlist[0]->specular_blue,
	     mlist[0]->specular_power);
	switch(normal_mode)
	{
	case FACE_NORMALS:
	    for (i=0; i<nfaces; i++)
		if ((!has_fmarked) || (flist[i]->marked == FALSE))
		    write_polygon(flist[i]);
	    break;
	case VERTEX_NORMALS:
	    for (i=0; i<nfaces; i++)
		if ((!has_fmarked) || (flist[i]->marked == FALSE))
		    write_vnormals_polygon(flist[i]);
	    break;
	default:
	    fprintf(stderr, "Invalid normal mode\n");
	    exit(-1);
	    break;
	}
	break;
    case FRONTBACK_COLORS:
	write_polygon_color(FRONT_RED, FRONT_GREEN, FRONT_BLUE,
			    BACK_RED, BACK_GREEN, BACK_BLUE);
	switch(normal_mode)
	{
	case FACE_NORMALS:
	    for (i=0; i<nfaces; i++)
		if ((!has_fmarked) || (flist[i]->marked == FALSE))
		    write_polygon(flist[i]);
	    break;
	case VERTEX_NORMALS:
	    for (i=0; i<nfaces; i++)
		if ((!has_fmarked) || (flist[i]->marked == FALSE))
		    write_vnormals_polygon(flist[i]);
	    break;
	default:
	    fprintf(stderr, "Invalid normal mode\n");
	    exit(-1);
	    break;
	}
	break;
    case RANDOM_COLORS:
	switch(normal_mode)
	{
	case FACE_NORMALS:
	    for (i=0; i<nfaces; i++)
		if ((!has_fmarked) || (flist[i]->marked == FALSE))
		    write_random_color_polygon(flist[i]);
	    break;
	case VERTEX_NORMALS:
	    for (i=0; i<nfaces; i++)
		if ((!has_fmarked) || (flist[i]->marked == FALSE))
		    write_random_color_vnormals_polygon(flist[i]);
	    break;
	default:
	    fprintf(stderr, "Invalid normal mode\n");
	    exit(-1);
	    break;
	}
	break;
    case VERTEX_COLORS:
	switch(normal_mode)
	{
	case FACE_NORMALS:
	    for (i=0; i<nfaces; i++)
		if ((!has_fmarked) || (flist[i]->marked == FALSE))
		    write_vcolors_polygon(flist[i]);
	    break;
	case VERTEX_NORMALS:
	    for (i=0; i<nfaces; i++)
		if ((!has_fmarked) || (flist[i]->marked == FALSE))
		    write_vcolors_vnormals_polygon(flist[i]);
	    break;
	default:
	    fprintf(stderr, "Invalid normal mode\n");
	    exit(-1);
	    break;
	}
	break;
    case FACE_COLORS:
	switch(normal_mode)
	{
	case FACE_NORMALS:
	    for (i=0; i<nfaces; i++)
		if ((!has_fmarked) || (flist[i]->marked == FALSE))
		    write_fcolors_polygon(flist[i]);
	    break;
	case VERTEX_NORMALS:
	    for (i=0; i<nfaces; i++)
		if ((!has_fmarked) || (flist[i]->marked == FALSE))
		    write_fcolors_vnormals_polygon(flist[i]);
	    break;
	default:
	    fprintf(stderr, "Invalid normal mode\n");
	    exit(-1);
	    break;
	}
	break;
    default:
	fprintf(stderr, "invalid color mode\n");
	exit(-1);
	break;
    }
    
    write_structure_end();

    if (has_fmarked)
    {
	write_structure_header("marked_ply_faces");
	write_polygon_color(MARKED_RED, MARKED_GREEN, MARKED_BLUE,
			    MARKED_RED, MARKED_GREEN, MARKED_BLUE);
	for (i=0; i<nfaces; i++)
	    if (flist[i]->marked == TRUE)
		write_polygon(flist[i]);
	write_structure_end();
    }

    if (has_vmarked)
    {
	calc_vert_min_lengths();
	write_structure_header("marked_ply_verts");
	write_sphere_color(MARKED_RED, MARKED_GREEN, MARKED_BLUE);
	for (i=0; i<nverts; i++)
	    if (vlist[i]->marked == TRUE)
		write_vertex(vlist[i], vert_min_lengths[i]);
	write_structure_end();
    }

}

calc_vert_min_lengths()
{
    int i, j, vert1, vert2;
    float length;
    
    vert_min_lengths = (float *)calloc(nverts, sizeof(float));
    if (!vert_min_lengths)
    {
	fprintf(stderr, "Couldn't allocate vertex min lengths\n");
	exit(-1);
    }

    for (i=0; i<nverts; i++)
	vert_min_lengths[i] = MAXFLOAT;
    
    for (i=0; i<nfaces; i++)
	for (j=0; j<flist[i]->nverts; j++)
	{
	    vert1 = flist[i]->verts[j];
	    vert2 = flist[i]->verts[(j+1)%(flist[i]->nverts)];
	    length = sqrt(SQ(vlist[vert1]->x-vlist[vert2]->x) +
			  SQ(vlist[vert1]->y-vlist[vert2]->y) +
			  SQ(vlist[vert1]->z-vlist[vert2]->z));
	    if (length < vert_min_lengths[vert1])
		vert_min_lengths[vert1] = length;
	    if (length < vert_min_lengths[vert2])
		vert_min_lengths[vert2] = length;
	}
}




void random_color(unsigned char *red, unsigned char *green,
		  unsigned char *blue)
{
    *red = rand() % 256;
    *green = rand() % 256;
    *blue = rand() % 256;
}



void random_face_color(unsigned char *red, unsigned char *green,
		       unsigned char *blue)
{
    int done = FALSE;
    int rdiff, gdiff, bdiff;
    
    while (!done)
    {
	random_color(red, green, blue);

	if (has_fmarked)
	{
	    rdiff = fabs(*red - MARKED_RED);
	    gdiff = fabs(*green - MARKED_GREEN);
	    bdiff = fabs(*blue - MARKED_BLUE);

	    if ((rdiff > 50) || (gdiff > 50) || (bdiff > 50))
		done = TRUE;
	}
	else
	    done = TRUE;
    }
    
}



void write_structure_header(char *name)
{
    printf("\nstructure %s posted {", name);
}

void write_structure_end()
{
  printf("\n};");
}

void write_polygon_color(int front_red, int front_green, int front_blue,
			 int back_red, int back_green, int back_blue)
{
    printf("\ncolor polygon {");
    printf("\ndiff %d %d %d 0 %d %d %d 0",
	   front_red, front_green, front_blue,
	   back_red, back_green, back_blue);
    printf("\nspec 255 255 255 1 255 255 255 1\n};");
}

void write_polygon_front_material_color(int diff_red, int diff_green,
					int diff_blue, int diff_alpha,
					int spec_red, int spec_green,
					int spec_blue, int spec_power)
{
    printf("\ncolor polygon {");
    printf("\ndiff %d %d %d %d %d %d %d %d",
	   diff_red, diff_green, diff_blue, diff_alpha,
	   diff_red, diff_green, diff_blue, diff_alpha);	   
    printf("\nspec %d %d %d %d %d %d %d %d\n};",
	   spec_red, spec_green, spec_blue, spec_power,
	   spec_red, spec_green, spec_blue, spec_power);
}

void write_sphere_color(int red, int green, int blue)
{
  printf("\ncolor sphere {");
  printf("\ndiff %d %d %d 0 %d %d %d 0",
	  red, green, blue, red, green, blue); 
  printf("\nspec 255 255 255 1 255 255 255 1\n};");
}

void write_line_color(int red, int green, int blue)
{
  printf("\ncolor line {");
  printf("\ndiff %d %d %d 0 %d %d %d 0",
	  red, green, blue, red, green, blue); 
  printf("\nspec 255 255 255 1 255 255 255 1\n};");
}

void write_polygon(Face *face)
{
    int i;
    
    printf("\npolygon %d { ", face->nverts);
    for (i=0; i<face->nverts; i++)
	printf("\n%3.4f %3.4f %3.4f;",
	       vlist[face->verts[i]]->x,
	       vlist[face->verts[i]]->y,
	       vlist[face->verts[i]]->z);
    printf("\n};");
}

void write_vnormals_polygon(Face *face)
{
    int i;
    
    printf("\npolygon %d { ", face->nverts);
    for (i=0; i<face->nverts; i++)
	printf("\n%3.4f %3.4f %3.4f %3.4f %3.4f %3.4f;",
	       vlist[face->verts[i]]->x,
	       vlist[face->verts[i]]->y,
	       vlist[face->verts[i]]->z,
	       vlist[face->verts[i]]->nx,
	       vlist[face->verts[i]]->ny,
	       vlist[face->verts[i]]->nz);
    printf("\n};");
}

void write_vcolors_polygon(Face *face)
{
    int i;

    printf("\npolygon5 %d 3 { ",
	   face->nverts);
    for (i=0; i<face->nverts; i++)
	printf("\n%3.4f %3.4f %3.4f diff %d %d %d 0;",
	       vlist[face->verts[i]]->x,
	       vlist[face->verts[i]]->y,
	       vlist[face->verts[i]]->z,
	       vlist[face->verts[i]]->red,
	       vlist[face->verts[i]]->green,
	       vlist[face->verts[i]]->blue);
    printf("\n};");
}

void write_fcolors_polygon(Face *face)
{
    int i;
    
#ifdef CULL_BACK
    printf("\npolygon5 %d 131 diff %d %d %d 0 { ",
	   face->nverts, face->red, face->green, face->blue);
#else
    printf("\npolygon5 %d 3 diff %d %d %d 0 { ",
	   face->nverts, face->red, face->green, face->blue);
#endif
    for (i=0; i<face->nverts; i++)
	printf("\n%3.4f %3.4f %3.4f;",
	       vlist[face->verts[i]]->x,
	       vlist[face->verts[i]]->y,
	       vlist[face->verts[i]]->z);
    printf("\n};");
}

void write_vcolors_vnormals_polygon(Face *face)
{
    int i;

    printf("\npolygon5 %d 3 { ",
	   face->nverts);
    for (i=0; i<face->nverts; i++)
	printf("\n%3.4f %3.4f %3.4f %3.4f %3.4f %3.4f diff %d %d %d 0;",
	       vlist[face->verts[i]]->x,
	       vlist[face->verts[i]]->y,
	       vlist[face->verts[i]]->z,
	       vlist[face->verts[i]]->nx,
	       vlist[face->verts[i]]->ny,
	       vlist[face->verts[i]]->nz,
	       vlist[face->verts[i]]->red,
	       vlist[face->verts[i]]->green,
	       vlist[face->verts[i]]->blue);
    printf("\n};");
}

void write_fcolors_vnormals_polygon(Face *face)
{
    int i;
    
    printf("\npolygon5 %d 3 diff %d %d %d 0 { ",
	   face->nverts, face->red, face->green, face->blue);
    for (i=0; i<face->nverts; i++)
	printf("\n%3.4f %3.4f %3.4f %3.4f %3.4f %3.4f;",
	       vlist[face->verts[i]]->x,
	       vlist[face->verts[i]]->y,
	       vlist[face->verts[i]]->z,
	       vlist[face->verts[i]]->nx,
	       vlist[face->verts[i]]->ny,
	       vlist[face->verts[i]]->nz);
    printf("\n};");
}

void write_random_color_polygon(Face *face)
{
    unsigned char red, green, blue;
    unsigned char old_r, old_g, old_b;

    random_face_color(&red, &green, &blue);

    old_r = face->red;
    old_g = face->green;
    old_b = face->blue;

    face->red = red;
    face->green = green;
    face->blue = blue;
    
    write_fcolors_polygon(face);

    face->red = old_r;
    face->green = old_g;
    face->blue = old_b;
}

void write_random_color_vnormals_polygon(Face *face)
{
    unsigned char red, green, blue;
    unsigned char old_r, old_g, old_b;
    
    random_face_color(&red, &green, &blue);

    old_r = face->red;
    old_g = face->green;
    old_b = face->blue;

    face->red = red;
    face->green = green;
    face->blue = blue;
    
    write_fcolors_vnormals_polygon(face);

    face->red = old_r;
    face->green = old_g;
    face->blue = old_b;
}

void write_vertex(Vertex *vertex, float min_length)
{
    float radius;

    radius = min_length / 3.0;
    printf("\nsphere %3.4lf %3.4lf %3.4lf %3.4lf;",
	   vertex->x, vertex->y, vertex->z, radius);
}

#if 0
void write_line(Point p1, Point p2)
{
    printf("\nline 2 { ");
    printf("\n%3.4f %3.4f %3.4f;", p1[X], p1[Y], p1[Z]);
    printf("\n%3.4f %3.4f %3.4f;", p2[X], p2[Y], p2[Z]);
    printf("\n};");
}


void write_vertex(VertType *vert)
{
    double radius, length, min_length;
    int i;

    min_length = HUGE;
    
    /* calculate an appropriate radius */
    for (i=0; i<=vert->num_ptrs; i++)
    {
	if (!(vert->edge_ptrs[i])) continue;
	length = sqrt(SQ_DIST3(vert->edge_ptrs[i]->vert_ptrs[0]->coord,
			       vert->edge_ptrs[i]->vert_ptrs[1]->coord));
	min_length = FMIN(length, min_length);
    }

    /* should _really_ also take a curvature estimate at this vertex into
       account to prevent really big spheres -- or some max radius */
    radius = min_length / 3.0;

    
    write_sphere(vert->coord, radius);
}

void write_edge(EdgeType *edge)
{
    /* for now, just draw an edge as a line (could use a polygonal cylinder
       later if I want) */
    if ((edge->vert_ptrs[0]) && (edge->vert_ptrs[1]))
	write_line(edge->vert_ptrs[0]->coord, edge->vert_ptrs[1]->coord);
    else
	printf("write_edge: cannot write Edge %d because of NULL pointer\n",
	       edge->id);
}

/*****************
NOTE, WHEN I AM READY TO USE THIS, SETTING TRIS SELECTED MAKES THEM
A DIFFERENT COLOR -- SO I CAN PLAY WITH THIS
******************/


/* write triangles */
void write_tris(TriType *tris, char *name, int color_code)
{ 
  int		i;
  int           red, green, blue;
  
  printf("Writing %d tris of %s", Num_tris, name);
  fflush(stdout);

  write_structure_header(name);
  
  switch (color_code)
  {
  case 0:
      red = 50;
      green = 250;
      blue = 50;
      break;
  case 1:
      red = 250;
      green = 50;
      blue = 50;
      break;
  case 2:
      red = 0;
      green = 0;
      blue = 255;
      break;
  case 3:
  default:
      red = REDC;
      green = GREENC;
      blue = BLUEC;
      break;
  }

  write_polygon_color(red, green, blue);
  

  /*   tris[4].selected = TRUE; */
  
  /* print the usual tris */
  for(i = 0; i < Num_tris; i++)
  { 
      if (tris[i].id < 0) continue;

      if (tris[i].selected)
      {
	  write_local_color_triangle(fp,
				     tris[i].vert_ptrs[0]->coord,
				     tris[i].vert_ptrs[1]->coord,
				     tris[i].vert_ptrs[2]->coord,
				     250, 250, 0);
      }
      else
      {
	  if (color_code != 4)
	      write_triangle(fp,
			     tris[i].vert_ptrs[0]->coord,
			     tris[i].vert_ptrs[1]->coord,
			     tris[i].vert_ptrs[2]->coord);
	  else
	      write_random_color_triangle(fp,
					  tris[i].vert_ptrs[0]->coord,
					  tris[i].vert_ptrs[1]->coord,
					  tris[i].vert_ptrs[2]->coord);
      }    

      if ((i + 1) % OUTPUT_INDICATOR == 0) 
      {
	  printf("."); fflush(stdout);
      }
      
  }

  write_structure_end(fp);

  printf(" done \n");
}

/* write vertices */
void write_special_verts(char *name)
{ 

  int           i;
  int           num_vert_ids;
  int		vert_ids[] = { 2688, 2714, 4210 };

  num_vert_ids = sizeof(vert_ids)/sizeof(int);
  
  write_structure_header(name);
  
  write_sphere_color(250, 50, 50);

  for (i=0; i<num_vert_ids; i++)
      write_vertex(&(Vert_list[vert_ids[i]]));
  
  write_structure_end(fp);
}

/* write solution triangles */
void write_aprx_tris(AprxType **tris, int num_tris, char *name)
{ 
  int		i;

  printf("Writing %d tris of %s ", num_tris, name);
  fflush(stdout);

  write_structure_header(name);
  write_polygon_color(REDC, GREENC, BLUEC);
  
  /* print the usual tris */
  for(i = 0; i < num_tris; i++)
  {
#ifndef DEBUG_RECURSIVE_HOLE_FILL
      write_triangle(tris[i]->verts[0]->coord, tris[i]->verts[1]->coord,
		     tris[i]->verts[2]->coord);
#else
      write_random_color_triangle(tris[i]->verts[0]->coord,
				  tris[i]->verts[1]->coord, 
				  tris[i]->verts[2]->coord);
#endif
      
    /* if ((i + 1) % OUTPUT_INDICATOR == 0) */
    if ((i + 1) % 1000 == 0) 
    { printf("."); fflush(stdout); }
  }

  write_structure_end(fp);
  
  printf("done \n");
}

void write_selected_tris(char *name)
{
    int i;

    write_structure_header(name);
    write_polygon_color(255, 0, 0);

    for(i=0; i<Num_tris; i++)
	if (Tri_list[i].selected)
	    write_triangle(Tri_list[i].vert_ptrs[0]->coord,
			   Tri_list[i].vert_ptrs[1]->coord,
			   Tri_list[i].vert_ptrs[2]->coord);
    write_structure_end(fp);
}

void write_selected_edges(char *name)
{
    int i;

    write_structure_header(name);
    write_line_color(255, 0, 0);

    for (i=0; i<Num_edges; i++)
	if (Edge_list[i].selected)
	    write_edge(&(Edge_list[i]));
    write_structure_end(fp);
}

void write_selected_verts(char *name)
{
    int i;

    write_structure_header(name);
    write_sphere_color(255, 0, 0);

    for(i=0; i<Num_verts; i++)
	if (Vert_list[i].selected)
	    write_vertex(&(Vert_list[i]));
    write_structure_end(fp);
}
#endif
