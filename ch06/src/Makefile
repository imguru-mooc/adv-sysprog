# Chapter 6 전체 Build: make / make clean
DIRS := L26 L27 L28 L29 L30
.PHONY: all clean $(DIRS)
all: $(DIRS)
$(DIRS):
	$(MAKE) -C $@
clean:
	for d in $(DIRS); do $(MAKE) -C $$d clean; done
