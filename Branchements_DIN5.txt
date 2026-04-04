Branchements :

Dos du Minitel             Carte Arduino

DIN 1 : RX    <--------->  TX
DIN 4 :       <---/ /--->
DIN 2 : GND   <--------->  GND
DIN 5 : +8,5V <---/ /--->
DIN 3 : TX    <----+---->  RX
                   |
                   R
                   |
                  +5V

La sortie TX au niveau du Minitel est à collecteur ouvert : 
elle nécessite donc une résistance R de tirage (pull-up). J'utilise R = 10 kilohms
(une valeur comprise entre les 2,3 et 200 kilohms indiqués sur la documentation technique p.69).
Je récupère le +5V par l'intermédiaire de la broche DIN 5 (+8,5V) sur laquelle je branche un régulateur 5V.
