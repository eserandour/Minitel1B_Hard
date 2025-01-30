#include <Minitel1B_Hard.h>  // Voir https://github.com/eserandour/Minitel1B_Hard
// On n'utilise pas la bibliothèque SoftwareSerial.

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
  
  minitel.newScreen();
}

////////////////////////////////////////////////////////////////////////

void loop() {
  minitel.print("Hello World ! ");
}

////////////////////////////////////////////////////////////////////////
