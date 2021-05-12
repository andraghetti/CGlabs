CC = gcc
CFLAGS = -Wall -Wdeclaration-after-statement -Wno-unused -Wextra #-ansi #-pedantic 
LDLIBS = -lGL -lGLU -lglut

UNAME = $(shell uname)
ifeq ($(UNAME), Darwin)  # Mac OS X
LDLIBS = -framework OpenGL -framework GLUT
endif

BINS = drawSplines lab-03

all: $(BINS)

drawSplines: evalSpline.o
lab-03: evalSpline.o

clean:
	@rm -f *.o *~ $(BINS)
