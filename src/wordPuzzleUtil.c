#include <stdio.h>
#include <glib-object.h>
#include <libgda/libgda.h>

/*
 * Reads lines of a file returning them as a GString
 */
GString* nextWord(FILE *fp) {
	char *line = NULL;
	size_t len = 0;

	while ((getline(&line, &len, fp)) != -1) {

		line = g_strstrip(line);
		GString *linestr = g_string_new(line);

		return linestr;
	}
	return NULL;
}

GdaConnection*
open_generic_connection(char *connectString) {
	GdaConnection *cncp;
	GError *error = NULL;
	GdaSqlParser *parser;

	/* open connection */
	cncp = gda_connection_open_from_string("SQLite", connectString, NULL,
			GDA_CONNECTION_OPTIONS_NONE, &error);
	if (!cncp) {
		g_print("Could not open connection to SQLite database file: %s | %s\n",
				connectString,
				error && error->message ? error->message : "No detail");
		exit(1);
	}

	/* create an SQL parser */
	parser = gda_connection_create_parser(cncp);
	if (!parser) /* @cnc does not provide its own parser => use default one */
		parser = gda_sql_parser_new();
	/* attach the parser object to the connection */
	g_object_set_data_full(G_OBJECT(cncp), "parser", parser, g_object_unref);

	return cncp;
}

/*
 * run a non SELECT command and stops if an error occurs
 */
void run_sql_non_select(GdaConnection *cnc, const gchar *sql) {
	GdaStatement *stmt;
	GError *error = NULL;
	gint nrows;
	const gchar *remain;
	GdaSqlParser *parser;

	parser = g_object_get_data(G_OBJECT(cnc), "parser");
	stmt = gda_sql_parser_parse_string(parser, sql, &remain, &error);
	if (remain)
		g_print("REMAINS: %s\n", remain);

	nrows = gda_connection_statement_execute_non_select(cnc, stmt, NULL, NULL,
			&error);
	if (nrows == -1)
		g_error("NON SELECT error: %s\n",
				error && error->message ? error->message : "no detail");
	g_object_unref(stmt);
}


gint get_gvalue_contents_as_int(const GValue *value) {
	gint src = -1;

	g_return_val_if_fail(G_IS_VALUE (value), NULL);

	if (G_VALUE_HOLDS_INT(value)) {
		src = g_value_get_int(value);
		return src;
	}

	return src;
}

gchar*
get_gvalue_contents_as_string(const GValue *value) {
	const gchar *src;
	gchar *contents;

	g_return_val_if_fail(G_IS_VALUE (value), NULL);

	if (G_VALUE_HOLDS_STRING(value)) {
		src = g_value_get_string(value);

		if (!src)
			contents = g_strdup("NULL");
		else {
			gchar *s = g_strescape(src, NULL);

			contents = g_strdup_printf("%s", s);
			g_free(s);
		}
		return contents;
	}

	g_error("Enable to get content as string.");

	return NULL;
}



GdaDataModel* selectinternal(GdaConnection *cncp, gchar *sql) {
	GdaDataModel *data_model;
	GdaSqlParser *parser;
	GdaStatement *stmt;
	GError *error = NULL;

	parser = g_object_get_data(G_OBJECT(cncp), "parser");
	stmt = gda_sql_parser_parse_string(parser, sql, NULL, NULL);
	data_model = gda_connection_statement_execute_select(cncp, stmt, NULL,
			&error);
	g_object_unref(stmt);
	if (!data_model)
		g_error("Could not get the contents of the 'products' table: %s\n",
				error && error->message ? error->message : "No detail");

	return data_model;
}

gint qselecti(GdaConnection *cncp, gchar *sql) {

	GdaDataModel *data_model = selectinternal(cncp, sql);

	GdaDataModelIter *iter = gda_data_model_create_iter(data_model);
	if ( TRUE != gda_data_model_iter_move_next(iter)) {
		g_print("1st ERROR\n");
	}

	GError *err = NULL;

	const GValue *letter = gda_data_model_iter_get_value_at_e(iter, 0, &err);

	if (err != NULL) {
		g_error("Error: %s\n", err->message);
	}

	gint ret = get_gvalue_contents_as_int(letter);

	g_object_unref(iter);
	g_object_unref(data_model);
	return ret;

}

gchar* qselects(GdaConnection *cncp, gchar *sql) {

	GdaDataModel *data_model = selectinternal(cncp, sql);

	GdaDataModelIter *iter = gda_data_model_create_iter(data_model);
	if ( TRUE != gda_data_model_iter_move_next(iter)) {
		g_print("1st ERROR\n");
	}

	GError *err = NULL;

	const GValue *letter = gda_data_model_iter_get_value_at_e(iter, 0, &err);

	if (err != NULL) {
		g_error("Error: %s\n", err->message);
	}

	gchar *lstring = (gchar*) get_gvalue_contents_as_string(letter);

	g_object_unref(iter);
	g_object_unref(data_model);
	return lstring;

}
