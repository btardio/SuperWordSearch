#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gmodule.h>
#include <errno.h>
#include "simpleWordPuzzle.h"

#include <libgda/libgda.h>
#include <sql-parser/gda-sql-parser.h>

void populateWordsNeedFound(GArray *words, gint size) {
	FILE *fp;

	fp = fopen(PUZZLEWORDS, "w");

	if (fp == NULL) {
		puts("Failed to open file.");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < size; i++) {

		fprintf(fp, "%s\n", g_array_index(words, gchar*, i));
	}

	fclose(fp);

}

void populateInternalPuzzle(GArray *words, gboolean wrap, gint size) {
	FILE *fp;

	fp = fopen(PUZZLEFILENAMEDEFAULT, "w");

	if (fp == NULL) {
		puts("Failed to open file.");
		exit(EXIT_FAILURE);
	}

	if (wrap == FALSE) {
		for (int i = 0; i < size; i++) {

			fprintf(fp, "%s\n", g_array_index(words, gchar*, i));
		}
	} else if (wrap == TRUE) {

		for (int i = 0; i < size * 3; i++) {

			fprintf(fp, "%s%s%s\n", g_array_index(words, gchar*, i % size),
					g_array_index(words, gchar*, i % size),
					g_array_index(words, gchar*, i % size));
		}

	}

	fclose(fp);

}

int main(int argc, char **argv) {

	g_print("~!~ Word Puzzle ~!~\n\n");

	g_print("The default input file is called \"puzzle\"\n");
	g_print(
			"You can specify a different filename with the first command line argument ie: wordPuzzle <filename>.\n");
	g_print("Example Input Format:\n");

	g_print("3 3\n");
	g_print("ABC\n");
	g_print("DEF\n");
	g_print("GHI\n");
	g_print("NO_WRAP\n");
	g_print("5\n");
	g_print("FED\n");
	g_print("CAB\n");
	g_print("GAD\n");
	g_print("BID\n");
	g_print("HIGH\n");

	g_print("\n");

	g_print("Proceed? [Y/N]: ");
	char *p;
	int n;

	errno = 0;
	n = scanf("%m[a-zA-Z]", &p);
	if (n == 1) {
		if (!(strlen(p) > 0 && (p[0] == 'y' || p[0] == 'Y'))) {
			free(p);
			return 0;
		}
		free(p);

	} else if (errno != 0) {
		perror("scanf");
	} else {
		fprintf(stderr, "Please input Y or N.\n");
		return -1;
	}

	g_print("Starting...\n");

	char *filename = NULL;
	FILE *fp;
	GString *word;

	if (argc > 1) {
		filename = malloc(sizeof(argv[1]));
		strcpy(filename, argv[1]);
	}

	if (filename == NULL) {
		fp = fopen("puzzle", "r");
	} else {
		fp = fopen(filename, "r");
	}

	if (fp == NULL) {
		puts("Failed to open file.");
		exit(EXIT_FAILURE);
	}

	// for lines of puzzle

	word = nextWord(fp);

	gchar **split = g_strsplit(word->str, " ", 2);

	gint n_linesinpuzzle = g_ascii_strtoll(split[0], NULL, 10);

	g_free(split[0]);
	g_free(split[1]);
	g_string_free(word, TRUE);

	// for internal/puzzle

	GArray *words = g_array_sized_new(FALSE, FALSE, sizeof(gchar*),
			n_linesinpuzzle);

	for (int i = 0; i < n_linesinpuzzle; i++) {
		word = nextWord(fp);
		g_array_insert_val(words, i, word->str);
		g_string_free(word, FALSE);
	}

	// for wrap

	gboolean wrap = FALSE;

	word = nextWord(fp);

	if (strncmp(word->str, "WRAP", 4) == 0) {
		wrap = TRUE;
	}
	g_string_free(word, TRUE);

	//for lines of words needing found

	word = nextWord(fp);

	split = g_strsplit(word->str, " ", 2);

	gint n_lineswordsneedfound = g_ascii_strtoll(split[0], NULL, 10);

	g_free(split[0]);
	g_free(split[1]);
	g_string_free(word, TRUE);

	// for internal/puzzlewords

	GArray *wordsneedfound = g_array_sized_new(FALSE, FALSE, sizeof(gchar*),
			n_linesinpuzzle);

	for (int i = 0; i < n_lineswordsneedfound; i++) {
		word = nextWord(fp);

		g_array_insert_val(wordsneedfound, i, word->str);

		g_string_free(word, FALSE);
	}

	// populate internal/puzzle
	populateInternalPuzzle(words, wrap, n_linesinpuzzle);
	g_array_free(words, TRUE);

	// populate puzzlewords
	populateWordsNeedFound(wordsneedfound, n_lineswordsneedfound);
	g_array_free(wordsneedfound, TRUE);

	if (filename != NULL) {
		free(filename);
	}

	fclose(fp);

	// run the other programs
	g_print("populating puzzle\n");
	_main_wordPuzzlePopulatePuzzle(0, NULL);

	g_print("populating possibilities (this takes awhile)\n");
	_main_wordPuzzlePopulatePossibilities(wrap);

	g_print("puzzle checker\n");
	_main_wordPuzzleChecker(0, NULL);

	g_print("Exiting...\n");
	return EXIT_SUCCESS;

}
