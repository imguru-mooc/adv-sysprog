# Chapter 8 전체 Build: make / make clean
DIRS := L36 L37 L38 L39 L40
.PHONY: all clean $(DIRS)
all: $(DIRS)
$(DIRS):
	$(MAKE) -C $@
clean:
	for d in $(DIRS); do $(MAKE) -C $$d clean; done
