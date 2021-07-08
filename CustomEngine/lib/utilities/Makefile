############;-*-fundamental-*-###############################################
# 
#               Makefile for PLY library and simple applications
#               ================================================
#
#############################################################################


# These are the names of various programs used during "making".
# Pick whichever flavor of C you like with the CC variable.

CC	= gcc
RM	= rm -f
CO	= co
YACC    = yacc
LEX     = lex

# Useful paths
PROJ            = ../..

# Name of the makefile
MAKEFILE	= Makefile

# INCLUDE supplies paths to header files.
INCLUDE		= -I. 

# optimization flags 
OPTIMIZE	= -O3
#OPTIMIZE       = -g

# DBFLAGS contains flags for debugging code
#DBFLAGS         = -pg
DBFLAGS         =

# CFLAGS is all of the compiler options.  Put any extras here.
CFLAGS		=  $(OPTIMIZE) $(INCLUDE) $(DBFLAGS) #-Wall

# LDFLAGS contains all linker flags, which is mostly just paths to libraries.
LDFLAGS		= 

# These are the libraries linked to the program.  example: -lm
LIBS		= -lm 
#-lmalloc

# Flags used during the header file dependency generating stage.
DEPENDFLAGS	= $(CFLAGS) -Em

#------------------------------ Lists of Files -------------------------------#

# Groups of source files
PLY_SRCS = igrip2ply.c				\
	   movie.byu2ply.c			\
	   ply2igrip.c				\
	   ply2pol.c				\
	   ply2pphigs.c				\
	   ply2stl.c				\
	   plybound.c				\
	   plycomps.c				\
	   plyconvert.c				\
	   plycrunch.c				\
	   plycurvaturecolor.c			\
	   plycurvatures.c			\
	   plyfile.c				\
	   plyflip.c				\
	   plyindex.c				\
	   plymark.c				\
	   plynormals.c				\
	   plyorient.c				\
	   plyremove.c				\
	   plyremoveface.c			\
	   plyremovevert.c			\
	   plyshared.c				\
	   plyskel.c				\
	   plytriangulate.c			\
	   plyunshared.c			\
	   plyvertepsilons.c			\
	   plyxform.c				\
	   stl2ply.c  


# Groups of header files
SIMPLIFY_HDRS = $(SRCS:.c=.h) 

#Other files (RCS files like lex/yacc that aren't source or header)
OTHER_FILES       = 

# Groups of object files
SIMPLIFY_OBJS = $(SIMPLIFY_SRCS:.c=.o) 


#All source files not including generated ones 
ALL_SOURCES     = $(SIMPLIFY_SRCS) 

#All header files
ALL_HEADERS     = $(SIMPLIFY_HDRS) 

#All RCS files
ALL_RCS_FILES	= $(MAKEFILE) $(ALL_SOURCES) $(ALL_HEADERS) $(OTHER_FILES)

#All executables
EXECUTABLES       = igrip2ply				\
	   	    movie.byu2ply			\
                    ply2ascii                           \
                    ply2binary                          \
	   	    ply2igrip				\
	   	    ply2pol				\
	   	    ply2pphigs				\
	   	    ply2stl				\
	   	    plybound				\
	   	    plycomps				\
	   	    plycrunch				\
	   	    plycurvaturecolor			\
	   	    plycurvatures			\
	   	    plyflip				\
	   	    plyindex				\
	   	    plymark				\
	   	    plynormals				\
	   	    plyorient				\
	   	    plyremove				\
	   	    plyremoveface			\
	   	    plyremovevert			\
	   	    plyshared				\
	   	    plyskel				\
	   	    plytriangulate			\
	   	    plyunshared				\
	   	    plyvertepsilons			\
	   	    plyxform				\
	   	    stl2ply  


#The compiled library
LIB = libply.a

#--------------------------------- Main Rules --------------------------------#

#IMPORTANT - the object files must come BEFORE the libraries!!!

all: $(LIB) $(EXECUTABLES) 

libply.a: plyfile.c ply.h
	$(CC) $(CFLAGS) -o plyfile.o -c plyfile.c
	ar cruv libply.a plyfile.o 
	-ranlib libply.a
	@$(RM) plyfile.o

ply2ascii: plyconvert.c $(LIB) $(MAKEFILE)
# AFS on HPs won't overwrite executable, so remove
	@$(RM) $@    
	$(CC) $(CFLAGS) -o $@ plyconvert.c -DWRITE_ASCII $(LIB) $(LIBS)

ply2binary: plyconvert.c $(LIB) $(MAKEFILE)
# AFS on HPs won't overwrite executable, so remove first
	@$(RM) $@    
	$(CC) $(CFLAGS) -o $@ plyconvert.c -DWRITE_BINARY $(LIB) $(LIBS)


.c : 
# AFS on HPs won't overwrite executable, so remove first
	@$(RM) $@    
	$(CC) $(CFLAGS) -o $@ $@.c $(LIB) $(LIBS)

#--------------------------------- Handy Rules -------------------------------#

# Run lint on all of the source files.
lint : 
	lint $(LINT_FLAGS) $(ALL_SOURCES)

# Remove any intermediate files.
clean :
	@-$(RM) *.o *~ *% $(EXECUTABLES) $(LIB)


# Strip and move the executables to the /bin directory.
install:
	strip $(EXECUTABLES); mv $(EXECUTABLES) $(PROJ)/bin
	mv $(LIB) $(PROJ)/lib
	$(RM) $(PROJ)/include/ply.h; cp ply.h $(PROJ)/include

# Empty Rule
dummy:


#------------------------------RCS Rules--------------------------------------#

# Check out any new source files from RCS.
# uses RCS make depend rules to know how to check out
co: Makefile dummy
	make `for f in RCS/*,v ; do basename $$f ,v ; done`

	
# Check out copies of every file under RCS.  This does NOT lock them.
co_all: 
	co -u RCS/*

	
# check out a named revision of all RCS files
# use "make co_named NAME=name_to_get"
# dies on the undescriptive line [ "" != "" ] if no name is given
co_named:
	[ "$(NAME)" != "" ]
	co -r$(NAME) RCS/*

	
# give name to current revision of all RCS files
# make with "make named_rev NAME=name_to_give"
# dies on the undescriptive line [ "" != "" ] if no name is given
# does weird perl/touch stuff to not cause global checkout
named_rev:
	[ "$(NAME)" != "" ]
	for f in RCS/* ; do \
	  tim=`perl -e '($$m,$$h,$$d,$$mo,$$y) =' \
		-e '(localtime((stat("$$ARGV[0]"))[8]))[1..5];' \
	      -e 'printf("%02d%02d%02d%02d%02d\n",$$mo+1,$$d,$$h,$$m,$$y);' \
	      $$f`; \
	  rev=`rlog -h $$f | awk '{if ($$1 == "head:") print $$2}'`; \
	  rcs -N$(NAME):$$rev $$f; \
	  chmod +w $$f; touch $$tim $$f; chmod -w $$f; \
	done

#-----------------------------Dependency Rules--------------------------------#

# This one's from the pxpl5 makefiles
# It tacks dependencies for header files and RCS onto the end of the makefile.

depend: Makefile $(ALL_RCS_FILES)
	makedepend -- $(CFLAGS) -- $(ALL_SOURCES)
	-if [ -d RCS ] ; then \
	  echo ; echo '# RCS dependencies' ; echo -n '.PRECIOUS: \\' ; \
	  for f in RCS/*,v ; do \
	    echo `basename $$f ,v` ; \
	  done | awk  '{if (length(line $$1) < 65) \
			  line = line $$1 " "; \
			else {print line,"\\"; line = $$1 " "}} \
		      END {print line}' ; echo ; \
	  for f in RCS/*,v ; do \
	    echo "`basename $$f ,v`: $$f ; \$$(CO) \$$?" ; \
	  done ; \
	fi >> Makefile

# DO NOT DELETE THIS LINE -- make depend depends on it.
