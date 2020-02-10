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

gint currentdiagonal = 0;
gint n_rows = 0;
gint n_cols = 0;

typedef struct rc rc;

struct rc {
	gint r;
	gint c;
};

typedef struct coords coords;

struct coords{
	gint frst_r;
	gint frst_c;
	gint last_r;
	gint last_c;
};

gchar* getCoordsString(coords cds){
	return g_strdup_printf("(%d,%d)(%d,%d)", cds.frst_r, cds.frst_c, cds.last_r, cds.last_c);
}

void insert_possibilities_data(GdaConnection *cnc, gchar *word, gboolean wrap, coords cds) {

	gchar *falsestr = g_strdup("iFALSE");
	gchar *truestr = g_strdup("iTRUE");

	gchar* coordsstr = getCoordsString(cds);

	typedef struct {
		gchar *word;
		gchar *bool;
		gchar *coords;
	} RowData;
	RowData data = { word, (wrap == TRUE ? truestr : falsestr), coordsstr };

	GError *error = NULL;
	GValue *v1, *v2, *v3;

	v1 = gda_value_new_from_string(data.word, G_TYPE_STRING);
	v2 = gda_value_new_from_string(data.bool, G_TYPE_STRING);
	v3 = gda_value_new_from_string(data.coords, G_TYPE_STRING);
	gda_connection_insert_row_into_table(cnc, "words", &error, "word", v1,
			"wrap", v2, "coords", v3, NULL);

	gda_value_free(v1);
	gda_value_free(v2);
	gda_value_free(v3);
	g_free(falsestr);
	g_free(truestr);
	g_free(coordsstr);

}

/*
 * Create a "products" table
 */
void create_possibilities_table(GdaConnection *cnc) {
	run_sql_non_select(cnc, "DROP table IF EXISTS words");
	run_sql_non_select(cnc,
			"CREATE table words (word string not null, wrap string not null, coords string not null, PRIMARY KEY (word, wrap))");
}





gint select_max_rows(DBConnection *cncp) {
	gchar *sql;
	sql = malloc(sizeof(gchar) * 27);
	g_sprintf(sql, "select max(row) from puzzle");
	return qselecti(cncp->cnc, sql) + 1;
}

gint select_max_columns(DBConnection *cncp) {
	gchar *sql;
	sql = malloc(sizeof(gchar) * 27);
	g_sprintf(sql, "select max(col) from puzzle");
	return qselecti(cncp->cnc, sql) + 1;
}

gint nextdiagonal() {
	return currentdiagonal++;
}

void destroySeq(gpointer data) {
	g_free(data);
}

GSequence* countreversediagonallyfrom(gint row, gint col) {
	GSequence *seq;
	seq = g_sequence_new(destroySeq);
	int r = row;
	int c = col;
	for (; r < n_rows && c >= 0;) {

		rc *rcs;
		rcs = malloc(sizeof(rc));

		rcs->r = r;
		rcs->c = c;

		r++;
		c--;
		g_sequence_append(seq, rcs);

	}

	return seq;
}

GSequence* countinversereversediagonallyfrom(gint row, gint col) {
	GSequence *seq;
	seq = g_sequence_new(destroySeq);
	int r = row;
	int c = col;
	for (; r >= 0 && c < n_cols;) {

		rc *rcs;
		rcs = malloc(sizeof(rc));

		rcs->r = r;
		rcs->c = c;

		r--;
		c++;
		g_sequence_append(seq, rcs);

	}

	return seq;
}

GSequence* countdiagonallyfrom(gint row, gint col) {
	GSequence *seq;
	seq = g_sequence_new(destroySeq);
	int r = row;
	int c = col;
	for (; r < n_rows && c < n_cols;) {

		rc *rcs;
		rcs = malloc(sizeof(rc));

		rcs->r = r;
		rcs->c = c;

		r++;
		c++;
		g_sequence_append(seq, rcs);

	}

	return seq;
}

GSequence* countinversediagonallyfrom(gint row, gint col) {
	GSequence *seq;
	seq = g_sequence_new(destroySeq);
	int r = row;
	int c = col;
	for (; r >= 0 && c >= 0;) {

		rc *rcs;
		rcs = malloc(sizeof(rc));

		rcs->r = r;
		rcs->c = c;

		r--;
		c--;
		g_sequence_append(seq, rcs);

	}

	return seq;
}

GSequence* counthorizontallyfrom(gint row) {
	GSequence *seq;
	seq = g_sequence_new(destroySeq);

	for (int c = 0; c < n_cols; c++) {

		rc *rcs;
		rcs = malloc(sizeof(rc));

		rcs->r = row;
		rcs->c = c;

		g_sequence_append(seq, rcs);

	}

	return seq;
}

GSequence* countverticallyfrom(gint column) {
	GSequence *seq;
	seq = g_sequence_new(destroySeq);

	for (int r = 0; r < n_cols; r++) {

		rc *rcs;
		rcs = malloc(sizeof(rc));

		rcs->r = r;
		rcs->c = column;

		g_sequence_append(seq, rcs);

	}

	return seq;
}

gchar* getString(DBConnection *cncp, gchar *final, GSequenceIter *iter) {
	rc *rcs = g_sequence_get(iter);

	gchar *sql = g_strdup_printf(
			"SELECT letter FROM puzzle WHERE row=\"%i\" AND col=\"%i\"", rcs->r,
			rcs->c);

	gchar *rslt = qselects(cncp->cnc, sql);

	gchar *oldfinal = final;

	final = g_strconcat(final, rslt, NULL);

	g_free(oldfinal);
	g_free(rslt);
	return final;
}

gchar* buildFromSequence(DBConnection *cncp, GSequence *seq) {

	if (g_sequence_get_length(seq) == 0) {
		return NULL;
	}

	gchar *final = g_strdup("");

	GSequenceIter *iter = g_sequence_get_begin_iter(seq);

	do {

		final = getString(cncp, final, iter);
		iter = g_sequence_iter_next(iter);

	} while (!g_sequence_iter_is_end(iter));

	return final;
}

gchar* buildFromSequenceReverse(DBConnection *cncp, GSequence *seq) {

	if (g_sequence_get_length(seq) == 0) {
		return NULL;
	}

	gchar *final = g_strdup("");

	GSequenceIter *iter = g_sequence_get_end_iter(seq);
	iter = g_sequence_iter_prev(iter);
	do {

		final = getString(cncp, final, iter);
		iter = g_sequence_iter_prev(iter);

	} while (!g_sequence_iter_is_begin(iter));

	if (g_sequence_get_begin_iter(seq)
			!= g_sequence_iter_prev(g_sequence_get_end_iter(seq))) {
		final = getString(cncp, final, iter);
	}

	return final;
}

char* substring(char *string, int position, int length) {
	char *pointer;
	int c;

	pointer = malloc(length + 1);

	if (pointer == NULL) {
		printf("Unable to allocate memory.\n");
		exit(1);
	}

	for (c = 0; c < length; c++) {
		*(pointer + c) = *(string + position - 1);
		string++;
	}

	*(pointer + c) = '\0';

	return pointer;
}

coords adjustForWrap(coords cds){

	cds.frst_r = cds.frst_r % (n_rows/3);
	cds.frst_c = cds.frst_c % (n_cols/3);
	cds.last_r = cds.last_r % (n_rows/3);
	cds.last_c = cds.last_c % (n_cols/3);

	return cds;
}
coords getCoords(GSequence *seq, char straightorreverse, gint startofseq,
		gint lengthofseq, gboolean wrap) {

	coords cds;

	gint seqlen = g_sequence_get_length(seq);

	if (straightorreverse == 's') {
		GSequenceIter *startseq = g_sequence_get_iter_at_pos(seq, startofseq);
		GSequenceIter *endseq = g_sequence_get_iter_at_pos(seq,
				startofseq + lengthofseq - 1);
		cds.frst_r = ((rc*) g_sequence_get(startseq))->r;
		cds.frst_c = ((rc*) g_sequence_get(startseq))->c;
		cds.last_r = ((rc*) g_sequence_get(endseq))->r;
		cds.last_c = ((rc*) g_sequence_get(endseq))->c;
	} else {
		GSequenceIter *startseq = g_sequence_get_iter_at_pos(seq,
						seqlen - startofseq - 1);
		GSequenceIter *endseq = g_sequence_get_iter_at_pos(seq,
				(seqlen - (startofseq + lengthofseq - 1)) - 1);
		cds.frst_r = ((rc*) g_sequence_get(startseq))->r;
		cds.frst_c = ((rc*) g_sequence_get(startseq))->c;
		cds.last_r = ((rc*) g_sequence_get(endseq))->r;
		cds.last_c = ((rc*) g_sequence_get(endseq))->c;
	}
	return (wrap ? adjustForWrap(cds) : cds);

}

void allsubs(DBConnection *cncp, char *string, gboolean wrap, char direction,
		char straightorreverse, GSequence *seq) {

	if (string == NULL) {
		return;
	}

	char *pointer;
	int position = 1, length = 1, temp, string_length;
	temp = string_length = strlen(string);

	int startofseq = 0;

	while (position <= string_length) {

		int lengthofseq = 1;

		while (length <= temp) {
			pointer = substring(string, position, length);


			if (direction == 'h') {
				if (strlen(pointer) <= (wrap ? (n_cols / 3) : n_cols ) ) {

					coords cds = getCoords(seq, straightorreverse, startofseq, lengthofseq, wrap);
#ifdef DEBUG
				g_print("inserting: %s (%d,%d)(%d,%d)\n", pointer, cds.frst_r, cds.frst_c, cds.last_r, cds.last_c);
#endif
					insert_possibilities_data(cncp->cnc, pointer, wrap, cds);

				}
			} else if (direction == 'v') {
				if (strlen(pointer) <= (wrap ? (n_rows / 3) : n_rows ) ) {

					coords cds = getCoords(seq, straightorreverse, startofseq, lengthofseq, wrap);
#ifdef DEBUG
				g_print("inserting: %s (%d,%d)(%d,%d)\n", pointer, cds.frst_r, cds.frst_c, cds.last_r, cds.last_c);
#endif
					insert_possibilities_data(cncp->cnc, pointer, wrap, cds);
				}
			} else if (direction == 'd') {

				coords cds = getCoords(seq, straightorreverse, startofseq, lengthofseq, wrap);
#ifdef DEBUG
				g_print("inserting: %s (%d,%d)(%d,%d)\n", pointer, cds.frst_r, cds.frst_c, cds.last_r, cds.last_c);
#endif
				insert_possibilities_data(cncp->cnc, pointer, wrap, cds);
				//}
			}

			lengthofseq++;
			free(pointer);
			length++;
		}
		temp--;
		position++;
		length = 1;
		startofseq++;
	}
}

void horizontal(DBConnection *cncp, DBConnection *cncpp, int r, gboolean wrap) {
	char *somestr;
	GSequence *gseq;

	gseq = counthorizontallyfrom(r);
	somestr = buildFromSequence(cncp, gseq);
#ifdef DEBUG
	if ( strlen(somestr) != g_sequence_get_length(gseq) ) { g_error("11Sequence size must be equal to string length.\n"); }
#endif
	allsubs(cncpp, somestr, wrap, 'h', 's', gseq);
	g_free(somestr);
	g_sequence_free(gseq);

	gseq = counthorizontallyfrom(r);
	somestr = buildFromSequenceReverse(cncp, gseq);
#ifdef DEBUG
	if ( strlen(somestr) != g_sequence_get_length(gseq) ) { g_error("10Sequence size must be equal to string length.\n"); }
#endif
	allsubs(cncpp, somestr, wrap, 'h', 'r', gseq);
	g_free(somestr);
	g_sequence_free(gseq);

}

void vertical(DBConnection *cncp, DBConnection *cncpp, int c, gboolean wrap) {
	char *somestr;
	GSequence *gseq;

	gseq = countverticallyfrom(c);
	somestr = buildFromSequence(cncp, gseq);
#ifdef DEBUG
	if ( strlen(somestr) != g_sequence_get_length(gseq) ) { g_error("9Sequence size must be equal to string length.\n"); }
#endif
	allsubs(cncpp, somestr, wrap, 'v', 's', gseq);
	g_free(somestr);
	g_sequence_free(gseq);

	gseq = countverticallyfrom(c);
	somestr = buildFromSequenceReverse(cncp, gseq);
#ifdef DEBUG
	if ( strlen(somestr) != g_sequence_get_length(gseq) ) { g_error("8Sequence size must be equal to string length.\n"); }
#endif
	allsubs(cncpp, somestr, wrap, 'v', 'r', gseq);
	g_free(somestr);
	g_sequence_free(gseq);
}

void diagonal(DBConnection *cncp, DBConnection *cncpp, int r, int c, gboolean wrap) {
	char *somestr;
	GSequence *gseq;

	gseq = countdiagonallyfrom(r, c);
	somestr = buildFromSequence(cncp, gseq);
#ifdef DEBUG
	if ( strlen(somestr) != g_sequence_get_length(gseq) ) { g_error("0Sequence size must be equal to string length.\n"); }
#endif
	allsubs(cncpp, somestr, wrap, 'd', 's', gseq);
	g_free(somestr);
	g_sequence_free(gseq);

	gseq = countdiagonallyfrom(r, c);
	somestr = buildFromSequenceReverse(cncp, gseq);
#ifdef DEBUG
	if ( strlen(somestr) != g_sequence_get_length(gseq) ) { g_error("1Sequence size must be equal to string length.\n"); }
#endif
	allsubs(cncpp, somestr, wrap, 'd', 'r', gseq);
	g_free(somestr);
	g_sequence_free(gseq);

	gseq = countinversediagonallyfrom(r, c);
	somestr = buildFromSequence(cncp, gseq);
#ifdef DEBUG
	if ( strlen(somestr) != g_sequence_get_length(gseq) ) { g_error("2Sequence size must be equal to string length.\n"); }
#endif
	allsubs(cncpp, somestr, wrap, 'd', 's', gseq);
	g_free(somestr);
	g_sequence_free(gseq);

	gseq = countinversediagonallyfrom(r, c);
	somestr = buildFromSequenceReverse(cncp, gseq);
#ifdef DEBUG
	if ( strlen(somestr) != g_sequence_get_length(gseq) ) { g_error("3Sequence size must be equal to string length.\n"); }
#endif
	allsubs(cncpp, somestr, wrap, 'd', 'r', gseq);
	g_free(somestr);
	g_sequence_free(gseq);

	gseq = countreversediagonallyfrom(r, c);
	somestr = buildFromSequence(cncp, gseq);
#ifdef DEBUG
	if ( strlen(somestr) != g_sequence_get_length(gseq) ) { g_error("4Sequence size must be equal to string length.\n"); }
#endif
	allsubs(cncpp, somestr, wrap, 'd', 's', gseq);
	g_free(somestr);
	g_sequence_free(gseq);

	gseq = countreversediagonallyfrom(r, c);
	somestr = buildFromSequenceReverse(cncp, gseq);
#ifdef DEBUG
	if ( strlen(somestr) != g_sequence_get_length(gseq) ) { g_error("5Sequence size must be equal to string length.\n"); }
#endif
	allsubs(cncpp, somestr, wrap, 'd', 'r', gseq);
	g_free(somestr);
	g_sequence_free(gseq);

	gseq = countinversereversediagonallyfrom(r, c);
	somestr = buildFromSequence(cncp, gseq);
#ifdef DEBUG
	if ( strlen(somestr) != g_sequence_get_length(gseq) ) { g_error("6Sequence size must be equal to string length.\n"); }
#endif
	allsubs(cncpp, somestr, wrap, 'd', 's', gseq);
	g_free(somestr);
	g_sequence_free(gseq);

	gseq = countinversereversediagonallyfrom(r, c);
	somestr = buildFromSequenceReverse(cncp, gseq);
#ifdef DEBUG
	if ( strlen(somestr) != g_sequence_get_length(gseq) ) { g_error("7Sequence size must be equal to string length.\n"); }
#endif
	allsubs(cncpp, somestr, wrap, 'd', 'r', gseq);
	g_free(somestr);
	g_sequence_free(gseq);

}

void buildFromSequences(DBConnection *cncp, DBConnection *cncpp, gboolean wrap) {

	// column 0
	for (int r = 0; r < n_rows; r++) {
		horizontal(cncp, cncpp, r, wrap);

		int c = 0;
		diagonal(cncp, cncpp, r, c, wrap);

	}

	// column n_col
	for (int r = 0; r < n_rows; r++) {

		int c = n_cols - 1;
		diagonal(cncp, cncpp, r, c, wrap);

	}

	for (int c = 0; c < n_cols; c++) {
		vertical(cncp, cncpp, c, wrap);

		int r = 0;
		diagonal(cncp, cncpp, r, c, wrap);

	}

	for (int c = 0; c < n_cols; c++) {

		int r = n_rows - 1;
		diagonal(cncp, cncpp, r, c, wrap);

	}
}

int _main_wordPuzzlePopulatePossibilities(gboolean wrap) {
	gda_init();
	//g_print("word puzzle possibilities.\n");

	DBConnection *cncp;
	cncp = (DBConnection*) malloc(sizeof(DBConnection));
	cncp->cnc = NULL;

	cncp->cnc = open_generic_connection(PUZZLE_DB_CONNECTION_STRING);
	//g_print("Opened connection.\n");

	DBConnection *cncpp;
	cncpp = (DBConnection*) malloc(sizeof(DBConnection));
	cncpp->cnc = NULL;

	cncpp->cnc = open_generic_connection(POSSIBILITIES_DB_CONNECTION_STRING);
	//g_print("Opened connection.\n");

	create_possibilities_table(cncpp->cnc);

	n_rows = select_max_rows(cncp);
	n_cols = select_max_columns(cncp);

	//g_print("n_rows: %d\n", n_rows);
	//g_print("n_cols: %d\n", n_cols);

	buildFromSequences(cncp, cncpp, wrap);

	gda_connection_close(cncp->cnc);
	g_object_unref(cncp->cnc);

	//g_print("Closed connection.\n");
	//g_print("Exiting...\n");

	gda_connection_close(cncpp->cnc);
	g_object_unref(cncpp->cnc);

	//g_print("Closed connection.\n");
	//g_print("Exiting...\n");

	return EXIT_SUCCESS;
}









