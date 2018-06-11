******************   Projet temps réel 2017 - SICOM Phelma    *******************
******************   Débora Kirscher & Damien Chabannes       *******************
Deux fichiers duplex sont fournis dans le dossier test du dossier rtaudio : 
un fichier duplex_temp et un fichier "duplex_freq".
Il faut renommer le fichier que l'on souhaite utiliser en "duplex" avant d'effectuer un make all.





*******************  duplex_temp :   *******************  


A renommer en "duplex" avant d'effectuer un make all dans le dossier "tests". 

Il faut ensuite executer la commande : "./duplex 1 44100 2 2" avec un 
micro et un casque branchés.
Dans le fichier "duplex_temp", 
certains paramètres peuvent être modifiés :

- décommenter la ligne 116 pour afficher dans le terminal les temps de calcul

- modofier le nombre d'échantillons des buffers input et output à la ligne 165

- modifier le nombre d'échantillons de la réponse impulsionnelle à la ligne 186


remarque : duplex_temp a donné une bonne reverberation en salle Z102 avec les 
3 types de micro (Philips, Trust et genius) avec un casque personnel, bufferFrames = 512 
et une réponse impulsionnelle de 10000, 20000 et 30000 échantillons. 
Au delà de 30000 échantillons, le résultat devient mauvais, avec des sortes de saccades.




*******************  duplex_freq :  *******************  
A renommer en "duplex" avant d'effectuer un make all dans le dossier "tests". 

Il faut ensuite executer la commande : "./duplex 1 44100 2 2" avec un micro et un casque branchés.

Dans le fichier "duplex_freq", certains paramètres peuvent être modifiés :

- décommenter la ligne 134 pour afficher dans le terminal les temps de calcul

- le nombre d'échantillons des buffers input et output à la ligne 182

- le nombre d'échantillons de la réponse impulsionnelle à la ligne 204


remarque : duplex_freq a donné une bonne reverbation en salle z102 avec les 3 types de micro 
avec bufferframes = 512 et une réponse impulsionnelle de 10000, 20000, 30000, et 40000 échantillons. 
Au dela de 500000 échantillons, le résultat est toujours correct mais des distorsions apparaissent et 
des échos ne sonnant pas naturels apparaissent.







*******************  Remarques générales *******************

Bien penser à baisser le son du micro (ne pas mettre d'amplification) et à monter le son du 
casque pour pouvoir tout de même s'entendre.


Quelques temps de calcul :

-- en temporel : 
0.008 pour bufferframes = 512 échantillons et tailleRepImp = 20000 échantillons

-- 
en fréquentiel :
0.0053 pour bufferframes = 512 échantillons et tailleRepImp = 20000 échantillons

0.012 pour bufferFrames = 512 échantillons et tailleRepImp = 60000 échantillons


Mdp pdf : phelma