                              //serveur_reservation.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include "header_serveur.h"

void
reservation_places(const char nom_spectacle[],const char nbr_places_souhaite[],const char pid_client[]){

    printf("\nreservation\n");
    pid_t retour=fork();//creation processus fils reservation
    if(retour==0){
        reponse_vers_client reponse_serveur;//creation objet qui sera envoye au client
        int ack=0;//variable ==1 si reservation effectuée 0 sinon
        spectacles *ptr_mem_partagee=acceder_shm();//pointeur vers la shm
        int msqId=acceder_msq();//id de la msq
        strcpy(reponse_serveur.nom_spectacle,nom_spectacle);//copie du nom de spectacle dans l'objet
        reponse_serveur.nbr_places_souhaite=atoi(nbr_places_souhaite);//cast et copie du nombre de places souhaitees dans l'objet
        reponse_serveur.nbr_places_disponible=update_nbr_places_libres_serveur(nom_spectacle,ptr_mem_partagee,reponse_serveur.nbr_places_souhaite,&ack);
        //mise a jour du nombre de places libres après reservation
        reponse_serveur.requete_type=atol(pid_client);//requete type=pid client

        reponse_serveur.ack=ack;
        envoi_msg(msqId,&reponse_serveur);//envoi structure au client
        shmdt(ptr_mem_partagee);//detachement de la shm
        exit(0);//fin du processus fils
    }else{
        wait(0);
        exit(0);
    }

}

int main(int argc, char **argv)
{
    reservation_places(argv[0],argv[1],argv[2]);
    return 0;
}
