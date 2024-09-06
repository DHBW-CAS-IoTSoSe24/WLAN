# WLAN

## Installation und Einrichtung der Entwicklungsumgebung für den ESP32-C6

Zur Bearbeitung des Programmcodes sowie zum Flashen des ESP32-C6 wurde die Arduino IDE verwendet.
Um den ESP32-C6 erfolgreich zu flashen, müssen zusätzlich die Dev-Module von Espressif installiert werden.

### Schritt 1: Installation der Arduino IDE

Die Arduino IDE kann gemäß den Anweisungen im offiziellen
[Installationsleitfaden der Arduino-Website](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE)
heruntergeladen und installiert werden.

### Schritt 2: Installation der Espressif-Module

Für das Flashen des ESP32-C6 müssen die entsprechenden Espressif-Module hinzugefügt werden.
Eine detaillierte Anleitung zur Installation dieser Module ist im
[SparkFun-Guide](https://docs.sparkfun.com/SparkFun_Thing_Plus_ESP32_C6/software_setup/) zu finden.

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