build: main.o gengetopt/cmdline.o
	gcc -o sort_index $^
