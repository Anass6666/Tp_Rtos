# Tp_Rtos
 
0.1 Premiers pas: 
0.1.1. Où se situe le fichier main.c ?
Le fichier main.c se trouve dans le dossier Core/Src. C'est le fichier principal qui contient le code source du programme.

0.1.2. À quoi servent les commentaires indiquant BEGIN et END (balises) ?
Ces balises permettent de délimiter des sections de code que l'outil de génération de STM32CubeIDE ne doit pas modifier lors de la génération automatique du code. Cela permet d'insérer du code personnalisé sans qu'il soit écrasé lors des mises à jour automatiques .

0.1.3. Quels sont les paramètres à passer à HAL_Delay et HAL_GPIO_WritePin ?

HAL_Delay requiert un paramètre Delay qui spécifie la durée du délai en millisecondes.
HAL_GPIO_WritePin nécessite les paramètres GPIOx pour sélectionner le port GPIO, GPIO_Pin pour spécifier le numéro de pin, et PinState pour définir l'état du pin (HAUT ou BAS) .
0.1.4. Dans quel fichier les ports d’entrée/sorties sont-ils définis ?
Les ports d'entrée/sortie sont définis dans le fichier gpio.h, qui contient les configurations et définitions relatives aux GPIO .


FreeRTOS : tâches et sémaphores:


1.1.1. En quoi le paramètre TOTAL_HEAP_SIZE a-t-il de l'importance ?
Avec une taille de 15360 bytes, ce paramètre détermine la quantité de mémoire dynamique disponible pour FreeRTOS. Une taille adéquate est cruciale pour la création et la gestion des objets de FreeRTOS comme les tâches et les sémaphores, évitant ainsi les échecs de création dus à un manque de mémoire.

1.1.2. Quel est le rôle de la macro portTICK_PERIOD_MS ?
Cette macro facilite la conversion des périodes de temps en millisecondes en nombre de ticks système, permettant une abstraction du temps indépendamment de la fréquence du processeur et de la configuration de FreeRTOS.

1.2.6. Changez les priorités. Expliquez les changements dans l’affichage.
Modifier les priorités influence l'ordonnancement des tâches. Une tâche avec une priorité plus élevée sera exécutée plus fréquemment, préemptant les tâches de priorité inférieure. Cela affecte la fréquence d'affichage des messages des tâches, selon leur nouvelle priorité.

1.5.11. Observez attentivement la sortie dans la console. Expliquez d’où vient le problème.
Le problème provient de l'accès concurrent à printf ou d'autres fonctions d'I/O par plusieurs tâches sans synchronisation appropriée, menant à des corruptions de données. Cela est dû à l'accès non protégé à des ressources partagées.

1.5.12. Proposez une solution en utilisant un sémaphore Mutex.
Pour résoudre les problèmes de concurrence, utilisez un Mutex :

Déclarer le Mutex : SemaphoreHandle_t xMutex;
Créer le Mutex : Dans le main(), initialisez le mutex avec xMutex = xSemaphoreCreateMutex();
Utiliser le Mutex autour de printf : Acquérez le mutex avant printf et relâchez-le ensuite pour protéger l'accès à la fonction entre les tâches.


2 On joue avec le Shell


2. Que se passe-t-il si l’on ne respecte pas les priorités décrites précédemment ?
   Réponse :

3. Écrire une fonction led(), appelable depuis le shell, permettant de faire cli-gnoter la LED (PI1 sur la carte). Un paramètre de cette fonction configure la periode de clignotement. Une valeur de 0 maintient la LED éteinte.
Le clignotement de la LED s’effectue dans une tâche. Il faut donc trouver un moyen de faire communiquer *proprement* la fonction led avec la tâche de clignotement.
   Réponse:
   



4.Écrire une fonction spam(), semblable à la fonction led() qui affiche du textedans la liaison série au lieu de faire clignoter les LED. On peut ajouter comme argument le message à afficher et le nombre de valeurs à afficher. Ce genre de fonction peut être utile lorsque l’on travaille avec un capteur.
 Réponse:







 3 Debug, gestion d’erreur et statistiques

 3.1 Gestion du tas

 1. Quel est le nom de la zone réservée à l’allocation dynamique ?
    Réponse:

2. Est-ce géré par FreeRTOS ou la HAL ?
   Réponse:

4. Notez la mémoire RAM et Flash utilisée:


6.Notez la nouvelle utilisation mémoire:


8. Notez la nouvelle utilisation mémoire aprés modification de la taille du tas (TOTAL_HEAP_SIZE)


3.2 Gestion des piles:


3. Écrivez la fonction vApplicationStackOverflowHook. (Rappel : C’est une fonction appelée automatiquement par FreeRTOS, vous n’avez pas à l’appeler vous-même)


4.Débrouillez vous pour remplir la pile d’une tâche pour tester. Notez que, vu le contexte d’erreur, il ne sera peut-être pas possible de faire grand chose dans cette fonction. Utilisez le debugger.





5. Il existe d’autres hooks. Expliquez l’intérêt de chacun d’entre eux.

6.  Réponse :


3.3 Statistiques dans l’IDE




 






