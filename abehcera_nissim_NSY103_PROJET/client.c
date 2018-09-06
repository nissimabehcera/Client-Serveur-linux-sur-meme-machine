                                //client.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#define CLE_MSQ 1990
#define TAILLE 5
#define TAILLE_TAB 11



//test v2.0
typedef struct{//structure acheminant les informations (consultation/reservation)entre client et
    //serveur
    long requete_type;
    char nom_spectacle[TAILLE_TAB];
    unsigned int requete_id;
    unsigned int pid_client;
    unsigned int nbr_places_disponible;
    unsigned int nbr_places_souhaite;
    unsigned int ack;

}requete;

//retourne 1 si nom spectacle present dans liste spectacle, 0 sinon
int choix_spectacle(char indice_spectacle[]){

    int i,indice_num;
    char indice[3];
    for(i=0; i<TAILLE;i++){
        indice_num=i+1;
        sprintf(indice,"%d",indice_num);//cast int =>char []
        if(strcmp(indice_spectacle,indice)==0)return 1;
    }
    return 0;
}
//envoi un message dans la file, au serveur, si erreur exit()
void envoi_msg(int msqId,requete *requete_vers_serveur){

    int rep=msgsnd(msqId,requete_vers_serveur,sizeof(*requete_vers_serveur),0);
    if(rep==-1){
        perror("msgsnd process consultation");
        exit(EXIT_FAILURE);
    }
}
//recupere un message provenant du serveur de la file de msg
void receive_msg(int msqId,requete *reponse_serveur){

    while(1){
        int taille=msgrcv(msqId,reponse_serveur,sizeof(*reponse_serveur),getpid(),0);
        if(taille==-1){//si erreur
            perror("msgrcv process consultation 1");
            exit(EXIT_FAILURE);//arret immediat de la fonction
        }
        if(taille>0)break;//si reception de données, arret de la boucle
    }
}
//affiche les spectacles present dans la liste
void affiche_spectacles(char spectacles[][11]){
    int i;
    printf("\n");
    for(i=0; i<TAILLE;i++){
        if(i<TAILLE-1)
            printf("%d)%s\n",(i+1),spectacles[i]);
        else
            printf("%d)%s ",(i+1),spectacles[i]);
    }
}
//nettoie le buffer
static void purger(void){
    int c;
    while ((c = getchar()) != '\n' && c != EOF){}

}

static void clean (char *chaine)

{
    char *p = strchr(chaine, '\n');//si saut de ligne rencontre
    if(p){
        *p = 0;//supprimer saut de ligne
    }
    else{
        purger();
    }
}
void requete_consultation(char indice_spectacle[],int msqId,requete *requete){

    char nom_spectacle[11];//stocke le nom du spectacle a consulter
    requete->requete_type=10;//requete type serveur=10
    strcpy(nom_spectacle,"spectacle");//copie nom spectacle
    strcat(nom_spectacle,indice_spectacle);//rajoute numero spectacle
    strcpy(requete->nom_spectacle,nom_spectacle);//copie le nom du spectacle a consulter
    requete->requete_id=1;//1 =>demande de consultation
    requete->pid_client=getpid();//rajoute pid client
    envoi_msg(msqId,requete);//envoi structure vers serveur
    receive_msg(msqId,requete);//retour structure avec nombre de places dispo
    printf("\nLe nombre de places disponible est de :  %u\n",requete->nbr_places_disponible);
    //affiche nombre de places disponible
}
void requete_reservation(char indice_spectacle[],int msqId,int nbr_places,requete *requete){

    char nom_spectacle[11];//stocke nom spectacle
    requete->requete_type=10;//requete type serveur =10
    requete->nbr_places_souhaite=nbr_places;//affectation du nombre de places souhaitees
    strcpy(nom_spectacle,"spectacle");//copie mot spectacle
    strcat(nom_spectacle,indice_spectacle);//rajoute numero spectacle
    strcpy(requete->nom_spectacle,nom_spectacle);//copie du nom du spectacle concerne
    requete->requete_id=2;// id=2 =>demande reservation
    requete->pid_client=getpid();//rajoute pid client
    envoi_msg(msqId,requete);//envoi structure au serveur
    receive_msg(msqId,requete);//reponse du serveur
    if(requete->ack==1){//ack=1 =>reservation effectuée
        printf("\nReservation réussie\nIl reste %u place(s) libre(s)\n",requete->nbr_places_disponible);
    }
    else{//reservation rejetée
        printf("\nEchec de la réservation\nLe nombre de place(s) disponible(s) est  :  %u\n",requete->nbr_places_disponible);
    }
}
int acceder_msq(){
    int msqId=msgget(CLE_MSQ,0);//acceder a la msq
    if(msqId==-1){//si erreur
        perror("msgget client");//affiche erreur
        exit(EXIT_FAILURE);//fin de la fonction
    }
    return msqId;
}

int main(void)
{
    int msqId=acceder_msq();//variable stockant l'id msq
    int choix_menu=0;//variable stockant le choix utilisateur
    requete requete_vers_serveur;//objet qu'on envoi au serveur
    memset(&requete_vers_serveur,0,sizeof requete_vers_serveur);//mettre a 0 tous les champs
    int nbr_places=0;
    char spectacles[TAILLE][11]={"spectacle1","spectacle2","spectacle3",
                            "spectacle4","spectacle5"};//liste nom spectacles
    char choix_user[3],choix_user_nbr_places[5];
    char choix_spectacle_indice[3];

    do{
        do{
        printf("\n1)Consultation\n2)Reservation\n3)Arret du programme \n");//menu programme
        fgets(choix_user,3,stdin);//saisie 1 ou 2 ou 3
        clean(choix_user);//nettoyer le buffer
        }while(strcmp(choix_user,"1")!=0 && strcmp(choix_user,"2")!=0 && strcmp(choix_user,"3")!=0);//boucle tant que le choix utilisateur different de 1 ou 2 ou 3
        choix_menu=atoi(choix_user);//conversion char =>int du choix utilisateur
        if(choix_menu==3){//si arret du programme

            printf("\nAu revoir\n");
            break;//sortie de boucle
        }
        else if(choix_menu==1){//si consultation souhaitée
            do{
                printf("\nQuel spectacle voulez-vous consulter ? : \n");
                affiche_spectacles(spectacles);//affiche liste spectacles
                fgets(choix_spectacle_indice,3,stdin);//saisie du numero de spectacle
                clean(choix_spectacle_indice);//nettoie le buffer
            }while(choix_spectacle(choix_spectacle_indice)!=1);
            requete_consultation(choix_spectacle_indice,msqId,&requete_vers_serveur);
        }
        else if(choix_menu==2){//si reservation souhaitée
            do{
                printf("\nPour quel spectacle voulez-vous reserver des places ? : \n");
                affiche_spectacles(spectacles);//affiche liste spectacles
                fgets(choix_spectacle_indice,3,stdin);//saisie numero spectacle
                clean(choix_spectacle_indice);//nettoie le buffer
            }while(choix_spectacle(choix_spectacle_indice)!=1);
            do{
                printf("\nCombien de places voulez-vous reserver pour le spectacle%s : ",choix_spectacle_indice);
                fgets(choix_user_nbr_places,5,stdin);//saisie nombre places spectacle
                clean(choix_user_nbr_places);//nettoie le buffer
                nbr_places=atoi(choix_user_nbr_places);//cast char =>int
            }while(nbr_places==0);
            requete_reservation(choix_spectacle_indice,msqId,nbr_places,&requete_vers_serveur);
        }

    }while(choix_menu!=3);//tant que l'utilisateur ne tape pas 3
    return 0;
}
