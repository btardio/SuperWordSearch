/*
 * simpleWordPuzzle.h
 *
 *  Created on: Feb 7, 2020
 *      Author: btardio
 */
#include <libgda/libgda.h>

#ifndef SIMPLEWORDPUZZLE_H_
#define SIMPLEWORDPUZZLE_H_

#define DEBUG // if you would like to see debug, uncomment

#define PUZZLEFILENAMEDEFAULT "internal/puzzle"
#if defined(WIN32) && !defined(UNIX)
#define PUZZLEFILENAMEDEFAULT "internal\puzzle"
#elif defined(UNIX) && !defined(WIN32)
#define PUZZLEFILENAMEDEFAULT "internal/puzzle"
#endif

#define PUZZLE_DB_CONNECTION_STRING "DB_DIR=.;DB_NAME=/internal/puzzle_db"
#if defined(WIN32) && !defined(UNIX)
#define PUZZLEDB_CONNECTION_STRING "DB_DIR=.;DB_NAME=\internal\puzzle_db"
#elif defined(UNIX) && !defined(WIN32)
#define PUZZLE_DB_CONNECTION_STRING "DB_DIR=.;DB_NAME=/internal/puzzle_db"
#endif

#define POSSIBILITIES_DB_CONNECTION_STRING "DB_DIR=.;DB_NAME=/internal/possibilities_db"
#if defined(WIN32) && !defined(UNIX)
#define POSSIBILITIES_CONNECTION_STRING "DB_DIR=.;DB_NAME=/internal/possibilities_db"
#elif defined(UNIX) && !defined(WIN32)
#define POSSIBILITIES_DB_CONNECTION_STRING "DB_DIR=.;DB_NAME=/internal/possibilities_db"
#endif

#define PUZZLEWORDS "internal/puzzlewords"
#if defined(WIN32) && !defined(UNIX)
#define PUZZLEWORDS "internal\puzzlewords"
#elif defined(UNIX) && !defined(WIN32)
#define PUZZLEWORDS "internal/puzzlewords"
#endif

typedef struct DBConnection DBConnection;

// utils
GdaDataModel* selectinternal(GdaConnection *cncp, gchar *sql);
gint qselecti(GdaConnection *cncp, gchar *sql);
gchar* qselects(GdaConnection *cncp, gchar *sql);
gint get_gvalue_contents_as_int(const GValue *value);
gchar* get_gvalue_contents_as_string(const GValue *value);
void insertLineIntoStruct(DBConnection*, GString*);
GdaConnection* open_connection();
GdaConnection* open_generic_connection(char *connectString);
GString* nextWord(FILE *fp);
void run_sql_non_select(GdaConnection *cnc, const gchar *sql);
GdaConnection* open_puzzle_connection(char *connectString);


struct DBConnection {
	GdaConnection *cnc;
};


int _main_wordPuzzlePopulatePuzzle(int argc, char **argv);

int _main_wordPuzzlePopulatePossibilities(gboolean wrap);

int _main_wordPuzzleChecker(int argc, char **argv);

#endif /* SIMPLEWORDPUZZLE_H_ */
