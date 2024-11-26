#include<sys/types.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sqlite3.h>

#include "fullwrapper.h"


//Una variabile di tipo "pthread_mutex_t" è utilizzata per implementare la mutua esclusione nei thread. 
//Viene utilizzata per impedire che due o più thread accedano contemporaneamente alla stessa area di memoria condivisa.
pthread_mutex_t obstruct;

//il primo parametro indica il numero degli argomenti passati al programma, il secondo è un puntatore ad un array di caratteri che rappresenta gli argomenti stessi.
int main (int argc , char *argv[]) {
	socklen_t lght;
 	pthread_t thread;
	int list_fd,conn_fd;
	struct sockaddr_in serv_add,client;
 	

 	// Viene creato il socket del server.
 	if ((list_fd = socket(AF_INET, SOCK_STREAM, 0) ) < 0) {
    	perror("socket");
    	exit(1);
    }
    
    //settings del serverV
 	serv_add.sin_family = AF_INET; //AF_INET famiglia per la comunicazione tra host remoti tramite internet (dominio di internet)
 	serv_add.sin_addr.s_addr = htonl(INADDR_ANY); //configurazione indirizzo, indirizzo IP di qualsiasi interfaccia di rete disponibile sulla macchina in cui il server è in esecuzione.
 	serv_add.sin_port = htons(1025); //configurazione della porta

	//Viene assegnato l'indirizzo al server
	if (bind(list_fd, (struct sockaddr *) &serv_add, sizeof(serv_add)) < 0) {
    	perror("bind");
    	exit(1);
  	}

 	//configurazione della lista d'attesa dei client
 	if ( listen(list_fd, 1024) < 0 ) {
    	perror("listen");
    	exit(1);
  	}

    //il server continua ad aspettare nuove connessioni all'infinito, finché non viene interrotto manualmente o si verifica un errore.
 	while(1) {
	 	lght = sizeof (client);
	 	//Viene accettata la richiesta di un client
     	printf("serverV in attesa, inserire i dati \n");
	 	if ((conn_fd = accept(list_fd, (struct sockaddr *) &client, &lght)) < 0) {
      		perror("accept");
      		exit(1);
    	}
	 
		// Viene generato un nuovo thread per la gestione dei vari client.
		int * desc_to_thread = (int *) malloc(sizeof(int));
		desc_to_thread = &conn_fd;
		if(pthread_create(&thread, NULL, gestioneC, (void *) desc_to_thread)!=0) {
			perror (" pthread_create errorr ");
			exit ( -1);
		}
         
		pthread_join(thread, NULL);

        //chiude la connessione una volta che il thread ha terminato la sua esecuzione.
		close (conn_fd);
	}

 	exit(0);
}

void *gestioneC(void * arg) {
	int *filedes = (int *) arg;
	int nread;
	 informazioni r_client;

	//ricezione della richiesta del client attraverso la FullRead
    
	if ((nread = FullRead(*filedes, (void *) &r_client, sizeof( informazioni))) < 0) {
        printf("Connection interrupted by client.\n");
        printf("Child process exit...\n\n");
        close(*filedes);
        exit(0);
    }

	//richiesta del centro vaccinale
    if(strcmp(r_client.tiporichiesta, "CV")== 0) {
        printf("dati ricevuti dal centro vaccinale...\n");	

  	sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;
    
    int rc = sqlite3_open("greenpass.db", &db); //apertura del db creato in creazionetab.c
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
		exit(1);
    }
		
		pthread_mutex_lock(&obstruct);
		  
		//INSERT dei dati utente nel database
		char *sqll ="INSERT INTO User(tipo,Codicefiscale,mesivalidita,validita) VALUES (?,?,?,?);";
		rc = sqlite3_prepare_v2(db, sqll, -1, &res, 0);

    	if (rc == SQLITE_OK) {
			sqlite3_bind_text(res, 1,r_client.tiporichiesta, -1, SQLITE_TRANSIENT);
			int err =sqlite3_bind_text(res, 2,r_client.codicefiscale, strlen(r_client.codicefiscale)-1, SQLITE_STATIC);			
			sqlite3_bind_int(res, 3, r_client.mesivalidita);
			sqlite3_bind_text(res, 4,r_client.validita, -1, SQLITE_TRANSIENT);
			if (err != SQLITE_OK)
			{	fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
				exit(1);   
			}
			err = sqlite3_step(res);
			//gestione degli errori dello statement e dei vincoli nel database ES(vincoli di unique sul CF)
			if (err != SQLITE_DONE){
				fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
				exit(1);   
			}
			sqlite3_finalize(res);

		} else {
        	fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
			exit(1);
    	}
		
		//query per visualizzare i dati inseriti
		char *sql = "select Codicefiscale,mesivalidita from User where Codicefiscale = (?) ;";
  		sqlite3_prepare_v2(db, sql, -1, &res, 0);
      	sqlite3_bind_text(res, 1, r_client.codicefiscale,strlen(r_client.codicefiscale)-1, SQLITE_STATIC);
        int step = sqlite3_step(res);
    
    if (step == SQLITE_ROW) {
        
        printf("\ncodice fiscale : %s ", sqlite3_column_text(res, 0));
		printf("\n");
        printf("mesi di validità : %d", sqlite3_column_int(res, 1));
		printf("\n");
    
    } 
    sqlite3_finalize(res);
    sqlite3_close(db);
    
		pthread_mutex_unlock(&obstruct);

		//richiesta del serverG (arrivata da clientS)
	} else if(strcmp(r_client.tiporichiesta, "SGS")==0 ) {
        printf("In attesa di dati dal serverG...\n");
		informazioni a_serverG;
		sqlite3 *db;
    	char *err_msg = 0;
   		sqlite3_stmt *res;
		
		
		pthread_mutex_lock(&obstruct);

		//connessione al database
  		int rc = sqlite3_open("greenpass.db", &db);
    
    	if (rc != SQLITE_OK) {
			fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
			exit(1);
		}

		//query per visualizzare i dati inseriti
  		char *sql = "select Codicefiscale,mesivalidita,validita from User where Codicefiscale = (?) ;";
  		sqlite3_prepare_v2(db, sql, -1, &res, 0);
      	sqlite3_bind_text(res, 1, r_client.codicefiscale,strlen(r_client.codicefiscale)-1, SQLITE_STATIC);
        int step = sqlite3_step(res);

  		
			//scorrimento dei dati se sono presenti le righe
			if (step == SQLITE_ROW) {
				strcpy(a_serverG.codicefiscale, sqlite3_column_text(res, 0));
				a_serverG.mesivalidita = atoi(sqlite3_column_text(res, 1));
				strcpy(a_serverG.validita, sqlite3_column_text(res, 2));
				printf("verifichiamo la validità del greenpass con codice fiscale: %s\n", a_serverG.codicefiscale);

			//invio del pacchetto applicazione di risposta al serverG
				if ((FullWrite(*filedes, (void *) &a_serverG, sizeof(informazioni))) < 0) {
					fprintf(stderr, "FULLWRITE: error\n");
					exit(1);
				}
		    
				printf("dati inviati al serverG\n");
			} else {
				printf("green pass non presente nel database\n");
			}

		sqlite3_finalize(res);
    	sqlite3_close(db);
		pthread_mutex_unlock(&obstruct);

		//richiesta del serverG (arrivata da clientT)
	} else if(strcmp(r_client.tiporichiesta, "SGT")== 0) {
		printf("In attesa di dati dal serverG...\n");
		informazioni a_serverG;
		char sql_statement[2048];
		sqlite3 *db;
    	char *err_msg = 0;
   		sqlite3_stmt *res;


		pthread_mutex_lock(&obstruct);

		//connessione al database
  		int rc = sqlite3_open("greenpass.db", &db);
    
    	if (rc != SQLITE_OK) {
			fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
			exit(1);
		}

		//query per visualizzare i dati inseriti
  		char *sql = "select validita from User where Codicefiscale = (?);";
  		sqlite3_prepare_v2(db, sql, -1, &res, 0);
      	sqlite3_bind_text(res, 1, r_client.codicefiscale,strlen(r_client.codicefiscale)-1, SQLITE_STATIC);
        int step = sqlite3_step(res);

		if (step == SQLITE_ROW) {
				if (strcmp(sqlite3_column_text(res, 0),"valido")==0)
					strcpy(a_serverG.validita,"non valido");
				else
					strcpy(a_serverG.validita,"valido");

				printf("validita del green pass con codice fiscale %s aggiornata\n", r_client.codicefiscale);
				
			//invio del pacchetto di risposta a serverG
			if ((FullWrite(*filedes, (void *) &a_serverG, sizeof(informazioni))) < 0) {
                fprintf(stderr, "FULLWRITE: error\n");
                exit(1);
            }
		    	
			printf("dati inviati al serverG\n");

			
			
			//update per il passaggio da valido a non valido e viceversa nel database
			if (strcmp(a_serverG.validita,"valido")==0) {
				sqlite3_stmt *res1;
				char *sqL= "UPDATE User SET validita = 'valido' WHERE Codicefiscale= (?);";
				sqlite3_prepare_v2(db, sqL, -1, &res1, 0);
      			sqlite3_bind_text(res1, 1, r_client.codicefiscale,strlen(r_client.codicefiscale)-1, SQLITE_STATIC);
				sqlite3_step(res1);
				sqlite3_finalize(res1);
				
			} else if (strcmp(a_serverG.validita,"non valido")==0) {
				sqlite3_stmt *res2;
				char *sqL= "UPDATE User SET validita = 'non valido' WHERE Codicefiscale = (?);";
				sqlite3_prepare_v2(db, sqL, -1, &res2, 0);
      			sqlite3_bind_text(res2, 1, r_client.codicefiscale,strlen(r_client.codicefiscale)-1, SQLITE_STATIC);
				sqlite3_step(res2);
				sqlite3_finalize(res2);
			
			}
			 sqlite3_finalize(res);

		} else {
			printf("green pass non presente nel database\n");
		}

        //chiude il database
		sqlite3_close(db);

        //rilascia la mutua esclusione
		pthread_mutex_unlock(&obstruct);
	}

	close(*filedes);
}
