build:
	make -C consola
	make -C kernel
	make -C cpu
	make -C filesystem
	make -C memoria

debug:
	make debug -C consola
	make debug -C kernel
	make debug -C cpu
	make debug -C filesystem
	make debug -C memoria

clean:
	make clean -C consola
	make clean -C kernel
	make clean -C cpu
	make clean -C filesystem
	make clean -C memoria

rebuild: clean build
