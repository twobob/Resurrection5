all: dehumanise

dehumanise:
	gcc -c dehumanise.c

	gcc -o dehuman -g -lm -lsndfile dehumanise.o

clean:
	rm -f *.o  dehuman
