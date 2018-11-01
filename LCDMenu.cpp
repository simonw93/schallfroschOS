#include "LCDMenu.h"
#include <LiquidCrystal_I2C.h>

/**
Menüstruktur
• Lautstärke
  ° Lautstärke ändern
• Modus
  ° Eco (an/aus)
• Geräte
  ° 12V an/aus
  ° USB-Lader an/aus
  ° Soundboard an/aus
• Akku
  ° Statistiken anzeigen?
  ° Prozentanzeige, Spannung
  ° Restlaufzeit
  ° Energiesparmodus aktivieren
• Signalquellen
  ° Bluetooth
  ° AUX
  ° Radio
  ° Raspberry Pi
• Radio
  ° Frequenz anzeigen
  ° Frequenz ändern
  ° Senderliste
• Beleuchtung
  ° Ein-/ Ausschalten
  ° Modi
    * Automatisch
    * Sound2Light
    * Fade (Farbübergänge durchgehend)
    * Statisch
    * Programme (vordefinierte Abfolgen)
    * Helligkeit
• Schließfach
  ° Öffnen (braucht Authorisierung)
• Alarm
  ° Bewegungsalarm Aktivieren (braucht Authorisierung)
  ° Standortalarm aktivieren (braucht Authorisierung)
  ° Deaktivieren (braucht Authorisierung)
  ° Einstellungen
    * Alarmlautstärke
• DSP
  • Audioprofile
    ° Indoor
    ° Outdoor
    ° Subwoofer aktiv (wenn Subs angeschlossen)
    ° MaxSPL
• WiFi
  • Ein-/Ausschalten
  • verbundene Geräte anzeigen?
  • Verbindungsinformationen anzeigen
    ° erfordert Authorisierung
• Einschränkungen
  ° Maximallautstärke
  ° USB-Lader
  ° Minimaler Ladestand
• Debug
  ° Sensoren
    * Temperaturen
    * Accelerometer
    * Ladestrom
    * Laufzeit
  ° Version
• Administration
  ° Token hinzufügen (Authorisierung)
  ° Token entfernen
• Sperren

*/

LCDMenu::LCDMenu(){

}

void LCDMenu::init(){

}
