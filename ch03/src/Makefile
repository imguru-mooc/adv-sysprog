# Chapter 3 전체 Build: make / make clean
DIRS := L11 L12 L13 L14 L15
.PHONY: all clean $(DIRS)
all: $(DIRS)
$(DIRS):
	$(MAKE) -C $@
clean:
	for d in $(DIRS); do $(MAKE) -C $$d clean; done
