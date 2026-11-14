# Minitel1B_Hard

Bibliothèque Minitel pour Arduino et ESP32, utilisant un port série matériel (`HardwareSerial`).

*Minitel library for Arduino/ESP32 (with HardwareSerial).*

Elle permet de piloter un Minitel 1B (ou compatible) par sa prise péri-informatique : affichage de texte et de semi-graphiques, couleurs et tailles, gestion du curseur, lecture du clavier et commandes du protocole (vitesse, modes, aiguillages, modem).

Documentation utilisée : [Spécifications Techniques d'Utilisation du Minitel 1B](http://543210.free.fr/TV/stum1b.pdf). Les numéros de page cités dans le code renvoient à ce document.

**NOUVEAU : un croquis et un shield Arduino qui transforment le Minitel en petit ordinateur autonome, avec une horloge et reBASIC, un interpréteur BASIC écrit pour l'écran et le clavier du Minitel. C'est ici : https://github.com/eserandour/rebootMinitel.**

## Sommaire

- [Matériel et environnement](#matériel-et-environnement)
- [Installation](#installation)
- [Branchements](#branchements)
- [Démarrage rapide](#démarrage-rapide)
- [Exemples](#exemples)
- [Fonctions](#fonctions)
- [Valeurs de retour et délais d'attente](#valeurs-de-retour-et-délais-dattente)
- [Licence et remerciements](#licence-et-remerciements)
- [Historique](#historique)

## Matériel et environnement

- Cartes utilisées : ATmega 2560 (Arduino Mega), ATmega 1284P et ESP32-WROOM-32E.
- Compilation avec Arduino 1.8.19.
- Pour l'ESP32 : carte esp32 d'Espressif Systems, version 3.3.7 (<https://github.com/espressif/arduino-esp32>).
- Avec l'Arduino Mega, j'utilise le [shield 3615](https://entropie.org/3615/index.php/hardware-2023/) (version 2023).

## Installation

1. Télécharger le dépôt et copier le dossier `Minitel1B_Hard` dans le dossier `libraries` du carnet de croquis Arduino.
2. Redémarrer le logiciel Arduino : les exemples apparaissent dans Fichier > Exemples > Minitel1B_Hard.

Pour ajouter la carte ESP32 dans le logiciel Arduino :

1. Fichier > Préférences > URL de gestionnaire de cartes supplémentaires : `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
2. Outils > Type de carte > Gestionnaire de carte > Type > Contribué > esp32 by Espressif Systems, puis installer la version 3.3.7.

## Branchements

### Carte fonctionnant en 5V (Arduino Mega, ATmega 1284P)

```
Dos du Minitel             Carte Arduino

DIN 1 : RX    <--------->  TX    (TX1 = D18 sur Arduino Mega)
DIN 4 :       <---/ /--->
DIN 2 : GND   <--------->  GND
DIN 5 : +8,5V <---/ /--->
DIN 3 : TX    <----+---->  RX    (RX1 = D19 sur Arduino Mega)
                   |
                   R
                   |
                  +5V
```

La sortie TX du Minitel est à collecteur ouvert : elle nécessite une résistance de tirage R (pull-up). J'utilise R = 10 kilohms (la documentation technique indique une valeur comprise entre 2,3 et 200 kilohms, p.69). Le +5V est récupéré sur la broche DIN 5 (+8,5V), sur laquelle est branché un régulateur 5V.

### Carte fonctionnant en 3,3V (ESP32)

**Attention : l'ESP32 fonctionne en 3,3V et ses broches ne supportent pas le 5V.** Il ne faut donc pas relier directement sa broche RX à la ligne TX du Minitel, tirée au +5V par la résistance R : il faut intercaler un convertisseur de tension (convertisseur de niveaux logiques bidirectionnel 3,3V <-> 5V) entre le Minitel et l'ESP32.

```
Dos du Minitel             Convertisseur de niveaux             ESP32-WROOM-32E
                           côté 5V (HV)  côté 3,3V (LV)

DIN 1 : RX    <--------->  HV1           LV1  <--------->  TX    (TX2 = GPIO17)
DIN 4 :       <---/ /--->
DIN 2 : GND   <--------->  GND           GND  <--------->  GND
DIN 5 : +8,5V <---/ /--->
DIN 3 : TX    <----+---->  HV2           LV2  <--------->  RX    (RX2 = GPIO16)
                   |
                   R
                   |
                  +5V ---->  HV            LV   <----------  3,3V  (broche 3V3 de l'ESP32)
```

- La résistance R de tirage reste du côté 5V (HV) du convertisseur.
- Le côté 3,3V (LV) est alimenté par la broche 3V3 de l'ESP32.
- Le +5V (régulateur branché sur la broche DIN 5) alimente le côté 5V du convertisseur et peut alimenter l'ESP32 par sa broche 5V (VIN).

Voir aussi le fichier [Branchements_DIN5.txt](Branchements_DIN5.txt).

## Démarrage rapide

```cpp
#include <Minitel1B_Hard.h>

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
Minitel minitel(Serial2, 16, 17);  // ESP32 : port série, broche RX, broche TX
#else
Minitel minitel(Serial1);          // ATmega 1284P ou ATmega 2560 notamment
#endif

void setup() {
  // À la mise sous tension, le Minitel échange à 1200 bauds.
  // On envisage cependant le cas où il se trouve dans un autre état.
  minitel.changeSpeed(minitel.searchSpeed());
  minitel.newScreen();
}

void loop() {
  minitel.print("Hello World ! ");
}
```

Sur ESP32, `rxPin` et `txPin` sont les broches de l'ESP32 : `rxPin` va au TX du Minitel et `txPin` au RX du Minitel.

La vitesse peut aussi se régler au clavier du Minitel : Fnct P + 3 (300 bauds), Fnct P + 1 (1200 bauds), Fnct P + 4 (4800 bauds), Fnct P + 9 (9600 bauds, Minitel 2 seulement). Si le Minitel et la carte n'échangent pas à la même vitesse, la liaison est perdue ; `searchSpeed()` permet de la retrouver.

## Exemples

| Exemple | Contenu |
|---|---|
| `HelloWorld` | Le minimum pour afficher du texte. |
| `Demo` | Texte et semi-graphique avec ou sans lignage, tailles, couleurs, curseur. |
| `Portrait` | Affichage d'une image vidéotex stockée en mémoire programme (voir [Conversion_Videotex_Hex](https://github.com/eserandour/Conversion_Videotex_Hex)). |
| `Tweet_3615` | Saisie d'un texte au clavier dans un champ, avec les touches ENVOI, ANNULATION et CORRECTION. |
| `TestCaracteresSpeciaux` | Affichage et saisie des caractères accentués et spéciaux, avec copie dans la console série. |

## Fonctions

La liste complète, commentée, se trouve dans [Minitel1B_Hard.h](Minitel1B_Hard.h).

**Liaison série et identification**

- `int changeSpeed(int bauds)` : change la vitesse (300, 1200, 4800 ; 9600 pour le Minitel 2).
- `int currentSpeed()` : vitesse en cours, le Minitel et la carte échangeant à la même vitesse.
- `int searchSpeed(unsigned long timeout = 0)` : recherche la vitesse du Minitel.
- `unsigned long identifyDevice()` : constructeur, type et version du logiciel du Minitel.
- `void writeByte(byte b)`, `void writeWord(word w)`, `void writeCode(unsigned long code)`, `byte readByte()` : accès direct aux octets.

**Écran et curseur**

- `newScreen()`, `newXY(x, y)` : nouvel écran, nouvelle position (ces deux fonctions réinitialisent les attributs).
- `cursor()`, `noCursor()`, `moveCursorXY(x, y)`, `moveCursorLeft(n)`, `moveCursorRight(n)`, `moveCursorUp(n)`, `moveCursorDown(n)`, `moveCursorReturn(n)`.
- `getCursorX()`, `getCursorY()` : position du curseur.
- `clearScreen()`, `clearScreenFromCursor()`, `clearScreenToCursor()`, `clearLine()`, `clearLineFromCursor()`, `clearLineToCursor()`, `cancel()`.
- `deleteChars(n)`, `insertChars(n)`, `startInsert()`, `stopInsert()`, `deleteLines(n)`, `insertLines(n)`.

**Contenu**

- `print(String chaine)`, `println(String chaine)`, `println()` : texte en UTF-8, converti en codes Minitel.
- `printChar(char caractere)`, `printSpecialChar(byte b)` : caractère du jeu G0, caractère du jeu G2.
- `attributs(byte attribut)` : couleurs de caractère et de fond, taille, clignotement, lignage, inversion (constantes `CARACTERE_...`, `FOND_...`, `DOUBLE_HAUTEUR`, `CLIGNOTEMENT`, etc.).
- `textMode()`, `graphicMode()` : jeu G0 (alphanumérique) ou G1 (semi-graphique).
- `graphic(byte b)`, `graphic(byte b, int x, int y)` : caractère semi-graphique de 6 pseudo-pixels, de `0b000000` à `0b111111`.
- `repeat(int n)`, `bip()`.
- `rect(x1, y1, x2, y2)`, `hLine(x1, y, x2, position)`, `vLine(x, y1, y2, position, sens)`.

**Clavier**

- `unsigned long getKeyCode(bool unicode = true)` : code de la touche frappée, converti en Unicode par défaut ; 0 si aucune touche. Les touches de fonction renvoient les constantes `ENVOI`, `RETOUR`, `REPETITION`, `GUIDE`, `ANNULATION`, `SOMMAIRE`, `CORRECTION`, `SUITE`, `CONNEXION_FIN`.
- `String getString(unsigned long code)`, `int getNbBytes(unsigned long code)` : conversion d'un code Unicode en chaîne UTF-8.
- `smallMode()`, `capitalMode()` : clavier en minuscules ou en majuscules.
- `extendedKeyboard()`, `standardKeyboard()` : clavier étendu (touches de gestion du curseur) ou standard.
- `echo(boolean commande)` : active ou désactive l'écho à l'écran de ce qui est tapé au clavier.

**Modes, standards et protocole**

- `pageMode()`, `scrollMode()` : mode page ou mode rouleau.
- `modeMixte()`, `modeVideotex()` : 80 colonnes ou 40 colonnes.
- `standardTeleinformatique()`, `standardTeletel()`.
- `aiguillage(commande, emetteur, recepteur)`, `statusAiguillage(module)`.
- `connexion(boolean commande)` : équivalent de la touche CONNEXION / FIN.
- `reset()`.

**Caractères affichés par `print`**

- Les caractères du jeu G0, sauf `^` et `` ` `` qui ne sont pas visualisables seuls.
- `à â ä è é ê ë î ï ô ö ù û ü ç`
- `£ § ° ± ¼ ½ ¾ ÷ Œ œ β`
- `— ← ↑ → ↓`
- Les majuscules accentuées sont affichées sans accent (le Minitel n'en dispose pas, voir p.90).
- Les autres caractères (et les caractères de contrôle) sont ignorés.

## Valeurs de retour et délais d'attente

Aucune fonction ne bloque le programme si le Minitel ne répond pas, à l'exception de `searchSpeed()` appelée sans paramètre.

| Fonction | Sans réponse du Minitel |
|---|---|
| `changeSpeed`, `currentSpeed` | -1 au bout d'une seconde. `changeSpeed` renvoie aussi -1, sans rien modifier, si la vitesse demandée n'est pas prévue. |
| `searchSpeed()` | Cherche jusqu'à obtenir une réponse, ce qui laisse au Minitel le temps d'être mis sous tension. |
| `searchSpeed(timeout)` | -1 au bout de `timeout` millisecondes. |
| `getCursorX`, `getCursorY` | -1 au bout d'une seconde. |
| `identifyDevice` | 0 au bout d'une seconde. |
| `pageMode`, `scrollMode`, `smallMode`, `capitalMode`, `extendedKeyboard`, `standardKeyboard`, `echo`, `aiguillage`, `statusAiguillage`, `connexion` | 0 au bout d'une seconde. |
| `modeMixte`, `modeVideotex`, `standardTeleinformatique`, `standardTeletel`, `reset` | 0 au bout de 100 ms (on peut alors supposer que le mode demandé était déjà actif), 1 si l'acquittement est reçu. |

`getKeyCode` rend toujours la main : après une touche d'accent ou de tréma frappée seule, elle renvoie 0 tant que la lettre n'est pas frappée, puis le caractère accentué.

## Licence et remerciements

Copyright 2016-2026 - Eric Sérandour - <https://entropie.org/3615/>

Bibliothèque distribuée sous licence GNU GPL version 3 ou ultérieure (voir [LICENSE](LICENSE)).

Remerciements à BorisFR et iodeo.

## Historique

Dernière version : 12/11/2026.

### 12/11/2026

**Les fonctions ne bloquent plus le programme lorsque le Minitel ne répond pas** (voir [Valeurs de retour et délais d'attente](#valeurs-de-retour-et-délais-dattente)) :

- Délai d'attente maximal d'une seconde dans les fonctions du protocole, qui renvoient alors 0 ; `getCursorX()` et `getCursorY()` renvoient -1.
- `int searchSpeed(unsigned long timeout = 0)` : nouveau paramètre facultatif pour limiter la durée de la recherche.
- `getKeyCode` ne bloque plus après une touche d'accent ou de tréma frappée seule, ni sur une séquence incomplète.

Corrections de bugs :

- `getCursorX()`, `getCursorY()` et `connexion()` (sur ESP32 pour cette dernière) se bloquaient si une touche était frappée juste avant la demande.
- `print` envoyait des codes de contrôle au Minitel pour les caractères UTF-8 non gérés (lettres d'autres langues, emojis) ; ils sont maintenant ignorés.
- `getKeyCode` : le délai d'attente après la touche Esc dépend de la vitesse (les séquences du clavier étendu pouvaient être coupées à 300 bauds).
- `changeSpeed` renvoie -1 pour une vitesse non prévue, sans modifier le port série (la liaison était perdue).

**Branchements** : précision pour l'ESP32, qui fonctionne en 3,3V : il faut un convertisseur de tension entre le Minitel et l'ESP32 (voir [Branchements](#branchements)).

**Changement de nom** : la constante `SUPRESSION_CARACTERE` (faute de frappe) devient `SUPPRESSION_CARACTERE`. Les programmes qui utilisent l'ancien nom doivent être mis à jour.

### 25/03/2026

Intégration des modifications proposées par iodeo + Mise à jour des exemples en tenant compte des modifications chez Espressif Systems (version 3.3.7 de esp32 testée) :

- Utilisation de la config SERIAL_7E1 de HardwareSerial (calcul de parité natif des noyaux arduino et esp32)
- On ne ferme pas la liaison série au moment de changer la vitesse pour l'esp32 car cela parasite l'acquittement dans certains cas. La fermeture reste nécessaire pour les arduinos (ajout macro #else)
- Renommage de la variable SPEED en _SPEED (conflit avec le noyau)
- Ajout d'un constructeur pour ESP32 permettant la relocalisation des broches à utiliser pour le port Série. Les paramètres rxPin et txPin sont relatifs à l'esp32 : rxPin va au tx du minitel et txPin va au rx du minitel searchSpeed: index incrementation corrected
- Add timeout in identificationBytes function

### 12/03/2023

- Correction de deux bugs mineurs dans getKeyCode(bool unicode)
- Mise à jour de l'exemple `TestCaracteresSpeciaux.ino`

### 11/03/2023

- Suppression de l'exemple Couleurs_3615.ino (car redondant).
- Mise à jour de tous les autres exemples pour tenir compte des différences entre ce qui concerne l'ATmega 1284P et l'ESP32-WROOM-32E.

### 06/03/2023

Mise à jour des exemples :

- `TestCaracteresSpeciaux.ino`
- `Tweet_3615.ino`
- `Couleurs_3615.ino`

### 05/03/2023

Ajout d'une nouvelle fonction :

- `int getNbBytes(unsigned long code)` à utiliser (si besoin) en association avec String getString(unsigned long code) afin de déterminer le nombre d'octets utilisés pour un caractère codé en UTF-8.

Mise à jour des exemples :

- `TestCaracteresSpeciaux.ino`
- `Couleurs_3615.ino`
- `Tweet_3615.ino`

### 28/02/2023

- La fonction write(unsigned long code) est renommée writeCode(unsigned long code).
- J'ai dû changer son nom car avec ce nom un problème est apparu dans l'usage de la bibliothèque Minitel1B_Soft : le Minitel se bloquait. Il s'agissait probablement d'un conflit avec la fonction write de la bibliothèque SoftwareSerial. Par souci de cohérence entre les bibliothèques Minitel1B_Hard et Minitel1B_Soft, j'ai renommé write en writeCode.

### 27/02/2023

- Correction d'un bug dans unsigned long identificationBytes()
- Mise à jour de l'exemple `TestCaracteresSpeciaux.ino`

### 27/02/2023

Sur proposition de iodeo :

- Correction d'un bug sur le caractère Bêta.

Ajout d'une fonction pour identifier le Minitel :

- `unsigned long identifyDevice()`

### 26/02/2023

- Des exemples où il est question de caractères...

Ajout d'un nouvel exemple :

- `TestCaracteresSpeciaux.ino`

Modification des exemples :

- `Couleurs_3615.ino`
- `Tweet_3615.ino`

### 26/02/2023

Sur proposition de iodeo :

Pour une meilleure gestion des caractères spéciaux (notamment dans la console série) :

Ajout de nouvelles fonctions :

- `void write(unsigned long code)`
- `String getString(unsigned long code)`
- `boolean isVisualisable(unsigned long code)`

Modification de :

- `void print(String chaine)`
- `unsigned long getKeyCode(bool unicode = true)`

Suppression, car devenues inutiles, de :

- `void printDiacriticChar(unsigned char caractere)`
- `boolean isDiacritic(unsigned char caractere)`

### 25/06/2022

Sur proposition de iodeo :

- Correction d'un bug dans la fonction byte reset()

### 24/04/2022

- Ajout d'un exemple : Portrait.ino (affichage d'une image)

### 13/03/2022

- Ajout d'un exemple basique : `HelloWorld.ino`

### 21/02/2022

Sur proposition de iodeo :

Ajout d'une option qui permet de désactiver la conversion des caractères en ascii étendue (cela sert en particulier pour la communication avec des services Minitel externes) :

- `unsigned long getKeyCode(bool ascii = true)`

### 08/10/2021

Sur proposition de iodeo :

- Correction de bugs mineurs dans echo(boolean commande) et changeSpeed(int bauds).

Ajout d'une nouvelle fonction liée au modem (équivalent de la touche CONNEXION / FIN) :

- `byte connexion(boolean commande)`
- Patch pour Arduino-ESP32 core v1.0.6 https://github.com/espressif/arduino-esp32

### 02/07/2021

1 nouvelle fonction :

- `byte reset()`

### 01/07/2021

1 nouvelle fonction :

- `byte statusAiguillage(byte module)`

et mise à jour de :

- `byte pageMode()`
- `byte scrollMode()`
- `byte modeMixte()`
- `byte modeVideotex()`
- `byte standardTeleinformatique()`
- `byte standardTeletel()`
- `byte smallMode()`
- `byte capitalMode()`
- `byte extendedKeyboard()`
- `byte standardKeyboard()`
- `byte echo(boolean commande)`
- `byte aiguillage(boolean commande, byte emetteur, byte recepteur)`

### 29/06/2021

5 nouvelles fonctions :

- `void writeWord(word w)`
- `int modeMixte()`
- `int modeVideotex()`
- `int standardTeleinformatique()`
- `int standardTeletel()`

### 28/06/2021

Modification de la fonction :

- `unsigned long getKeyCode()` pour tenir compte des codes du clavier étendu.
- Dans le fichier d'entête (.h), ajout de plusieurs #define dans la partie "Clavier".

2 nouvelles fonctions :

- `void echo(boolean commande)`
- `void aiguillage(boolean commande, byte emetteur, byte recepteur)`

### 11/05/2018

Modification de deux fonctions :

- `void printDiacriticChar(unsigned char caractere)`
- `boolean isDiacritic(unsigned char caractere)`

pour tenir compte de la remarque de BorisFR sur les caractères diacritiques majuscules.

### 18/06/2017

- Exemple `Tweet_3615.ino`

### 15/06/2017

2 nouvelles fonctions qui donnent la position du curseur :

- `int getCursorX()`
- `int getCursorY()`

### 06/06/2017

Exemple d'utilisation du clavier et d'affichage d'une image :

- `Couleurs_3615.ino`

### 06/06/2017

2 nouvelles fonctions graphiques :

- `void graphic(byte b, int x, int y)`
- `void graphic(byte b)`

### 12/03/2016

- Début de la version HardwareSerial de Minitel1B sur GitHub
