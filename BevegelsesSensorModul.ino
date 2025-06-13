//Kode for bevegelses Sensoren modul.
//Bevegelses Sensor modulen har 3 essensielle moduler. 1. HC-SR505 PIR bevegelses sensor. 2. piezo høytaler. 3. NRF2010 reciver/transmitter. 
//Når jeg bruker ordet NRF i koden referer jeg til modulen som brukes for å mota signaler. (radio).
//koden fungerer med at arduinoen står og lytter etter signaler. i perioder hvor ingen signal motas gjør ikke arduinoen noe annet en å sjekke etter signaler. 
//Når arduinoen motar ett signal via NRF modulen (radio) vil registrere dette som at det er flamme (flamme = sann).
//sålenge flamme er sann vil arduinoen ha "aktivert" bevegelses sensoren.
//Dette gjøres ved å sjekke om bevegelses sensoren registrerer bevegelser.
//hvis den registrerer bevegelser vil piezo høytaleren slå alarm. 
//Når arduinoen slutter å mota signal/signaer, vil flamme settes til false og arduinoen slutter å se etter bevegelse.
//NB alle kommentarene som starter med "*" har som formål å hjelpe koderen når man tester og justerer koden. Disse kan kommenteres ut eller fjernes om ønskelig.  


#include <SPI.h> //legger til biblotek som tillater meg og bruke NRF senneren.
#include <RF24.h>
#include <nRF24L01.h>


#define CE_PIN      9
#define CSN_PIN     10
long nesteSjekk; //nesteSjekk, bufferMills og nesteTom er tre variabler som jeg skal bruke sammens med millis() for å velge når jeg ønsker å gjøre diverse.
long  nesteTom;
long bufferMillis;
bool flamme = false;   //en bool for å se om flamme er true eller false. 
int SENSOR_PIN = 4;   //lager en variael som heter SENSOR_PIN som refererer til bevegelses sensoren. sensoren er koblet til port/pin 4.
   //jeg gjør detet for at det blir lettere å bruke bevegelses senosren i koden og det blir mere lesbart.
int BUZZER_PIN = 3;   //jeg bruker samme prinsipp for BUZZER_PIN, denne refererer til piezoen (høytaleren). Denne er koblet til port/pin 3


RF24 radio(9, 10); // deklarerer NRF senderen som radio og kobler CE og CSN pins til pin 9 og 10.
const byte addr[6] = "00001";  // lager adressen til "kanalen" jeg øsnker å mota på. i dette tilfelle "00001". samme som flammesensoren i den ander koden.  

void setup() {
  //setup for bevegelses sensor og piezo. 
  Serial.begin(9600);  
  pinMode(SENSOR_PIN, INPUT);   //setter sensoren til å ta input, siden jeg bil registrere når det er bevegelse.
  pinMode(BUZZER_PIN, OUTPUT);   // setter BUZZER_PIN til output. siden jeg ønsker å "sende" lyd ut av piesoen. (output).
  noTone(BUZZER_PIN);    //setter piezoen til noTone for at den ikke skal lage lyd. 

  nesteSjekk = millis();   //setter  nesteTom og nesteSjekk lik millis() i det arduinoen starter  
  nesteTom = millis();   //(skulle være nesteTøm men ø blir kompilerings feil)
  bufferMillis = millis();

    //setter opp NRF sender/motaker.
  radio.begin();   // setter opp radio og tillater meg og kalle på andre radio funskjoner.
  radio.openReadingPipe(0, addr);  // setter opp NRF til å motta signal. 0 står for hvilken pipe jeg ønsker å bruke for mottakelse av signal. addr er adressen 
  radio.setPALevel(RF24_PA_MIN);  //senderstyrke settes til det laveste for å spare strøm. 
  radio.startListening();   //NRF senderen settes til å lytte etter signaler. 
  
  Serial.println("Flame Receiver klar - venter på signal..."); //*printer melding til hjelp når jobber med koden. 
}




void loop() {

  if(flamme == true && millis() > nesteSjekk && bevegelse()){   //denne linjen sjekker om 3 ting er sant/true (statments).
    //sjekken starter med å sjekke om variabelen flamme er true. hvis sann så ->
    //-> sjekker den om millis() er større en nesteSjekk, millis() er anntal sekunder siden aruduinoen ble slått på. hvis sann så->
    //-> sjekker den om bevegelse() er true. metoden bevegelse() returnerer true hvis bevegelses sensoren registrerer bevegelse.
    //hvis alle disse stemmer går vi inn i denne kodeblokken. nå er det flamme, millis() er større en NesteSjekk og noen beveger seg forran sensoren.  

    alarm(); //kaller på alarm metoden, som får piesoen til "slå alarm".
    nesteSjekk = millis() + 4000; //denne setter nestesjekk sin verdi til og være millis() + 4 sekunder.
    //i dette øyeblikket er nesteSjekk 4 sekunder mere en millis(). det vil nå ta 4 sekunder i den virkelige verden før millis() er større en nestesjekk. 
    //formålet med nestesjekk er å unngå at man skal kalle på alarm kontinuerlig når det er noe bevegelse forran sesnsoer. 
    //derfor bruker man nesteSjekk i if statmenten, slik at alarm() metoden bare kalles på i et intervall på minst 4 sekunder. 
  }

 
  if (millis() > bufferMillis && radio.available()) { //denne sjekker om radio.availeble() er true. og at man ikke er i stadiet hvor man nettopp har tømmt køen.
          //forklarer bufferMillis mekanismen lenger ned i koden. 
          //radio.available() metoden sjekker om det er noen signlaer som er sendt til NRF motakeren på arduinoen, hvis det er noen vil den returnere true.
          //vi vet at hvis det er sendt et signal til denne aruinoen betyr det at ett lys er tent. 
    flamme = true; //setter variabelen flamme til true.
    Serial.println("FLAMME DETEKTERT! "); //* printer til monitoren, dette brukes for feilsøking og oversikt under kjøring (for koderen).
  }else if(millis() > bufferMillis){
    flamme = false;} //hvis radio.available() ikke har motatt noen signaler og man ikke venter på bufferMillis setter vi flamme til false.


  if(millis() > nesteTom){ //her bruker vi samme prinsipp som med nesteSjekk. 
      radio.flush_rx();//radio.flush_rx() gjør at vi nulstiller køen til radioen. hvis det står mange "signaler" i kø i radioen blir disse nullstilt.
      //Formålet med denne er og sjekke om lyset er blitt slukket. siden vi nå fjerner eventuelle køer av signaler vil vi nå vente på ett nytt signal.
      //etter denne er kjørt vet vi at hvis radio.available() er sann har det nylig kommet et nytt signal, og flammen er fortsatt tent.
      //hvis denne kjører og radio.available() er false vet vi at vi ikke har fått et nytt signal og lyse vil da være slukket. 

    bufferMillis = millis() + 1050;//formålet med bufferMills er at etter vi har tømmt radioen, gir vi NRF motakeren tid til å motta ett nytt signal om flammen fortsatt er tent
    nesteTom = millis() + 3000;//setter neste tom til 3 sekunder mere en millis() intervall på å tømme køen er 3 sekunder. 

  }
}


void alarm(){ //metode for at piezoen skal lage alarm lyd.
      Serial.println("Alarm"); //*skriver til monitor for å hjelpe koderen når man tester.
      for(int i = 0; i < 5; i++){ //dene blokken kjører 5 ganger. denne kan man justre etter hvor mange "beeps" man vil ha.
        tone(BUZZER_PIN, 600);  //tone() er innebygt i arduino IDE. denne metoden sender "bølge" til BUZZER_PIN.
        //tallet 600 er frekvensen man øsnker at piezoen skal vibrere i. Dette vil da tilsvare hvilke tone man får. høyere tall = lysere tone. 
        delay(500);  //jeg setter et delay på et havlt sekund. Dette vil gjøre at piezoen holder en konstant tone i et halt sekund.
        noTone(BUZZER_PIN);  //når delayen er ferdig etter et halt sekund setter jeg noTone(BUZZER_PIN) som gjør at pezoen ikke lager noe lyd.
        delay(300);  //jeg venter så i 0,3 sekunder hvor piesoen ikke lager lyd.
        
        //man kan justere delayene ettersom hvor raskt man ønsker pipene skal være og relasjon mellom stille og pip. 
        //siden denne blokken gjennomføres 5 ganger vil man få resultatet at piezoen piper 5 ganger. 
      }
}


bool bevegelse(){ //Denne metoden returnerer true dersom bevegelses sensoren registrerer bevegelse.
  //bruker digitalRead(SENSOR_PIN) for å lese av om bevegelses sensoren er HIGH (bevegelse detectet) eller LOW (ingen bevegelse)
  if(digitalRead(SENSOR_PIN) == HIGH){//*hvis sensoren regisrerer bevegelse skriver jeg ut en melding til proggrammereren. brukes for debugging og oversikt. 
    Serial.println("----- BEVEGELSE-----"); //*
  }
  return digitalRead(SENSOR_PIN) == HIGH; //Hvis bevegelses sensoren har registrerer bevegelse vil denne returnere true. 
}

