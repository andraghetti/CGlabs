CC = gcc
CFLAGS = -g3 -O0 -Wall -Wdeclaration-after-statement -Wno-unused -Wextra #-ansi #-pedantic 
LDLIBS = -lm -lGL -lGLU -lglut

UNAME = $(shell uname)
ifeq ($(UNAME), Darwin)  # Mac OS X
LDLIBS = -framework OpenGL -framework GLUT
endif

BINS = lab-05

all: $(BINS)

lab-05: RgbImage.o

clean:
	@rm -f *.o *~ $(BINS)

