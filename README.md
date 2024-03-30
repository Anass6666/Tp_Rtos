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
