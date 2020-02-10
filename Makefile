CFLAGS = -Wall -g -DGDA_DISABLE_DEPRECATED `pkg-config --cflags libgda-5.0`
LDFLAGS = `pkg-config --libs libgda-5.0` -lm

all: wordPuzzle

wordPuzzle: src/wordPuzzleUtil.c src/wordPuzzle.c src/wordPuzzlePopulatePuzzle.c src/wordPuzzlePopulatePossibilities.c src/wordPuzzleChecker.c
	$(CC) -o wordPuzzle src/wordPuzzleUtil.c src/wordPuzzle.c src/wordPuzzlePopulatePuzzle.c src/wordPuzzlePopulatePossibilities.c src/wordPuzzleChecker.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *~
	rm -f *.o
	rm -f wordPuzzle
	rm -f wordPuzzleChecker
	rm -f wordPuzzlePopulatePuzzle        
	rm -f wordPuzzlePopulatePossibilities

	