//kode for flammesenroren.
//Flammesensor modulen har to essensielle komponenter. 1. IR flammesensor. 2. NRF2010 reciver/transmitter.
//Måten denne fungerer på er med at arduinonen sjekker om IR sensoren registrerer flamme, dette skjer 1 gang i sekundet.
//sålenge sensoren ikke registrerer flammer vil arduinoen ikke gjøre noe annet.
//når den registrerer en flamme vil den sende ett signal via NRF senderen til bevegelses Sensor modulen.
//sålenge den ser en flamme vil den fortsette å sende signal med ett intervall på 1 gang pr sekund.
//hvis den så ikke ser en flamme vil den slutte å sende ut signaler.

#include <SPI.h> //inkluderer bibliotek for å kunne bruke NRF Sender/reciver samt radio metoder.
#include <RF24.h>



const int flameThreshold = 500; // int som man justerer etter øsnket sensitivitet. Brukes for å sette nivået for registrering av flamme. 
//IR sensoren ligger sirka på 25 og 300 som verdi når den registrerer en flamme. Når den ikke registrerer flamme ligger den mella 800 og 1000. 
int FLAMME_PIN  = A0; //variabel som heter FLAMME_PIN som refererer til IR flammesensor.


// sedner/reciver deklerasjon.
RF24 radio(9, 10); // deklarerer NRF senderen som radio og kobler CE og CSN pins til pin 9 og 10.
const byte address[6] = "00001"; // lager adressen til "kanalen" som er hvilke kanal jeg øsnker og sende på. i dette tilfelle "00001". 



void setup()  {
    Serial.begin(9600);  
    pinMode(FLAMME_PIN, INPUT); //flamme sensor skal ta input.

    radio.begin();  // setter opp radio samt tillater meg å kalle på andre radio funskjoner fra bibloteket.
    radio.openWritingPipe(address);  // åpner en kanal/pipe for og sende data på. og bruker adress som er "00001"
    radio.setPALevel(RF24_PA_MIN);  // setter senderstyrken til det laveste for å minimere strøm bruk, samt trenger man mindre strøm for sending. 
    radio.stopListening();  //radio stopper å lytte etter signaler, dette fordi vi bare ønsker å sende signaler med denne. 
}



void loop() {//dette er hoved loopen som kjøres når arduinoen er påslått. 
  if(sjekkFlamme()){ //bruker en if sjekk som finner ut om det er flamme.
    sendSignal(); //hvis det er flamme sender man signal via NRF sendere.
  }
  delay(1000);  //delayer arduinoen i 1 sekund. intervallen for å sjekke flamme blir da 1 sekunder og om det er flamme blir singal sendingene og ha intervall på 1 sekund.
  Serial.println(analogRead(FLAMME_PIN)); //denne brukes for testing og debugging for de som jobber med koden, denne kan kommeneres ut (fjernes) når man leverer endelig produkt. 
}




bool sjekkFlamme(){//egen metode for og sjekke flamme
    //analogread(FLAMME_PIN) vil gi åss en verdi mellom 0 og 1023. 
    return analogRead(FLAMME_PIN) < flameThreshold; //i vårt tillfelle vil verdien til flammesensorern være lavere en flameThreshold når den registrerer en flamme.
    //sjekkFlamme metoden returnerer true hvis flammesensoren registrerer en flamme, hvis ikke returneres false.
  }


void sendSignal() {
  byte signal = 0;  //det er ikke viktig hva jeg sender til den andre arduinoen, men heller at et signal sendes. 
  //Derfor velger vi å sende en byte siden det er en liten mengden data (8 bits), dette gjør at det går fortere og bruker mindre strøm. 
  
  bool harSendt = radio.write(&signal, sizeof(signal)); //i denne koden sender arduinoen ut signal til den andre arduinoen gjennom kanalen jeg åpnet i setup.
  // &signal gir minneadressen til hvor dataen til variabelen signal ligger, sizeof(signal) gir størrelsen på det som skal sendes. I dette tilfelle 1 byte. 
  //harSendt er en bool som lagrer resultatet av radio.write() denne blir enten true(hvis signalet som ble sendt kom fram til motaker) eller false hvis noe gikk galt.
  
  if (harSendt) { //denne har ingen invirkning på den tenkiske løsningen men blir brukt til debugging og feilsøking.
    Serial.println("Signal sent successfully!"); //hvis man får sendt signal prnter man denne meldingen til monitor.
  } else {
    Serial.println("Signal failed to send");//hvis man ikke får sendt signal printer man denne medlingen til monitor
  }
}



