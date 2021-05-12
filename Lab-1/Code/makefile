CFLAGS = -Wall -Wdeclaration-after-statement -Wno-unused -Wextra #-ansi #-pedantic 
LDLIBS = -lm -lGL -lGLU -lglut #-lX11 -lXmu

BINS = connectDots 

all: $(BINS)

clean:
	-rm -f *.o *~ $(BINS)

%.o: %.c
	@echo -ne 'compiling $<'
	cc -c $(CFLAGS) $<

%: %.o
	@echo -ne '\nlinking\n'	$^
	cc $(LDFLAGS) -o$@ $^ $(LDLIBS)

%: %.c
	@echo -ne '\ncompiling and linking $@\n'
	cc $(CFLAGS) $(LDFLAGS) -o$@ $^ $(LDLIBS)

#%.o : %.c
#	cc -c $(CFLAGS) $< -o $@
#%: %.o  
#	cc $(LDFLAGS) -o$@ $^ $(LDLIBS)
