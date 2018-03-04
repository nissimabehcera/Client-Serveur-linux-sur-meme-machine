                                //header_serveur.h
#ifndef HEADER_SERVEUR_H_INCLUDED
#define HEADER_SERVEUR_H_INCLUDED
#define TAILLE_SPECTACLE 11
#define TAILLE 5

typedef struct{//structure dans shm
    char nom_spectacle[TAILLE_SPECTACLE];
    unsigned int nbr_places_dispo;
}spectacles;

typedef struct{//structure entre client et serveur
    long requete_type;
    char nom_spectacle[TAILLE_SPECTACLE];
    unsigned int requete_id;
    unsigned int pid_client;
    unsigned int nbr_places_disponible;
    unsigned int nbr_places_souhaite;
    unsigned int ack;

}reponse_vers_client;

typedef struct {//structure en parametre de la thread

    char nom_spectacle[TAILLE_SPECTACLE];
	  unsigned int nbr_places_souhaite;
    spectacles *ptr_shm;
    int msq_id;
    int pid_client;
}parametre_fonction;

extern spectacles spectacle[TAILLE];
spectacles *acceder_shm();
int acceder_msq();
void envoi_msg(int msqId,reponse_vers_client *reponse_serveur);
spectacles *init_shm(spectacles *ptr_mem_partagee);
void init_tab_spectacle_shm(spectacles *spectacle);
int update_nbr_places_libres_serveur(const char nom_spectacle[],spectacles *ptr,const int nbr_places_souhaite,int *ack);
//void handler(int a);
void fork_child_serveur(const char nom_spectacle_souhaite[],const char *path_to_executable,const char nbr_places_souhaite[],const char pid_client[]);
unsigned int nbr_places_dispo_spectacle(const char nom_spectacle[],const spectacles *ptr);
int create_msq();
void receive_msg(int msqId,reponse_vers_client *reponse_serveur,long type_requete);
void handler_thread(int a);
#endif // HEADER_SERVEUR_H_INCLUDED
