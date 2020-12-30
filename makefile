.PHONY: src/utils/test.o

src/utils/test.o:
	g++ -c -O3  src/utils/test.cpp -o src/utils/test.o -I libs/fmt/include/
