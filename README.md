# GSeries Lab AU — prototype de recherche DSP (v0.1)

Projet indépendant non affilié à Solid State Logic. Aucun code, circuit propriétaire, logo ou interface SSL n'est reproduit.

## État réel
- Source C++ JUCE pour plugin Audio Unit macOS (Apple Silicon) et application standalone.
- Compresseur stéréo à détecteur de crête dominant, ratios et temps sélectionnables, filtre passe-haut sidechain, relâchement Auto expérimental, gain de sortie, bypass du circuit de compression et drive expérimental.
- Interface générique JUCE, automatisation et sauvegarde d'état.
- **Non compilé ni validé sur Logic Pro dans cet environnement Linux.**
- **Ce n'est PAS une émulation fidèle du SSL physique** : seuil dépendant du ratio, vraie boucle VCA, modélisation du THAT 2181, courbes mesurées, saturation calibrée et modèle Auto d'origine restent à développer.
- Le bypass « Compressor In » coupe la réduction de gain mais conserve le gain de sortie et le drive expérimental. Pour un bypass complet, utiliser celui de Logic Pro.

## Prérequis Mac mini M2
Xcode et outils de ligne de commande, CMake 3.22+, JUCE récent téléchargé depuis juce.com ou GitHub.

## Compilation (Terminal macOS)
```bash
xcode-select --install
cmake -S . -B build -G Xcode -DJUCE_DIR="$HOME/JUCE" -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --config Release --target GSeriesLab_AU
```
Adapter `JUCE_DIR` au dossier local JUCE. Le bundle .component est généralement copié dans ~/Library/Audio/Plug-Ins/Components/ grâce à COPY_PLUGIN_AFTER_BUILD. Sinon rechercher `GSeries Lab.component` dans `build/` et le copier manuellement.

Redémarrer Logic Pro / rescanner via Gestionnaire de modules. Tester avec `auval -a` puis `auval -v aufx GsLb Idsp` (les codes doivent correspondre au bundle généré).

## Architecture de fidélité cible
1. Mesurer le matériel précis (500-series G-Comp ou Bus Comp Classic 1U) à niveaux et réglages connus.
2. Mesurer courbes statiques de gain réduction, genou et dépendance du seuil au ratio.
3. Mesurer attaques/releases en burst, modes Auto, stéréo dominante, HPF sidechain.
4. Mesurer THD/IMD et réponse en fréquence avec et sans compression, différents niveaux, calibrations dBFS↔dBu.
5. Implémenter VCA non linéaire calibré, architecture de feedback/sidechain conforme aux mesures, puis oversampling et antialiasing.
6. Comparer automatiquement captures hardware vs plugin (null tests compensés en délai, erreur dynamique, THD, écoute ABX).

Important : les valeurs de release retenues correspondent à la fiche officielle du module 500 (0.1, 0.2, 0.4, 0.8, 1.6 s, Auto), pas à toutes les variantes historiques.
