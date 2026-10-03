# Chapter 7 전체 Build: make / make clean
DIRS := L31 L32 L33 L34 L35
.PHONY: all clean $(DIRS)
all: $(DIRS)
$(DIRS):
	$(MAKE) -C $@
clean:
	for d in $(DIRS); do $(MAKE) -C $$d clean; done
