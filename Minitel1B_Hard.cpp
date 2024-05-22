////////////////////////////////////////////////////////////////////////
/*
   Minitel1B_Hard - Fichier source - Version du 12 mars 2023 à 04h55
   Copyright 2016-2023 - Eric Sérandour
   https://entropie.org/3615/
   
   Remerciements à :
   BorisFR, iodeo

   Documentation utilisée :
   Spécifications Techniques d'Utilisation du Minitel 1B
   http://543210.free.fr/TV/stum1b.pdf

////////////////////////////////////////////////////////////////////////

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
////////////////////////////////////////////////////////////////////////

#include "Minitel1B_Hard.h"

////////////////////////////////////////////////////////////////////////
/*
   Public
*/
////////////////////////////////////////////////////////////////////////

Minitel::Minitel(HardwareSerial& serial) : mySerial(serial) {
  // A la mise sous tension du Minitel, la vitesse des échanges entre
  // le Minitel et le périphérique est de 1200 bauds par défaut.
  mySerial.begin(1200);
}
/*--------------------------------------------------------------------*/

void Minitel::writeByte(byte b) {
  // Le bit de parité est mis à 0 si la somme des autres bits est paire
  // et à 1 si elle est impaire.
  boolean parite = 0;
  for (int i=0; i<7; i++) {
    if (bitRead(b,i) == 1)  {
      parite = !parite;
    }
  }
  if (parite) {
    bitWrite(b,7,1);  // Ecriture du bit de parité
  }
  else {
    bitWrite(b,7,0);  // Ecriture du bit de parité
  }
  mySerial.write(b);  // Envoi de l'octet sur le port série
}
/*--------------------------------------------------------------------*/

void Minitel::writeWord(word w) {
  writeByte(highByte(w));
  writeByte(lowByte(w));
}
/*--------------------------------------------------------------------*/

void Minitel::writeCode(unsigned long code) {
  // Fonction proposée par iodeo sur GitHub en février 2023
  // Initialement, cette fonction se nommait write, mais j'ai dû changer
  // son nom car avec ce nom un problème est apparu dans l'usage de la
  // bibliothèque Minitel1B_Soft : le Minitel se bloquait. Il s'agissait
  // probablement d'un conflit avec la fonction write de la bibliothèque
  // SoftwareSerial. Par souci de cohérence entre les bibliothèques
  // Minitel1B_Hard et Minitel1B_Soft, j'ai renommé write en writeCode.
  if (code > 0x00FFFFFF) writeByte((byte) (code >> 24));
  if (code > 0x0000FFFF) writeByte((byte) (code >> 16));
  if (code > 0x000000FF) writeByte((byte) (code >> 8));
  writeByte((byte) code);
}
/*--------------------------------------------------------------------*/

byte Minitel::readByte() {
  byte b = mySerial.read();
  // Le bit de parité est à 0 si la somme des autres bits est paire
  // et à 1 si elle est impaire.
  boolean parite = 0;
  for (int i=0; i<7; i++) {
    if (bitRead(b,i) == 1)  {
      parite = !parite;
    }
  }
  if (bitRead(b,7) == parite) {  // La transmission est bonne, on peut récupérer la donnée.
    if (bitRead(b,7) == 1) {  // Cas où le bit de parité vaut 1.
      b = b ^ 0b10000000;  // OU exclusif pour mettre le bit de parité à 0 afin de récupérer la donnée.
    }
    return b;
  }
  else {
    return 0xFF;  // Pour indiquer une erreur de parité.
  }
}
/*--------------------------------------------------------------------*/

unsigned long Minitel::identifyDevice() {  // Voir p.139
  // Fonction proposée par iodeo sur GitHub en février 2023
  // Demande
  writeBytesPRO(1);  // 0x1B 0x39
  writeByte(ENQROM);  // 0x7B
  // Réponse
  return identificationBytes();  // 3 octets
                                 // octet définissant le constructeur du Minitel
                                 // octet définissant le type du Minitel
                                 // octet définissant la version du logiciel

  // Codes d'identification de l'octet de poids fort :
  /*
     Philips       : 0x42
     Telic-Alcatel : 0x43
     à compléter...
  */

  // Codes d'identification de l'octet du milieu (voir p.103 du Guide STU du Minitel 2) :
  /*
     Minitel 1             : 0x62, 0x63 ou 0x72 suivant les modèles
     Minitel 1 Couleur     : 0x73
     Minitel 1 Dialogue    : 0x72
     Minitel 10            : 0x64 ou 0x66 suivant les modèles
     Minitel 1 Bistandard  : 0x75
     Minitel 10 Bistandard : 0x77
     Minitel 2             : 0x76
     Minitel 12            : 0x7A
     Minitel 5             : 0x79
  */
}
/*--------------------------------------------------------------------*/

int Minitel::changeSpeed(int bauds) {  // Voir p.141
  // Fonction modifiée par iodeo sur GitHub en octobre 2021
  // Format de la commande
  writeBytesPRO(2);  // 0x1B 0x3A
  writeByte(PROG);   // 0x6B
  switch (bauds) {
    case  300 : writeByte(0b1010010); break;  // 0x52
    case 1200 : writeByte(0b1100100); break;  // 0x64
    case 4800 : writeByte(0b1110110); break;  // 0x76
    case 9600 : writeByte(0b1111111); break;  // 0x7F (pour le Minitel 2 seulement)
  }
  #if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
  mySerial.flush(false); // Patch pour Arduino-ESP32 core v1.0.6 https://github.com/espressif/arduino-esp32
  #endif
  mySerial.end();
  mySerial.begin(bauds);
  // Acquittement
  return workingSpeed();  // En bauds (voir section Private ci-dessous)
}
/*--------------------------------------------------------------------*/

int Minitel::currentSpeed() {  // Voir p.141
  // Demande
  writeBytesPRO(1);
  writeByte(STATUS_VITESSE);
  // Réponse
  return workingSpeed();  // En bauds (voir section Private ci-dessous)
}
/*--------------------------------------------------------------------*/

int Minitel::searchSpeed() {
  const int SPEED[4] = { 1200, 4800, 300, 9600 };  // 9600 bauds pour le Minitel 2 seulement
  int i = 0;
  int speed;
  do {
    mySerial.begin(SPEED[i]);
    if (i++ > 3) { i = 0; }
    speed = currentSpeed();
  } while (speed < 0);
  return speed;  // En bauds
}
/*--------------------------------------------------------------------*/

void Minitel::newScreen() {
  writeByte(FF);
  currentSize = GRANDEUR_NORMALE;
}
/*--------------------------------------------------------------------*/

void Minitel::newXY(int x, int y) {
  if (x==1 && y==1) {
    writeByte(RS);
  }
  else {
    // Le code US est suivi de deux caractères non visualisés. Si les
    // octets correspondants à ces deux caractères appartiennent tous deux
    // aux colonnes 4 à 7, ils représentent respectivement (sous forme
    // binaire avec 6 bits utiles) le numéro de rangée et le numéro de
    // colonne du premier caractère du sous-article (voir p.96).
    writeByte(US);
    writeByte(0x40 + y);  // Numéro de rangée
    writeByte(0x40 + x);  // Numéro de colonne
  }
  currentSize = GRANDEUR_NORMALE;
}
/*--------------------------------------------------------------------*/

void Minitel::cursor() {
  writeByte(CON);
}
/*--------------------------------------------------------------------*/

void Minitel::noCursor() {
  writeByte(COFF);
}
/*--------------------------------------------------------------------*/

void Minitel::moveCursorXY(int x, int y) {  // Voir p.95
  writeWord(CSI);   // 0x1B 0x5B
  writeBytesP(y);   // Pr : Voir section Private ci-dessous
  writeByte(0x3B);
  writeBytesP(x);   // Pc : Voir section Private ci-dessous
  writeByte(0x48);
}
/*--------------------------------------------------------------------*/

void Minitel::moveCursorLeft(int n) {  // Voir p.94 et 95
  if (n==1) { writeByte(BS); }
  else if (n>1) {
    // Curseur vers la gauche de n colonnes. Arrêt au bord gauche de l'écran.
    writeWord(CSI);   // 0x1B 0x5B
    writeBytesP(n);   // Pn : Voir section Private ci-dessous
    writeByte(0x44);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::moveCursorRight(int n) {  // Voir p.94
  if (n==1) { writeByte(HT); }
  else if (n>1) {
    // Curseur vers la droite de n colonnes. Arrêt au bord droit de l'écran.
    writeWord(CSI);   // 0x1B 0x5B
    writeBytesP(n);   // Pn : Voir section Private ci-dessous
    writeByte(0x43);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::moveCursorDown(int n) {  // Voir p.94
  if (n==1) { writeByte(LF); }
  else if (n>1) {
    // Curseur vers le bas de n rangées. Arrêt en bas de l'écran.
    writeWord(CSI);   // 0x1B 0x5B
    writeBytesP(n);   // Pn : Voir section Private ci-dessous
    writeByte(0x42);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::moveCursorUp(int n) {  // Voir p.94
  if (n==1) { writeByte(VT); }
  else if (n>1) {
    // Curseur vers le haut de n rangées. Arrêt en haut de l'écran.
    writeWord(CSI);   // 0x1B 0x5B
    writeBytesP(n);   // Pn : Voir section Private ci-dessous
    writeByte(0x41);
  }	
}
/*--------------------------------------------------------------------*/

void Minitel::moveCursorReturn(int n) {  // Voir p.94
  writeByte(CR);
  moveCursorDown(n);  // Pour davantage de souplesse
}
/*--------------------------------------------------------------------*/

int Minitel::getCursorX() {
  return (getCursorXY() & 0x0000FF) - 0x40;
}
/*--------------------------------------------------------------------*/

int Minitel::getCursorY() {
  return ((getCursorXY() & 0x00FF00) >> 8) - 0x40;
}
/*--------------------------------------------------------------------*/

void Minitel::cancel() {  // Voir p.95
  writeByte(CAN);
}
/*--------------------------------------------------------------------*/

void Minitel::clearScreenFromCursor() {  // Voir p.95
  writeWord(CSI);  // 0x1B 0x5B
  // writeByte(0x30);  Inutile
  writeByte(0x4A);
}
/*--------------------------------------------------------------------*/

void Minitel::clearScreenToCursor() {  // Voir p.95
  writeWord(CSI);  // 0x1B 0x5B
  writeByte(0x31);
  writeByte(0x4A);
}
/*--------------------------------------------------------------------*/

void Minitel::clearScreen() {  // Voir p.95
  writeWord(CSI);  // 0x1B 0x5B
  writeByte(0x32);
  writeByte(0x4A);
}
/*--------------------------------------------------------------------*/

void Minitel::clearLineFromCursor() {  // Voir p.95
  writeWord(CSI);  // 0x1B 0x5B
  // writeByte(0x30);  Inutile
  writeByte(0x4B);
}
/*--------------------------------------------------------------------*/

void Minitel::clearLineToCursor() {  // Voir p.95
  writeWord(CSI);  // 0x1B 0x5B
  writeByte(0x31);
  writeByte(0x4B);
}
/*--------------------------------------------------------------------*/

void Minitel::clearLine() {  // Voir p.95
  writeWord(CSI);  // 0x1B 0x5B
  writeByte(0x32);
  writeByte(0x4B);
}
/*--------------------------------------------------------------------*/

void Minitel::deleteChars(int n) {  // Voir p.95
  writeWord(CSI);  // 0x1B 0x5B
  writeBytesP(n);  // Voir section Private ci-dessous
  writeByte(0x50);
}
/*--------------------------------------------------------------------*/

void Minitel::insertChars(int n) {  // Voir p.95
  writeWord(CSI);  // 0x1B 0x5B
  writeBytesP(n);  // Voir section Private ci-dessous
  writeByte(0x40);
}
/*--------------------------------------------------------------------*/

void Minitel::startInsert() {  // Voir p.95
  writeWord(CSI);  // 0x1B 0x5B
  writeByte(0x34);
  writeByte(0x68);
}
/*--------------------------------------------------------------------*/

void Minitel::stopInsert() {  // Voir p.95
  writeWord(CSI);  // 0x1B 0x5B
  writeByte(0x34);
  writeByte(0x6C);
}
/*--------------------------------------------------------------------*/

void Minitel::deleteLines(int n) {  // Voir p.95
  writeWord(CSI);  // 0x1B 0x5B
  writeBytesP(n);  // Voir section Private ci-dessous
  writeByte(0x4D);
}
/*--------------------------------------------------------------------*/

void Minitel::insertLines(int n) {  // Voir p.95
  writeWord(CSI);  // 0x1B 0x5B
  writeBytesP(n);  // Voir section Private ci-dessous
  writeByte(0x4C);
}
/*--------------------------------------------------------------------*/

void Minitel::textMode() {
  writeByte(SI);  // Accès au jeu G0 (voir p.100)
}
/*--------------------------------------------------------------------*/

void Minitel::graphicMode() {
  writeByte(SO);  // Accès au jeu G1 (voir p.101 & 102)
}
/*--------------------------------------------------------------------*/

byte Minitel::pageMode() {
  // Commande
  writeBytesPRO(2);    // 0x1B 0x3A
  writeByte(STOP);     // 0x6A
  writeByte(ROULEAU);  // 0x43
  // Acquittement
  return workingMode();  // Renvoie un octet
}
/*--------------------------------------------------------------------*/

byte Minitel::scrollMode() {
  // Commande
  writeBytesPRO(2);    // 0x1B 0x3A
  writeByte(START);    // 0x69
  writeByte(ROULEAU);  // 0x43
  // Acquittement
  return workingMode();  // Renvoie un octet
}
/*--------------------------------------------------------------------*/

byte Minitel::modeMixte() {  // Voir p.144
  // Passage du standard Télétel mode Vidéotex au standard Télétel mode Mixte
  // Commande
  writeBytesPRO(2);   // 0x1B 0x3A
  writeWord(MIXTE1);  // 0x32 0x7D
  // Acquittement
  return workingStandard(0x1370);  // SEP (0x13), 0x70
}
/*--------------------------------------------------------------------*/

byte Minitel::modeVideotex() {  // Voir p.144
  // Passage du standard Télétel mode Mixte au standard Télétel mode Vidéotex
  // Commande
  writeBytesPRO(2);   // 0x1B 0x3A
  writeWord(MIXTE2);  // 0x32 0x7E
  // Acquittement
  return workingStandard(0x1371);  // SEP (0x13), 0x71
}
/*--------------------------------------------------------------------*/

byte Minitel::standardTeleinformatique() {  // Voir p.144
  // Passage du standard Télétel au standard Téléinformatique
  // Commande
  writeBytesPRO(2);    // 0x1B 0x3A
  writeWord(TELINFO);  // 0x31 0x7D
  // Acquittement
  return workingStandard(0x1B5B3F7A);  // CSI (0x1B,0x5B), 0x3F, 0x7A
}
/*--------------------------------------------------------------------*/

byte Minitel::standardTeletel() {  // Voir p.144
  // Passage du standard Téléinformatique au standard Télétel
  // Commande
  writeWord(CSI);  // 0x1B Ox5B
  writeByte(0x3F);
  writeByte(0x7B);
  // Acquittement
  return workingStandard(0x135E);  // SEP (0x13), 0x5E
}
/*--------------------------------------------------------------------*/

void Minitel::attributs(byte attribut) {
  writeByte(ESC);  // Accès à la grille C1 (voir p.92)
  writeByte(attribut);
  if (attribut == DOUBLE_HAUTEUR || attribut == DOUBLE_GRANDEUR) {
    moveCursorDown(1);
    currentSize = attribut;
  }
  else if (attribut == GRANDEUR_NORMALE || attribut == DOUBLE_LARGEUR) {
    currentSize = attribut;
  }
}
/*--------------------------------------------------------------------*/

void Minitel::print(String chaine) {
  // Fonction modifiée par iodeo sur GitHub en février 2023
/*
  // Fonction initiale (pour mémoire)  // Obsolète depuis le 26/02/2023
  for (int i=0; i<chaine.length(); i++) {
    unsigned char caractere = chaine.charAt(i);
    if (!isDiacritic(caractere)) {
      printChar(caractere);
    }
    else {
      i+=1;  // Un caractère accentué prend la place de 2 caractères
      caractere = chaine.charAt(i);
      printDiacriticChar(caractere);
    }
  }
*/
  // codes UTF-8 vers codes Minitel
  unsigned int i = 0;
  while (i < chaine.length()) {
    unsigned long code = (byte) chaine.charAt(i++);
    if (code < SP) code = 0;
    else if (code >= SP && code <= DEL) {
      switch (code) {
        case 0x5E: code = 0; break; // ^ non visualisable seul
        case 0x60: code = 0; break; // ` non visualisable seul
      }
    }
    else if (code == 0xC2 || code == 0xC3 || code == 0xC5 || code == 0xCE) {
      // Caractères sur 2 octets
      code = (code << 8) + (byte) chaine.charAt(i++);
      switch (code) {                        // Voir p.90 pour VGP5 ou VGP2
                         // 0x19 => SS2 (Accès au jeu G2)
                         // 0x0F => SI (Accès au jeu G0)
        case 0xC2A3: code = 0x1923; break;   // £ (VGP5 et VGP2)
        case 0xC2A7: code = 0x1927; break;   // § (VGP5 seulement)
        case 0xC2B0: code = 0x1930; break;   // ° (VGP5 et VGP2)
        case 0xC2B1: code = 0x1931; break;   // ± (VGP5 et VGP2)
        case 0xC2BC: code = 0x193C; break;   // ¼ (VGP5 et VGP2)
        case 0xC2BD: code = 0x193D; break;   // ½ (VGP5 et VGP2)
        case 0xC2BE: code = 0x193E; break;   // ¾ (VGP5 et VGP2)
        case 0xC380: code = 0x0F41; break;   // À (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC382: code = 0x0F41; break;   // Â (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC384: code = 0x0F41; break;   // Ä (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC387: code = 0x0F43; break;   // Ç (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC388: code = 0x0F45; break;   // È (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC389: code = 0x0F45; break;   // É (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC38A: code = 0x0F45; break;   // Ê (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC38B: code = 0x0F45; break;   // Ë (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC38E: code = 0x0F49; break;   // Î (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC38F: code = 0x0F49; break;   // Ï (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC394: code = 0x0F4F; break;   // Ô (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC396: code = 0x0F4F; break;   // Ö (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC399: code = 0x0F55; break;   // Ù (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC39B: code = 0x0F55; break;   // Û (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC39C: code = 0x0F55; break;   // Ü (Aucune lettre accentuée majuscule n'est disponible - voir p.90)
        case 0xC3A0: code = 0x194161; break; // à (VGP5 et VGP2)
        case 0xC3A2: code = 0x194361; break; // â (VGP5 et VGP2)
        case 0xC3A4: code = 0x194861; break; // ä (VGP5 seulement)
        case 0xC3A7: code = 0x194B63; break; // ç (VGP5 et VGP2)
        case 0xC3A8: code = 0x194165; break; // è (VGP5 et VGP2)
        case 0xC3A9: code = 0x194265; break; // é (VGP5 et VGP2)
        case 0xC3AA: code = 0x194365; break; // ê (VGP5 et VGP2)
        case 0xC3AB: code = 0x194865; break; // ë (VGP5 et VGP2)
        case 0xC3AE: code = 0x194369; break; // î (VGP5 et VGP2)
        case 0xC3AF: code = 0x194869; break; // ï (VGP5 et VGP2)
        case 0xC3B4: code = 0x19436F; break; // ô (VGP5 et VGP2)
        case 0xC3B6: code = 0x19486F; break; // ö (VGP5 seulement)
        case 0xC3B7: code = 0x1938; break;   // ÷ (VGP5 et VGP2)
        case 0xC3B9: code = 0x194175; break; // ù (VGP5 et VGP2)
        case 0xC3BB: code = 0x194375; break; // û (VGP5 et VGP2)
        case 0xC3BC: code = 0x194875; break; // ü (VGP5 seulement)
        case 0xC592: code = 0x196A; break;   // Œ (VGP5 et VGP2)
        case 0xC593: code = 0x197A; break;   // œ (VGP5 et VGP2)
        case 0xCEB2: code = 0x197B; break;   // β (VGP5 seulement)
        default: code = 0; // supposé non-visualisable
      }
    }
    else if (code == 0xE2) {
      // Caractères sur 3 octets
      code = (code << 8) + (byte) chaine.charAt(i++);
      code = (code << 8) + (byte) chaine.charAt(i++);
      switch (code) {
        case 0xE28094: code = 0x60; break;   // —
        case 0xE28690: code = 0x192C; break; // ←
        case 0xE28691: code = 0x5E; break;   // ↑
        case 0xE28692: code = 0x192E; break; // →
        case 0xE28693: code = 0x192F; break; // ↓
        default: code = 0; // supposé non-visualisable
      }
    }
    if (code != 0) writeCode(code);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::println(String chaine) {
  print(chaine);
  if (currentSize == DOUBLE_HAUTEUR || currentSize == DOUBLE_GRANDEUR) {
    moveCursorReturn(2);
  }
  else {
    moveCursorReturn(1);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::println() {
  if (currentSize == DOUBLE_HAUTEUR || currentSize == DOUBLE_GRANDEUR) {
    moveCursorReturn(2);
  }
  else {
    moveCursorReturn(1);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::printChar(char caractere) {
  // Peut s'utiliser de 2 manières : printChar('A') ou printChar(0x41) par exemple
  //                                 printChar("A") ne fonctionne pas
  byte charByte = getCharByte(caractere);
  if (isValidChar(charByte)) {
    writeByte(charByte);
  }
}
/*--------------------------------------------------------------------*/
/*
void Minitel::printDiacriticChar(unsigned char caractere) {  // Obsolète depuis le 26/02/2023
  writeByte(SS2);  // Accès au jeu G2 (voir p.103)
  String diacritics = "àâäèéêëîïôöùûüçÀÂÄÈÉÊËÎÏÔÖÙÛÜÇ";
  // Dans une chaine de caractères, un caractère diacritique prend la
  // place de 2 caractères simples, ce qui explique le /2.
  int index = (diacritics.indexOf(caractere)-1)/2;
  char car;
  switch (index) {
    case( 0): car = 'a'; writeByte(ACCENT_GRAVE); break;
    case( 1): car = 'a'; writeByte(ACCENT_CIRCONFLEXE); break;
    case( 2): car = 'a'; writeByte(TREMA); break;
    case( 3): car = 'e'; writeByte(ACCENT_GRAVE); break;
    case( 4): car = 'e'; writeByte(ACCENT_AIGU); break;
    case( 5): car = 'e'; writeByte(ACCENT_CIRCONFLEXE); break;
    case( 6): car = 'e'; writeByte(TREMA); break;
    case( 7): car = 'i'; writeByte(ACCENT_CIRCONFLEXE); break;
    case( 8): car = 'i'; writeByte(TREMA); break;
    case( 9): car = 'o'; writeByte(ACCENT_CIRCONFLEXE); break;
    case(10): car = 'o'; writeByte(TREMA); break;
    case(11): car = 'u'; writeByte(ACCENT_GRAVE); break;
    case(12): car = 'u'; writeByte(ACCENT_CIRCONFLEXE); break;      
    case(13): car = 'u'; writeByte(TREMA); break;
    case(14): car = 'c'; writeByte(CEDILLE); break;
    // Pour les cas où on essaye d'afficher un caractère diacritique majuscule,
    // ce que ne peut pas faire le Minitel.
    case(15): car = 'A'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
    case(16): car = 'A'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
    case(17): car = 'A'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
    case(18): car = 'E'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
    case(19): car = 'E'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
    case(20): car = 'E'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
    case(21): car = 'E'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
    case(22): car = 'I'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
    case(23): car = 'I'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
    case(24): car = 'O'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
    case(25): car = 'O'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
    case(26): car = 'U'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
    case(27): car = 'U'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
    case(28): car = 'U'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
    case(29): car = 'C'; writeByte(SI); break; // Accès au jeu G0 (voir p.100)
  }
  printChar(car);
}
*/
/*--------------------------------------------------------------------*/

void Minitel::printSpecialChar(byte b) {
  // N'est pas fonctionnelle pour les diacritiques (accents, tréma et cédille)
  writeByte(SS2);  // Accès au jeu G2 (voir p.103)
  writeByte(b);
}
/*--------------------------------------------------------------------*/

byte Minitel::getCharByte(char caractere) {
  // Voir les codes et séquences émis en mode Vidéotex (Jeu G0 p.100).
  // Dans la chaine ci-dessous, on utilise l'échappement (\) :
  // \" rend au guillemet sa signification littérale.
  // \\ donne à l'antislash sa signification littérale .
  String caracteres = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_xabcdefghijklmnopqrstuvwxyz{|}";
  return (byte) caracteres.lastIndexOf(caractere);
}
/*--------------------------------------------------------------------*/

String Minitel::getString(unsigned long code) {
  // Fonction proposée par iodeo sur GitHub en février 2023
  // Convertit un caractère Unicode en String UTF-8
  // Renvoie "" si le code ne correspond pas à un caractère visualisable
  String str = "";
  // Pour test
  /*
  Serial.print("isVisual ");
  Serial.print(code);
  Serial.print("\t : ");
  Serial.println(isVisualisable(code));
  */
  if (isVisualisable(code)) {
    if (code < 0x80) { // U+0000 à U+007F
      str += char(code);
    } else if (code < 0x800) { // U+0080 à U+07FF
      str += char((0b110 << 5) | (code >> 6));
      str += char((0b10 << 6) | (code & 0x3F));
    } else if(code < 0x10000) { // U+0800 à U+FFFF
      str += char((0b1110 << 4) | (code >> 12));
      str += char((0b10 << 6) | ((code >> 6) & 0x3F));
      str += char((0b10 << 6) | (code & 0x3F));
    }
  }
  return str;
}
/*--------------------------------------------------------------------*/

int Minitel::getNbBytes(unsigned long code) {
  // Cette fonction est à utiliser en association avec getString(unsigned long code) juste ci-dessus
  // Elle renvoie le nombre d'octets d'un caractère codé en String UTF-8
  int nbBytes = 0;
  if (isVisualisable(code)) {
    if (code < 0x80) { // U+0000 à U+007F
      nbBytes = 1;  // 1 octet
    } else if (code < 0x800) { // U+0080 à U+07FF
      nbBytes = 2;  // 2 octets
    } else if(code < 0x10000) { // U+0800 à U+FFFF
      nbBytes = 3;  // 3 octets
    }
  }
  return nbBytes;
}

/*--------------------------------------------------------------------*/

void Minitel::graphic(byte b, int x, int y) {
  newXY(x,y);
  graphic(b);
}
/*--------------------------------------------------------------------*/

void Minitel::graphic(byte b) {
  // Voir Jeu G1 page 101.
  if (b <= 0b111111) {
    b = 0x20
      + bitRead(b,5) 
      + bitRead(b,4) * 2
      + bitRead(b,3) * 4
      + bitRead(b,2) * 8
      + bitRead(b,1) * 16
      + bitRead(b,0) * 64;
    if (b == 0x7F) {  // 0b1111111
      b= 0x5F;
    }    
  writeByte(b);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::repeat(int n) {  // Voir p.98
  writeByte(REP);
  writeByte(0x40 + n);
}
/*--------------------------------------------------------------------*/

void Minitel::bip() {  // Voir p.98
  writeByte(BEL);
}
/*--------------------------------------------------------------------*/

void Minitel::rect(int x1, int y1, int x2, int y2) {
  hLine(x1,y1,x2,BOTTOM);
  vLine(x2,y1+1,y2,RIGHT,DOWN);
  hLine(x1,y2,x2,TOP);
  vLine(x1,y1,y2-1,LEFT,UP);
}
/*--------------------------------------------------------------------*/

void Minitel::hLine(int x1, int y, int x2, int position) {
  textMode();
  newXY(x1,y);
  switch (position) {
    case TOP    : writeByte(0x7E); break;
    case CENTER : writeByte(0x60); break;
    case BOTTOM : writeByte(0x5F); break;
  }
  repeat(x2-x1);
}
/*--------------------------------------------------------------------*/

void Minitel::vLine(int x, int y1, int y2, int position, int sens) {
  textMode();
  switch (sens) {
    case DOWN : newXY(x,y1); break;
    case UP   : newXY(x,y2); break;
  }
  for (int i=0; i<y2-y1; i++) {
    switch (position) {
      case LEFT   : writeByte(0x7B); break;
      case CENTER : writeByte(0x7C); break;
      case RIGHT  : writeByte(0x7D); break;
    }
    switch (sens) {
      case DOWN : moveCursorLeft(1); moveCursorDown(1); break;
      case UP   : moveCursorLeft(1); moveCursorUp(1); break;
    }
  }
}
/*--------------------------------------------------------------------*/

unsigned long Minitel::getKeyCode(bool unicode) {
  // Renvoie le code brut émis par le clavier (unicode = false)
  // ou sa conversion unicode si applicable (unicode = true, choix par défaut)
  unsigned long code = 0;
  // Code unique
  if (mySerial.available()>0) {
    code = readByte();
  }
  // Séquences de deux ou trois codes (voir p.118)
  if (code == 0x19) {  // SS2
    while (!mySerial.available()>0);  // Indispensable
    code = (code << 8) + readByte();
    // Les diacritiques (3 codes)
    if ((code == 0x1941) || (code == 0x1942) || (code == 0x1943) || (code == 0x1948) || (code == 0x194B)) {  // Accents, tréma, cédille
      // Bug 1 : Pour éviter de compter un caractère lorsqu'on appuie plusieurs fois de suite sur une touche avec accent ou tréma
      byte caractere = 0x19;
      while (caractere == 0x19) {  
        while (!mySerial.available()>0);  // Indispensable
        caractere = readByte();
        if (caractere == 0x19) {
          while (!mySerial.available()>0);  // Indispensable
          caractere = readByte();
          caractere = 0x19;
        }
      }
      // Bug 2 : Pour éviter de compter un caractère lorsqu'on appuie sur les touches de fonction après avoir appuyé sur une touche avec accent ou tréma
      if (caractere == 0x13) {  // Les touches RETOUR REPETITION GUIDE ANNULATION SOMMAIRE CORRECTION SUITE CONNEXION_FIN ont un code qui commence par 0x13
          while (!mySerial.available()>0);  // Indispensable
          caractere = readByte();  // Les touches de fonction sont codées sur 2 octets (0x13..)
          caractere = 0;
          code = 0;
      }
      code = (code << 8) + caractere;
      if (unicode) {
        switch (code) {  // On convertit le code reçu en unicode
          case 0x194161 : code = 0xE0; break;  // à
          case 0x194165 : code = 0xE8; break;  // è
          case 0x194175 : code = 0xF9; break;  // ù
          case 0x194265 : code = 0xE9; break;  // é
          case 0x194361 : code = 0xE2; break;  // â
          case 0x194365 : code = 0xEA; break;  // ê
          case 0x194369 : code = 0xEE; break;  // î
          case 0x19436F : code = 0xF4; break;  // ô
          case 0x194375 : code = 0xFB; break;  // û
          case 0x194861 : code = 0xE4; break;  // ä
          case 0x194865 : code = 0xEB; break;  // ë
          case 0x194869 : code = 0xEF; break;  // ï
          case 0x19486F : code = 0xF6; break;  // ö
          case 0x194875 : code = 0xFC; break;  // ü
          case 0x194B63 : code = 0xE7; break;  // ç
          default : code = caractere; break;
        }
      }
    }
    // Les autres caractères spéciaux disponibles sous Arduino (2 codes)
    else {
      if (unicode) {
        switch (code) {  // On convertit le code reçu en unicode
          case 0x1923 : code = 0xA3; break;    // Livre
          case 0x1927 : code = 0xA7; break;    // Paragraphe
          case 0x192C : code = 0x2190; break;  // Flèche gauche
          case 0x192E : code = 0x2192; break;  // Flèche droite
          case 0x192F : code = 0x2193; break;  // Flèche bas
          case 0x1930 : code = 0xB0; break;    // Degré
          case 0x1931 : code = 0xB1; break;    // Plus ou moins
          case 0x1938 : code = 0xF7; break;    // Division
          case 0x196A : code = 0x0152; break;  // Ligature OE
          case 0x197A : code = 0x0153; break;  // Ligature oe
          case 0x197B : code = 0x03B2; break;  // Bêta
        }
      }
    }
  }
  // Touches de fonction (voir p.123)
  else if (code == 0x13) {
    while (!mySerial.available()>0);  // Indispensable
    code = (code << 8) + readByte();
  }  
  // Touches de gestion du curseur lorsque le clavier est en mode étendu (voir p.124)
  // Pour passer au clavier étendu manuellement : Fnct C + E
  // Pour revenir au clavier vidéotex standard  : Fnct C + V
  else if (code == 0x1B) {
    delay(20);  // Indispensable. 0x1B seul correspond à la touche Esc,
                // on ne peut donc pas utiliser la boucle while (!available()>0).           
    if (mySerial.available()>0) {
      code = (code << 8) + readByte();
      if (code == 0x1B5B) {
        while (!mySerial.available()>0);  // Indispensable
        code = (code << 8) + readByte();
        if ((code == 0x1B5B34) || (code == 0x1B5B32)) {
          while (!mySerial.available()>0);  // Indispensable
          code = (code << 8) + readByte();
        }
      }
    }
  }
  else {
    if (unicode) {  // On convertit les codes uniques en unicode
      switch (code) {
        case 0x5E : code = 0x2191; break;  // Flèche haut
        case 0x60 : code = 0x2014; break;  // Tiret cadratin
      }
    }
  }
// Pour test
/*
  if (code != 0) {
    Serial.print(code,HEX);
    Serial.print(" ");
    Serial.write(code);
    Serial.println("");
  }
*/
  return code;
}
/*--------------------------------------------------------------------*/

byte Minitel::smallMode() {
  // Commande
  writeBytesPRO(2);       // 0x1B 0x3A
  writeByte(START);       // 0x69
  writeByte(MINUSCULES);  // 0x45
  // Acquittement
  return workingMode();   // Renvoie un octet
}
/*--------------------------------------------------------------------*/

byte Minitel::capitalMode() {
  // Commande
  writeBytesPRO(2);       // 0x1B 0x3A
  writeByte(STOP);        // 0x6A
  writeByte(MINUSCULES);  // 0x45
  // Acquittement
  return workingMode();   // Renvoie un octet
}
/*--------------------------------------------------------------------*/

byte Minitel::extendedKeyboard() {
  // Commande
  writeBytesPRO(3);                   // 0x1B 0x3B
  writeByte(START);                   // 0x69
  writeByte(CODE_RECEPTION_CLAVIER);  // 0x59
  writeByte(ETEN);                    // 0x41
  // Acquittement
  return workingKeyboard();  // Renvoie un octet
}
/*--------------------------------------------------------------------*/

byte Minitel::standardKeyboard() {
  // Commande
  writeBytesPRO(3);                   // 0x1B 0x3B
  writeByte(STOP);                    // 0x6A
  writeByte(CODE_RECEPTION_CLAVIER);  // 0x59
  writeByte(ETEN);                    // 0x41
  // Acquittement
  return workingKeyboard();  // Renvoie un octet
}
/*--------------------------------------------------------------------*/

byte Minitel::echo(boolean commande) {  // Voir p.81, p.135 et p.156
  // Fonction modifiée par iodeo sur GitHub en octobre 2021
  // commande peut prendre comme valeur :
  // true, false
  return aiguillage(commande, CODE_EMISSION_CLAVIER, CODE_RECEPTION_MODEM);
}
/*--------------------------------------------------------------------*/

byte Minitel::aiguillage(boolean commande, byte emetteur, byte recepteur) {  // Voir p.135
  // commande peut prendre comme valeur :
  // true, false
  // emetteur peut prendre comme valeur :
  // CODE_EMISSION_ECRAN, CODE_EMISSION_CLAVIER, CODE_EMISSION_MODEM, CODE_EMISSION_PRISE
  // recepteur peut prendre comme valeur :
  // CODE_RECEPTION_ECRAN, CODE_RECEPTION_CLAVIER, CODE_RECEPTION_MODEM, CODE_RECEPTION_PRISE
  // Commande
  writeBytesPRO(3);                                     // 0x1B 0x3B
  writeByte(commande ? AIGUILLAGE_ON : AIGUILLAGE_OFF); // 0x61 ou 0x60
  writeByte(recepteur);
  writeByte(emetteur);
  // Acquittement
  return workingAiguillage(recepteur);  // Renvoie un octet
}
/*--------------------------------------------------------------------*/

byte Minitel::statusAiguillage(byte module) {  // Voir p. 136
  // module peut prendre comme valeur :
  // CODE_EMISSION_ECRAN, CODE_EMISSION_CLAVIER, CODE_EMISSION_MODEM, CODE_EMISSION_PRISE
  // CODE_RECEPTION_ECRAN, CODE_RECEPTION_CLAVIER, CODE_RECEPTION_MODEM, CODE_RECEPTION_PRISE
  // Commande
  writeBytesPRO(2);  // 0x1B 0x3A
  writeByte(TO);     // 0x62
  writeByte(module);
  // Acquittement
  return workingAiguillage(module);  // Renvoie un octet
}
/*--------------------------------------------------------------------*/

byte Minitel::connexion(boolean commande) {  // Voir p.139
  // Fonction proposée par iodeo sur GitHub en octobre 2021
  // commande peut prendre comme valeur :
  // true, false
  // Commande
  writeBytesPRO(1);  // 0x1B 0x39
  writeByte(commande ? CONNEXION : DECONNEXION);  // 0x68 ou 0x67
  // Acquittement
  return workingModem();  // Renvoie un octet
}
/*--------------------------------------------------------------------*/

byte Minitel::reset() {  // Voir p.145
  // Commande
  writeBytesPRO(1);  // 0x1B 0x39
  writeByte(RESET);  // 0x7F
  // Acquittement
  return workingStandard(0x135E);  // SEP (0x13), 0x5E
}
/*--------------------------------------------------------------------*/




////////////////////////////////////////////////////////////////////////
/*
   Private
*/
////////////////////////////////////////////////////////////////////////

boolean Minitel::isValidChar(byte index) {
  // On vérifie que le caractère appartient au jeu G0 (voir p.100).
  // SP (0x20) correspond à un espace et DEL (0x7F) à un pavé plein.
  if (index >= SP && index <= DEL) {
    return true;
  }
  return false;
}
/*--------------------------------------------------------------------*/
/*
boolean Minitel::isDiacritic(unsigned char caractere) {  // Obsolète depuis le 26/02/2023
  String diacritics = "àâäèéêëîïôöùûüçÀÂÄÈÉÊËÎÏÔÖÙÛÜÇ";
  if (diacritics.indexOf(caractere) >= 0) {
    return true; 
  }
  return false;
}
*/
/*--------------------------------------------------------------------*/

boolean Minitel::isVisualisable(unsigned long code) {
  // Fonction proposée par iodeo sur GitHub en février 2023
  // Fonction utilisée dans getString(unsigned long code)
  // Teste la conversion d'un code brut clavier en équivalent Unicode
  // Renvoie 0 si le code ne correspond pas à un caractère visualisable
  // Voir https://iodeo.github.io/MinitelKeyboardHelper/
  
  // Les caractères de contrôle ne sont pas visualisables
  if (code < SP) return false;
  // Les autres caractères de 7 bits sont visualisables
  if (code <= DEL) return true;
  switch (code) {
    case 0x00A3: return true; // £
    case 0x00A7: return true; // § (VGP5 seulement, pas VGP2 - voir p.90)
    case 0x00B0: return true; // ° Ctrl 0 au clavier
    case 0x00B1: return true; // ± Ctrl * au clavier
    case 0x00BC: return true; // ¼
    case 0x00BD: return true; // ½
    case 0x00BE: return true; // ¾
    /*  iodeo, je ne comprends pas cette partie car
        aucune lettre accentuée majuscule n'est disponible sur le Minitel - voir page 90

    case 0x00C0: return true; // À (VGP2)
    case 0x00C7: return true; // Ç (VGP2)
    case 0x00C8: return true; // È (VGP2)
    case 0x00C9: return true; // É (VGP2)
    */
    case 0x00E0: return true; // à
    case 0x00E2: return true; // â
    case 0x00E4: return true; // ä (VGP5 seulement, pas VGP2 - voir p.90)
    case 0x00E7: return true; // ç
    case 0x00E8: return true; // è
    case 0x00E9: return true; // é
    case 0x00EA: return true; // ê
    case 0x00EB: return true; // ë
    case 0x00EE: return true; // î
    case 0x00EF: return true; // ï
    case 0x00F4: return true; // ô
    case 0x00F6: return true; // ö (VGP5 seulement, pas VGP2 - voir p.90)
    case 0x00F7: return true; // ÷ Ctrl 7 au clavier
    case 0x00F9: return true; // ù
    case 0x00FB: return true; // û
    case 0x00FC: return true; // ü (VGP5 seulement, pas VGP2 - voir p.90)
    case 0x0152: return true; // Œ
    case 0x0153: return true; // œ
    case 0x03B2: return true; // β (VGP5 seulement, pas VGP2 - voir p.90)
    case 0x2014: return true; // — Ctrl 5 au clavier
    case 0x2190: return true; // ← Ctrl 8 au clavier
    case 0x2191: return true; // ↑
    case 0x2192: return true; // → Ctrl 9 au clavier
    case 0x2193: return true; // ↓ Ctrl # au clavier
  }
  return false; // Les caractères non listés sont supposés non visualisables
}
/*--------------------------------------------------------------------*/

void Minitel::writeBytesP(int n) {
  // Pn, Pr, Pc : Voir remarques p.95 et 96
  if (n<=9) {
    writeByte(0x30 + n);
  }
  else {
    writeByte(0x30 + n/10);
    writeByte(0x30 + n%10);
  }
}
/*--------------------------------------------------------------------*/

void Minitel::writeBytesPRO(int n) {  // Voir p.134
  writeByte(ESC);  // 0x1B
  switch (n) {
    case 1 : writeByte(0x39); break;
    case 2 : writeByte(0x3A); break;
    case 3 : writeByte(0x3B); break;
  }
}
/*--------------------------------------------------------------------*/

unsigned long Minitel::identificationBytes() {  // Voir p.138
  while (!mySerial);  // On attend que le port soit sur écoute.
  unsigned long trame = 0;  // 32 bits = 4 octets
  while (trame >> 24 != 0x01) {  // La trame doit débuter par SOH (0x01)
    if (mySerial.available() > 0) {
      trame = (trame << 8) + readByte();
    }
  }
  while (!mySerial.available()>0); // Indispensable
  if (readByte() != 0x04) return 0;  // La trame doit se terminer par EOT (0x04)
  trame = (trame << 8) >> 8;  // On élimine l'octet SOH (0x01) de la trame
  return trame;  // 3 octets
                 // octet définissant le constructeur du Minitel
                 // octet définissant le type du Minitel
                 // octet définissant la version du logiciel
}
/*--------------------------------------------------------------------*/

int Minitel::workingSpeed() {
  int bauds = -1;
  while (!mySerial);  // On attend que le port soit sur écoute.
  unsigned long time = millis();
  unsigned long duree = 0;
  unsigned long trame = 0;  // 32 bits = 4 octets
  // On se donne 1000 ms pour récupérer une trame exploitable
  while ((trame >> 8 != 0x1B3A75) && (duree < 1000)) {  // Voir p.141
    if (mySerial.available() > 0) {
      trame = (trame << 8) + readByte();
      //Serial.println(trame, HEX);
    }
    duree = millis() - time;
  }
  switch (trame) {
    case 0x1B3A7552 : bauds =  300; break;
    case 0x1B3A7564 : bauds = 1200; break;
    case 0x1B3A7576 : bauds = 4800; break;
    case 0x1B3A757F : bauds = 9600; break;  // Pour le Minitel 2 seulement
  }
  return bauds;
}
/*--------------------------------------------------------------------*/

byte Minitel::workingStandard(unsigned long sequence) {
  // Fonction modifiée par iodeo sur GitHub en octobre 2021
  while (!mySerial);  // On attend que le port soit sur écoute.
  unsigned long time = millis();
  unsigned long duree = 0;
  unsigned long trame = 0;  // 32 bits = 4 octets  
  // On se donne 100ms pour recevoir l'acquittement
  // Sinon, on peut supposer que le mode demandé était déjà actif
  while ((trame != sequence) && (duree < 100)) {
    if (mySerial.available() > 0) {
      trame = (trame << 8) + readByte();
      //Serial.println(trame, HEX);
    }
    duree = millis() - time;
  }
  return (trame == sequence)? 1 : 0;
}
/*--------------------------------------------------------------------*/

byte Minitel::workingMode() {  // Voir p.143
  // On récupère notamment les 4 bits de poids faibles suivants : ME PC RL F
  // ME : mode minuscules / majuscules du clavier (1 = minuscule)
  // PC : PCE (1 = actif)
  // RL : rouleau (1 = actif)
  // F  : format d'écran (1 = 80 colonnes)
  while (!mySerial);  // On attend que le port soit sur écoute.
  unsigned long trame = 0;  // 32 bits = 4 octets  
  while (trame >> 8 != 0x1B3A73) {  // PRO2 (0x1B,0x3A), REP_STATUS_FONCTIONNEMENT (0x73)
    if (mySerial.available() > 0) {
      trame = (trame << 8) + readByte();
      //Serial.println(trame, HEX);
    }
  }
  return (byte) trame;
}
/*--------------------------------------------------------------------*/

byte Minitel::workingKeyboard() {  // Voir p.142
  // On récupère notamment les 3 bits de poids faibles suivants : C0 0 Eten
  // Eten : mode étendu (1 = actif)
  // C0   : codage en jeu C0 des touches de gestion du curseur (1 = actif)
  while (!mySerial);  // On attend que le port soit sur écoute.
  unsigned long trame = 0;  // 32 bits = 4 octets  
  while (trame != 0x1B3B7359) {  // PRO3 (0x1B,0x3B), REP_STATUS_CLAVIER (0x73), CODE_RECEPTION_CLAVIER (0x59)
    if (mySerial.available() > 0) {
      trame = (trame << 8) + readByte();
      //Serial.println(trame, HEX);
    }
  }
  while (!mySerial.available()>0);  // Indispensable
  return readByte(); // Octet de statut fonctionnement clavier
}
/*--------------------------------------------------------------------*/

byte Minitel::workingAiguillage(byte module) {  // Voir p.136
  // On récupère l'octet de statut d'aiguillage associé à un module :
  // b7 : bit de parité
  // b6 : 1
  // b5 : 0
  // b4 : 0
  // b3 : prise
  // b2 : modem             1 : liaison établie
  // b1 : clavier           0 : liaison coupée
  // b0 : écran
  // L'octet de statut contient également l'état de la ressource que constitue le module lui-même (0 : module bloqué ; 1 : module actif)
  while (!mySerial);  // On attend que le port soit sur écoute.
  unsigned long trame = 0;  // 32 bits = 4 octets
  while (trame != (0x1B3B63 << 8 | module)) {  // PRO3 (0x1B,0x3B), FROM (0x63), code réception ou émission du module
    if (mySerial.available() > 0) {
      trame = (trame << 8) + readByte();
      //Serial.println(trame, HEX);
    }
  }
  while (!mySerial.available()>0);  // Indispensable
  return readByte(); // Octet de statut associé à un module
}
/*--------------------------------------------------------------------*/

byte Minitel::workingModem() {  // Voir p.126
  // Fonction proposée par iodeo sur GitHub en octobre 2021
  // On récupère uniquement la séquence immédiate 0x1359
  // en cas de connexion confirmé, la séquence 0x1353 s'ajoutera - non traité ici
  // en cas de timeout (environ 40sec), la séquence 0x1359 s'ajoutera - non traité ici
  while (!mySerial);  // On attend que le port soit sur écoute.
  unsigned int trame = 0;  // 16 bits = 2 octets
  while (trame >> 8 != 0x13) {
    if (mySerial.available() > 0) {
      trame = (trame << 8) + readByte();
      //Serial.println(trame, HEX);
    }
  }
  return (byte) trame;
}
/*--------------------------------------------------------------------*/

unsigned long Minitel::getCursorXY() {  // Voir p.98
  // Demande
  writeByte(ESC);
  writeByte(0x61);
  // Réponse
  while (!mySerial);  // On attend que le port soit sur écoute.
  unsigned long trame = 0;  // 32 bits = 4 octets  
  while (trame >> 16 != US) {
    if (mySerial.available() > 0) {
      trame = (trame << 8) + readByte();
    }
  }
  return trame;
}
/*--------------------------------------------------------------------*/
