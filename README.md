# Minitel1B_Hard
<b>Minitel Library for Arduino (with HardwareSerial)</b>

Documentation utilisée : Spécifications Techniques d'Utilisation du Minitel 1B<br>
http://543210.free.fr/TV/stum1b.pdf

<b>Historique</b> :

<b>Dernière Version :</b> 25/03/2026 à 21h32.<br>
J'utilise la version 1.8.19 d'Arduino pour compiler.<br>
Je travaille avec un ATmega 1284P, un ATmega 2560 et un ESP32-WROOM-32E.<br>
Pour la carte ESP32, j'utilise esp32 d'Espressif Systems (version 3.3.7) - URL de gestionnaire de cartes supplémentaires : https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
Avec la carte Arduino Mega (ATmega 2560), j'utilise le <a href="https://entropie.org/3615/index.php/hardware-2023/" target="_blank">shield 3615</a> (version 2023).

25/03/2026<br>
Intégration des modifications proposées par iodeo + Mise à jour des exemples en tenant compte des modifications chez Espressif Systems (version 3.3.7 de esp32 testée) :<br>
* Utilisation de la config SERIAL_7E1 de hardwareSerial (calcul de parité natif des noyaux arduino et esp32)<br>
* On ne ferme pas la liaison série au moment de changer la vitesse pour l'esp32 car cela parasite l'acquittement dans certains cas. La fermeture reste nécessaire pour les arduinos (ajout macro #else)<br>
* Renommage de la variable SPEED en _SPEED (conflit avec le noyau)<br>
* Ajout d'un constructeur pour ESP32 permettant la relocalisation des broches à utiliser pour le port Série. Les paramètres rxPin et txPin sont relatifs à l'esp32 : rxPin va au tx du minitel et txPin va au rx du minitel<br>

12/03/2023<br>
<b>Correction de deux bugs mineurs</b> dans getKeyCode(bool unicode)<br>
<b>Mise à jour de l'exemple</b> TestCaracteresSpeciaux.ino<br>

11/03/2023<br>
Suppression de l'exemple Couleurs_3615.ino (car redondant).<br>
<b>Mise à jour de tous les autres exemples</b> pour tenir compte des différences entre ce qui concerne l'ATmega 1284P et l'ESP32-WROOM-32E.<br>

06/03/2023<br>
<b>Mise à jour des exemples :</b><br>
TestCaracteresSpeciaux.ino<br>
Tweet_3615.ino<br>
Couleurs_3615.ino<br>

05/03/2023<br>
<b>Ajout d'une nouvelle fonction :</b><br>
int getNbBytes(unsigned long code)<br>
à utiliser (si besoin) en association avec String getString(unsigned long code) afin de déterminer le nombre d'octets utilisés pour un caractère codé en UTF-8.<br>
<b>Mise à jour des exemples :</b><br>
TestCaracteresSpeciaux.ino<br>
Couleurs_3615.ino<br>
Tweet_3615.ino<br>


28/02/2023<br>
<b>La fonction write(unsigned long code) est renommée writeCode(unsigned long code).</b><br>
J'ai dû changer son nom car avec ce nom un problème est apparu dans l'usage de la bibliothèque Minitel1B_Soft : le Minitel se bloquait. Il s'agissait probablement d'un conflit avec la fonction write de la bibliothèque SoftwareSerial. Par souci de cohérence entre les bibliothèques Minitel1B_Hard et Minitel1B_Soft, j'ai renommé write en writeCode.<br>

27/02/2023<br>
Correction d'un bug dans unsigned long identificationBytes()<br>
Mise à jour de l'exemple TestCaracteresSpeciaux.ino<br>

27/02/2023 - Sur proposition de iodeo :<br>
Correction d'un bug sur le caractère Bêta.<br>
<b>Ajout d'une fonction pour identifier le Minitel :</b><br>
unsigned long identifyDevice()<br>

26/02/2023<br>
Des exemples où il est question de caractères...<br>
<b>Ajout d'un nouvel exemple :</b><br>
TestCaracteresSpeciaux.ino<br>
<b>Modification des exemples :</b><br>
Couleurs_3615.ino<br>
Tweet_3615.ino<br>

26/02/2023 - Sur proposition de iodeo :<br>
Pour une meilleure gestion des <b>caractères spéciaux</b> (notamment dans la console série) :<br>
<b>Ajout de nouvelles fonctions :</b><br>
void write(unsigned long code)<br>
String getString(unsigned long code)<br>
boolean isVisualisable(unsigned long code)<br>
<b>Modification de :</b><br>
void print(String chaine)<br>
unsigned long getKeyCode(bool unicode = true)<br>
<b>Suppression, car devenues inutiles, de :</b><br>
void printDiacriticChar(unsigned char caractere)<br>
boolean isDiacritic(unsigned char caractere)<br>

25/06/2022 - Sur proposition de iodeo :<br>
Correction d'un bug dans la fonction byte reset()

24/04/2022<br>
Ajout d'un exemple : Portrait.ino (affichage d'une image)<br>

13/03/2022<br>
Ajout d'un exemple basique : HelloWorld.ino<br>

21/02/2022 - Sur proposition de iodeo :<br>
Ajout d'une option qui permet de désactiver la conversion des caractères en ascii étendue (cela sert en particulier pour la communication avec des services Minitel externes) :<br>
unsigned long getKeyCode(bool ascii = true)

08/10/2021 - Sur proposition de iodeo :<br>
Correction de bugs mineurs dans echo(boolean commande) et changeSpeed(int bauds).<br>
Ajout d'une nouvelle fonction liée au modem (équivalent de la touche CONNEXION / FIN) :<br>
byte connexion(boolean commande)<br>
Patch pour Arduino-ESP32 core v1.0.6 https://github.com/espressif/arduino-esp32<br>

02/07/2021<br>
1 nouvelle fonction :<br>
byte reset()<br>

01/07/2021<br>
1 nouvelle fonction :<br>
byte statusAiguillage(byte module)<br>
et mise à jour de :<br>
byte pageMode()<br>
byte scrollMode()<br>
byte modeMixte()<br>
byte modeVideotex()<br>
byte standardTeleinformatique()<br>
byte standardTeletel()<br>
byte smallMode()<br>
byte capitalMode()<br>
byte extendedKeyboard()<br>
byte standardKeyboard()<br>
byte echo(boolean commande)<br>
byte aiguillage(boolean commande, byte emetteur, byte recepteur)<br>

29/06/2021<br>
5 nouvelles fonctions :<br>
void writeWord(word w)<br>
int modeMixte()<br>
int modeVideotex()<br>
int standardTeleinformatique()<br>
int standardTeletel()<br>


28/06/2021<br>
Modification de la fonction :<br>
unsigned long getKeyCode()<br>
pour tenir compte des codes du clavier étendu.<br>
Dans le fichier d'entête (.h), ajout de plusieurs #define dans la partie "Clavier".<br>
2 nouvelles fonctions :<br>
void echo(boolean commande)<br>
void aiguillage(boolean commande, byte emetteur, byte recepteur)

11/05/2018<br>
Modification de deux fonctions :<br>
void printDiacriticChar(unsigned char caractere)<br>
boolean isDiacritic(unsigned char caractere)<br>
pour tenir compte de la remarque de BorisFR sur les caractères diacritiques majuscules.

18/06/2017<br>
Exemple Tweet_3615.ino<br>

15/06/2017<br>
2 nouvelles fonctions qui donnent la position du curseur : <br>
int getCursorX()<br>
int getCursorY()<br>

06/06/2017<br>
Exemple d'utilisation du clavier et d'affichage d'une image :<br>
Couleurs_3615.ino<br>

06/06/2017<br>
2 nouvelles fonctions graphiques :<br>
void graphic(byte b, int x, int y)<br>
void graphic(byte b)<br>

12/03/2016<br>
Début de la version HardwareSerial de Minitel1B sur GitHub<br>
