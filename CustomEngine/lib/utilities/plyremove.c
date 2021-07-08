/*

Remove properties or elements from a PLY file.

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

typedef struct OtherStruct {
  void *other_props;
} OtherStruct;

/*** the PLY object ***/

PlyOtherElems *other_elements = NULL;
int nelems;
char **elist;
int file_type;
int num_comments;
char **comments;
int num_obj_info;
char **obj_info;
PlyOtherProp **other_props;
OtherStruct ***olist;
int *elem_counts;


/* list of properties to remove */

#define MAX 50
static char *prop_list[MAX];
static char *prop_elem_list[MAX];
static int prop_count = 0;

/* list of elements to remove */

static char *elem_list[MAX];
static int elem_count = 0;


/******************************************************************************
Read in, process, and write out a PLY file.
******************************************************************************/

main(int argc, char *argv[])
{
  int i,j;

  parse_command_line (argc, argv);

  read_file();

  remove_stuff();

  write_file();
  return 0;
}


/******************************************************************************
Remove an element from the list of elements in a PLY object.
******************************************************************************/

remove_element(char *elem)
{
  int i,j;

  for (i = 0; i < nelems; i++)
    if (equal_strings (elem, elist[i])) {
      for (j = i; j < nelems+1; j++) {
        elist[j] = elist[j+1];
        olist[j] = olist[j+1];
        other_props[j] = other_props[j+1];
        elem_counts[j] = elem_counts[j+1];
      }
      nelems--;
      return;
    }
}


/******************************************************************************
Remove a property from an other property description.
******************************************************************************/

remove_property(PlyOtherProp *other, int index)
{
  int i;

  for (i = index; i < other->nprops-1; i++)
    copy_property (other->props[i], other->props[i+1]);

  other->nprops--;
}


/******************************************************************************
Remove references to specified elements and properties.
******************************************************************************/

remove_stuff()
{
  int i,j,k;
  PlyOtherProp *other;
  int found_element;

  /* find and remove each element that is on the list for removal */

  for (j = 0; j < elem_count; j++) {

    found_element = 0;

    for (i = 0; i < nelems; i++)
      if (equal_strings (elist[i], elem_list[j])) {
        /* if so, remove it */
        remove_element (elem_list[j]);
        found_element = 1;
        break;
      }

    if (!found_element) {
      fprintf (stderr, "Can't find element '%s'\n", elem_list[j]);
      exit (-1);
    }
  }

  /* remove each specified property */

  for (i = 0; i < prop_count; i++) {

    found_element = 0;

    for (j = 0; j < nelems; j++) {

      if (equal_strings (elist[j], prop_elem_list[i])) {

        found_element = 1;
        other = other_props[j];

        for (k = 0; k < other->nprops; k++) {
          /* remove property by writing over it */
          if (equal_strings (prop_list[i], other->props[k]->name)) {
            remove_property (other, k);
            if (other->nprops == 0)
              remove_element (prop_elem_list[i]);
            goto here;
          }
        }

        /* if we get here, we didn't find property */
        fprintf (stderr, "Can't find property '%s.%s'\n",
                 prop_elem_list[i], prop_list[i]);
        exit (-1);
      }
      here: ; /* for jumping out of loop */
    }

    if (!found_element) {
      fprintf (stderr, "Can't find element '%s'\n", prop_elem_list[i]);
      exit (-1);
    }
  }
}


/******************************************************************************
Find from command line which properties and which elements are to be
removed.
******************************************************************************/

parse_command_line(int argc, char *argv[])
{
  int i,j;
  char *elem;
  char *prop;
  int got_prop;
  int len;

  /* Examine each argument and see if it can be split up into */
  /* a dot-separated pair of element.property.  If not, the */
  /* whole argument is taken as an element name */

  for (i = 1; i < argc; i++) {

    elem = strdup (argv[i]);
    len = strlen (elem);
    got_prop = 0;

    /* look for dot */
    for (j = 0; j < len; j++)
      if (elem[j] == '.') {
        elem[j] = '\0';
        prop = &(elem[j+1]);
        got_prop = 1;
        break;
      }

    if (got_prop) {
      prop_list[prop_count] = prop;
      prop_elem_list[prop_count] = elem;
      prop_count++;
    }
    else {
      elem_list[elem_count] = elem;
      elem_count++;
    }
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

  ply  = ply_read (stdin, &nelems, &elist);
  ply_get_info (ply, &version, &file_type);

  other_props = (PlyOtherProp **) malloc (sizeof (PlyOtherProp *) * nelems);
  olist = (OtherStruct ***) malloc (sizeof (OtherStruct **) * nelems);
  elem_counts = (int *) malloc (sizeof (int) * nelems);

  for (i = 0; i < nelems; i++) {

    /* get the description of the element */
    elem_name = elist[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);
    elem_counts[i] = num_elems;

    /* create a list of structures to hold all the elements */
    olist[i] = (OtherStruct **) malloc (sizeof (OtherStruct *) * num_elems);

    /* set up for getting the elements */

    other_props[i] = ply_get_other_properties (ply, elem_name,
                   offsetof(OtherStruct,other_props));

    /* grab all the individual elements */
    for (j = 0; j < num_elems; j++) {
      olist[i][j] = (OtherStruct *) malloc (sizeof (OtherStruct));
      ply_get_element (ply, (void *) olist[i][j]);
    }
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

  ply = ply_write (stdout, nelems, elist, file_type);

  /* describe what properties go into the vertex and face elements */

  for (i = 0; i < nelems; i++) {
    ply_element_count (ply, elist[i], elem_counts[i]);
    ply_describe_other_properties (ply, other_props[i],
                                   offsetof(OtherStruct,other_props));
  }

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  /* set up and write the elements */

  for (i = 0; i < nelems; i++) {
    /* write out elements */
    ply_put_element_setup (ply, elist[i]);
    for (j = 0; j < elem_counts[i]; j++)
      ply_put_element (ply, (void *) olist[i][j]);
  }


  /* close the PLY file */
  ply_close (ply);
}

