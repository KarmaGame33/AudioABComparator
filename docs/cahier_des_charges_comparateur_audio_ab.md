# Cahier des charges — Comparateur audio A/B multiplateforme

**Version :** 1.1  
**Date :** 19 août 2026  
**Cibles produit :** Linux, Windows et macOS  
**Cibles développées et validées dans un premier temps :** Arch Linux/KDE Plasma et Windows 11  
**macOS :** cible différée jusqu'à l'accès à un Mac ou à un runner macOS  
**Nom :** Audio A/B Comparator

## 1. Vision du produit

L'application permet de comparer deux versions d'un même contenu audio, appelées **A** et **B**, dans des conditions d'écoute aussi neutres et instantanées que possible.

Elle vise deux usages :

- **Express** : écouter rapidement un passage en boucle, basculer entre A et B et noter ses impressions sans surcharge visuelle ;
- **Étendu** : conserver le même workflow tout en affichant les mesures utiles à une décision de mixage ou de mastering.

L'application est locale : les fichiers sont **ouverts depuis le disque** et ne sont envoyés vers aucun service distant.

## 2. Objectifs

- offrir un basculement A/B immédiat, sans perte de position ;
- garantir que A et B utilisent une horloge de lecture et une timeline communes ;
- permettre l'écoute en boucle d'une zone précisément sélectionnée ;
- enregistrer une appréciation positive ou négative pour la piste entendue au moment du vote ;
- présenter des statistiques simples, compréhensibles et réinitialisables ;
- fournir en mode Étendu des mesures conformes aux usages audio professionnels ;
- proposer une interface moderne et lisible, pensée dès le départ pour les trois systèmes ;
- produire d'abord des exécutables installables et testés sous Linux et Windows ;
- préparer la portabilité macOS sans déclarer cette plateforme prise en charge avant un build et des tests natifs.

## 3. Périmètre fonctionnel

### 3.1 Mode Express — MVP

#### Import des fichiers

- deux zones identifiées **Piste A** et **Piste B** ;
- ouverture par bouton ou glisser-déposer ;
- affichage du nom, de la durée, du format, de la fréquence d'échantillonnage, du nombre de canaux et de la résolution lorsque l'information existe ;
- remplacement individuel de A ou B ;
- détection des fichiers illisibles, vides, protégés ou non pris en charge ;
- formats MVP : WAV, RF64, AIFF/AIFC, FLAC et CAF ;
- MP3/Ogg en option pour la première version publique, selon la validation du décodeur et des licences.

Les deux fichiers sont présumés représenter le même programme. Si leurs durées, canaux ou fréquences d'échantillonnage diffèrent, l'application affiche un avertissement non bloquant et applique les règles décrites à la section 6.

#### Timeline et sélection

- affichage de deux formes d'onde synchronisées et superposées verticalement ;
- une tête de lecture commune, visible sur les deux formes d'onde, déplaçable à la souris ;
- marqueurs **Début** et **Fin** clairement identifiés et déplaçables à la souris ;
- zones extérieures à la sélection visuellement assombries ;
- le glisser de la tête prévisualise la position et applique le saut audio au relâchement ;
- contrainte permanente : `0 ≤ Début < Fin ≤ durée commune` et `Fin − Début ≥ 5 secondes` ;
- durée minimale obligatoire entre Début et Fin : **5 secondes** ;
- si le déplacement d'un marqueur produirait une zone plus courte, le marqueur est bloqué à la limite autorisée et l'interface rappelle brièvement la durée minimale.

#### Transport

- **Lecture**, **Pause** et **Arrêt** ;
- la première lecture après le chargement des deux fichiers démarre sur **A** ;
- Pause conserve la position ;
- Arrêt interrompt la lecture et replace la tête au début de la sélection ;
- option **Boucle** : arrivé à Fin, le transport reprend exactement à Début ;
- si la boucle est désactivée, la lecture s'arrête à Fin ;
- `Flèche gauche` recule la tête de lecture de 5 secondes, sans dépasser Début ;
- `Flèche droite` avance la tête de lecture de 5 secondes, sans dépasser Fin ;
- `Espace` bascule entre A et B sans arrêter le transport et sans changer la position ;
- A/B peut aussi être changé à la souris ;
- l'indicateur de la piste active doit être visible immédiatement, mais sans animation distrayante ;
- un fondu très court, de 5 ms par défaut, évite les clics lors du basculement. Les valeurs 0, 5, 10 et 20 ms sont prévues dans les préférences.
- un bip de transition clair, aigu et court peut être activé ; son volume est réglable, et l'activation comme le niveau sont mémorisés localement. Il est produit uniquement pendant la lecture.

Par défaut, le raccourci Espace agit sur le basculement A/B et non sur Lecture/Pause. Les raccourcis A/B, d'évaluation et de déplacement de 5 secondes sont configurables dans l'écran **Paramètres > Raccourcis**. L'écran principal rappelle en permanence les touches actuellement affectées aux actions essentielles.

#### Évaluation

- `Flèche haut` ajoute **+1** à la piste active au moment de l'appui ;
- `Flèche bas` ajoute **−1** à la piste active au moment de l'appui ;
- la répétition automatique du clavier est ignorée : un appui physique correspond à un vote ;
- un retour visuel bref confirme le vote et la piste concernée ;
- les raccourcis sont désactivés lorsqu'un contrôle de saisie ou de configuration est actif ;
- les votes sont acceptés pendant la lecture et la pause, mais pas avant le chargement complet des deux pistes ;
- les compteurs existent uniquement en mémoire pour la paire A/B actuellement chargée ;
- remplacer un fichier demande confirmation avant de remettre les votes à zéro ;
- modifier la sélection ne remet pas automatiquement les votes à zéro.

Pour chaque piste, l'application affiche :

- nombre de votes positifs ;
- nombre de votes négatifs ;
- nombre total de votes ;
- **score net** = positifs − négatifs ;
- **moyenne** = score net / nombre total de votes, comprise entre −1 et +1 ;
- `—` lorsque la piste n'a encore reçu aucun vote.

Un bouton **Réinitialiser les évaluations** ouvre une confirmation, puis remet les deux pistes à zéro. Cette opération est indépendante du transport.

### 3.2 Blind Test

Le Blind Test conserve la paire, la sélection et la position du mode Express, mais masque toute indication de la piste active.

- l'entrée dans le mode démarre une nouvelle session avec des compteurs séparés et une piste initiale aléatoire ;
- `Espace` choisit aléatoirement A ou B, sans permettre plus de deux sélections identiques consécutives ;
- les cartes, statuts, commandes et scores ne révèlent pas la piste active ;
- les votes sont attribués en interne à la piste réellement écoutée et seul leur total est affiché pendant la session ;
- si le bip est activé, il joue à chaque commande de sélection pendant la lecture, même lorsque le tirage conserve la même piste ;
- **Révéler** met la lecture en pause, montre la piste active et affiche les résultats A/B de la session ;
- les résultats Express restent inchangés et réapparaissent au retour dans ce mode ;
- remplacer un fichier annule la session aveugle en cours.

### 3.3 Mode Étendu

Le mode Étendu reprend toutes les fonctions Express. Le passage d'un mode à l'autre ne doit ni arrêter la lecture, ni perdre la sélection, ni réinitialiser les votes.

#### Mesures de fichier et de sélection

Les résultats doivent distinguer clairement :

- **Fichier entier** : analyse globale, calculée en arrière-plan après import ;
- **Sélection** : analyse recalculée pour la zone Début–Fin ;
- **Temps réel** : mesures évoluant pendant la lecture.

Mesures minimales :

| Mesure | Unité | Portée | Présentation |
|---|---|---|---|
| Sample Peak | dBFS | fichier et sélection | A, B et écart |
| True Peak | dBTP | fichier et sélection | A, B et écart |
| Loudness intégrée | LUFS-I | fichier et sélection | A, B et écart |
| Loudness court terme | LUFS-S, fenêtre 3 s | temps réel | deux barres + valeurs |
| Loudness momentanée | LUFS-M, fenêtre 400 ms | temps réel | deux barres + valeurs |
| Loudness Range | LU | fichier et sélection | A, B et écart |
| RMS | dBFS | fichier et sélection | A, B et écart |
| Facteur de crête | dB | fichier et sélection | A, B et écart |
| Décalage DC | % ou dBFS | fichier et sélection | avertissement si anormal |
| Corrélation stéréo | −1 à +1 | temps réel et sélection | jauge |

Informations complémentaires : fréquence d'échantillonnage, résolution, canaux, durée et codec.

Les mesures LUFS, LRA et True Peak suivent l'EBU R 128. Une info-bulle explique chaque mesure et sa portée ; l'application ne qualifie pas automatiquement un master de « bon » ou « mauvais ».

#### Visualisations Étendues

- vumètres Peak/True Peak par canal ;
- graphe d'historique LUFS-M et LUFS-S sur le passage ;
- analyseur spectral commutable ;
- corrélomètre stéréo ;
- comparaison sous forme de colonnes A, B et différence `B − A` ;
- seuils d'alerte configurables, sans colorer une valeur comme erreur lorsqu'aucune cible n'a été choisie.

#### Égalisation perceptuelle optionnelle

Le mode Étendu propose **Égaliser le niveau d'écoute** :

- désactivé par défaut ;
- applique uniquement un gain de monitoring à la piste la plus forte ;
- ne modifie ni les fichiers ni les mesures affichées ;
- affiche en permanence le gain appliqué ;
- se fonde sur la loudness intégrée de la sélection lorsque celle-ci est disponible ;
- est automatiquement recalculé lorsque la sélection change.

Cette fonction limite le biais fréquent consistant à préférer automatiquement la version la plus forte.

## 4. Principes d'interface et de design

### 4.1 Direction visuelle

- thème sombre neutre par défaut, avec thème clair facultatif ;
- fond gris anthracite, contraste élevé et couleurs d'accent distinctes pour A et B ;
- typographie sans serif, chiffres tabulaires pour les temps et les mesures ;
- angles modérément arrondis, ombres discrètes et animations de 120 à 180 ms ;
- aucun effet décoratif susceptible de gêner la lecture des vumètres ;
- mise en page adaptable à partir de 1 100 × 700 px ;
- mise à l'échelle correcte sur écrans HiDPI.

### 4.2 Structure de l'écran Express

1. barre supérieure : sélecteur Express/Blind Test et accès aux paramètres ;
2. cartes A et B : import et métadonnées essentielles ;
3. timeline centrale interactive : deux formes d'onde, tête de lecture et marqueurs Début/Fin ;
4. transport : boucle, retour au début, lecture, pause, arrêt et piste active ;
5. panneau d'évaluation : compteurs A/B, moyenne et réinitialisation ;
6. barre d'aide contextuelle : `Espace = A/B`, `↑ = +1`, `↓ = −1`, `← = −5 s`, `→ = +5 s`.

### 4.3 Clavier et accessibilité

- écran **Paramètres > Raccourcis** permettant de modifier chaque raccourci ;
- détection et refus des conflits entre deux actions utilisant la même combinaison ;
- bouton de restauration des raccourcis par défaut ;
- rappel sur l'écran principal des touches actives pour A/B, vote positif et vote négatif ;
- commandes principales utilisables au clavier selon la configuration choisie ;
- focus visible ;
- couleurs A/B non utilisées comme unique moyen d'identification ;
- contrastes conformes au minimum WCAG AA ;
- libellés accessibles pour les lecteurs d'écran ;
- possibilité de réduire les animations ;
- aucune modification de raccourci ne doit interrompre la lecture en cours.

## 5. Règles de comparaison audio

### 5.1 Horloge et position communes

Le moteur utilise une seule sortie audio et une position exprimée en échantillons. A et B ne sont pas deux lecteurs indépendants. À chaque bloc audio, le moteur choisit la source A, B ou un très court mélange de transition. Ainsi, un changement de piste ne provoque ni redémarrage, ni seek, ni dérive temporelle.

### 5.2 Conversion interne

- décodage en PCM flottant 32 bits ;
- fréquence de sortie choisie selon le périphérique audio ;
- rééchantillonnage hors du thread audio temps réel ;
- conversion des canaux explicite et documentée ;
- absence de limiteur, normalisation ou traitement caché ;
- contrôle de volume de monitoring commun aux deux pistes ;
- le volume par piste n'est disponible que dans les outils avancés et reste visible lorsqu'il diffère.

### 5.3 Durées différentes

La **durée commune** vaut `min(durée A, durée B)` après prise en compte de l'alignement. La sélection Express ne peut dépasser cette durée. Les parties non comparables restent visibles mais grisées.

### 5.4 Alignement

Pour le MVP, A et B sont alignés à leur échantillon zéro. L'application avertit si leurs formes d'onde suggèrent un décalage important.

Pour la version Étendue :

- décalage manuel de B en millisecondes et en échantillons ;
- proposition d'alignement automatique par corrélation ;
- aperçu et validation de l'offset avant application ;
- aucune modification du fichier source.

### 5.5 Thread audio temps réel

Le callback audio ne réalise aucune lecture disque, allocation mémoire, analyse lourde ou attente de verrou. Deux buffers circulaires sont alimentés en avance par des workers. En cas d'underrun, l'application affiche une erreur explicite et compte l'incident dans le diagnostic d'exécution.

## 6. Cas d'erreur et comportements attendus

- fichier non pris en charge : explication et liste des formats acceptés ;
- fréquence différente : rééchantillonnage avec indication dans les métadonnées ;
- nombre de canaux différent : avertissement et règle de downmix affichée ;
- mono contre stéréo : duplication mono vers L/R uniquement après accord défini dans les préférences ;
- fichier multicanal : refus dans le MVP, prise en charge ultérieure ;
- durée très différente : avertissement, sans blocage ;
- fichier déplacé après ouverture : la lecture continue si le cache le permet, sinon erreur récupérable ;
- périphérique audio déconnecté : pause, choix d'un nouveau périphérique puis reprise ;
- changement de périphérique : pause courte autorisée, position conservée ;
- sélection invalide : correction immédiate et message accessible ;
- analyse annulée : la lecture Express reste disponible.

## 7. Absence de session et persistance limitée

L'application ne propose aucun fichier de projet et aucune sauvegarde de session. À sa fermeture, elle oublie :

- les fichiers A et B chargés ;
- les chemins et la liste des fichiers récents ;
- la sélection Début/Fin et l'offset d'alignement ;
- l'état du transport et de la boucle ;
- les votes, scores et moyennes ;
- les mesures et caches d'analyse associés aux fichiers.

Le remplacement de A ou B remet à zéro les données dépendant de la paire après confirmation. Fermer l'application ne demande jamais d'enregistrer le travail.

Seuls les paramètres généraux nécessaires au fonctionnement sont conservés dans le répertoire de configuration standard de l'OS : périphérique audio préféré, thème, réduction des animations, micro-fondu et affectation des raccourcis. Cette configuration ne contient aucun nom ou chemin de fichier audio, vote ou résultat d'analyse. Un bouton **Restaurer les paramètres par défaut** permet de la supprimer logiquement.

## 8. Exigences non fonctionnelles

### Performance

- démarrage à froid visé : moins de 3 s sur une machine récente ;
- réponse au basculement A/B : prise en compte au prochain bloc audio ;
- absence de coupure audible lors de 100 basculements successifs ;
- lecture stable de deux fichiers stéréo 24 bits / 192 kHz ;
- interface visant 60 images/s, avec un plancher acceptable de 30 images/s pendant une analyse lourde si cela protège la lecture audio temps réel ;
- la cadence visuelle peut être réduite automatiquement à 30 images/s sous charge puis revenir à 60 images/s ;
- chargement progressif des formes d'onde et métriques ;
- cache plafonné et nettoyable depuis les préférences.

### Fiabilité

- aucun écrasement ou traitement des fichiers source ;
- journal de diagnostic local, sans contenu audio ;
- récupération propre après erreur du périphérique audio ;
- tests automatisés du transport, de la boucle, du scoring et des calculs.

### Confidentialité

- fonctionnement hors ligne ;
- aucune télémétrie par défaut ;
- aucun envoi de fichier, mesure ou nom de fichier ;
- export volontaire du diagnostic avec prévisualisation des données incluses.

## 9. Architecture recommandée

### 9.1 Choix technologique

- **Interface :** Qt Quick/QML ;
- **cœur :** C++20 ;
- **build :** CMake + Ninja ;
- **sortie audio :** callback bas niveau unique, via Qt `QAudioSink` sur les plateformes prises en charge ou une couche miniaudio si le prototype montre une meilleure stabilité ;
- **décodage lossless :** libsndfile ;
- **mesures EBU R 128 :** libebur128 ;
- **tests C++ :** Qt Test ou Catch2 ;
- **CI initiale :** matrice native Linux et Windows ;
- **CI macOS :** ajoutée uniquement lorsqu'un runner macOS et une procédure de validation sont disponibles.

La décision `QAudioSink` ou miniaudio doit être prise après le prototype technique, sur la base de mesures de latence et d'underruns, pas avant.

### 9.2 Modules

```text
app/
├── ui/                 écrans QML, thèmes et composants
├── application/        orchestration, commandes et paramètres
├── audio/
│   ├── decode/         lecture et conversion en PCM
│   ├── transport/      position, boucle, pause et arrêt
│   ├── switch/         sélection A/B et micro-fondu
│   ├── buffers/        buffers circulaires A et B
│   └── devices/        périphériques et adaptateurs de plateforme
├── analysis/
│   ├── waveform/       enveloppes multi-résolution
│   ├── loudness/       LUFS, LRA et True Peak
│   └── stereo/         RMS, crête, corrélation et DC
├── scoring/            votes, agrégats et événements
├── settings/           préférences générales et raccourcis
└── platform/           Linux, Windows et macOS
```

### 9.3 Flux principal

1. l'UI transmet les chemins A et B ;
2. les métadonnées sont lues et validées ;
3. des workers construisent les aperçus et alimentent les buffers PCM ;
4. le moteur expose une timeline commune ;
5. le callback lit A ou B selon un état atomique ;
6. les votes sont associés en mémoire à la piste active et à la position ;
7. l'analyse Étendue fonctionne en arrière-plan sur les données décodées ;
8. l'UI reçoit des modèles immuables ou des signaux limités en fréquence.

## 10. Découpage des versions

### Version 0.1 — Prototype audio

- ouvrir deux WAV stéréo ;
- les décoder vers un format commun ;
- lire une timeline partagée ;
- basculer A/B au clavier sans perte de position ;
- boucler sur une plage codée en dur ;
- mesurer latence, clics, dérive et underruns sous Arch Linux/KDE Plasma et Windows 11.

### Version 0.2 — Express+ et Blind Test

- import par dialogue ;
- formes d'onde et timeline interactive ;
- sélection Début/Fin et tête de lecture déplaçables à la souris ;
- transport complet et boucle ;
- raccourcis Espace, Haut, Bas, Gauche et Droite ;
- scores, moyennes et remise à zéro ;
- bip A/B optionnel ;
- session Blind Test avec résultats séparés ;
- design Express+ ;
- gestion des erreurs principales.

### Version 0.3 — Paramètres et robustesse

- écran Paramètres/Raccourcis et configuration locale minimale ;
- cache d'analyse strictement temporaire, supprimé au plus tard à la fermeture ;
- sélection du périphérique ;
- formats lossless complets ;
- tests de charge et récupération d'erreur.

### Version 0.4 — Étendu

- analyse EBU R 128 ;
- métriques fichier, sélection et temps réel ;
- vumètres et graphiques ;
- égalisation optionnelle du niveau d'écoute ;
- alignement manuel puis automatique.

### Version 1.0 — Distribution

- exécutables et installeurs Linux et Windows ;
- signature Windows ;
- paquet Linux retenu après tests de compatibilité ;
- documentation utilisateur ;
- bêta fermée, corrections et publication.

La publication macOS constitue un jalon ultérieur : build natif, tests, signature, notarisation et DMG ne pourront être validés qu'avec un Mac ou un runner macOS.

## 11. Plan de réalisation

### Phase 0 — Cadrage et maquettes, 15 à 30 minutes avec Codex

- valider les formats, la stéréo et les OS minimums ;
- réaliser les wireframes Express et Étendu ;
- figer les règles de vote, pause, arrêt, boucle et réinitialisation ;
- définir les jeux de fichiers audio de référence ;
- produire une courte charte visuelle.

**Jalon :** maquettes et règles métier approuvées.

### Phase 1 — Spike technique audio, 30 à 45 minutes avec Codex

- comparer `QAudioSink` et miniaudio ;
- implémenter le double buffer et la position commune ;
- tester le basculement, le micro-fondu et la boucle ;
- valider 44,1 / 48 / 96 / 192 kHz ;
- instrumenter latence et underruns ;
- exécuter le même prototype sous Arch Linux/KDE Plasma et Windows 11.

**Go/No-Go :** aucun clic récurrent, aucune dérive et stabilité suffisante sous Linux et Windows. Ce résultat ne vaut pas validation de macOS.

### Phase 2 — Fondations et UI Express, 45 à 60 minutes avec Codex

- créer l'architecture C++/QML ;
- construire le thème et les composants réutilisables ;
- développer l'import, les métadonnées et les erreurs ;
- générer les formes d'onde multi-résolution ;
- implémenter la timeline, le zoom et les marqueurs.

**Jalon :** deux fichiers peuvent être importés et une zone peut être sélectionnée précisément.

### Phase 3 — Transport et évaluation, 60 à 90 minutes avec Codex

- finaliser lecture, pause, arrêt et boucle ;
- gérer Espace, Haut et Bas avec filtrage de l'auto-repeat ;
- développer le modèle de votes et les agrégats ;
- ajouter confirmations, états vides et aide contextuelle ;
- écrire les tests du transport et du scoring.

**Jalon :** MVP Express utilisable de bout en bout.

### Phase 4 — Paramètres et durcissement Express, 2 à 4 heures avec Codex

- écran Paramètres/Raccourcis, détection des conflits et restauration des valeurs par défaut ;
- cache temporaire, limites disque et suppression à la fermeture ;
- périphériques audio et changements à chaud ;
- tests HiDPI, accessibilité et raccourcis ;
- correction des cas de fréquences, canaux et durées différents.

**Jalon :** bêta Express.

### Phase 5 — Mesures Étendues, 4 à 8 heures avec Codex

- intégrer et tester libebur128 ;
- calculer Peak, True Peak, LUFS-I/M/S, LRA, RMS, facteur de crête, DC et corrélation ;
- séparer fichier, sélection et temps réel ;
- créer les colonnes A/B/différence et les vumètres ;
- ajouter l'égalisation de monitoring ;
- vérifier les résultats contre des fichiers étalons.

**Jalon :** valeurs reproductibles et cohérentes avec un outil de référence.

### Phase 6 — Alignement et finition Étendue, 4 à 8 heures avec Codex

- offset manuel ;
- détection automatique par corrélation ;
- analyse spectrale et historique loudness ;
- optimisation du rendu et du calcul en arrière-plan ;
- tests utilisateurs orientés mixage/mastering.

**Jalon :** bêta Étendue.

### Phase 7 — Packaging Linux/Windows et publication, 2 à 6 heures par OS avec Codex

- CI native Linux et Windows ;
- packaging dynamique de Qt et inventaire des licences ;
- Windows : installeur et signature ;
- Linux : AppImage ou Flatpak après tests KDE/GNOME, X11/Wayland ;
- smoke tests sur machines propres ;
- manuel utilisateur, procédure de publication et commandes reproductibles dans `installs.md`.

**Jalon :** version 1.0 installable et vérifiée sous Linux et Windows. Le port macOS reste différé.

### Estimation globale dans un développement piloté par Codex

Les durées ci-dessous représentent le temps de production active visé avec Codex, et non une estimation conventionnelle pour un développeur humain :

- MVP Express initial sur l'OS principal : **objectif de 3 heures**, si l'environnement Qt est déjà opérationnel ;
- consolidation, tests automatisés et corrections : **une journée** ;
- compilation et validation sur le second OS : **une journée supplémentaire**, selon l'accès à la machine et les problèmes de backend audio ;
- mode Étendu initial : **1 à 3 jours** ;
- packaging et validation matérielle : temps distinct, dépendant des installations, certificats et tests d'écoute réels.

L'objectif de trois heures correspond à un MVP fonctionnel, pas encore à une garantie de comportement identique sur tous les périphériques Windows et Linux. Cette estimation exclut le multicanal, les plugins audio, le cloud, le mobile et un système ABX scientifique complet.

## 12. Stratégie de tests

### Tests unitaires

- transitions d'état du transport ;
- bornes, durée minimale de 5 secondes et bouclage en nombre d'échantillons ;
- attribution des votes à la piste active ;
- score net et moyenne, notamment zéro vote ;
- validation et résolution des conflits de raccourcis ;
- absence de persistance des fichiers, votes et analyses ;
- calculs métriques sur signaux synthétiques.

### Tests audio de référence

- sinus à amplitude connue pour Peak/RMS ;
- fichiers officiels ou reconnus pour EBU R 128 ;
- impulsions pour vérifier l'alignement ;
- fichiers avec sample rates et profondeurs différents ;
- inversions de phase pour la corrélation ;
- boucle très courte et changements A/B répétés.

### Tests d'intégration

- déconnexion/reconnexion du périphérique ;
- fichiers longs et 24 bits / 192 kHz ;
- chemin Unicode et chemins très longs ;
- mise en veille et reprise ;
- changement Express/Étendu pendant la lecture ;
- analyse simultanée sans drop audio.

### Matrice plateformes

- Windows 11 x64 ;
- Arch Linux x64 avec KDE Plasma, Wayland et PipeWire ;
- tests Linux complémentaires sous X11 ou sur une distribution plus stable avant distribution large ;
- macOS Intel et Apple Silicon reportés jusqu'à disponibilité d'une infrastructure native ;
- écrans standard et HiDPI ;
- sorties intégrées, USB et Bluetooth, ce dernier avec avertissement de latence.

## 13. Critères d'acceptation de la version 1.0

- l'utilisateur charge A et B sans tutoriel ;
- la lecture commence sur A au début de la sélection ;
- l'application empêche toute sélection inférieure à 5 secondes ;
- Espace change la piste sans modifier la position perceptible ;
- la boucle respecte les marqueurs à un bloc audio près, avec précision interne à l'échantillon ;
- Haut et Bas modifient une seule fois la piste effectivement active ;
- les compteurs, le score net et la moyenne sont exacts ;
- la remise à zéro ne modifie ni fichiers, ni sélection, ni préférences ;
- le passage Express/Étendu conserve l'état courant en mémoire sans rien écrire dans une session ;
- les raccourcis sont configurables, sans conflit, et rappelés sur l'écran principal ;
- les mesures sont vérifiées sur des fichiers étalons ;
- aucune opération lourde ne provoque de coupure audio ;
- les fichiers source ne sont jamais modifiés ;
- les paquets Linux et Windows s'installent et démarrent sur des machines propres ;
- aucune prise en charge macOS n'est annoncée avant validation native ;
- les licences et composants redistribués sont inventoriés.

## 14. Hors périmètre initial

- édition destructive ou export audio ;
- hébergement ou partage cloud ;
- VST, AU ou AAX ;
- comparaison de plus de deux pistes ;
- lecture vidéo ;
- multicanal immersif ;
- application mobile ;
- collaboration en temps réel ;
- test ABX en double aveugle avec calcul de significativité.

Ces fonctions pourront être évaluées après validation du cœur A/B. Un véritable mode ABX aveugle serait une extension pertinente, mais il doit rester séparé du score subjectif demandé ici.

## 15. Risques principaux et réponses

| Risque | Effet | Réponse |
|---|---|---|
| Deux lecteurs indépendants | décalage lors du basculement | une sortie et une timeline communes |
| Décodage ou analyse dans le callback | coupures audio | buffers préchargés et workers dédiés |
| Biais de niveau | préférence artificielle | gain match optionnel, explicite et non destructif |
| Fichiers non alignés | comparaison invalide | avertissement, offset manuel et auto-alignement |
| Métriques ambiguës | mauvaise interprétation | unité, fenêtre, portée et info-bulle explicites |
| Différences de backend selon l'OS | comportement non reproductible | spike précoce et CI native |
| Packaging Qt/licences | retard de publication | inventaire et builds installables dès la bêta |
| Interface Étendue surchargée | perte de simplicité | Express reste l'écran par défaut |

## 16. Références techniques

- [Plateformes desktop prises en charge par Qt](https://doc.qt.io/qt-6/supported-platforms.html)
- [Sortie audio bas niveau QAudioSink](https://doc.qt.io/qt-6/qaudiosink.html)
- [Formats pris en charge par libsndfile](https://libsndfile.github.io/libsndfile/formats.html)
- [libebur128 — LUFS, LRA et True Peak](https://github.com/jiixyj/libebur128)
- [Recommandation EBU R 128](https://tech.ebu.ch/fr/publications/r128)
- [Licences Qt](https://doc.qt.io/qt-6/licensing.html)
- [Déploiement des applications Qt](https://doc.qt.io/qt-6/deployment.html)
