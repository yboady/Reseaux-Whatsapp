#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server.h"
#include "client.h"

typedef struct
{
   char nom[BUF_SIZE];
   Client membres[10];
   int nombre;
   char mdp[BUF_SIZE];
} Groupe;

Groupe Groupes[10];
int nbGroupes = 0;

enum
{
   mp = 1,
   create,
   join
};

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
         int usurpateur = 1;
         for (i = 0; i < actual - 1; i++)
         {
            if (strcmp(c.name, clients[i].name) == 0)
            {
               closesocket(clients[actual - 1].sock);
               remove_client(clients, actual - 1, &actual);
               strncpy(buffer, c.name, BUF_SIZE - 1);
               strncat(buffer, " someone else tried to connect with your UserName", BUF_SIZE - strlen(buffer) - 1);
               write_client(clients[i].sock, buffer);
               usurpateur = 0;
               break;
            }
         }
         if (usurpateur)
         {
            write_client(c.sock, "Vous êtes connecté à Whatsapp");
            /*Récupération historique*/
            FILE *filePointer;
            int bufferLength = 255;
            char buffer[bufferLength];
            char buffer2[bufferLength];

            if ((filePointer = fopen(c.name, "r")))
            {
               
               fseek(filePointer, -1, SEEK_END);
               char courant;
               int compteur = 0;
               while(ftell(filePointer))
               {
                  courant = fgetc(filePointer);
                  if (courant == '-')
                  {
                     compteur++;
                  }
                  else
                  {
                     compteur = 0;
                  }
                  fseek(filePointer, -2, SEEK_CUR);
                  if (compteur == 5) break;
               }

               while (fgets(buffer, bufferLength, filePointer))
               {
                  write_client(c.sock, buffer);
               }
               fclose(filePointer);
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
               char *arg2;

               //On récupère le premier argument qui va nous servir de commande

               if (reste != NULL)
               {  
                  size_t lengthOfFirst = reste - buffer;
                  cmd = (char *)malloc((lengthOfFirst + 1) * sizeof(char));
                  strncpy(cmd, buffer, lengthOfFirst); // "hello"
               }
               else
               {  

                  //Si jamais la ligne de commande renseignée ne comporte qu'un seul argument

                  if(strcmp(buffer , "quit") == 0){
                     printf("--------------------DEBUT du if \"deco\"--------------------\n");
                     printf("%s s'est déconnecté\n", client);
                     closesocket(client.sock);
                     remove_client(clients, i, &actual);
                     
                     FILE *f = NULL;
                     f = fopen(client.name, "a+");
                     fputs("-----", f);
                     fputs("\n", f);
                     fclose(f);
                     printf("--------------------FIN du if \"deco\"--------------------\n");
                  }else if(strcmp(buffer , "loggroupe") == 0){
                     printf("--------------------DEBUT du if \"loggroupe\"--------------------\n");
                     for (int l = 0; l < nbGroupes; l++)
                     {

                        printf("Le groupe numéro [%d] a pour nom [%s] et pour mdp [%s]\n", l, Groupes[l].nom, Groupes[l].mdp);
                     }
                     printf("--------------------FIN du if \"loggroupe\"--------------------\n");
                  }

                  //Si on ne reconnait pas l'argument alors on relance simplement la boucle

                  //write_client(clients[i].sock, "Usage : [cmd][nom][contenu] faute de copie\n");
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
                  // send_message_to_all_clients(clients, client, actual, buffer, 1);
               }
               else
               {

                  char *nouveau_message = strchr(reste, ' '); // " world2";
                  // nouveau_message = nouveau_message + 1;
                  if (nouveau_message != NULL)
                  {
                     size_t lengthOfSecond = nouveau_message - reste;
                     arg2 = (char *)malloc((lengthOfSecond + 1) * sizeof(char));
                     strncpy(arg2, reste, lengthOfSecond); // "hello"
                  }
                  else
                  {

                     printf("Le message final est null\n");
                     printf("%s\n", cmd);
                     continue;
                  }
                  nouveau_message = nouveau_message + 1;

                  if (strcmp(cmd, "mp") == 0)
                  {
                     printf("--------------------DEBUT du if \"mp\"--------------------\n");
                     send_private_message(clients, client, actual, nouveau_message, arg2, 0);
                     printf("[cmd] = %s\n", cmd);
                     printf("[message] = %s\n", nouveau_message);
                     printf("--------------------FIN du if \"mp\"--------------------\n");
                  }
                  else if (strcmp(cmd, "create") == 0)
                  {
                     printf("--------------------DEBUT du if \"create\"--------------------\n");
                     printf("%s\n", cmd);
                     printf("%s\n", nouveau_message);
                     printf("%d\n", nbGroupes);
                     create_group(clients, client, actual, arg2, nouveau_message);
                     printf("[cmd] = %s\n", cmd);
                     printf("[message] = %s\n", nouveau_message);
                     printf("--------------------FIN du if \"create\"--------------------\n");
                  }
                  else if (strcmp(cmd, "mg") == 0)
                  {
                     printf("--------------------DEBUT du if \"mg\"--------------------\n");
                     send_message_to_group(clients, client, actual, nouveau_message, arg2, 0);
                     printf("Je suis dans l'envoie de message au groupe\n");
                     printf("[cmd] = %s\n", cmd);
                     printf("[message] = %s\n", nouveau_message);
                     printf("--------------------FIN du if \"mg\"--------------------\n");
                  }
                  else if (strcmp(cmd, "join") == 0)
                  {
                     printf("--------------------DEBUT du if \"join\"--------------------\n");
                     if (join_group(client, arg2, nouveau_message))
                        send_message_to_group(clients, client, actual, "Je viens de rejoindre la meute\n", arg2, 0);
                     printf("Je suis dans l'ajout au groupe\n");
                     printf("[cmd] = %s\n", cmd);
                     printf("[message] = %s\n", nouveau_message);
                     printf("--------------------FIN du if \"join\"--------------------\n");
                  }
                  else if (strcmp(cmd, "loggroupe") == 0)
                  {
                     printf("--------------------DEBUT du if \"loggroupe\"--------------------\n");
                     for (int l = 0; l < nbGroupes; l++)
                     {

                        printf("Le groupe numéro [%d] a pour nom [%s] et pour mdp [%s]\n", l, Groupes[l].nom, Groupes[l].mdp);
                     }
                     printf("--------------------FIN du if \"loggroupe\"--------------------\n");
                  }
                  else if (strcmp(cmd, "deco") == 0)
                  {
                     printf("--------------------DEBUT du if \"deco\"--------------------\n");

                     closesocket(clients[i].sock);
                     remove_client(clients, i, &actual);
                     printf("%s s'est déconnecté\n", clients[i].name);
                     FILE *f = NULL;
                     f = fopen(clients[i].name, "a+");
                     fputs("-----", f);
                     fputs("\n", f);
                     fclose(f);
                     printf("--------------------FIN du if \"deco\"--------------------\n");
                  }
                  else
                  {
                     continue;
                  }

                  // send_message_to_all_clients(clients, client, actual, message, 0);
                  // send_private_message(clients, client, actual, message, cible, 0);
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
   if (from_server == 0)
   {
      strncpy(message, sender.name, BUF_SIZE - 1);
      strncat(message, " : ", sizeof message - strlen(message) - 1);
   }
   strncat(message, buffer, sizeof message - strlen(message) - 1);
   for (i = 0; i < actual; i++)
   {
      if (sender.sock != clients[i].sock && strcmp(clients[i].name, cible) == 0)
      {

         write_client(clients[i].sock, message);
      }
   }
   FILE *fichier = NULL;
   fichier = fopen(cible, "a+");
   fputs(message, fichier);
   fputs("\n", fichier);
   fclose(fichier);
}

static void send_message_to_group(Client *clients, Client sender, int actual, const char *buffer, char *nomGroupe, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   char envoyeur[BUF_SIZE];
   envoyeur[0] = 0;
   if (from_server == 0)
   {
      strncpy(envoyeur, sender.name, BUF_SIZE - 1);
      strncat(envoyeur, " : ", sizeof envoyeur - strlen(message) - 1);
      strncpy(message, nomGroupe, BUF_SIZE - 1);
      strncat(message, " - ", sizeof message - strlen(message) - 1);
   }
   strncat(envoyeur, buffer, sizeof envoyeur - strlen(envoyeur) - 1);
   strncat(message, envoyeur, sizeof message - strlen(message) - 1);

   printf("Je suis dans la méthode d'envoie au groupe\n");
   for (int j = 0; j < nbGroupes; j++)
   {
      if (strcmp(Groupes[j].nom, nomGroupe) == 0)
      {
         for (int k = 0; k < Groupes[j].nombre; k++)
         {

            if (strcmp(Groupes[j].membres[k].name, sender.name) == 0)
            {
               for (i = 0; i < Groupes[j].nombre; i++)
               {
                  if (strcmp(Groupes[j].membres[i].name, sender.name) != 0)
                  {
                     for (int p = 0; p < actual; p++)
                     {
                        if (strcmp(clients[p].name, Groupes[j].membres[i].name) == 0)
                           write_client(Groupes[j].membres[i].sock, message);
                     }
                     FILE *fichier = NULL;
                     fichier = fopen(Groupes[j].membres[i].name, "a+");
                     fputs(message, fichier);
                     fputs("\n", fichier);
                     fclose(fichier);
                  }
               }
            }
         }
      }
      else
      {
         printf("Je n'ai pas trouver de groupe à ce nom\n");
      }
   }
}

static void create_group(Client *clients, Client sender, int actual, char *NomGroupe, char *mdp)
{
   printf("Je suis dans la méthode de création de groupe\n");
   if (nbGroupes < 10)
   {
      Groupe NouveauGroupe;
      NouveauGroupe.membres[0] = sender;
      NouveauGroupe.nombre = 1;

      // Passage par une copie de la chaîne de caractère afin d'éviter les problèmes liés à de la surcharge de pointeur

      strcpy(NouveauGroupe.nom, NomGroupe);
      // NouveauGroupe.nom = buffer_temp;

      strcpy(NouveauGroupe.mdp, mdp);
      // NouveauGroupe.mdp = buffer_temp2 ;

      // strcpy(NouveauGroupe.mdp , mdp);
      printf("Un groupe vient de se créer avec comme nom %s et mdp %s\n", NomGroupe, mdp);
      printf("Un groupe vient de se créer avec comme nom %s et mdp %s\n", NouveauGroupe.nom, NouveauGroupe.mdp);
      Groupes[nbGroupes] = NouveauGroupe;
      nbGroupes++;
      printf("Un groupe vient de se créer avec comme nom %s et mdp %s\n", Groupes[nbGroupes - 1].nom, Groupes[nbGroupes - 1].mdp);
      write_client(sender.sock, "\"La meute a été créée avec succès !\"\n");
   }
   else
   {
      printf("Il n'y a plus de groupes disponibles\n");
   }

   // free(NomGroupe);
   // free(mdp);
}

static int join_group(Client sender, char *NomGroupe, char *motDePasse)
{

   printf("Je suis dans la méthode de rejoint de groupe\n");
   printf("Je vais tester pour %s avec le mdp %s\n", NomGroupe, motDePasse);
   for (int i = 0; i < nbGroupes; i++)
   {

      printf("Test du groupe %s contenant le mdp %s\n", Groupes[i].nom, Groupes[i].mdp);

      if (strcmp(Groupes[i].nom, NomGroupe) == 0 && strcmp(Groupes[i].mdp, motDePasse) == 0)
      {

         if (Groupes[i].nombre < 10)
         {

            Groupes[i].membres[Groupes[i].nombre] = sender;
            Groupes[i].nombre++;
            printf("Ajout réussi\n");
            write_client(sender.sock, "Vous faites partie de la meute\n");
            return 1;
         }
         else
         {

            printf("Trop de monde dans ce groupe\n");
         }
      }
      else
      {

         printf("Aucun groupe à ce nom ou mdp faux\n");
      }

   }
   return 0;
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
