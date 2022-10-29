A lire pour comprendre le projet de messagerie de Hugo LAFACE et Yanice BOADY

%%%%%%%%%%%%%%%%%%%%%%%%%%%
- COMPILATIONS PREALABLES -
%%%%%%%%%%%%%%%%%%%%%%%%%%%

1. Pour utiliser le code, compiler server.c dans Server/
gcc server.c -o server

2. Ainsi que client.c dans Client/
gcc client.c -o client

%%%%%%%%%%%%%%%%%%
- LANCER LE CODE -
%%%%%%%%%%%%%%%%%%

1. Pour lancer le server, lancer la commande suivante dans Server/
./server

2. Pour se connecter en tant qu'utilisateur (client), lancer la commande suivante dans Client/
./client [IP du server] [identifiant]
L'identifiant est un nom unique choisi par chaque utilisateur, c'est le nom qui sera perçu lorsqu'il enverra des message

Pour ajouter d'autres utilisateurs, il suffit de lancer la commande 2. dans différents clients.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
- COMMANDES DISPONIBLES POUR UN CLIENT -
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

1. Envoyer un message privé 
mp [destinataire] [contenu du message]
Si le destinataire est connecté, il reçoit directement le message, sinon, il le verra à sa prochaine connexion

2. Créer un groupe
create [nom du groupe] [mot de passe du groupe]
Le créateur du groupe y est automatiquement ajouté et c'est alors le seul membre

3. Rejoindre un groupe
join [nom du groupe] [mot de passe du groupe]

4. Envoyer un message à un groupe
mg [nom du groupe] [contenu du message]
Si les destinataires sont connectés, ils reçoivent directement le message, sinon, ils le verront à leur prochaine connexion

5. Se déconnecter du server
quit
Tous les messages reçus pendant cette période de déconnexion appaitront lors de la prochaine connexion avec le même identifiant