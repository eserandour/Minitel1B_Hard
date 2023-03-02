// Version du 28 février 2023 à 21h55

#include <Minitel1B_Hard.h>  // Voir https://github.com/eserandour/Minitel1B_Hard

// Le deuxième port série matériel de l'ATMega 1284P (Serial1 / RXD1 TXD1)
// est utilisé pour la connexion avec le Minitel.
Minitel minitel(Serial1);

#define TITRE "TEST CARACTÈRES SPÉCIAUX"

String texte="";
int nbCaracteres=0;
const int PREMIERE_LIGNE_EXPRESSION = 7;
const int NB_LIGNES_EXPRESSION = 15;
const String VIDE = ".";

unsigned long touche;

///////////////////////////////////////////////////////////////////////////////////

void setup() {  
  // Le premier port série matériel de l'ATMega 1284P (Serial / RXD0 TXD0)
  // est utilisé pour la connexion avec le PC.
  Serial.begin(9600);  

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
}

///////////////////////////////////////////////////////////////////////////////////

void loop() {
  // Affichage de la page
  //newPage(TITRE);
  
  // Lecture du champ expression
  lectureChamp(PREMIERE_LIGNE_EXPRESSION, NB_LIGNES_EXPRESSION);
  Serial.println(texte);
}

///////////////////////////////////////////////////////////////////////////////////

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
    if (texte.charAt(index) >> 7) { // caractère spécial
      while ((texte.charAt(index) >> 6) != 0b11) index--;
    }
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
        (touche != ENVOI)) {
      if (nbCaracteres < 40*nbLignes) {
        nbCaracteres++;
        // texte += char(touche);
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

///////////////////////////////////////////////////////////////////////////////////

