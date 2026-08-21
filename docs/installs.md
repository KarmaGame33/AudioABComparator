# Installation de l'environnement et compilation

**Projet :** A/B Compare  
**Date de référence :** 21 août 2026  
**État :** builds validés sous Windows 10 et Windows 11 x64 ; ZIP de publication validé sous Windows 10 ; AppImage Linux x86_64 construite sous Ubuntu 22.04 LTS et validée sous KDE/Wayland

## 1. Portée de ce document

Ce document fixe les commandes permettant de produire l'exécutable à partir de la racine du dépôt.

Les commandes de compilation supposent que le projet respectera ce contrat :

- cible CMake : `ab-compare` ;
- exécutable Linux : `ab-compare` ;
- exécutable Windows : `ab-compare.exe` ;
- bundle macOS : `AB Compare.app` ;
- tests enregistrés avec CTest ;
- règles CMake `install()` installant l'application dans `bin/` sous Linux/Windows et comme bundle sous macOS ;
- dépendances Qt du MVP Express déclarées dans le dépôt ;
- libsndfile et libebur128 réservées au futur mode Étendu si leur intégration reste nécessaire ;
- aucun script ne dépend d'un IDE.

Les commandes Windows ci-dessous utilisent Qt 6.9.3 et Visual Studio 2022. Elles s'appliquent à Windows 10 et Windows 11 x64. Les commandes Arch Linux ont été validées pour le build natif ; la production AppImage reste distincte.

## 2. État de validation par système

| Système | Build prévu | Tests natifs | Publication |
|---|---:|---:|---:|
| Arch Linux x64, KDE Plasma, Wayland, PipeWire | validé depuis les sources et via AppImage | automatisés et smoke tests Wayland/XWayland | AppImage bêta |
| Ubuntu 22.04 LTS x64 | build AppImage reproductible en conteneur | automatisés avec sortie PulseAudio virtuelle | AppImage bêta ; essai GNOME interactif à compléter |
| Windows 10 x64 | validé | validé | ZIP portable bêta |
| Windows 11 x64 | validé | validé | ZIP portable bêta |
| macOS Intel/Apple Silicon | préparé | non disponible actuellement | différée |

Il n'est pas prévu de compiler Windows ou macOS depuis Arch Linux. Chaque version doit être construite nativement sur son OS ou sur un runner CI du même OS.

## 3. Arch Linux — KDE Plasma, Konsole et fish

### 3.1 Installer les outils et bibliothèques

Dans Konsole avec fish :

```fish
sudo pacman -Syu
sudo pacman -S --needed base-devel cmake ninja git pkgconf qt6-base qt6-declarative qt6-multimedia qt6-multimedia-ffmpeg qt6-wayland libsndfile libebur128
```

Les paquets Qt, libsndfile et libebur128 proviennent des dépôts officiels Arch ; aucun paquet AUR n'est requis.

Vérification :

```fish
cmake --version
ninja --version
qtpaths6 --qt-version
pkg-config --modversion sndfile
pkg-config --modversion libebur128
```

### 3.2 Configurer une compilation Release

Depuis la racine du dépôt :

```fish
cmake -S . -B build/linux-release -G Ninja -DCMAKE_BUILD_TYPE=Release
```

### 3.3 Compiler

```fish
cmake --build build/linux-release --parallel (nproc)
```

L'exécutable de développement attendu est :

```text
build/linux-release/app/ab-compare
```

Si l'arborescence CMake finale place la cible dans un autre sous-répertoire, ce chemin devra être mis à jour ici ; le nom `ab-compare` reste imposé.

### 3.4 Exécuter les tests

```fish
ctest --test-dir build/linux-release --output-on-failure
```

### 3.5 Produire une arborescence installable locale

```fish
cmake --install build/linux-release --prefix dist/linux
```

L'exécutable installé attendu est :

```text
dist/linux/bin/ab-compare
```

Lancer cette version :

```fish
./dist/linux/bin/ab-compare
```

Cette arborescence utilise encore les bibliothèques Qt et audio du système. Utiliser la section suivante pour produire le paquet AppImage distribuable.

### 3.6 Refaire une compilation après modification

```fish
cmake --build build/linux-release --parallel (nproc)
ctest --test-dir build/linux-release --output-on-failure
```

Il n'est pas nécessaire de relancer la commande de configuration sauf si les fichiers CMake, les dépendances ou les options changent.

### 3.7 Produire l’AppImage Linux x86_64

Docker doit être installé et accessible à l’utilisateur courant. Depuis la racine du dépôt :

```fish
./scripts/linux/build-appimage-container.sh
```

Le script construit l’application dans une image Ubuntu 22.04 LTS figée, installe Qt 6.9.3, exécute CTest avec des pistes WAV, FLAC et MP3 et assemble les plugins Qt Multimedia, XCB et Wayland. Les outils `linuxdeploy` et `linuxdeploy-plugin-qt` téléchargés sont contrôlés par SHA-256.
La construction vérifie aussi que l’intégration cliente Qt Wayland-EGL est réellement présente dans l’AppImage finale.

Les fichiers produits sont :

```text
dist/release/AudioABComparator-0.2.1-beta.4-linux-x86_64.AppImage
dist/release/SHA256SUMS-linux
```

Validation locale :

```fish
chmod +x dist/release/AudioABComparator-0.2.1-beta.4-linux-x86_64.AppImage
./dist/release/AudioABComparator-0.2.1-beta.4-linux-x86_64.AppImage --smoke-test
./dist/release/AudioABComparator-0.2.1-beta.4-linux-x86_64.AppImage
env QT_QPA_PLATFORM=xcb ./dist/release/AudioABComparator-0.2.1-beta.4-linux-x86_64.AppImage
```

Les deux derniers lancements doivent afficher et maintenir la fenêtre, respectivement sous Wayland natif et XWayland/XCB. Ils sont indispensables : le smoke test seul se termine avant l’initialisation du rendu de la première trame.

Le compte rendu détaillé se trouve dans [`validation_linux.md`](validation_linux.md).

## 4. Windows 10 et Windows 11 x64 — PowerShell

### 4.1 Installer les outils généraux

Ouvrir PowerShell en tant qu'administrateur pour les installations :

```powershell
winget install --id Git.Git -e --source winget
winget install --id Kitware.CMake -e --source winget
winget install --id Ninja-build.Ninja -e --source winget
winget install --id Python.Python.3.12 -e --source winget
winget install --id 7zip.7zip -e --source winget
```

Fermer puis rouvrir PowerShell pour actualiser `PATH`.

### 4.2 Installer le compilateur MSVC

Dans PowerShell administrateur :

```powershell
winget install --id Microsoft.VisualStudio.2022.BuildTools -e --source winget --override "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
```

Ce workload installe le compilateur C++, les outils CMake/MSBuild nécessaires à Visual Studio et un SDK Windows pris en charge.

### 4.3 Installer Qt 6.9.3

`aqtinstall` permet une installation reproductible de Qt en ligne de commande :

```powershell
py -3.12 -m ensurepip --upgrade
py -3.12 -m pip install --user --upgrade aqtinstall
py -3.12 -m aqt install-qt windows desktop 6.9.3 win64_msvc2022_64 -O C:\Qt --modules qtmultimedia
```

Installation Qt attendue :

```text
C:\Qt\6.9.3\msvc2022_64
```

Vérification :

```powershell
& 'C:\Qt\6.9.3\msvc2022_64\bin\qmake.exe' -query QT_VERSION
```

La version Qt est volontairement fixée pour rendre les builds reproductibles. Sa mise à jour devra être faite explicitement dans ce document et dans la CI.

### 4.4 Préparer vcpkg et les bibliothèques du futur mode Étendu (facultatif pour Express)

Choisir `C:\Dev\vcpkg` comme emplacement stable :

```powershell
New-Item -ItemType Directory -Force -Path 'C:\Dev' | Out-Null
git clone https://github.com/microsoft/vcpkg.git C:\Dev\vcpkg
& 'C:\Dev\vcpkg\bootstrap-vcpkg.bat' -disableMetrics
& 'C:\Dev\vcpkg\vcpkg.exe' install libsndfile:x64-windows libebur128:x64-windows --disable-metrics
```

Si `C:\Dev\vcpkg` existe déjà, ne pas relancer `git clone`. Mettre vcpkg à jour séparément et contrôler les versions avant de changer le build de référence.

Le MVP Express actuel n'utilise pas encore ces deux bibliothèques. Cette section peut donc être ignorée pour sa compilation.

### 4.5 Charger l'environnement MSVC

Dans une nouvelle fenêtre PowerShell non administrateur :

```powershell
& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -HostArch amd64
cl.exe
```

`cl.exe` doit afficher sa version. Le message indiquant qu'aucun fichier source n'a été fourni est normal.

### 4.6 Configurer une compilation Release

Depuis la racine du dépôt, dans cette même fenêtre PowerShell :

```powershell
cmake -S . -B build\windows-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.9.3\msvc2022_64
```

### 4.7 Compiler et tester

```powershell
cmake --build build\windows-release --parallel
$env:PATH = 'C:\Qt\6.9.3\msvc2022_64\bin;' + $env:PATH
ctest --test-dir build\windows-release --output-on-failure
```

Exécutable de développement attendu :

```text
build\windows-release\ab-compare.exe
```

### 4.8 Produire le dossier distribuable

```powershell
cmake --install build\windows-release --prefix dist\windows
& 'C:\Qt\6.9.3\msvc2022_64\bin\windeployqt.exe' --release --qmldir app\ui dist\windows\bin\ab-compare.exe
```

Exécutable installé attendu :

```text
dist\windows\bin\ab-compare.exe
```

Lancer cette version :

```powershell
& '.\dist\windows\bin\ab-compare.exe'
```

Le MVP Express n'utilise pas libsndfile ni libebur128. `windeployqt` déploie Qt, ses plugins et le backend FFmpeg fourni par Qt. Une vérification sur une machine Windows propre reste obligatoire avant publication.

### 4.9 Produire le ZIP de release

Après un build et des tests réussis :

```powershell
& '.\scripts\windows\package-release.ps1' -BuildDirectory 'build\windows-release' -QtRoot 'C:\Qt\6.9.3\msvc2022_64'
Get-Content '.\dist\release\SHA256SUMS'
```

Le script crée `AudioABComparator-0.2.1-beta.4-windows-x86_64.zip` et `SHA256SUMS`. Le ZIP contient l'exécutable à sa racine, les DLL/plugins/QML nécessaires, `README.txt`, `LICENSE`, `THIRD_PARTY_NOTICES.md` et les textes LGPL. Il est destiné uniquement aux pièces jointes GitHub Releases et ne doit pas être committé dans Git.

## 5. macOS — procédure préparée mais non validée

Cette section ne constitue pas une prise en charge officielle. Elle devra être exécutée sur un Mac Intel et/ou Apple Silicon ou dans une CI macOS, puis corrigée à partir des résultats réels.

### 5.1 Installer Xcode et Homebrew

Dans Terminal avec zsh :

```zsh
xcode-select --install
```

Installer ensuite Homebrew depuis son site officiel si `brew` n'est pas déjà disponible.

### 5.2 Installer Qt et les outils de build

```zsh
brew update
brew install cmake ninja qt git
```

### 5.3 Installer vcpkg et les bibliothèques audio

```zsh
mkdir -p "$HOME/Dev"
git clone https://github.com/microsoft/vcpkg.git "$HOME/Dev/vcpkg"
"$HOME/Dev/vcpkg/bootstrap-vcpkg.sh" -disableMetrics
"$HOME/Dev/vcpkg/vcpkg" install libsndfile:arm64-osx libebur128:arm64-osx --disable-metrics
```

Sur un Mac Intel, remplacer `arm64-osx` par `x64-osx` dans cette section et dans la configuration CMake.

### 5.4 Configurer, compiler et tester sur Apple Silicon

Depuis la racine du dépôt :

```zsh
cmake -S . -B build/macos-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$(brew --prefix qt)" \
  -DCMAKE_TOOLCHAIN_FILE="$HOME/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx
cmake --build build/macos-release --parallel "$(sysctl -n hw.logicalcpu)"
ctest --test-dir build/macos-release --output-on-failure
cmake --install build/macos-release --prefix dist/macos
```

Bundle attendu :

```text
dist/macos/AB Compare.app
```

### 5.5 Déployer Qt dans le bundle

```zsh
"$(brew --prefix qt)/bin/macdeployqt" "dist/macos/AB Compare.app" -dmg
```

Cette commande ne signe pas et ne notarise pas l'application. La signature, la notarisation, les permissions audio, les chemins réels des bibliothèques vcpkg et le fonctionnement Intel/Apple Silicon restent à valider sur macOS.

## 6. Vérification minimale après chaque build

Sur chaque OS validé :

1. démarrer l'exécutable depuis son dossier de distribution ;
2. charger deux fichiers WAV stéréo ;
3. créer une sélection d'au moins 5 secondes ;
4. lire, mettre en pause, arrêter et activer la boucle ;
5. basculer A/B au clavier au moins 100 fois ;
6. vérifier Haut/Bas et les compteurs ;
7. modifier les raccourcis puis redémarrer l'application ;
8. vérifier que les raccourcis persistent mais que fichiers, sélection, votes et analyses ont disparu ;
9. vérifier l'absence de clic récurrent, dérive ou underrun ;
10. exécuter l'application sur une machine propre avant publication.

## 7. Sources des procédures

- [Paquet Qt Multimedia d'Arch Linux](https://archlinux.org/packages/extra/x86_64/qt6-multimedia/)
- [Paquet libsndfile d'Arch Linux](https://archlinux.org/packages/extra/x86_64/libsndfile/)
- [Paquet libebur128 d'Arch Linux](https://archlinux.org/packages/extra/x86_64/libebur128/)
- [Installation de Qt avec aqtinstall](https://aqtinstall.readthedocs.io/en/stable/getting_started.html)
- [Paramètres d'installation de Visual Studio](https://learn.microsoft.com/visualstudio/install/use-command-line-parameters-to-install-visual-studio)
- [Port libsndfile de vcpkg](https://vcpkg.io/en/package/libsndfile.html)
- [Port libebur128 de vcpkg](https://vcpkg.io/en/package/libebur128.html)
- [Formule Homebrew Qt](https://formulae.brew.sh/formula/qt.html)
