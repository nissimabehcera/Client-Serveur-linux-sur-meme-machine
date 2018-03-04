                              //serveur_fork.c
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
#include "header_serveur.h"

int shmid,msqId;
spectacles *ptr_mem_partagee;
void handler(int a){

    if(a==SIGINT){//si signal SIGINT recu
        shmdt(ptr_mem_partagee);//detachement du processus a la shm
        shmctl(shmid,IPC_RMID,NULL);//supprimer la shm
        msgctl(msqId,IPC_RMID,NULL);//supprimer msq
        wait(0);//attendre mort des fils
        exit(0);//fin du processus
    }

}
int main(int argc, char **argv)
{
    signal(SIGINT,handler);//associer signal SIGINT a son handler
    ptr_mem_partagee=NULL;//pointeur type spectacles initialisé a null
    reponse_vers_client reponse_vers_client;//objet contenant réponse du serveur
    memset(&reponse_vers_client,0,sizeof reponse_vers_client);
    ptr_mem_partagee=init_shm(ptr_mem_partagee);//initialisation shm, recupere l'id
    init_tab_spectacle_shm(ptr_mem_partagee);//initialisation tableau dans shm(nom spectacles + nombre places)
    msqId=create_msq();//creation msq
    do{

        receive_msg(msqId,&reponse_vers_client,10);//recoit messages de clients
        //declaration tableau de char qui stocke le nombre de places souhaitèes
        char nbr_places_souhaite_tab[4];
        char pid_client_tab[12];//tableau char  pid client
        int pid_client=reponse_vers_client.pid_client;
        printf("pid_client%d\n",pid_client );
        sprintf(pid_client_tab,"%d",pid_client);
        if(reponse_vers_client.requete_id==2){//id=2 =>reservation
            int nbr_places_souhaite=reponse_vers_client.nbr_places_souhaite;//récupere dans la variable nbr_places_souhaite le contenu du champ nbr_places_souhaite de la structure
            sprintf(nbr_places_souhaite_tab,"%d",nbr_places_souhaite);//copie valeur nbr_places_souhaite dans tableau de char tab2
            fork_child_serveur(reponse_vers_client.nom_spectacle,"serveur_reservation.o",nbr_places_souhaite_tab,pid_client_tab);
            //fork processus reservation
        }else if (reponse_vers_client.requete_id==1){//id=1 =>consultation
            fork_child_serveur(reponse_vers_client.nom_spectacle,"serveur_consultation.o",nbr_places_souhaite_tab,pid_client_tab);
            //fork processus consultation

        }
        wait(0);//attente de la mort du fils (consultation ou reservation)
}while(1);
    return 0;
}
