////////////////////////////////////////////////////////////////////////
/*
   3615 Tweet - Version du 17 mars 2023 à 14h55
   Copyright 2017-2023 - Eric Sérandour
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
#elif defined(ARDUINO_ARCH_RP2040)
// On RP2040, only some pins can be used. If wrong pins are passed, the UART will start with default pins.
Minitel minitel(Serial1, 13, 12);
#else  // Pour ATmega 1284P ou ATmega 2560 notamment
// Le deuxième port série matériel de l'ATMega (Serial1 / RXD1 TXD1)
// est utilisé pour la connexion avec le Minitel.
Minitel minitel(Serial1);
#endif

////////////////////////////////////////////////////////////////////////

#define TITRE "3615 TWEET"

String texte="";
int nbCaracteres=0;
const int PREMIERE_LIGNE_EXPRESSION = 4;
const int NB_LIGNES_EXPRESSION = 7;
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

  minitel.changeSpeed(minitel.searchSpeed());
  minitel.smallMode();
}

////////////////////////////////////////////////////////////////////////

void loop() {
  
  // Affichage de la page
  newPage(TITRE);

  // Lecture du champ expression
  lectureChamp(PREMIERE_LIGNE_EXPRESSION, NB_LIGNES_EXPRESSION);
  Serial.println(texte);
  
  // Message de fin
  minitel.clearScreenFromCursor();
  minitel.noCursor();
  minitel.moveCursorXY(14,24);
  minitel.print("C'est envoyé !");
  delay(2000);
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
