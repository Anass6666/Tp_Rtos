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


# FreeRTOS : tâches et sémaphores:
# 1.1 Tâche simple


1.1.1. En quoi le paramètre TOTAL_HEAP_SIZE a-t-il de l'importance ?
Avec une taille de 15360 bytes, ce paramètre détermine la quantité de mémoire dynamique disponible pour FreeRTOS. Une taille adéquate est cruciale pour la création et la gestion des objets de FreeRTOS comme les tâches et les sémaphores, évitant ainsi les échecs de création dus à un manque de mémoire.
Le paramètre TOTAL HEAP SIZE sert à définir la RAM allouée à FreeRtos , et par conséquent la quantité des Tasks qu’on peut mettre.Ces paramètres sont enregistrés dans le fichier FreeRTOSConfig.h , si on change une de ces paramètres dans configuration de la carte, il y aura un changement dans le fichier FreeRTOSConfig.h

1.1.2. Quel est le rôle de la macro portTICK_PERIOD_MS ?
Cette macro facilite la conversion des périodes de temps en millisecondes en nombre de ticks système, permettant une abstraction du temps indépendamment de la fréquence du processeur et de la configuration de FreeRTOS.

![clignotement_led](https://github.com/Anass6666/Tp_Rtos/assets/145018011/b9fd473b-617f-4a4e-bbda-2d4b56781feb)
# 1.2 Sémaphores pour la synchronisation

1.2.6. Changez les priorités. Expliquez les changements dans l’affichage.
Modifier les priorités influence l'ordonnancement des tâches. Une tâche avec une priorité plus élevée sera exécutée plus fréquemment, préemptant les tâches de priorité inférieure. Cela affecte la fréquence d'affichage des messages des tâches, selon leur nouvelle priorité.

![give 2 take 3](https://github.com/Anass6666/Tp_Rtos/assets/145018011/3284575f-a480-4e85-883a-dab64e2c2f2a) ![give 3 take 2](https://github.com/Anass6666/Tp_Rtos/assets/145018011/03a076b4-30e8-4b9d-94d4-ca18ac8b1d6d)


# 1.5 Réentrance et exclusion mutuelle

Dans notre configuration, nous avons défini les priorités et les délais des tâches comme suit :
#define TASK1_PRIORITY 1
#define TASK2_PRIORITY 2
#define TASK1_DELAY 1
#define TASK2_DELAY 2
Cependant, lors de l'exécution, la sortie affichée est la suivante :Je suis la tâche 2 et je m'endors pour 2 ticks Je suis la tâche 1 et je m'endors pour 2 ticks Je suis la tâche 2 et je m'endors pour 2 ticks Je suis la tâche 2 et je m'endors pour 2 ticks Je suis la tâche 1 et je m'endors pour 2 ticks. 

Ce qui pose problème, car les deux tâches s'endorment pour le même nombre de ticks, alors que la tâche 1 devrait s'endormir pour seulement 1 tick. En examinant le code, nous constatons que les deux tâches accèdent à la fonction printf simultanément, ce qui influe sur la valeur du délai d'attente. Étant donné que la tâche 2 est initialisée en dernier, seule la valeur de TASK2_DELAY est prise en compte.

Pour remédier à cette situation, j'ai encadré l'appel à printf avec une section critique et instauré l'utilisation d'un sémaphore mutex. Cette démarche a abouti à la sortie suivante :
Je suis la tâche 2 et je m'endors pour 2 ticks Je suis la tâche 1 et je m'endors pour 1 tick Je suis la tâche 2 et je m'endors pour 2 ticks Je suis la tâche 1 et je m'endors pour 1 tic

Il faut pas oublié d'initialisez le mutex avec Mutex1 = xSemaphoreCreateMutex();
![solution mutex](https://github.com/Anass6666/Tp_Rtos/assets/145018011/21d837b8-7217-4a96-a994-cd9f816d8135)
# 2 On joue avec le Shell
Dans cette partie l’idée est de communiquer entre le terminal et la carte Stm32.
Le shell est représenté par le fichier shell.c qui contient des fonctions qui seront utilisées lors de la communication, parmi ces fonctions utilisent la connexion uart par le périphérique USART1, et la configuration de la carte reste la même de la première partie. Au début de cette partie, il faut modifier la fonction “fonction” dans main.c pour qu’elle affiche les arguments saisies par l’utilisateur via le terminal, donc il faut afficher les chaînes de caractères envoyés qui sont dans le tableau argv et le nombre des arguments est défini par argc. 
(capture)

les arguments extraits de la ligne de commande sont passés à la fonction cible sous forme d'un tableau de chaînes (char** argv) et leur nombre (int argc) pour les traiter. Après l'exécution de la commande, le shell revient en attente pour la prochaine entrée utilisateur, continuant ainsi la boucle.Pour permettre le fonctionnement simultané du shell avec toutes les tâches sous le planificateur

Le problème de ce fonctionnement est que le shell est toujours actif surtout dans uart_read ,ce qui est inconvénient au niveau de la consommation, pour cela on propose d’utiliser une interruption, dans cette fonction, on peut faire appel à la fonction HAL_UART_Receive_IT qui recoit l’interruption de UART, après elle prend le sémaphore qui active la fonction.

2.3
on passe maintenant à créer des tâches, et gérer leur fonctionnement, on a commençé par la Led.La première chose est configurer le pin PI1 (Led dans la carte ) comme sortie qui sera utilisé dans cette partie, après on passe à la création de la fonction led . La tache de Led , elle permet de lire les données dans la boîte aux lettres xQueue1 et faire
clignoter led.La communication entre la tâche vTaskLed et la fonction Led se fait par boîte aux lettres pour transmettre les données reçus par l’UART, et le sémaphore pour rendre actif soit la tâche.Pour intégrer la fonction led dans le shell, on a ajouté une ligne dans la tache vTaskShell : shell_add('l', led, "Allumer Led")
(capture led et message ) 
2.4 
Dans la section "spam", nous avons développé une logique similaire à celle de la fonction "led". Cette fonction récupère les caractères ou les chaînes de caractères entrés par l'utilisateur dans le shell, avec le nombre d'arguments transmis. Une boucle itère à travers les paramètres de argv[], déterminant la taille de argv[1] avec strlen, puis copiant les caractères dans un tableau. Ces données sont ensuite envoyées dans une boîte aux lettres pour une utilisation ultérieure.
![Clignotement_Led (1)](https://github.com/Anass6666/Tp_Rtos/assets/145018011/40fde3b5-b4bd-410d-a0e3-e1bac694f862)
![Monsieur Shell](https://github.com/Anass6666/Tp_Rtos/assets/145018011/8b878067-ea7b-40fb-ab09-606c98583309)

 # 3 Debug, gestion d’erreur et statistiques

  # 3.1 Gestion du tas
  
La zone de mémoire réservée pour l'allocation dynamique est appelée le tas, gérée par le HAL. Avant la création répétée de tâches, la RAM était utilisée à 5,83%. Lorsque des tâches bidon sont créées jusqu'à l'apparition d'une erreur, le code plante après la création de 400 tâches bidon. Le total_heap_size de FreeRTOS était par défaut de 15360. Après modification du tas de FreeRTOS pour permettre la création de plus de tâches, portant sa taille à 153600, 131 tâches peuvent maintenant être créées avant un plantage. L'utilisation de la RAM atteint alors 48,11%.Cela est dû à la grande disponibilité de l'espace mémoire aprés modification de total_heap_size.

# 3.2 Gestion des piles:
Dans cette phase, notre objectif est de provoquer un dépassement de pile (overflow). Pour ce faire, conformément à la documentation FreeRTOS, nous utilisons la fonction vApplicationStackOverflowHook, appelée automatiquement en cas de dépassement de pile. Cette fonction fera clignoter une LED en cas de dépassement. Pour effectuer le test, nous créons une tâche bidon qui crée un tableau de grande taille et le remplit. Ce tableau dépasse la taille de la pile, provoquant ainsi notre cas d'overflow. En mode débogage, nous plaçons un point d'arrêt et observons le code s'arrêter dans la fonction d'overflow

Lorsqu'un dépassement de pile se produit dans une tâche, cette fonction est automatiquement appelée par FreeRTOS. Elle utilise une boucle infinie pour clignoter une LED à un intervalle de 100 ms, ce qui permet de signaler visuellement le dépassement de pile. Cette boucle continue indéfiniment tant que le dépassement de pile persiste, fournissant ainsi une indication visuelle claire de l'erreur.
on met un point d'arrêt puis on observe le code s'arrêter dans la fonction d'overflow. En mode debbug 

3.5 VOICI l'utilité des autres hooks . 
Méthode 1 :
Vérifie que le pointeur de pile reste dans l'espace de pile valide après le retrait de la tâche de l'exécution, rapide mais pas garanti pour tous les dépassements de pile.
Méthode 2 :
Remplit la pile d'une tâche avec des valeurs connues et vérifie qu'elles ne sont pas écrasées après le retrait de la tâche de l'exécution, moins efficace mais toujours rapide et probable de détecter les dépassements de pile.
Méthode 3 :
Spécifique à certains ports de FreeRTOS, vérifie les dépassements de pile dans les routines d'interruption (ISR), déclenche une assertion en cas de dépassement de pile ISR, ne déclenche pas la fonction de crochet de dépassement de pile spécifique aux ISRs.

# 3.3 Statistiques dans l’IDE
3.3.4
En mode debbug , on affiche les queues et les sémaphores. 





On affiche les sémaphores et les queues.




# 3.4 Affichage des statistiques dans le shell
cette fonction fournit un moyen pratique d'afficher les statistiques du système d'exploitation dans le terminal, ce qui peut être utile pour surveiller les performances du système et diagnostiquer les problèmes potentiels.

on a utilisé une  fonction statut  pour récupérer les statistiques de temps d'exécution des tâches du système d'exploitation temps réel (RTOS) et les stocke dans un tampon (pcWriteBuffer). Ensuite, elle utilise la fonction printf pour afficher ces statistiques dans le terminal.
L'utilisateur peut saisir le nom de la fonction dans le shell , dans ce cas : "c", et elle sera exécutée, affichant ainsi les statistiques dans le terminal.

# 4 Écriture d’un driver
4.1 Interfacer l’ADXL345
![communication SPI](https://github.com/Anass6666/Tp_Rtos/assets/145018011/7c98c52e-4b0b-415c-a91e-48c8436d8a1a)

 ![adx45](https://github.com/Anass6666/Tp_Rtos/assets/145018011/a9ea76a5-2d5c-44ac-915c-b97c7979c332)







