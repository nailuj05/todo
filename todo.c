#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#define DB_STRUCTURE "id INTEGER PRIMARY KEY, task TEXT"

sqlite3 *db;
int rc;
char* errMsg = 0;

// open and initalize db connection
int todo_init() {
  int rc = sqlite3_open("todo.db", &db);
  if (rc) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    return 1;
  }
  
  const char *sqlCreateTable = "CREATE TABLE IF NOT EXISTS todos("DB_STRUCTURE");";
  rc = sqlite3_exec(db, sqlCreateTable, NULL, 0, &errMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", errMsg);
    sqlite3_free(errMsg);
  }
  
  return rc;
}

int todo_length() {
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM todos;", -1, &stmt, NULL);
  sqlite3_step(stmt); 

  int size = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);
  
  return size;
}

// insert new task into todos
int todo_add(char* task) {
  const char *sqlInsert ="INSERT INTO todos (task) VALUES (?);";
  sqlite3_stmt *stmt;

  rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return 1;
  }

  rc = sqlite3_bind_text(stmt, 1, task, -1, SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return 1;
  }

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
  }

  sqlite3_finalize(stmt);
  return 0;
}

// callback for displaying todos
int todo_callback(void *data, int argc, char **argv, char **colName) {
    printf("%-3s| %s\n", argv[0], argv[1] ? argv[1] : "---");
    return 0;
}

int todo_read() {
  const char *sqlSelect = "SELECT * FROM todos;";
  rc = sqlite3_exec(db, sqlSelect, todo_callback, 0, &errMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error (select): %s\n", errMsg);
    sqlite3_free(errMsg);
    return 1;
  }
  return 0;
}

int todo_del(int id) {
  const char *sqlInsert ="DELETE FROM todos WHERE id IN (?);";
  sqlite3_stmt *stmt;

  rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    return 1;
  }

  rc = sqlite3_bind_int(stmt, 1, id);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    return 1;
  }

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
  }

  sqlite3_finalize(stmt);
  return 0;
}

int main(int argc, char *argv[]) {
  todo_init();
  int size = todo_length(); 

  if (argc > 1) {
    for(int i = 0; i < argc; ++i) {
      if (strcmp(argv[i], "add") == 0 && argc > ++i) {
	todo_add(argv[i]);
	printf("added task\n");
	size++;
      }
      else if (strcmp(argv[i], "del") == 0 && argc > i) {
	while (++i < argc) {
          char *endptr;
          int id = strtol(argv[i], &endptr, 10);
          if (*endptr == '\0' && id > 0) {
            todo_del(id);
	    printf("deleted %i\n", id);
            size--;
          }
	  else {
	    --i;
            break;
          }
        }
      }
      else if (strcmp(argv[i], "reset") == 0) {
	sqlite3_exec(db, "DROP TABLE todos;", NULL, NULL, NULL);
	printf("resetted todos\n");
      }
      else if (strcmp(argv[i], "ls") == 0) {
	todo_read();
      }
    }
  }
  else {
    todo_read();
  }

  sqlite3_close(db);
  return 0;
}
