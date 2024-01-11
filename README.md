# ESP8266 xDrip monitor

Questo progetto è un MOC (My Own Code) per implementare un piccolo monitor della glicemia rilevata da xDrip+ installato sul mio cellulare.

`v0.0.1b`
![image](https://github.com/DavideGavagnin/xDripMonitor/assets/156298257/bcbbf635-2ecd-4a19-8897-fb42681e4ea9)


Il dispositivo si collega alla rete WiFi "di casa", nella quale deve essere presente anche il cellulare con xDrip+ installato e funzionante. xDrip+ va configurato a dovere per pubblicare le API.

IL codice è tarato per la board [ESP8266 ESP12F NodeMCU ESP8266 Scheda di sviluppo con display OLED da 0,96 pollici, driver CH340, modulo wireless WiFi ESP-12E e micro USB Ottimo per la programmazione Arduino IDE/Micropython](https://amzn.eu/d/ib1W2zs) (non è un link affiliato, n.d.r.)

Una volta clonato il repo dovete creare nella root del progetto un file `settings.h` contenente il nome dell'SSID, la password di rete e l'API secret di xDrip.

`settings.h`
```
#define WIFI_SSID "<nomessid>"
#define WIFI_PASS "<passwordwifi>"

#define XDRIP_API_SECRET "<apisecret>"
```
Sulla destra appaiono le icone (in ordine dall'alto al basso):

- Wifi (lampeggia se non connesso)
- Fantasma Pacman (acceso se non c'è collegamento con il telefono)
- Punto esclamativo (acceso se capita un errore di decodifica del JSON fornito dalle API xDrip+)

Nota: i pin SDA e SCL, di comunicazione con il display, potrebbero differire da modello a modello della board. I miei differivano dal codice di esempio fornito dal venditore Amazon.

Sentitevi liberi di forkare o di migliorare il codice tramite pull request. In linea di massima non fornisco supporto a bug segnalati o feature requests. Non mi assumo nessuna responsabilità sull'uso del codice.
