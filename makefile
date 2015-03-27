CC = g++
CFLAGS = -lGL -lGLU -lglut
all:	HW1
HW1:	HW1-QiyaoWang.cpp
	$(CC) -o $@ $< $(CFLAGS)
clean:
	rm -rf *o HW1



