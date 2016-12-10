OS := $(shell uname)
all_srcs := $(shell find . -mindepth 2 -type f -name '*.nasm')
all_deps := $(shell find . -type f -name '*.d')
all_objects := $(patsubst ./%.nasm, obj/%, $(all_srcs))

all:		make_dirs $(all_objects) obj/main

make_dirs:
			echo $(OS) > platform
			unzip -n $(OS).zip

snapshot:
			zip -ru9ov -x*.d -x*.o -xobj/main $(OS).zip obj/*

obj/main:	obj/main.o
ifeq ($(OS),Darwin)
			clang -o $@ -e main $@.o -Wl,-framework,SDL2 -Wl,-framework,SDL2_ttf
endif
ifeq ($(OS),Linux)
			clang -o $@ -e main $@.o $(shell sdl2-config --libs) -lSDL2_ttf
endif

obj/main.o:	main.nasm Makefile $(all_objects)
ifeq ($(OS),Darwin)
			nasm -dOS=$(OS) -f macho64 -o $@ $<
endif
ifeq ($(OS),Linux)
			nasm -dOS=$(OS) -f elf64 -o $@ $<
endif

obj/%:		%.nasm Makefile
			nasm -dOS=$(OS) -f bin $< -o $@ -MD $@.d

-include	$(all_deps)

clean:
			rm -rf obj/ tst/
