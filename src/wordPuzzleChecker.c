/*
 * wordPuzzle.c
 *
 *  Created on: Feb 7, 2020
 *      Author: btardio
 */

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <libgda/libgda.h>
#include <glib/gprintf.h>
#include "simpleWordPuzzle.h"

gboolean qselect2s(GdaConnection *cncp, gchar *sql, gchar **first, gchar **second) {

	GdaDataModel *data_model = selectinternal(cncp, sql);

	GdaDataModelIter *iter = gda_data_model_create_iter(data_model);
	if ( TRUE != gda_data_model_iter_move_next(iter)) {
		//g_print("1st ERROR\n");
		*first = g_strdup_printf("Not Found");
		*second = g_strdup_printf("Not Found");
		return FALSE;
	}

	GError *err = NULL;

	const GValue *gfirst = gda_data_model_iter_get_value_at_e(iter, 0, &err);

	if (err != NULL) {
		g_error("Error: %s\n", err->message);
	}

	const GValue *gsecond = gda_data_model_iter_get_value_at_e(iter, 1, &err);

	if (err != NULL) {
		g_error("Error: %s\n", err->message);
	}

	*first = (gchar*) get_gvalue_contents_as_string(gfirst);
	*second = (gchar*) get_gvalue_contents_as_string(gsecond);

	g_object_unref(iter);
	g_object_unref(data_model);

	return TRUE;

}

void selectFromSQLite(DBConnection *cncp, gchar *word) {

	if (cncp->cnc == NULL) {

		cncp->cnc = open_generic_connection(POSSIBILITIES_DB_CONNECTION_STRING);
		//g_print("Opened connection.\n");

	}

	gchar *returnword;
	gchar *returncoords;

	gchar *sql = g_strdup_printf(
			"SELECT word, coords FROM words WHERE word=\"%s\"", word);

	if ( qselect2s(cncp->cnc, sql, &returnword, &returncoords) ){
		g_print("    found: %s at %s\n", returnword, returncoords);
	} else {
		g_print("not found: %s\n", word);
	}

	g_free(sql);
	g_free(returnword);
	g_free(returncoords);
}

int _main_wordPuzzleChecker(int argc, char **argv) {

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
		fp = fopen(PUZZLEWORDS, "r");
	} else {
		fp = fopen(filename, "r");
	}

	if (fp == NULL) {
		puts("Failed to open file.");
		exit(EXIT_FAILURE);
	}

	while ((word = nextWord(fp)) != NULL) {

		selectFromSQLite(cncp, word->str);

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
