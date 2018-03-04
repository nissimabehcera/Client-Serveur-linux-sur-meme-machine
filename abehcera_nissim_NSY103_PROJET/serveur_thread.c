                              //serveur_thread.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <signal.h>
#include <pthread.h>
#include "header_serveur.h"

spectacles *ptr_mem_partagee;
int shmid,msqId;
pthread_t thread_serveur;
void handler_thread(int a){

    if(a==SIGINT){//si signal SIGINT recu
        shmdt(ptr_mem_partagee);//detachement du processus a la shm
        shmctl(shmid,IPC_RMID,NULL);//supprimer shm
        msgctl(msqId,IPC_RMID,NULL);
        pthread_join(thread_serveur,NULL);
        exit(0);//fin du processus

    }

}
void reservation_places(parametre_fonction *parametre_fonction){

    printf("\nreservation\n");
    reponse_vers_client reponse_serveur;//objet envoyé au client
    int ack=0;//ack=1 si reservation effectuée 0 sinon
    strcpy(reponse_serveur.nom_spectacle,parametre_fonction->nom_spectacle);//copie nom spectacle dans objet
    reponse_serveur.nbr_places_souhaite=parametre_fonction->nbr_places_souhaite;//copie dans objet nombre de places souhaitee
    reponse_serveur.nbr_places_disponible=update_nbr_places_libres_serveur(parametre_fonction->nom_spectacle,parametre_fonction->ptr_shm,parametre_fonction->nbr_places_souhaite,&ack);
    reponse_serveur.requete_type=parametre_fonction->pid_client;//requete_type=pid client

    reponse_serveur.ack=ack;
    envoi_msg(parametre_fonction->msq_id,&reponse_serveur);//envoi objet au client
    pthread_exit(NULL);//fin de la thread
}
void consultation_nb_places_spectacle(parametre_fonction *parametre_fonction){

        printf("\nconsultation\n");
        reponse_vers_client reponse_vers_client;
        reponse_vers_client.requete_type=parametre_fonction->pid_client;//requete_type=pid client
        strcpy(reponse_vers_client.nom_spectacle,parametre_fonction->nom_spectacle);
        reponse_vers_client.nbr_places_disponible=nbr_places_dispo_spectacle(
        parametre_fonction->nom_spectacle,parametre_fonction->ptr_shm);//renvoi le nombre de places disponible
        envoi_msg(parametre_fonction->msq_id,&reponse_vers_client);//envoi objet au client
        pthread_exit(NULL);//fin de la thread
}
void
start_pthread(pthread_t *thread, void *fonction,parametre_fonction *parametre_fonction){
	if(pthread_create(thread,NULL,fonction,(void *)parametre_fonction)!=0){
		perror("thread create");
		exit(EXIT_FAILURE);
	}//creation de la thread, si erreur arret immediat de la fonction
}

int main(int argc, char **argv)
{
    signal(SIGINT,handler_thread);//associe un handler au signal sigint
    ptr_mem_partagee=NULL;//pointeur pour shm initialisé a null
    //pthread_t thread_serveur;//declaration thread
    reponse_vers_client requete_client;//objet renvoye au client
   	parametre_fonction parametre_fonction;//objet passé en parametre de pthread_create
    ptr_mem_partagee=init_shm(ptr_mem_partagee);//initialisation shm
    init_tab_spectacle_shm(ptr_mem_partagee);//creation liste spectacle (nom + nombre places) dans shm
    msqId=create_msq();//id msq
    do{

        receive_msg(msqId,&requete_client,10);//recevoir messages depuis client
        parametre_fonction.ptr_shm=ptr_mem_partagee;//copie pointeur shm en parametre thread
        parametre_fonction.msq_id=msqId;//envoyer pointeur shm et id msq a la thread
        parametre_fonction.pid_client=requete_client.pid_client;
        if(requete_client.requete_id==2){//si requete_id==2 =>demande de reservation
            strcpy(parametre_fonction.nom_spectacle,requete_client.nom_spectacle);
            parametre_fonction.nbr_places_souhaite=requete_client.nbr_places_souhaite;
            start_pthread(&thread_serveur,&reservation_places,&parametre_fonction);//creation thread
        }else{//sinon demande de consultation
        	strcpy(parametre_fonction.nom_spectacle,requete_client.nom_spectacle);
          //creation thread avec objet parametre_fonction comme parametre
          start_pthread(&thread_serveur,&consultation_nb_places_spectacle,&parametre_fonction);
        }
        pthread_join(thread_serveur,NULL); //synchronisation threads
    }while(1);

        return 0;
}
