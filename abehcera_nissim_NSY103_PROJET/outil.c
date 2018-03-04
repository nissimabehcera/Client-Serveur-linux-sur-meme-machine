                                  //outil.c
#include "header_serveur.h"
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
#include <pthread.h>
#include <signal.h>
#include <sys/sem.h>
#define TAILLE_TAB 11
#define CLE_MSQ 1990
#define CLE_SHM 1975
#define NBR_SPECTACLES_TAB_SHM 5
int shmid,msqId;
spectacles *ptr_mem_partagee=NULL;

/*fonction qui initialise la structure spectacle,
avec le nom et le nombre de chaque spectacles
et insertion de cette structure dans la mémoire partagée*/
void init_tab_spectacle_shm(spectacles *spectacle){
    int i,j=0;
    char num_spectacle[2];//tableau contenant numero spectacle
    for(i=0; i<NBR_SPECTACLES_TAB_SHM;i++){
        sprintf(num_spectacle,"%d",(++j));//cast int => tableau de char
        //copie mot "spectacle" dans structure
        strcpy(spectacle[i].nom_spectacle,"spectacle");
        //concatenation "spectacle" avec numero//copie mot "spectacle" dans structure
        strcat(spectacle[i].nom_spectacle,num_spectacle);
        spectacle[i].nbr_places_dispo=100*(i+1);//stocke nombre places spectacle

    }
}

void fork_child_serveur(const char nom_spectacle_souhaite[],const char *path_to_executable,const char nbr_places_souhaite[],const char pid_client[]){
    pid_t valeur=fork();//fork
        if(valeur==-1){//si erreur
            perror("fork process serveur");//affichage erreur
            exit(EXIT_FAILURE);//arret immediat de la fonction
        }
        if(valeur==0){//si coté processus fils
            execl(path_to_executable,nom_spectacle_souhaite,nbr_places_souhaite,pid_client,NULL);
            /*recouvrement avec chemin vers executable,
            plus comme parametres: nom spectacle
            et nombre de places souhaites et pid du client*/

        }
}

spectacles *init_shm(spectacles *ptr_mem_partagee){//creation d'une shm
    shmid=shmget((key_t)CLE_SHM,sizeof(spectacle),0750|IPC_CREAT|IPC_EXCL);
    if(shmid==-1){//si erreur
        perror("shmid");//affichage erreur
        exit(EXIT_FAILURE);//arret immediat
    }
    //attachement de la shm
    if((ptr_mem_partagee=shmat(shmid,NULL,0))==(spectacles *)-1){
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    return ptr_mem_partagee;
}
int acceder_msq(){//acceder file message existante, grace a une cle externe
    int msqId=msgget(CLE_MSQ,0);
    if(msqId==-1){
        perror("msgget process serveur");
        exit(EXIT_FAILURE);
    }
    return msqId;
}
int create_msq(){//creation msq a l'aide de la clé externe
    int msqId=msgget(CLE_MSQ,IPC_CREAT|IPC_EXCL|0666);
    if(msqId==-1){
        perror("msqid process client");
        exit(EXIT_FAILURE);
    }
    return msqId;
}
void
receive_msg(int msqId,reponse_vers_client *requete_client,long requete_type){
    while(1){
        //fonction qui permet de recevoir des messages a partir d'une file
        int taille=msgrcv(msqId,requete_client,sizeof(*requete_client),requete_type,0);
        if(taille==-1){//si erreur
            perror("msgrcv process serveur");//affichage erreur
            exit(EXIT_FAILURE);//arret fonction
        }
        if(taille>0)break;//si données recues ,quitte la boucle
    }
}

spectacles *acceder_shm(){
    int shmid=shmget((key_t)CLE_SHM,0,0);//acceder a une shm deja cree
    spectacles *ptr_mem_partagee;//pointeur de type spectacles
    if((ptr_mem_partagee=shmat(shmid,NULL,0))==(spectacles *)-1){
      /*attachement du pointeur
      ptr_mem_partagee a la shm, si erreur fin de la fonction*/
        perror("shmat serveur consultation");
        exit(EXIT_FAILURE);
    }

    return ptr_mem_partagee;
}

int
update_nbr_places_libres_serveur(const char nom_spectacle[],spectacles *ptr,const int nbr_places_souhaite,int *ack){
    int i;
    for(i=0; i<NBR_SPECTACLES_TAB_SHM;i++){
      //si nom_spectacle est dans la liste de la shm
        if(strcmp(nom_spectacle,ptr[i].nom_spectacle)==0){
          /*si nombre de places disponible est superieur ou egale
           au nombre de places souhaitees*/
            if(ptr[i].nbr_places_dispo>=nbr_places_souhaite){
                *ack=1;//si reservation effectuée => ack=1 sinon ack=0
                //semaphore, avec cle externe 15
                int sem_id=semget(15,1,IPC_CREAT|IPC_EXCL|0600);
                struct sembuf operation;
                semctl(sem_id,0,SETVAL,1);
                operation.sem_num=0;
                operation.sem_op=-1;//opration p
                operation.sem_flg=0;//flag=0
                semop(sem_id,&operation,1);//verrouillage ressource critique a 1 jeton
                //le nombre de places dispo apres reservation
                ptr[i].nbr_places_dispo-=nbr_places_souhaite;
                operation.sem_num=0;
                operation.sem_op=1;//operation v
                operation.sem_flg=0;//flag=0
                semop(sem_id,&operation,1);//liberation jetton
                semctl(sem_id,0,IPC_RMID,0);//destruction semaphore
                return ptr[i].nbr_places_dispo;
            }else{
                *ack=0;//reservation non effectuée
                return ptr[i].nbr_places_dispo;//retourne nombre places dispo
            }
        }
    }
    return -1;
}

void envoi_msg(int msqId,reponse_vers_client *reponse_vers_client){
    //fonction servant a envoyer des messages dans la file de messages
    int rep=msgsnd(msqId,reponse_vers_client,sizeof(*reponse_vers_client),0);
    if(rep==-1){//si erreur
        perror("msgsnd process consultation");//affichage erreur
        exit(EXIT_FAILURE);//fin fonction
    }
}

unsigned int
nbr_places_dispo_spectacle(const char nom_spectacle[],const spectacles *ptr){
    int i;
    for(i=0; i<NBR_SPECTACLES_TAB_SHM;i++){
      //si "nom_spectacle" est dans la shm
        if(strcmp(nom_spectacle,ptr[i].nom_spectacle)==0)
            return ptr[i].nbr_places_dispo;//retourne nombre places dispo
    }
    return 0;
}
