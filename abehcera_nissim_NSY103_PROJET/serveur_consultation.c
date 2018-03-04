                            //serveur_consultation.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include "header_serveur.h"

void consultation_nbr_places_spectacle(const char nom_spectacle[],const char pid_client[]){

        printf("\nconsultation\n");
        reponse_vers_client reponse_vers_client;//création objet de type reponse_vers_client
        //qui contiendra les infos utiles pour le serveur
        spectacles *ptr_mem_partagee=acceder_shm();//acceder a la zone de mémoire partagée
        reponse_vers_client.requete_type=atol(pid_client);//requete type=pid client
        strcpy(reponse_vers_client.nom_spectacle,nom_spectacle);//placer nom spectacle dans l'objet
        reponse_vers_client.nbr_places_disponible=nbr_places_dispo_spectacle(
        nom_spectacle,ptr_mem_partagee);//remplir champ nombre places disponibles, qui est stocké dans la shm
        int msqId=acceder_msq();//acceder msq
        envoi_msg(msqId,&reponse_vers_client);//envoi objet au serveur
        shmdt(ptr_mem_partagee);//détacher le processus de la shm
        exit(0);//mort du fils
}

int main(int argc, char **argv){

    consultation_nbr_places_spectacle(argv[0],argv[2]);//appel fonction avec comme parametre le nom du spectacle et pid client
    return 0;
}
