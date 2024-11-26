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
int main(int argc, char *argv[]) {
    int sock;
    bool vax = false;
    struct sockaddr_in centro_vaccinale;

    
    //Questa riga di codice crea un nuovo socket e ne restituisce il file descriptor.
    if ( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr,"socket error\n");
        exit (1);
    }


    centro_vaccinale.sin_family = AF_INET; //AF_INET famiglia per la comunicazione tra host remoti tramite internet (dominio di internet)
    centro_vaccinale.sin_port = htons(1024); //configurazione della porta


    //utilizza la funzione "inet_pton" per convertire un indirizzo IP da notazione testuale in notazione binaria e memorizzarlo nella struttura "centro_vaccinale".
    //Se la conversione ha successo, la funzione restituisce un valore maggiore di 0, altrimenti restituisce un valore minore o uguale a 0. 
    //Se la conversione non ha successo, viene stampato un messaggio di errore che indica il tipo di errore e il programma termina l'esecuzione.
	if (inet_pton(AF_INET, argv[1], &centro_vaccinale.sin_addr) < 0) {
        fprintf(stderr,"inet_pton error for %s\n", argv[1]);
        exit (1);
    }

    // connessione con il centro vaccinale 
    if (connect(sock, (struct sockaddr *) &centro_vaccinale, sizeof(centro_vaccinale)) < 0) {
        fprintf(stderr,"connect error\n");
        exit(1);
    }
    //la variabile vax viene messa a true visto che l'utente è stato vaccinato e può proseguire
    vax = true;

    
    InvioC(stdin, sock);
    return 0;
}

//Questa funzione in C, chiamata "InvioC", riceve un puntatore a file e un socket come argomenti. 
//In questa funzione, viene richiesto all'utente di inserire un codice fiscale, che viene poi memorizzato in una variabile di buffer denominata "send".
//Successivamente, se viene letto correttamente dal file, il codice fiscale viene inviato al socket tramite la funzione "FullWrite". 
//Se si verifica un errore durante l'invio, viene stampato un messaggio di errore sullo standard error e il programma viene terminato con exit(1).
void InvioC(FILE * filein, int socket) {
    char send[16];
    
    //svuotamento del buffer di input standard (stdin).
    fflush(stdin);

    //viene richiesto il codice fiscale
    printf("Inserire il codice fiscale: \n");

	strcpy(send, "");
    //controllo per verificare se il valore di ritorno della funzione fgets è NULL. La funzione fgets legge i dati dal file puntato da filein e li memorizza nella stringa send. 
    //Se fgets restituisce NULL, significa che si è verificato un errore o che si è raggiunto la fine del file.
    //In questo caso, se fgets restituisce NULL, la funzione chiude il socket e ritorna, interrompendo l'esecuzione del programma. 
	if (fgets(send, 15, filein) == NULL) { 
		close(socket);
		return;               
	} else {
                   
        //Questa riga di codice invia un messaggio dal client al server tramite la funzione "FullWrite". Invia, quindi, il codice fiscale al centro vaccinale.
        if ((FullWrite(socket, (void *) send, (size_t) strlen(send))) < 0) {
            fprintf(stderr, "FULLWRITE: error\n");
            exit(1);
        } 
    }
}
