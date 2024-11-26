#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "fullwrapper.h"

//il primo parametro indica il numero degli argomenti passati al programma, il secondo è un puntatore ad un array di caratteri che rappresenta gli argomenti stessi.
int main(int argc, char *argv[]){
    int sock;
    struct sockaddr_in server_G;

    //crea un nuovo socket e ne restituisce il file descriptor.
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr,"socket error\n");
        exit (1);
    }
  
	//settings serverG
    server_G.sin_family = AF_INET; //AF_INET famiglia per la comunicazione tra host remoti tramite internet (dominio di internet)
    server_G.sin_port = htons(1026); //configurazione della porta

   
    //Converte un indirizzo IP in formato testuale (passato come argomento alla funzione) in una struttura di tipo "struct in_addr", che viene utilizzata per rappresentare gli indirizzi IP in formato binario.
	if (inet_pton(AF_INET, argv[1], &server_G.sin_addr) < 0) {
      	fprintf(stderr,"inet_pton error for %s\n", argv[1]);
      	exit (1);
    }

    //stabilisce una connessione TCP verso un server remoto, utilizzando il socket "sock" creato in precedenza. Connessione con il serverG.
    if (connect(sock, (struct sockaddr *) &server_G, sizeof(server_G)) < 0) {
        fprintf(stderr,"connect error\n");
        exit(1);
    }

    
    VerificaG(stdin, sock);

    return 0;
}

//La funzione "VerificaG" riceve come input un puntatore a un file e un socket.  
//Utilizza le funzioni FullWrite e FullRead per inviare e ricevere i pacchetti tramite il socket. Se si verificano errori durante l'invio o la ricezione dei pacchetti, la funzione stampa un messaggio di errore e termina l'esecuzione.
void VerificaG(FILE * filein, int socket) {

    //Inizializza due variabili "informazioni" a_serverG e r_serverG, e assegna la stringa "SGT" alla variabile "tiporichiesta" di a_serverG.
    informazioni a_serverG, r_serverG;
	strcpy(a_serverG.tiporichiesta, "SGT");
    int nread;

    //svuotamento del buffer di input standard (stdin).
    fflush(stdin);

    //viene richiesto il codice fiscale
    printf("Per cambiare la validita' del tuo green pass, inserire il tuo codice fiscale: \n");

	//copia del codice fiscale
    strcpy(a_serverG.codicefiscale, "");
	if (fgets(a_serverG.codicefiscale, 16, filein) == NULL) { /* se non ci sono input si ferma il client */
		close(socket);
		return;             
	} else {               
        //Invia il pacchetto contenente le informazioni al serverG tramite la funzione "FullWrite". 
        if ((FullWrite(socket, &a_serverG, sizeof(informazioni))) < 0) {
            fprintf(stderr, "FULLWRITE: error\n");
            exit(1);
        }

	    printf("Dati inviati al serverG, in attesa di una risposta\n");

		//Riceve la risposta dal serverG tramite la funzione FullRead e memorizza le informazioni nella variabile r_serverG.
	    if ((nread = FullRead(socket, &r_serverG, sizeof(informazioni))) < 0) {
            printf("Connection interrupted by client.\n");
            printf("Child process exit...\n\n");
            close(socket);
            exit(0);
        }
	   	printf("Dati ricevuti dal serverG:\n");
	  	
		//verifica del cambio di validita
        if (strcmp(r_serverG.validita,"valido")==0)
				printf("Attenzione, il tuo greenpass è stato riattivato\n");
		else
				printf("Attenzione, il tuo greenpass è stato disabilitato\n");
	   	
	}
}
