# ESP8266 xDrip monitor

Questo progetto è un MOC (My Own Code) per implementare un piccolo monitor della glicemia rilevata da xDrip+ installato sul mio cellulare.

`v0.0.1b`
![image](https://github.com/DavideGavagnin/xDripMonitor/assets/156298257/bcbbf635-2ecd-4a19-8897-fb42681e4ea9)


Il dispositivo si collega alla rete WiFi "di casa", nella quale deve essere presente anche il cellulare con xDrip+ installato e funzionante. 
xDrip+ va configurato a dovere per pubblicare le API. L'endpoint utilizzato è `/sgv.json?count=1`.

IL codice è tarato per la board [ESP8266 ESP12F NodeMCU ESP8266 Scheda di sviluppo con display OLED da 0,96 pollici, driver CH340, modulo wireless WiFi 
ESP-12E e micro USB Ottimo per la programmazione Arduino IDE/Micropython](https://amzn.eu/d/ib1W2zs) (non è un link affiliato, n.d.r.)

Una volta clonato il repo dovete creare nella root del progetto un file `settings.h` contenente il nome dell'SSID, la password di rete e l'API secret di xDrip.

`settings.h`
```
#define WIFI_SSID "<nomessid>"
#define WIFI_PASS "<passwordwifi>"

#define XDRIP_API_SECRET "<apisecret>"
#define XDRIP_POLLING_INTERVAL 30000 //ms, aumentare a piacimento, sconsiglio di diminuire
#define XDRIP_HOSTNAME "<IP_O_URL_DEL_CELLULARE>"
#define XDRIP_PORT 17580 // standard xDrip
#define XDRIP_PROTOCOL "http" // standard xDrip

#define OLED_SCL 4 // # PIN CLOCK per OLED I2C integrato nella board
#define OLED_SDA 5 // # PIN DATA per OLED I2C integrato nella board
```
Nota: i pin SDA e SCL, di comunicazione con il display, potrebbero differire da modello a modello della board. 
I miei differivano perfino dal codice di esempio fornito dal venditore Amazon (12 e 14).

All'avvio viene mostrato un test dell'OLED, poi il contenuto viene cancellato ed inizia il programma vero e proprio, con la connessione al WiFi.
Sulla destra del display appaiono le seguenti icone (in ordine dall'alto al basso):

- Wifi (lampeggia se NON connesso, fisso se connesso)
- Fantasma Pacman (acceso se NON c'è collegamento con il telefono)
- Punto esclamativo (acceso se capita un ERRORE DI DECODIFICA del JSON fornito dalle API xDrip+)

Sentitevi liberi di forkare o di migliorare il codice tramite pull request. In linea di massima non fornisco supporto a bug segnalati o feature requests. 
Non mi assumo **nessuna responsabilità sull'uso del codice e sul suo funzionamento**. Stiamo pur sempre parlando di **persone con una malattia**, che possono aggravarsi 
a causa dell'affidamento alla tecnologia, quindi **tenete presente che il presente codice è a livello hobbistico e può funzionare male**.
