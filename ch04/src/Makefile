# Chapter 4 전체 Build: make / make clean
DIRS := L16 L17 L18 L19 L20
.PHONY: all clean $(DIRS)
all: $(DIRS)
$(DIRS):
	$(MAKE) -C $@
clean:
	for d in $(DIRS); do $(MAKE) -C $$d clean; done
