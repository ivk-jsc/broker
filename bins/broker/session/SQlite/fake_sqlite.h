//
// Created by bas on 01.10.2020.
//

#ifndef UPMQ_BINS_BROKER_SESSION_SQLITE_FAKE_SQLITE_H_
#define UPMQ_BINS_BROKER_SESSION_SQLITE_FAKE_SQLITE_H_

// NOTE : sqlite is a build-in component of PocoDataSqlite

enum SqliteErrors { SQLITE_BUSY = 5, SQLITE_LOCKED = 6, SQLITE_BUSY_SNAPSHOT = 517 };
extern "C" int sqlite3_exec(sqlite3 *,                                      /* An open database */
                            const char *sql,                                /* SQL to be evaluated */
                            int (*callback)(void *, int, char **, char **), /* Callback function */
                            void *,                                         /* 1st argument to callback */
                            char **errmsg                                   /* Error msg written here */
);

#endif  // UPMQ_BINS_BROKER_SESSION_SQLITE_FAKE_SQLITE_H_
