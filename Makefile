default:all

all:main

main:mytest.o
	g++  $^ -o $@ 

# %.o:%.c
# 	gcc -c $^ -o $@  -g
mytest.o:mytest.cpp
	g++ -c $^ -o $@  -g


.PHONY:
clean:
	rm *.o main
