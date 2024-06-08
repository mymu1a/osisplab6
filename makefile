.PHONY: build generate
	
build: main.o gengetopt/cmdline.o
	gcc -o sort_index $^

generate: generate.o gengetopt/generate_cmdline.o
	gcc -o generate $^
