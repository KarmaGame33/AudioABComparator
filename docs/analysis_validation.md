# Validation des analyses statiques et temps réel

## Contrat de calcul

Les analyses utilisent exclusivement le PCM natif décodé et conservé en mémoire. Elles ne passent pas par le PCM éventuellement converti pour la lecture. Pour une sélection, les bornes en secondes sont converties séparément dans la fréquence native de A et de B : les deux résultats portent donc sur la même plage temporelle.

- Sample Peak : maximum absolu par canal, puis maximum des canaux, converti en dBFS ;
- True Peak : maximum des canaux fourni par libebur128 1.2.6, converti en dBTP ;
- LUFS-I et LRA : libebur128 1.2.6 ;
- RMS : moyenne quadratique de tous les échantillons de tous les canaux ;
- facteur de crête : Sample Peak moins RMS ;
- décalage DC : moyenne signée du canal dont la moyenne absolue est la plus forte, multipliée par 100.

Les valeurs restent en `double` jusqu’à l’affichage. Le tableau arrondit les mesures audio à deux décimales et le décalage DC à trois. Un signal numérique nul affiche `−∞` pour les niveaux ; une grandeur non définie, notamment le facteur de crête ou la LRA du silence, affiche `—`.

Les mesures temps réel comparent simultanément les PCM de lecture de A et B à la même position, sans inclure le bip optionnel ni le fondu de bascule A/B. Sample Peak, True Peak, RMS et LUFS-M portent sur les 400 dernières millisecondes disponibles. LUFS-S porte sur les trois dernières secondes et reste affichée `—` tant que cette durée n’est pas disponible. Une demande appariée est prise toutes les 200 ms ; le contrôleur sérialise les calculs et ne conserve qu’un instantané en attente. Pour chaque piste et chaque vumètre, les repères minimum et maximum cumulent les valeurs atteintes depuis le démarrage de la lecture. Ils survivent à la pause, à la reprise, au déplacement de la position et à la bascule A/B ; un changement de sélection ou Stop les réinitialise.

## Signaux synthétiques et tolérances

CTest génère les signaux sans fichier intermédiaire à 48 kHz, dans les formats UInt8, Int16, Int32 et Float, en mono et stéréo :

| Signal | Référence | Tolérance automatisée |
|---|---:|---:|
| silence numérique, 5 s | niveaux `−∞`, DC 0 %, facteur de crête et LRA indisponibles | exact sur les états spéciaux |
| sinus 997 Hz, amplitude 0,5 | Sample Peak −6,0206 dBFS ; RMS −9,0309 dBFS ; facteur de crête 3,0103 dB | ±0,03 dB, portée à ±0,30 dB pour UInt8 |
| même sinus, True Peak libebur128 | environ −6,0206 dBTP | ±0,25 dBTP |
| même sinus, LUFS-I libebur128 | environ −9,07 LUFS-I mono ; −6,06 LUFS-I stéréo corrélée | ±0,15 LU, portée à ±0,35 LU pour UInt8 |
| sinus pleine échelle Float mono | Sample Peak 0 dBFS ; RMS −3,0103 dBFS | ±0,01 dB et ±0,03 dB |
| DC −5 % à gauche, +10 % à droite | +10 % | ±0,001 point |
| sinus avec saut de niveau de 18 dB | LRA supérieure à 5 LU | seuil minimal |
| sinus 997 Hz temps réel, amplitude 0,5 | Sample Peak −6,0206 dBFS ; RMS −9,0309 dBFS ; True Peak environ −6,0206 dBTP | tolérances statiques identiques |
| même sinus, LUFS-M et LUFS-S | environ −9,07 LUFS mono ; −6,06 LUFS stéréo corrélée | ±0,15 LU, portée à ±0,35 LU pour UInt8 |
| fenêtre temps réel de 500 ms | LUFS-M disponible ; LUFS-S indisponible | exact sur l’état spécial |

Ces références LUFS-I, LRA et True Peak sont exécutées avec la même version figée de libebur128 que l’application. Les tolérances plus larges de UInt8 couvrent sa quantification asymétrique.

## Asynchronisme et limites

Un test envoie deux recalculs de sélection successifs et vérifie que seul le résultat de la dernière génération est publié. Un autre annule une mesure temps réel en cours et vérifie que son résultat n’est pas réaffiché. Un test apparié vérifie que A et B correspondent à la même fenêtre, que leurs extrema sont indépendants, qu’une nouvelle valeur étend correctement les repères et que Clear les efface. Les calculs s’exécutent via Qt Concurrent ; le périphérique et le callback de lecture ne prennent aucun verrou d’analyse. La pause conserve les dernières mesures, la bascule A/B conserve les deux séries et Stop remet les vumètres et leurs repères à zéro. Les analyses mono et stéréo sont prises en charge. Un format à six canaux est vérifié comme explicitement non pris en charge, sans réutilisation d’anciennes valeurs.

La validation de publication 1.0.0 a été rejouée sous Linux et nativement sous Windows 10 avec sortie Remote Audio. Elle couvre les fixtures WAV, FLAC et MP3, une paire de PCM natifs communs acceptés et une paire 44,1 kHz mono / 48 kHz stéréo nécessitant une conversion. Les résultats propres à chaque plateforme sont consignés dans `validation_linux.md` et `validation_windows.md`.
