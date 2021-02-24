/*
 * Nombre del proyecto: Purificador de carbon activado
 * Descripcion: 
 
    El sistema se compone de: 
      - Cooler para forzar un flujo de aire
      - Sensor MQ135 (para detectar la presencia de varios gases nocivos)
      - Sensor DHT11 (para medir temperatura y humedad)
      - Módulo DFPLayer (reproductor de audio con amplificador integrado controlado por comunicación serie)
      - 2 pulsadores
      - Buzzer
      - Arduino UNO

    Con la ayuda de todo lo anterior el dispositivo lleva a cabo diversas tareas:
      - Mostrar en un display LCD temperatura, humedad y calidad del aire (dividida en 4 categorías: muy buena, buena, regular y alerta)
      - Al presionar un pulsador se nos indica la calidad del aire por medio de un audio
      - Al detectar una calidad de alerta se reproducirá una alarma por medio de un buzzer mientras que un audio nos recomienda que nos retiremos del lugar
      - Si la calidad del aire medida es regular o buena, se encenderá el cooler para purificar el aire por medio del filtro.
      - También es posible habilitar y deshabilitar el cooler manualmente por medio de otro pulsador.
 * 
 * Autor: Joan Lucas Martinez Díaz y Gisela Berdun
 * Contacto: joanlucasmd@gmail.com
 * Fecha: Diciembre de 2020S
 * 
 * LOOPEA SCHOOL]
 */
 
//***************************************************//
//******************* LIBRERIAS *********************//
//***************************************************//
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//***************************************************//
//******************* ETIQUETAS *********************//
//***************************************************//
#define SENSOR 9             //DHT11                   
#define BUZZER 2             //Buzzer de alerta
#define PUL1 5               //Pulsador 1, voz que declara temperatura, humedad y calidad de aire
#define PUL2 6               //Pulsador 2, enciendo y apago manualmente el cooler
#define FAN 3                //Cooler
#define DFPRX 11             //Comunicación con puerto serie
#define DFPTX 10             //Comunicacion con dfplayer
#define LED1 8              //Led interno 1
#define LED2 12              //Led interno 2

//***************************************************//
//********** VARIABLES Y OBJETOS GLOBALES ***********//
//***************************************************//
int temp;                               //variable de temperatura
int hum;                                //variable de humerdad
int valormq135 = 0;                     //variable del mq135
bool estadofan = false;
unsigned long tiempodereferencia = 0;


DHT dht(SENSOR, DHT11);                    //llamo objeto dht    //Donde SENSOR es el define asignado al pin y DHT11 es una variable definida dentro de la libreria predefinida.
LiquidCrystal_I2C lcd(0x27, 16, 2);        //llamo objeto lcd
Bounce pulsador1 = Bounce();
Bounce pulsador2 = Bounce();
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);


//***************************************************//
//********************* SETUP ***********************//
//***************************************************//
void setup() {

  //inicializaciones
    mySoftwareSerial.begin(9600);
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    while (true);
  }
  //----Set volume----
  myDFPlayer.volume(30);  //Set volume value (0~30).
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  dht.begin();


  //Asignación de los pines de entrada y salida
  pinMode(FAN, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pulsador1.attach(PUL1, INPUT_PULLUP);
  pulsador2.attach(PUL2, INPUT_PULLUP);
  pinMode (BUZZER, OUTPUT);
  Wire.begin();
  lcd.begin(16, 2);
  lcd.backlight();                //Seteo de luz del lcd
  lcd.setCursor(3, 0);            //Seteo del cursor del lcd
  lcd.print("BIENVENIDO!");      //Impresion en el lcd 
   myDFPlayer.play(1);
  digitalWrite(LED1, true);
  digitalWrite(LED2, true);
  tiempodereferencia = millis();
}

//***************************************************//
//********************** LOOP ***********************//
//***************************************************//
void loop() {


  //inicializo y leo sensores
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  valormq135 = analogRead(A0);

  //BUZZER
  digitalWrite (BUZZER, false);

  //interrupciones inmediatas de pulsadores
  pulsador1.update();
  pulsador2.update();
  bool pulsador1desactivado = pulsador1.rose();
  bool pulsador2desactivado = pulsador2.rose();
  bool pulsador1activado = pulsador1.fell();
  bool pulsador2activado = pulsador2.fell();

  //control de switches 1
  digitalWrite(FAN, estadofan);
  if (pulsador1activado) {
    lcd.clear();
    lcd.home();
    lcd.print("   Conmutando");
    delay(3000);
    estadofan = !estadofan;
  }
  
  //control de switches 2
  if (pulsador2activado) {
    //Muy bueno
    if (valormq135 <= 55) {
      myDFPlayer.play(2);
    }  
    //Bueno
     if (valormq135 > 56 && valormq135 < 65){
      myDFPlayer.play(3);
     }
     //Regular
      if (valormq135 > 74 && valormq135 < 200){
        myDFPlayer.play(4);
     }
  }

  //ALERTA
  if (valormq135 > 201)
  {
    digitalWrite(FAN, false);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALERTA PRESENCIA");
    lcd.setCursor(4, 1);
    lcd.print("DE GASES!!");
    myDFPlayer.play(5);
    for (int i = 0; i < 5; i++) {
      digitalWrite(BUZZER, true);
      delay(1000);
      digitalWrite(BUZZER, false);
      delay(1000);
    }
  }

  unsigned long tiempoactual = millis();
  unsigned  int tiempotranscurrido =  tiempoactual - tiempodereferencia;

  //Calidad de aire
  if (tiempotranscurrido >= 5000) {
    
    //aire muy bueno
    if (valormq135 <= 55) {
      lcd.setCursor(0, 0);
      lcd.print("Calidad de aire:");
      lcd.setCursor(0, 1);
      lcd.print("---MUY BUENO----");
    }

    //aire con un poco de co2
    if (valormq135 > 58 && valormq135 < 65)
    {
      digitalWrite(FAN, true);
      lcd.setCursor(0, 0);
      lcd.print("Calidad de aire:");
      lcd.setCursor(0, 1);
      lcd.print("-----BUENO------");
    }


    //dioxido de carbono
    if (valormq135 > 74 && valormq135 < 200)
    {
      digitalWrite(FAN, true);
      lcd.setCursor(0, 0);
      lcd.print("Calidad de aire:");
      lcd.setCursor(0, 1);
      lcd.print("-----REGULAR----");
    }
  }

  //TEMPERATURA Y HUMEDAD
  if (tiempotranscurrido >= 10000) {
    tiempodereferencia = tiempoactual;
    lcd.clear();
    lcd.home();
    lcd.print("Temperatura: ");
    lcd.print(temp);
    lcd.print("C");
    lcd.setCursor(0, 1);
    lcd.print("Humedad: ");
    lcd.print(hum);
    lcd.print("%");
  }
}
