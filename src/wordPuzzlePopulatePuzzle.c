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

unsigned int row = 0;

/*
 * Create a "products" table
 */
void create_puzzle_table(GdaConnection *cncp) {
	run_sql_non_select(cncp, "DROP table IF EXISTS puzzle");
	run_sql_non_select(cncp,
			"CREATE table puzzle (row STRING NOT NULL, col STRING NOT NULL, letter STRING NOT NULL, PRIMARY KEY (row, col))");
}

void insert_puzzle_data(GdaConnection *cncp, gint row, gint col, gchar letter) {
	typedef struct {
		gchar *row;
		gchar *col;
		gchar *letter;
	} RowData;
	RowData data = { g_strdup_printf("%i", row), g_strdup_printf("%i", col),
			g_strdup_printf("%c", letter) };

	gboolean res;
	GError *error = NULL;
	GValue *v1, *v2, *v3;

	v1 = gda_value_new_from_string(data.row, G_TYPE_STRING);
	v2 = gda_value_new_from_string(data.col, G_TYPE_STRING);
	v3 = gda_value_new_from_string(data.letter, G_TYPE_STRING);
	res = gda_connection_insert_row_into_table(cncp, "puzzle", &error, "row",
			v1, "col", v2, "letter", v3, NULL);

	if (!res) {
		g_error("Could not INSERT data into the 'puzzle' table: %s\n",
				error && error->message ? error->message : "No detail");
	}
	g_free(data.col);
	g_free(data.row);
	g_free(data.letter);

	gda_value_free(v1);
	gda_value_free(v2);
	gda_value_free(v3);

}

void insertPuzzleIntoSQLite(DBConnection *cncp, gint row, gint col,
		gchar letter) {

	if (cncp->cnc == NULL) {

		cncp->cnc = open_generic_connection(PUZZLE_DB_CONNECTION_STRING);
		//g_print("Opened connection.\n");
		create_puzzle_table(cncp->cnc);
		//g_print("Created table.\n");
		//g_printf("Populating Database...\n");
	}

	insert_puzzle_data(cncp->cnc, row, col, letter);

}

void insertLineIntoPuzzleDB(DBConnection *cncp, GString *word) {

	for (int i = 0; i < word->len; i++) {

		insertPuzzleIntoSQLite(cncp, row, i, word->str[i]);

	}

	row++;

}

int _main_wordPuzzlePopulatePuzzle(int argc, char **argv){
	gda_init();

	char *filename = NULL;
	FILE *fp;
	GString *word;

	DBConnection *cncp;
	cncp = (DBConnection*) malloc(sizeof(DBConnection));
	cncp->cnc = NULL;

	if (argc > 1) {
		filename = malloc(sizeof(argv[1]));
		strcpy(filename, argv[1]);
	}

	if (filename == NULL) {
		fp = fopen(PUZZLEFILENAMEDEFAULT, "r");
	} else {
		fp = fopen(filename, "r");
	}

	if (fp == NULL) {
		puts("Failed to open file.");
		exit(EXIT_FAILURE);
	}

	while ((word = nextWord(fp)) != NULL) {

		insertLineIntoPuzzleDB(cncp, word);

		g_string_free(word, TRUE);
	}
	if (word != NULL) {
		g_string_free(word, TRUE);
	}

	if (filename != NULL) {
		free(filename);
	}

	fclose(fp);

	if (cncp->cnc != NULL) {
		gda_connection_close(cncp->cnc);
		g_object_unref(cncp->cnc);

		//g_print("Closed connection.\n");
	}
	//g_print("Exiting...\n");

	return EXIT_SUCCESS;
}
