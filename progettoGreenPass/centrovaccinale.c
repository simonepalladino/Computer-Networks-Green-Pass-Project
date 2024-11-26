#include<sys/types.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//libreria di funzioni per le operazioni di lettura,scrittura e creazione della struct "informazioni".
#include "fullwrapper.h"

//il primo parametro indica il numero degli argomenti passati al programma, il secondo è un puntatore ad un array di caratteri che rappresenta gli argomenti stessi.
int main (int argc , char *argv[]) {
    //Vengono inizializzati diversi descrittori di file, tra cui il descrittore della socket che verrà utilizzato per l'ascolto delle connessioni dei client.
    int list_fd,conn_fd, serverV_fd;
    int nread;
    socklen_t lght;
    pid_t pid;
    struct sockaddr_in centro_vaccinale, client, server_V;
    char bufferStr[16];

    //creazione del socket tramite la funzione socket(), che prende in input il protocollo di rete, la tipologia di socket e l'eventuale protocollo utilizzato. 
    //In questo caso viene creato un socket di tipo SOCK_STREAM, ovvero un socket di tipo connessione orientata. 
    //Se la creazione della socket fallisce, viene stampato un messaggio di errore e il programma termina.
    if ((list_fd = socket(AF_INET, SOCK_STREAM, 0) ) < 0) {
        perror("socket");
        exit(1);
    }

    //configurazione dell'indirizzo del centro vaccinale
    centro_vaccinale.sin_family = AF_INET; //AF_INET famiglia per la comunicazione tra host remoti tramite internet (dominio di internet)
    centro_vaccinale.sin_addr.s_addr = htonl(INADDR_ANY);//server in ascolto su qualsiasi indirizzo locale ad esempio 127.0.0.0 associato al server
    centro_vaccinale.sin_port = htons(1024); //configurazione della porta

    // Esegue la bind della socket. La bind è necessaria per associare l'indirizzo e la porta alla socket e iniziare ad accettare le connessioni in ingresso. 
    // Se la bind fallisce, viene stampato un messaggio di errore e il programma termina con un codice di errore.
    if ( bind(list_fd, (struct sockaddr *) &centro_vaccinale, sizeof(centro_vaccinale)) < 0 ) {
        perror("bind");
        exit(1);
    }

    //configurazione della lista d'attesa dei client, in questo caso il server può accettare massimo 1024 connessioni contemporaneamente.
    if ( listen(list_fd, 1024) < 0 ) {
        perror("listen");
        exit(1);
    }

    // Ciclo infinito while(1) che accetta le richieste dei client attraverso una connessione di tipo listen(). 
    // Quando una richiesta viene accettata, il processo genera un processo figlio tramite la funzione fork(), che si occupa di gestire la richiesta del client.
    while(1) {

	    lght = sizeof ( client );
	    //accettazione delle richieste dei client;
	    if ( ( conn_fd = accept(list_fd, (struct sockaddr *) &client, &lght) ) < 0 ) {
            perror("accept");
            exit(1);
        }

	    printf("connessione andata a buon fine\n");

	    // fork del processo per generare un processo figlio che gestirà le richieste dei client 
	    if(fork()==0) {
            //processo figlio
		    close (list_fd);

		    // inizializzazione del serverV
		    if ((serverV_fd = socket(AF_INET, SOCK_STREAM, 0) ) < 0) {
    		    perror("socket");
    			exit(1);
    	    }

    	    server_V.sin_family = AF_INET; //AF_INET famiglia per la comunicazione tra host remoti tramite internet
    	    server_V.sin_port   = htons(1025); //configurazione della porta
		
	    	if (inet_pton(AF_INET, argv[1], &server_V.sin_addr) < 0) {
      			fprintf(stderr,"inet_pton error for %s\n", argv[1]);
      			exit (1);
    	    }
             

    	    // connessione con il serverV 
		    if (connect(serverV_fd, (struct sockaddr *) &server_V, sizeof(server_V)) < 0) {
    		    fprintf(stderr,"connect error\n");
    		 	exit(1);
  	    	}
            printf("In attesa di una richiesta del client\n");
		

		    //lettura della richiesta del client attraverso la FullRead
		    if ((nread = FullRead(conn_fd, (void *) bufferStr, (size_t) sizeof(bufferStr))) < 0) {
                printf("Connection interrupted by client.\n");
                printf("Child process exit...\n\n");
                close(conn_fd);
                exit(0);
            }

            //creazione della una variabile aserver_V di tipo informazioni
            informazioni aserver_V;
            strcpy(aserver_V.tiporichiesta , "CV");
            strcpy(aserver_V.codicefiscale, (char*) bufferStr);
            aserver_V.mesivalidita = 12;
            strcpy(aserver_V.validita , "valido");
            
           
            //invio della richiesta del client al serverV attraverso la FullWrite
            if ((FullWrite(serverV_fd, &aserver_V, sizeof(informazioni))) < 0) {
                fprintf(stderr, "FULLWRITE: error\n");
                exit(1);
            }

            printf("Dati inviati al serverV\n");

            //chiusura delle connessioni
            close (conn_fd);
		    close (serverV_fd);
 		    exit(0);
	    } else {
            //processo padre
		    close (conn_fd);
		}
	}

    exit(0);
}
