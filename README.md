# cantate
WLAN-Fernsteuerung für den (nicht mehr verfügbaren) Cantate-Liedanzeiger, der z.B. in der 
Stadtpfarrkirche Aichach verbaut ist.

Die Fernsteuerung basiert auf einem ESP8266-Modul (ESP-01), das über eine Lochrasterplatine 
(vgl. Liedanzeiger Schaltplan.pdf) mit einem ADO4-Stecker anstelle eines klassischen Tastenfelds
zur Eingabe der Liednummern angeschossen wird.

Das ESP-01 öffnet ein verschlüsseltes WLAN mit Captive-Portal, das auf der Captive-Seite die 
Eingabe von Nummern für den Liedanzeiger erlaubt. Der entsprechende Quellcode findet sich in 
Liedanzeiger.ino und kann mit der Arduino-IDE (mit passenden Board-Package) und einem USB-to-
UART-Converter installiert werden.