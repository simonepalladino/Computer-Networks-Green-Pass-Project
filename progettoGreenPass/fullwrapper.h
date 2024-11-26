#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/select.h>
#include <sys/errno.h>
#include <errno.h>


//tipo enumerativo per definire il tipo bool.
typedef enum {false, true} bool;

//La struttura "informazioni" rappresenta un pacchetto di livello applicazione con quattro campi: codicefiscale, mesivalidita, validita e tiporichiesta.
 typedef struct {
    char codicefiscale[16];
    int mesivalidita;
    char validita[15];
    char tiporichiesta[16];
} informazioni; 

//Le funzioni FullRead e FullWrite vengono utilizzate per leggere e scrivere dati da e verso un socket. Queste funzioni usano un 
//ciclo while per continuare a leggere o scrivere dati fino a quando tutti i dati non sono stati elaborati o si verifica un errore. 
//Le funzioni gestiscono anche il caso in cui l'operazione di lettura o scrittura viene interrotta da un segnale, in tal caso continueranno l'operazione.
ssize_t FullRead(int fd, void *buf, size_t count) {
    size_t nleft; 
    ssize_t nread; 
    nleft = count; 

    while (nleft > 0) {
		if( (nread=read(fd, buf, nleft)) < 0) {
		    if(errno=EINTR)
                continue;
			else
                exit(nread);
		} else if(nread==0)
            break;

		nleft-=nread;
		buf+=nread;
    }

    buf=0; 
    return (nleft); 
}

ssize_t FullWrite(int fd, const void *buf, size_t count) {
    size_t nleft; 
    ssize_t nwritten; 
    nleft = count;

    while (nleft > 0) {
        if ( (nwritten = write(fd, buf, nleft)) < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                return(nwritten);
            }
        }

        nleft -= nwritten;
        buf +=nwritten;
    }
    
    return (nleft); 
}

//dichiarazione di una funzione di nome "VerificaG" che prende come argomenti un puntatore a un file (di tipo "FILE *") e un intero "socket".
void VerificaG(FILE * filein, int socket);

//dichiarazione di una funzione di nome "InvioC" che prende come argomenti un puntatore a un file (di tipo "FILE *") e un intero "socket".
void InvioC(FILE * filein, int socket);

//dichiarazione di una funzione di nome "gestioneC" che riceve un argomento generico di tipo puntatore void.
void *gestioneC(void * arg);






















