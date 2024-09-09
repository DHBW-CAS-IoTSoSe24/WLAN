# WLAN
**Contributor**: Florian Brändle, Maximilian Girnt, René Kienzle, Marc Schedel

## Installation und Einrichtung der Entwicklungsumgebung für den ESP32

Zur Bearbeitung des Programmcodes sowie zum Flashen des ESP32 wurde die Arduino IDE verwendet.
Um den ESP32 erfolgreich zu flashen, müssen zusätzlich die Dev-Module von Espressif installiert werden.

### Schritt 1: Installation der Arduino IDE

Die Arduino IDE kann gemäß den Anweisungen im offiziellen
[Installationsleitfaden der Arduino-Website](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE)
heruntergeladen und installiert werden.

### Schritt 2: Installation der Espressif-Module

Für das Flashen des ESP32-C6 müssen die entsprechenden Espressif-Module hinzugefügt werden.
Eine detaillierte Anleitung zur Installation dieser Module ist im
[SparkFun-Guide](https://docs.sparkfun.com/SparkFun_Thing_Plus_ESP32_C6/software_setup/) zu finden.

### Probleme mit dem ESP32-C6 Modul

Das ESP32-C6 Modul bereitete von Beginn an Probleme. Die Entwicklung sollte zunächst in PlatformIO durchgeführt werden, in dem der ESP32-C6 jedoch nur mit dem Espressif eigenen Framework ESP-IDF unterstützt wird. Arduino kann hier aktuell (Stand 2024-09-07) nicht ausgewählt werden. Nach einigen zusätzlichen, leider nicht offensichtlichen, Projektkonfigurationen wie dem Angeben von platform, platform-packages und eigener board config json konnte schließlich das Arduino Framework für den C6 verwendet und Code kompiliert sowie aufgespielt werden.

Ein weiteres Problem ergab sich bei der Verwendung des DHT11 Sensors mit dem ESP32-C6. Der vorhandene Code, der auf einem älteren ESP32 WROOM32 Modul ohne Probleme lauffähig war, wurde auf dem C6 nicht ausgeführt. Eventuell liegen hier noch Kompatibilitätsprobleme vor, die im Rahmen der Zeit jedoch nicht näher untersucht werden konnten. Als WLAN-Sensoren kommen somit aktuell einfach die ESP32-WROOM32 Module zum Einsatz.

## Captive Portal

Der WLAN-Sensor kann über ein Captive Portal komfortabel eingerichtet und konfiguriert werden. Dazu muss sich ein WLAN-fähiges Endgerät (wie beispielsweise ein Smartphone) mit dem vom Sensor ausgestrahlten WLAN verbinden. Die ausgestrahlte SSID des Sensors lautet "ESP32-Sensor", das Passwort "esp32-sensor". Hat sich das Endgerät verbunden, so sollte automatisch das Captive Portal zur Einrichtung geöffnet werden. Sollte sich dieses nicht öffnen, kann auf das Captive Portal zugegriffen werden, indem im Browser eine beliebige HTTP-Domain mittels aufgerufen wird.

Eingerichtet werden die SSID und das Passwort des WLANs, über das der Sensor Zugriff auf ein Netzwerk bekommt, über das das MQTT-Gateway erreicht werden kann. Die Zugangsdaten für den MQTT-Server können ebenfalls eingegeben werden.

Durch Klicken auf den Button "Continue" oder "Speichern" werden die Einstellungen an den ESP32 übermittelt und auf diesem persistent gespeichert. Gespeicherte Konfigurationsparameter können zu jeder Zeit über das Captive Portal geändert werden. Dabei reicht das Eingeben des neuen Wertes des Konfigurationsparameters, der geändert werden soll - die anderen Felder können leer gelassen werden.

## RGB Status LED

Die an den ESP32 angeschlossene Status LED zeigt den aktuellen Status des Sensors an. Leuchtet die LED rot, ist der Sensor mit Strom versorgt. Das WLAN ist in diesem Zustand nicht verbunden. Konnte erfolgreich eine Verbindung mit dem konfigurierten WLAN hergestellt werden, leuchtet die LED blau. Besteht zusätzlich eine Verbindung zum MQTT Server, leuchtet die LED grün.

## Temperatur- und Luftfeuchtigkeitssensor

Als Sensor für die Messung von Temperatur und Luftfeuchtigkeit kommt der DHT11 zum Einsatz. Dieser misst im Luftfeuchtigkeitsbereich von ca. 20-80 % (5 % Genauigkeit) und im Temperaturbereich von ca. 0-50 °C (ca. 2 °C Genauigkeit) und kann mit der DHT-sensor-library von Adafruit ausgelesen werden. Angeschlossen wird er an 5V, GND und an GPIO32 des Arduino.

## MQTT

Die MQTT Funktionalität wird mit dem pubsubclient von knolleary bereitgestellt. Die Verbindung zum Server wird nach Eingabe der Zugangsdaten und allen anschließenden Bootvorgängen automatisch hergestellt, wenn dieser über das WLAN erreicht werden kann. Der Sensor sendet alle 10 Sekunden die aktuellen Temperatur- und Luftfeuchtigkeitswerte, sowie Events von der Lichtschranke in Echtzeit.

## Lichtschranken-Sensor

Der Lichtschranken-Sensor dient zur Erkennung von Bewegungen und zur Bestimmung der Bewegungsrichtung,
d. h. ob ein Objekt den Raum betritt oder verlässt.
Für die Implementierung des Sensors wurden Infrarot-Transmitter und -Receiver verwendet.
Diese Lösung bietet eine kostengünstige und einfache Methode zur Bewegungserkennung,
kann jedoch keine 100%ige Zuverlässigkeit garantieren.

### Einschränkungen

Messfehler können auftreten, wenn:
- Mehrere Objekte gleichzeitig oder überlappend den Sensorbereich durchqueren.
- Objekte sich zu nah aneinander befinden.
- Der Sensor in einer ungeeigneten Höhe angebracht wird, was dazu führen kann,
dass z. B. die Beine eines Tieres erfasst werden oder ein zu kleines Tier nicht erkannt wird.

### Verbesserungspotenzial

Für eine präzisere Erkennung wäre der Einsatz eines Lichtlasers mit passendem Empfänger zu empfehlen,
da dadurch die Lichtstreuung des Infrarot-Senders verringert wird.
Im Rahmen dieses Projekts standen jedoch keine entsprechenden Sensoren zur Verfügung,
weshalb auf die Infrarot-Lösung zurückgegriffen wurde.

### Aufbau des Sensors

Der gebaute Sensor besteht aus zwei Infrarot-Transmittern und zwei Infrarot-Receivern.
Die IR-Transmitter sind dauerhaft aktiviert, um eine Lichtschranke zu bilden.

**Pin-Belegung der IR-Transmitter:**
- Pin (-) ist mit GND des ESP32-Boards verbunden.
- Pin (+) ist mit 5V des ESP32-Boards verbunden.
- Pin (S) ist mit 3.3V des ESP32-Boards verbunden, um die Dioden dauerhaft einzuschalten.
Eine Verbindung mit 5V führte nicht zum gewünschten Ergebnis, weshalb 3.3V gewählt wurden.

**Pin-Belegung der IR-Receiver:**
- Pin (-) ist mit GND des ESP32-Boards verbunden.
- Pin (+) ist jeweils mit einem eigenen GPIO-Pin verbunden, um das Signal zu erfassen,
wenn die Lichtschranke unterbrochen wird. In diesem Fall wurden die GPIO-Pins 12 und 13 verwendet.
