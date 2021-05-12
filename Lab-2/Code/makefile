CC      = gcc
CFLAGS = -Wall -Wdeclaration-after-statement -Wno-unused -Wextra #-ansi #-pedantic 
LDLIBS = -lm -lGL -lGLU -lglut

UNAME = $(shell uname)
ifeq ($(UNAME), Darwin)  # Mac OS X
LDLIBS = -framework OpenGL -framework GLUT
endif

all: model_viewer

clean:
	rm -rf *.o *~ model_viewer
