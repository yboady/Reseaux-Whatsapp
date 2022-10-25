#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server.h"
#include "client.h"


typedef struct
{
   char nom[BUF_SIZE];
   Client membres [10];
   int nombre;
   char mdp[BUF_SIZE];
} Groupe;

Groupe Groupes [10];
int nbGroupes = 0;


enum {mp=1, create, join};

static void init(void)
{
#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if (err < 0)
   {
      puts("WSAStartup failed !");
      exit(EXIT_FAILURE);
   }
#endif
}

static void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

static void app(void)
{
   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int max = sock;
   /* an array for all clients */
   Client clients[MAX_CLIENTS];

   fd_set rdfs;

   while (1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for (i = 0; i < actual; i++)
      {
         FD_SET(clients[i].sock, &rdfs);
      }

      if (select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if (FD_ISSET(STDIN_FILENO, &rdfs))
      {
         /* stop process when type on keyboard */
         break;
      }
      else if (FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = {0};
         size_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
         if (csock == SOCKET_ERROR)
         {
            perror("accept()");
            continue;
         }

         /* after connecting the client sends its name */
         if (read_client(csock, buffer) == -1)
         {
            /* disconnected */
            continue;
         }

         /* what is the new maximum fd ? */
         max = csock > max ? csock : max;

         FD_SET(csock, &rdfs);

         Client c = {csock};

         strncpy(c.name, buffer, BUF_SIZE - 1);
         clients[actual] = c;
         actual++;

         for (i = 0; i < actual - 1; i++)
         {
            if (strcmp(c.name, clients[i].name) == 0)
            {
               closesocket(clients[actual - 1].sock);
               remove_client(clients, actual - 1, &actual);
               strncpy(buffer, c.name, BUF_SIZE - 1);
               strncat(buffer, " someone else tried to connect with your UserName", BUF_SIZE - strlen(buffer) - 1);
               send_message_to_all_clients(clients, clients[actual], actual, buffer, 1);
            }
         }
      }
      else
      {
         int i = 0;
         for (i = 0; i < actual; i++)
         {
            /* a client is talking */
            if (FD_ISSET(clients[i].sock, &rdfs))
            {
               Client client = clients[i];
               int c = read_client(clients[i].sock, buffer);
               char *reste = strchr(buffer, ' '); // " world"
               char *cmd;
               char * arg2;
               if (reste != NULL)
               {
                  size_t lengthOfFirst = reste - buffer;
                  cmd = (char *)malloc((lengthOfFirst + 1) * sizeof(char));
                  strncpy(cmd, buffer, lengthOfFirst); // "hello"
               } 
               else
               {
                  printf("Usage : [destinataire][message] faute de copie\n");
                  continue;
               }

               reste = reste + 1;

               /* client disconnected */
               if (c == 0)
               {
                  closesocket(clients[i].sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, client.name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_all_clients(clients, client, actual, buffer, 1);
               }
               else
               {  

                  char * nouveau_message = strchr(reste, ' '); // " world";
                  //nouveau_message = nouveau_message + 1;
                  if (nouveau_message != NULL)
                  {
                     size_t lengthOfSecond = nouveau_message - reste;
                     arg2 = (char *)malloc((lengthOfSecond + 1) * sizeof(char));
                     strncpy(arg2, reste, lengthOfSecond); // "hello"
                  }else {

                     printf("Le message final est null\n");
                     printf("%s\n" , cmd);
                     continue;

                  }
                  nouveau_message = nouveau_message + 1;
                  printf("On avance quand même\n");
                  if(strcmp(cmd , "mp") == 0)
                  {  
                     printf("Envoie d'un mp\n");
                     send_private_message(clients, client, actual, nouveau_message, arg2, 0); 

                  }else if(strcmp(cmd , "create") == 0){
                     printf("%s\n" , cmd);
                     printf("%s\n" , nouveau_message);
                     printf("%d\n" , nbGroupes);
                     create_group(clients, client, actual, arg2 , nouveau_message);
                     printf("Je suis aussi venu là\n");
                  }else if(strcmp(cmd , "mg") == 0){

                     send_message_to_group(clients, client, actual, nouveau_message, arg2, 0);
                     printf("Je suis dans l'envoie de message au groupe\n");
                     printf("%s\n" , cmd);
                     printf("%s\n" , nouveau_message);
                  }else if(strcmp(cmd , "join") == 0){

                     join_group(client , arg2 , nouveau_message);
                     printf("Je suis dans l'ajout au groupe\n");
                     printf("%s\n" , cmd);
                     printf("%s\n" , nouveau_message);
                  }else if(strcmp(cmd , "loggroupe") == 0){

                     for (int l = 0 ; l < nbGroupes ; l++){

                        printf("Le groupe numéro [%d] a pour nom [%s] et pour mdp [%s]\n" , l , Groupes[l].nom , Groupes[l].mdp);

                     }
                  }else{
                     continue;
                  }

                  

                  // send_message_to_all_clients(clients, client, actual, message, 0);
                  //send_private_message(clients, client, actual, message, cible, 0);
               }
               break;
            }
         }
      }
   }

   clear_clients(clients, actual);
   end_connection(sock);
}

static void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for (i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

static void remove_client(Client *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

static void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for (i = 0; i < actual; i++)
   {
      /* we don't send message to the sender */
      if (sender.sock != clients[i].sock)
      {
         if (from_server == 0)
         {
            strncpy(message, sender.name, BUF_SIZE - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
         strncat(message, buffer, sizeof message - strlen(message) - 1);
         write_client(clients[i].sock, message);
      }
   }
}

static void send_private_message(Client *clients, Client sender, int actual, const char *buffer, char *cible, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for (i = 0; i < actual; i++)
   {  
      
      if (sender.sock != clients[i].sock && strcmp(clients[i].name, cible) == 0)
      {
         if (from_server == 0)
         {
            strncpy(message, sender.name, BUF_SIZE - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
         strncat(message, buffer, sizeof message - strlen(message) - 1);
         write_client(clients[i].sock, message);
      }else{
         printf("Aucun user n'a ce nom\n");
         printf("%s  to  " , sender.name);
         printf("%s\n" , cible);
         printf("%s\n" , message);
      }
   }
}

static void send_message_to_group(Client *clients, Client sender, int actual, const char *buffer, char *nomGroupe, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   printf("Je suis dans la méthode d'envoie au groupe\n");
   for(int j = 0 ; j < nbGroupes ; j++){

      if( strcmp(Groupes[j].nom,nomGroupe) == 0)
      {

         for (int k = 0; k < Groupes[j].nombre; k++)
         {  

            if(strcmp(Groupes[j].membres[k].name , sender.name) == 0)
            {
               for (i = 0; i < Groupes[j].nombre; i++)
               {
               
                  if (from_server == 0)
                  {  
                     printf("Je suis dans la condition bizarre \n");
                     strncpy(message, sender.name, BUF_SIZE - 1);
                     strncat(message, " : ", sizeof message - strlen(message) - 1);
                  }
                  printf("Je suis dans la boucle d'envoie\n");
                  strncat(message, buffer, sizeof message - strlen(message) - 1);
                  write_client(Groupes[j].membres[i].sock, message);
               }
            }
         }
      }else{
         printf("Je n'ai pas trouver de groupe à ce nom\n");
      }

   }

}
 

static void create_group(Client *clients, Client sender, int actual, char *NomGroupe, char* mdp)
{  
   printf("Je suis dans la méthode de création de groupe\n");
   if (nbGroupes < 10)
   {  
      Groupe NouveauGroupe;
      NouveauGroupe.membres[0] = sender;
      NouveauGroupe.nombre = 1;

      //Passage par une copie de la chaîne de caractère afin d'éviter les problèmes liés à de la surcharge de pointeur

      char buffer_temp[BUF_SIZE];
      strcpy(NouveauGroupe.nom , NomGroupe);
      //NouveauGroupe.nom = buffer_temp;

      char buffer_temp2[BUF_SIZE];
      strcpy(NouveauGroupe.mdp , mdp);
      //NouveauGroupe.mdp = buffer_temp2 ; 

      //strcpy(NouveauGroupe.mdp , mdp);
      printf("Un groupe vient de se créer avec comme nom %s et mdp %s\n" , NomGroupe, mdp);
      printf("Un groupe vient de se créer avec comme nom %s et mdp %s\n" , NouveauGroupe.nom , NouveauGroupe.mdp);
      Groupes[nbGroupes] = NouveauGroupe;
      nbGroupes++;
      printf("Un groupe vient de se créer avec comme nom %s et mdp %s\n" , Groupes[nbGroupes-1].nom , Groupes[nbGroupes-1].mdp);
   }
   else
   {
      printf("Il ny a plus de groupes disponibles\n");
   }

   //free(NomGroupe);
   //free(mdp);
   
}

static void join_group(Client sender, char *NomGroupe , char* motDePasse)
{  

   printf("Je suis dans la méthode de rejoint de groupe\n");
   printf("Je vais tester pour %s avec le mdp %s\n" , NomGroupe , motDePasse );
   for(int i = 0 ; i < nbGroupes ; i++){

      printf("Test du groupe %s contenant le mdp %s\n" , Groupes[i].nom , Groupes[i].mdp);

      if(strcmp(Groupes[i].nom , NomGroupe) == 0 && strcmp(Groupes[i].mdp , motDePasse) == 0){

         if(Groupes[i].nombre < 10){

            Groupes[i].membres[Groupes[i].nombre] = sender;
            Groupes[i].nombre++;

         }else{

            printf("Trop de monde dans ce groupe\n");

         }

      }else{

         printf("Aucun groupe à ce nom ou mdp faux\n");

      }

   }
   
}

static int init_connection(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = {0};

   if (sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if (bind(sock, (SOCKADDR *)&sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer)
{
   int n = 0;

   if ((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}

static void write_client(SOCKET sock, const char *buffer)
{
   if (send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}
