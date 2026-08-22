# Résumé de version 0.3.0-beta.2

**Date :** 22 août 2026  
**Version technique :** 0.3.0  
**Version affichée et préparée :** 0.3.0-beta.2  
**Statut :** deuxième préversion 0.3 de développement, non publiée

## Résultat

Cette préversion conserve l’écran **Analyse** et le moteur PCM natif introduits en 0.3.0-beta.1, puis corrige l’intégration Linux et plusieurs ambiguïtés d’interface. Le sélecteur de fichiers système est utilisé dans les builds Linux directs comme dans l’AppImage lorsque le portail de bureau est disponible.

Dans l’écran Analyse, le sélecteur de portée affiche désormais `Tout | Sélection` avec le vert réservé au choix actif. Les poignées Début et Fin restent sous le contrôle de la souris pendant tout le glisser ; la plage est prévisualisée localement et l’analyse n’est déclenchée qu’au relâchement. L’indicateur de position possède une zone dédiée au-dessus du curseur et ne masque plus la forme d’onde.

## Contenu de la version

- analyses du fichier complet et de la sélection pour A et B : Sample Peak, True Peak, LUFS-I, LRA, RMS, facteur de crête et décalage DC ;
- conservation des buffers PCM natifs, décision testable du format de lecture et détail des conversions par piste ;
- libebur128 1.2.6 liée statiquement sous licence MIT ;
- sélecteur `Express | Blind Test | Analyse` traduit dans les sept langues ;
- sélecteur de fichiers système via `xdg-desktop-portal` dans tous les builds Linux, avec repli Qt portable ;
- sélecteur d’analyse `Tout | Sélection` traduit et état actif explicitement vert ;
- glisser des poignées protégé contre la capture du `ScrollView`, avec analyse reportée au relâchement ;
- indicateur temporel déplacé hors de la forme d’onde.

## Validation effectuée sous Linux

- build Release dans `trunk/build/linux-release` ;
- CTest sans échec ;
- compilation QML et catalogues de traduction sans erreur ;
- smoke tests réussis dans les sept langues ;
- version affichée contrôlée à `0.3.0-beta.2` dans le binaire direct.

## Limites et validation restante

- analyse limitée au mono et à la stéréo ;
- aucune normalisation, égalisation, compression ni limitation n’est appliquée ;
- la reconstruction et la validation natives Windows 10 de 0.3.0-beta.2 restent à effectuer ;
- l’AppImage Linux 0.3.0-beta.2 reste à reconstruire avant toute publication ;
- aucun tag ni aucune release 0.3.0-beta.2 n’a été créé ou publié.
