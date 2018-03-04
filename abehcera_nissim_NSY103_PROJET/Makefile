all:projet_nsy103

projet_nsy103:client.o serveur_fork.o serveur_consultation.o serveur_reservation.o serveur_thread.o header_serveur.h outil.c


serveur_thread.o:serveur_thread.c header_serveur.h outil.c
	gcc -Wall serveur_thread.c outil.c -o serveur_thread.o -lpthread

serveur_reservation.o:serveur_reservation.c header_serveur.h outil.c
	gcc -Wall serveur_reservation.c outil.c -o serveur_reservation.o -lpthread

serveur_consultation.o:serveur_consultation.c header_serveur.h outil.c
	gcc -Wall serveur_consultation.c outil.c -o serveur_consultation.o -lpthread

serveur_fork.o:serveur_fork.c  header_serveur.h outil.c
	gcc -Wall serveur_fork.c outil.c -o serveur_fork.o -lpthread

client.o:client.c
	gcc -Wall client.c -o client.o

clean:
	rm -f *.o
	ipcrm -Q 1990
	ipcrm -M 1975
