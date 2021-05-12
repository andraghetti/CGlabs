apps = BUMPMAP CUBE CUBEMAP CUBESHADE CUBETEX MORPH NONPHOTO PARTICLE PHONG WAVE

all:
	for app in $(apps); do $(MAKE) -C $${app}; done;

clean:
	for app in $(apps); do $(MAKE) -C $$app clean; done;
