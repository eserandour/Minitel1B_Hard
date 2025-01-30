////////////////////////////////////////////////////////////////////////
/*
   TestCaracteresSpeciaux - Version du 17 mars 2023 à 14h24
   Copyright 2023 - Eric Sérandour
   https://entropie.org/3615/
   
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
   
////////////////////////////////////////////////////////////////////////
  DEBUT DU PROGRAMME (Compile sous Arduino 1.8.19)
*///////////////////////////////////////////////////////////////////////

#include <Minitel1B_Hard.h>  // Voir https://github.com/eserandour/Minitel1B_Hard

#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)  // Pour ESP32
// Le troisième port série matériel de l'ESP32 (Serial2 / U2RXD U2TXD)
// est utilisé pour la connexion avec le Minitel.
Minitel minitel(Serial2, 16, 17);
#else  // Pour ATmega 1284P ou ATmega 2560 notamment
// Le deuxième port série matériel de l'ATMega (Serial1 / RXD1 TXD1)
// est utilisé pour la connexion avec le Minitel.
Minitel minitel(Serial1);
#endif

////////////////////////////////////////////////////////////////////////

#define TITRE "TEST CARACTÈRES SPÉCIAUX"

String texte="";
int nbCaracteres=0;
const int PREMIERE_LIGNE_EXPRESSION = 7;
const int NB_LIGNES_EXPRESSION = 15;
const String VIDE = ".";

unsigned long touche;

////////////////////////////////////////////////////////////////////////

void setup() {  
  // Le premier port série matériel de l'ATMega (Serial / RXD0 TXD0)
  // ou de l'ESP32 (Serial / U0RXD U0TXD) est utilisé pour la connexion
  // avec le PC.
  Serial.begin(9600);

  // Pour le cas où on utilise le shield 3615 avec une Arduino Mega 2560,
  // on relie avec un fil les broches 8 et 19 d'un côté et 9 et 18 de l'autre.
  // Les 2 broches 8 et 9 du shield 3615 sont dès lors respectivement reroutées
  // vers les broches 19 (RX1) et 18 (TX1).
  // Afin que les broches 8 et 9 de la carte Arduino ne perturbent pas
  // ce reroutage, on les définit comme des entrées.
  // pinMode(8,INPUT);
  // pinMode(9,INPUT);

  // A la mise sous tension du Minitel, la vitesse des échanges entre
  // le Minitel et le périphérique est de 1200 bauds par défaut.
  // On envisage cependant le cas où le Minitel se trouve dans un autre état.
  minitel.changeSpeed(minitel.searchSpeed());
  minitel.smallMode();
  
  // Affichage de la page
  newPage(TITRE);  

  texte = "àâäèéêëîïôöùûüçÀÂÄÈÉÊËÎÏÔÖÙÛÜÇ";
  minitel.println(texte);
  Serial.println();
  Serial.println(texte);
  texte = "£§°±¼½¾β÷Œœ";
  minitel.println(texte);
  Serial.println(texte);
  texte = "—←↑→↓";
  minitel.println(texte);
  Serial.println(texte);
  Serial.println();
}

////////////////////////////////////////////////////////////////////////

void loop() {
  // Affichage de la page
  //newPage(TITRE);
  
  // Lecture du champ expression
  lectureChamp(PREMIERE_LIGNE_EXPRESSION, NB_LIGNES_EXPRESSION);
  Serial.println(texte);
  Serial.println();
}

////////////////////////////////////////////////////////////////////////

void newPage(String titre) {
  minitel.newScreen();
  minitel.println(titre);
  for (int i=1; i<=40; i++) {
    minitel.writeByte(0x7E);
  }
}

////////////////////////////////////////////////////////////////////////

void champVide(int premiereLigne, int nbLignes)
{
  minitel.noCursor();
  minitel.moveCursorXY(1,premiereLigne);
  minitel.clearScreenFromCursor();
  for (int j=0; j<nbLignes; j++) {
    minitel.attributs(CARACTERE_BLEU);
    minitel.print(VIDE);
    minitel.repeat(39);
    minitel.attributs(CARACTERE_BLANC);
  }
  texte="";
  nbCaracteres=0;
  minitel.moveCursorXY(31,24);
  minitel.print("puis ");
  minitel.attributs(INVERSION_FOND);
  minitel.print("ENVOI");
  minitel.attributs(FOND_NORMAL);
  minitel.moveCursorDown(premiereLigne-1);
  minitel.cursor();
}

////////////////////////////////////////////////////////////////////////

void correction(int nbLignes) {
  if ((nbCaracteres > 0) && (nbCaracteres <= 40*nbLignes)) {
    if (nbCaracteres != 40*nbLignes) { minitel.moveCursorLeft(1); }
    minitel.attributs(CARACTERE_BLEU);
    minitel.print(VIDE);
    minitel.attributs(CARACTERE_BLANC);
    minitel.moveCursorLeft(1);
    unsigned int index = texte.length()-1;

    // Pour test
    Serial.println(texte.charAt(index), HEX);
    Serial.println(texte.charAt(index) >> 8, HEX);
    Serial.println(texte.charAt(index) >> 7, HEX);
    Serial.println(texte.charAt(index) >> 6, HEX);
    
    Serial.println(texte.charAt(index-1), HEX);
    Serial.println(texte.charAt(index-1) >> 8, HEX);
    Serial.println(texte.charAt(index-1) >> 7, HEX);
    Serial.println(texte.charAt(index-1) >> 6, HEX);
    
    Serial.println(texte.charAt(index-2), HEX);
    Serial.println(texte.charAt(index-2) >> 8, HEX);
    Serial.println(texte.charAt(index-2) >> 7, HEX);
    Serial.println(texte.charAt(index-2) >> 6, HEX);
    //

    #if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)  // Pour ESP32
    if (texte.charAt(index) >> 7 == 0x01) {  // Caractère spécial
      index--;
      // Les caractères spéciaux codés sur 3 octets commencent par 0xE2
      if (texte.charAt(index) >> 7 == 0x01 && texte.charAt(index-1) == 0xE2) index--;
    }
    #else  // Pour ATmega 1284P
    if (texte.charAt(index) >> 8 == 0xFFFFFFFF) {  // Caractère spécial
      index--;
      // Les caractères spéciaux codés sur 3 octets commencent par 0xE2
      if (texte.charAt(index) >> 8 == 0xFFFFFFFF && texte.charAt(index-1) == 0xFFFFFFE2) index--;
    }
    #endif
    texte.remove(index);
    nbCaracteres--;
    // Affichage des informations dans la console
    Serial.print("texte = "); Serial.println(texte);
    Serial.print("nbCaracteres = "); Serial.println(nbCaracteres);
    Serial.println();
  }
}

////////////////////////////////////////////////////////////////////////

void lectureChamp(int premiereLigne, int nbLignes) {
  champVide(premiereLigne,nbLignes);
  boolean fin = false;
  while (!fin) {
    touche = minitel.getKeyCode();
    if ((touche != 0) &&
        (touche != CONNEXION_FIN) &&
        (touche != SOMMAIRE) &&
        (touche != ANNULATION) &&
        (touche != RETOUR) &&
        (touche != REPETITION) &&
        (touche != GUIDE) &&
        (touche != CORRECTION) &&
        (touche != SUITE) &&
        (touche != ENVOI) &&
        (touche != CR)) {
      if (nbCaracteres < 40*nbLignes) {
        nbCaracteres++;
        texte += minitel.getString(touche);
        
        // Affichage des informations dans la console
        Serial.print("Caractère : "); Serial.println(minitel.getString(touche));
        Serial.print(touche, HEX); Serial.println(" (Unicode)");
        unsigned long index = texte.length()-1;
        int nbBytes = minitel.getNbBytes(touche);
        for (int i=nbBytes; i>0; i--) {
          Serial.print((byte) texte.charAt(index-i+1), HEX);
        }
        Serial.print(" (UTF-8) - Nb d'octets : "); Serial.println(nbBytes);
        Serial.print("texte = "); Serial.println(texte);
        Serial.print("nbCaracteres = "); Serial.println(nbCaracteres);
        Serial.println();
      }
      if (nbCaracteres == 40*nbLignes) {
        minitel.moveCursorXY(40,(premiereLigne-1)+nbLignes);
      }
    }
    switch (touche) {
      case ENVOI : 
        fin = true;
        break;
      case ANNULATION :
        champVide(premiereLigne,nbLignes);
        break;
      case CORRECTION :
        correction(nbLignes);
        break;
    }    
  }
}

////////////////////////////////////////////////////////////////////////





/*
// Version du 17 mars 2023 à 14h24
// Alternative au programme ci-dessus (avec cache)
// Version moins performante (si on tape trop vite, l'Arduino ne suit pas).
#include <Minitel1B_Hard.h>  // Voir https://github.com/eserandour/Minitel1B_Hard
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)  // Pour ESP32
// Le troisième port série matériel de l'ESP32 (Serial2 / U2RXD U2TXD)
// est utilisé pour la connexion avec le Minitel.
Minitel minitel(Serial2);
#else  // Pour ATmega 1284P ou ATmega 2560 notamment
// Le deuxième port série matériel de l'ATMega (Serial1 / RXD1 TXD1)
// est utilisé pour la connexion avec le Minitel.
Minitel minitel(Serial1);
#endif
#define TITRE "TEST CARACTÈRES SPÉCIAUX"
String texte="";
int nbCaracteres=0;
const int PREMIERE_LIGNE_EXPRESSION = 7;
const int NB_LIGNES_EXPRESSION = 15;
const String VIDE = ".";
unsigned long touche;
const int TAILLE_CACHE = 20;  // Peut être augmenté si besoin
int cache[TAILLE_CACHE] = {0};  // Utilisé pour enregistrer le nombre d'octets des caractères spéciaux
////////////////////////////////////////////////////////////////////////
void setup() {  
  // Le premier port série matériel de l'ATMega (Serial / RXD0 TXD0)
  // ou de l'ESP32 (Serial / U0RXD U0TXD) est utilisé pour la connexion
  // avec le PC.
  Serial.begin(9600);

  // Pour le cas où on utilise le shield 3615 avec une Arduino Mega 2560,
  // on relie avec un fil les broches 8 et 19 d'un côté et 9 et 18 de l'autre.
  // Les 2 broches 8 et 9 du shield 3615 sont dès lors respectivement reroutées
  // vers les broches 19 (RX1) et 18 (TX1).
  // Afin que les broches 8 et 9 de la carte Arduino ne perturbent pas
  // ce reroutage, on les définit comme des entrées.
  // pinMode(8,INPUT);
  // pinMode(9,INPUT);

  // A la mise sous tension du Minitel, la vitesse des échanges entre
  // le Minitel et le périphérique est de 1200 bauds par défaut.
  // On envisage cependant le cas où le Minitel se trouve dans un autre état.
  minitel.changeSpeed(minitel.searchSpeed());
  minitel.smallMode();
  
  // Affichage de la page
  newPage(TITRE);  
  texte = "àâäèéêëîïôöùûüçÀÂÄÈÉÊËÎÏÔÖÙÛÜÇ";
  minitel.println(texte);
  Serial.println(texte);
  texte = "£§°±¼½¾β÷Œœ";
  minitel.println(texte);
  Serial.println(texte);
  texte = "—←↑→↓";
  minitel.println(texte);
  Serial.println(texte);
  Serial.println();
}
////////////////////////////////////////////////////////////////////////
void loop() {
  // Affichage de la page
  //newPage(TITRE);
  
  // Lecture du champ expression
  lectureChamp(PREMIERE_LIGNE_EXPRESSION, NB_LIGNES_EXPRESSION);
  Serial.println(texte);
}
////////////////////////////////////////////////////////////////////////
void newPage(String titre) {
  minitel.newScreen();
  minitel.println(titre);
  for (int i=1; i<=40; i++) {
    minitel.writeByte(0x7E);
  }
}
////////////////////////////////////////////////////////////////////////
void champVide(int premiereLigne, int nbLignes)
{
  minitel.noCursor();
  minitel.moveCursorXY(1,premiereLigne);
  minitel.clearScreenFromCursor();
  for (int j=0; j<nbLignes; j++) {
    minitel.attributs(CARACTERE_BLEU);
    minitel.print(VIDE);
    minitel.repeat(39);
    minitel.attributs(CARACTERE_BLANC);
  }
  texte="";
  nbCaracteres=0;
  for (int i=0; i<TAILLE_CACHE; i++) {
    cache[i] = 0;
  }
  minitel.moveCursorXY(31,24);
  minitel.print("puis ");
  minitel.attributs(INVERSION_FOND);
  minitel.print("ENVOI");
  minitel.attributs(FOND_NORMAL);
  minitel.moveCursorDown(premiereLigne-1);
  minitel.cursor();
}
////////////////////////////////////////////////////////////////////////
void correction(int nbLignes) {
  boolean texteCorrige = false;
  if ((nbCaracteres > 0) && (nbCaracteres <= 40*nbLignes)) {
    unsigned int index = texte.length()-1;
    
    #if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)  // Pour ESP32
    if (texte.charAt(index) >> 7 == 0x01) {  // Caractère spécial
    #else  // Pour ATmega 1284P
    if (texte.charAt(index) >> 8 == 0xFFFFFFFF) {  // Caractère spécial
    #endif
      if (cache[0] != 0) {
        texte = texte.substring(0,texte.length()-cache[0]);
        texteCorrige = true;
        for (int i=0; i<TAILLE_CACHE-1; i++) {
          cache[i]=cache[i+1];
        }
        cache[TAILLE_CACHE-1] = 0;
      }
    }
    else {
      texte = texte.substring(0,texte.length()-1);
      texteCorrige = true;
    }
    if (texteCorrige) {
      nbCaracteres--;
      if (nbCaracteres != 40*nbLignes) { minitel.moveCursorLeft(1); }
      minitel.attributs(CARACTERE_BLEU);
      minitel.print(VIDE);
      minitel.attributs(CARACTERE_BLANC);
      minitel.moveCursorLeft(1);
    }
    // Affichage des informations dans la console
    Serial.print("texte = "); Serial.println(texte);
    Serial.print("nbCaracteres = "); Serial.println(nbCaracteres);
    Serial.print("Cache["); Serial.print(TAILLE_CACHE); Serial.print("]={");
    for (int i=0; i<TAILLE_CACHE; i++) {
      Serial.print(cache[i]);
      if (i<TAILLE_CACHE-1) Serial.print(";");
    }
    Serial.println("} - Nombre d'octets des caractères spéciaux");
    Serial.println();
  }
}
////////////////////////////////////////////////////////////////////////
void lectureChamp(int premiereLigne, int nbLignes) {
  champVide(premiereLigne,nbLignes);
  boolean fin = false;
  while (!fin) {
    touche = minitel.getKeyCode();
    if ((touche != 0) &&
        (touche != CONNEXION_FIN) &&
        (touche != SOMMAIRE) &&
        (touche != ANNULATION) &&
        (touche != RETOUR) &&
        (touche != REPETITION) &&
        (touche != GUIDE) &&
        (touche != CORRECTION) &&
        (touche != SUITE) &&
        (touche != ENVOI) &&
        (touche != CR)) {
      if (nbCaracteres < 40*nbLignes) {
        nbCaracteres++;
        texte += minitel.getString(touche);
        int nbBytes = minitel.getNbBytes (touche);
        if (nbBytes > 1) {  // Caractère spécial
          for (int i=TAILLE_CACHE-1; i>0; i--) {
            cache[i]=cache[i-1];
          }
          cache[0] = nbBytes;
        }
        // Affichage des informations dans la console
        Serial.print("Caractère : "); Serial.println(minitel.getString(touche));
        Serial.print(touche, HEX); Serial.println(" (Unicode)");
        unsigned long index = texte.length()-1;
        for (int i=nbBytes; i>0; i--) {
          Serial.print((byte) texte.charAt(index-i+1), HEX);
        }
        Serial.print(" (UTF-8) - Nb d'octets : "); Serial.println(nbBytes);
        Serial.print("texte = "); Serial.println(texte);
        Serial.print("nbCaracteres = "); Serial.println(nbCaracteres);
        Serial.print("Cache["); Serial.print(TAILLE_CACHE); Serial.print("]={");
        for (int i=0; i<TAILLE_CACHE; i++) {
          Serial.print(cache[i]);
          if (i<TAILLE_CACHE-1) Serial.print(";");
        }
        Serial.println("} - Nombre d'octets des caractères spéciaux");
        Serial.println();
      }
      if (nbCaracteres == 40*nbLignes) {
        minitel.moveCursorXY(40,(premiereLigne-1)+nbLignes);
      }
    }
    switch (touche) {
      case ENVOI : 
        fin = true;
        break;
      case ANNULATION :
        champVide(premiereLigne,nbLignes);
        break;
      case CORRECTION :
        correction(nbLignes);
        break;
    }    
  }
}
////////////////////////////////////////////////////////////////////////
*/
