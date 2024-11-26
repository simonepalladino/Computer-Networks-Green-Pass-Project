#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>


int main(void) {
    // Dichiarazione delle variabili necessarie per l'accesso al database SQLite
    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;
    
    // Apertura del database SQLite
    int rc = sqlite3_open("greenpass.db", &db);
    
    // Verifica se l'apertura del database è avvenuta correttamente
    if (rc != SQLITE_OK) {
        // Stampa un messaggio di errore in caso di fallimento dell'apertura
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        // Termina il programma restituendo un valore diverso da zero in caso di errore
        return 1;
    }
    
    // Definizione della stringa SQL per la creazione della tabella "User"
    char *sqL= "DROP TABLE IF EXISTS User;"
    "CREATE TABLE User(tipo TEXT, Codicefiscale VARCHAR(20) PRIMARY KEY, mesivalidita INT, validita TEXT);" ;

    // Esecuzione della query per la creazione della tabella "User"
    sqlite3_exec(db, sqL, 0, 0, &err_msg);

    printf("CREAZIONE DEL DATABASE GREENPASS.DB E TABELLA USER AVVENUTA CON SUCCESSO!\n");
    
    // Chiude il database
    sqlite3_close(db);
    
    return 0;
}
