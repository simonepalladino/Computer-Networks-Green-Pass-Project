#include<sys/types.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string.h>

#include "fullwrapper.h"

//il primo parametro indica il numero degli argomenti passati al programma, il secondo è un puntatore ad un array di caratteri che rappresenta gli argomenti stessi.
int main (int argc , char *argv[]) {
	int list_fd,conn_fd, serverV_fd;
    socklen_t lght;
 	pid_t pid;
 	int nread;
 	struct sockaddr_in server_G, server_V, client;

 	//configurazione dell'indirizzo del serverG  
 	if ((list_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    	perror("socket");
    	exit(1);
    }

    //settings del serverG
	server_G.sin_family = AF_INET; //AF_INET famiglia per la comunicazione tra host remoti tramite internet (dominio di internet)
 	server_G.sin_addr.s_addr = htonl(INADDR_ANY); //configurazione indirizzo, indirizzo IP di qualsiasi interfaccia di rete disponibile sulla macchina in cui il server è in esecuzione.
 	server_G.sin_port = htons(1026); //configurazione numero di porta

    //utilizzata per associare un socket creato con la funzione "socket()" ad un indirizzo IP e ad una porta specifici, in modo che il socket possa ascoltare e accettare connessioni in arrivo da client.
 	if (bind(list_fd, (struct sockaddr *) &server_G, sizeof(server_G)) < 0) {
    	perror("bind");
    	exit(1);
  	}

  	//configurazione della lista d'attesa dei client
 	if (listen(list_fd, 1024) < 0) {
    	perror("listen");
    	exit(1);
  	}


 	while(1) {
		lght = sizeof (client);

		//accettazione delle richieste dei client
		if ((conn_fd = accept(list_fd, (struct sockaddr *) &client, &lght)) < 0) {
			perror("accept");
			exit(1);
		}	

		// fork del processo per generare un processo figlio che gestirà le richieste dei client
		if((pid= fork())<0) {
			perror (" fork error ");
			exit ( -1);
		}

		if(pid==0) {
			//processo figlio
			close (list_fd);
			
			//connessione col serverV
			if ((serverV_fd = socket(AF_INET, SOCK_STREAM, 0) ) < 0) {
				perror("socket");
				exit(1);
			}

			//settings serverV
			server_V.sin_family = AF_INET;
			server_V.sin_port = htons(1025);
				
			
			if (inet_pton(AF_INET, argv[1], &server_V.sin_addr) < 0) {
				fprintf(stderr,"inet_pton error for %s\n", argv[1]);
				exit (1);
			}

			// connessione col server
			if (connect(serverV_fd, (struct sockaddr *) &server_V, sizeof(server_V)) < 0) {
				fprintf(stderr,"connect error\n");
				exit(1);
			}

			informazioni r_client;
			//lettura della richiesta del client attraverso la FullRead
			if ((nread = FullRead(conn_fd, &r_client, sizeof(informazioni))) < 0) {
				printf("Connection interrupted by client.\n");
				printf("Child process exit...\n\n");
				close(conn_fd);
				exit(0);
			}

			// richieste del clientS
			if(strcmp(r_client.tiporichiesta, "SGS")==0 ) {
				printf("In attesa di dati dal clientS...\n");
				//dichiarazione dei pacchetti applicazione per la ricezione e l'invio di informazioni
				informazioni rserver_v, aClientS;

				//invio della richiesta di clientS al serverV attraverso la FullWrite
				if ((FullWrite(serverV_fd, &r_client, sizeof(informazioni))) < 0) {
					fprintf(stderr, "FULLWRITE: error\n");
					exit(1);
				}
				printf("Dati inviati al serverV, in attesa di una sua risposta\n");

				//ricezione del pacchetto di risposta da serverV
				if ((nread = FullRead(serverV_fd, &rserver_v, sizeof(informazioni))) < 0) {
					printf("Connection interrupted by client.\n");
					printf("Child process exit...\n\n");
					close(conn_fd);
					exit(0);
				}
			
				//invio del pacchetto di risposta ricevuto a clientS
				if ((FullWrite(conn_fd, &rserver_v, sizeof(informazioni))) < 0) {
					fprintf(stderr, "FULLWRITE: error\n");
					exit(1);
				}
				
				printf("Risposta ricevuta, la risposta sarà inoltrata a clientS\n");

			//gestione delle richieste di clientT
			} else if(strcmp(r_client.tiporichiesta, "SGT")==0 ) {
				printf("In attesa di dati dal clientT...\n");
				
				informazioni rserver_v, aClientS;

				//invio della richiesta di clientT al serverV attraverso la FullWrite
				if ((FullWrite(serverV_fd, &r_client, sizeof(informazioni))) < 0) {
					fprintf(stderr, "FULLWRITE: error\n");
					exit(1);
				}

				printf("Dati inviati al serverV, in attesa di una sua risposta\n");
				
				//ricezione del pacchetto di risposta da serverV
				if ((nread = FullRead(serverV_fd, &rserver_v, sizeof(informazioni))) < 0) {
					printf("Connection interrupted by client.\n");
					printf("Child process exit...\n\n");
					close(conn_fd);
					exit(0);
				}
			
				//invio del pacchetto di risposta ricevuto a clientT
				if ((FullWrite(conn_fd, &rserver_v, sizeof(informazioni))) < 0) {
					fprintf(stderr, "FULLWRITE: error\n");
					exit(1);
				}
				
				printf("Risposta ricevuta, la risposta sarà inoltrata a clientT\n");
			}

			close (conn_fd);
			close(serverV_fd);
			exit (0);
		} else {
			
			close (conn_fd);
			close (serverV_fd);
		}
	}

 	exit (0);
	
}

