# Chapter 5 전체 Build: make / make clean
DIRS := L21 L22 L23 L24 L25
.PHONY: all clean $(DIRS)
all: $(DIRS)
$(DIRS):
	$(MAKE) -C $@
clean:
	for d in $(DIRS); do $(MAKE) -C $$d clean; done
