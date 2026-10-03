# Chapter 1 전체 Build: make / make clean
DIRS := L01 L02 L03 L04 L05
.PHONY: all clean $(DIRS)
all: $(DIRS)
$(DIRS):
	$(MAKE) -C $@
clean:
	for d in $(DIRS); do $(MAKE) -C $$d clean; done
