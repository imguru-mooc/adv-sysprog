# Chapter 2 전체 Build: make / make clean
DIRS := L06 L07 L08 L09 L10
.PHONY: all clean $(DIRS)
all: $(DIRS)
$(DIRS):
	$(MAKE) -C $@
clean:
	for d in $(DIRS); do $(MAKE) -C $$d clean; done
