//librerias
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <math.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

//const char* ssid = "Totalplay_9EA3_2.4Gnormal";
//const char* password =  "Llozocha2107";

//SSID y contraseña para la conexion del internet
const char* ssid = "ARRIS-3372";
const char* password =  "C4N9N9PD9MJKER4K";

//Hardware
#define DHTPIN 13 //pin del sensor de temperatura                                                                                                                                                                                                  //PIN
#define DHTTYPE DHT21 //Tipo de sensor  de temperatura 
DHT_Unified  dht(DHTPIN, DHTTYPE); //Inicializacion del sensor de temperatura 

//Pines del LED RGB
int R = 25;
int G = 26;
int B = 27;

//Pines de los relevadores
int Bomba = 21;
int mAlto = 22;
int mBajo = 23;

//estado de cada uno de los modos del aire lavado
bool estatusBomba;
bool estatusMotorAlto;
bool estatusMotorBajo;

//estado del modo automatico
bool autoMode;
//temperatura obtenida de la base de datos
int temperatura_deseada;

void setup() {
  //Configuracion de los pines del LED RGB
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  //Configuracion de los pines de los relevadores
  pinMode(Bomba, OUTPUT);
  pinMode(mAlto, OUTPUT);
  pinMode(mBajo, OUTPUT);
  Serial.begin(115200); //Inicializacion del puerto serial
  dht.begin(); //Inicializacion de la lectura del sensor DHT21
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);

  //Conectandose a internet
  WiFi.begin(ssid, password);
  Serial.print("Conectando...");
  setColor(250, 69, 0); //Establece el color en naranja mientras se conecta al internet.
  while (WiFi.status() != WL_CONNECTED) { //Verifica la conexion
    delay(500);
    Serial.print(".");
  }
  Serial.print("Conectado con éxito, mi IP es: ");
  Serial.println(WiFi.localIP()); //Imprime la IP local que asigno el DHCP
}

void loop() {
  float var_temp = getTemp(); //se declara la temperatura actual obtenida del sensor de temperatura.
  float var_humidity = getHum(); //se declara la humedad actual obtenida del sensor de temperatura.
  if(WiFi.status()== WL_CONNECTED){   //Verifica si existe conexion a internet.
    getLogs(); //Obtiene la coleccion de Logs de la base de datos.
    postTemp(); //Publica la temperatura actual y humedad a la base de datos.
  }
  double tempdif = var_temp - temperatura_deseada; //Obtiene la diferencia de la temperatura actual y la temperatura deseada por el usuario.

if(autoMode == true){ //Si el modo automatico es verdadero
    if(!isnan(var_temp)){ //Si la temperatura actual es un numero.
      if(var_temp > temperatura_deseada){ //Si la temperatura actual es mayor a la temperatura deseada.
        if(var_temp >= 30){ //Si la temperatura es mayor a 30 grados.
          digitalWrite(Bomba, 1); //Se activa la bomba
          digitalWrite(mBajo, 0); //Se desactiva la velocidad baja
          digitalWrite(mAlto, 1); //Se activa la velocidad alta
          //Se establece el estado de la velocidad y la bomba.
          estatusMotorAlto = true;
          estatusMotorBajo = false;
          estatusBomba = true;
          Serial.println("auto Mayor o igual a 30 grados.");
        } else { //Si la temperatura es mayor a la deseada y si la temperatura no es mayor a 30 grados.
          if(var_humidity > 70){ //Si la humedad es mayor a 70
            digitalWrite(Bomba, 0); //Se desactiva la bomba
            estatusBomba = false; //Se establece el estado de la bomba.
            Serial.println("auto Bomba apagada");
            if(tempdif <= 2.5 && tempdif >=0){ //Si la diferencia de la temperatura es menor o igual a 2.5 y la misma diferencia es mayor a 1.
              digitalWrite(mAlto, 0); //Desactiva la velocidad alta.
              digitalWrite(mBajo, 1); //Activa la velocidad baja
              //Se establece el estado de las velocidades.
              estatusMotorAlto = false;
              estatusMotorBajo = true;
              Serial.println("auto Motor bajo encendido");
            } else { //Si la diferencia de la temperatura es mayor a 2.5
              digitalWrite(mBajo, 0); //Se desactiva la velocidad baja.
              digitalWrite(mAlto, 1); //Se activa la velocidad alta
              //Se establece el estado de las velocidades
              estatusMotorAlto = true;
              estatusMotorBajo = false;
              Serial.println("auto Motor alto encendido");
            }
          } else { //Si la humedad es menor a 70
            digitalWrite(Bomba, 1); //Se activa la bomba
            estatusBomba = true; //Se establece el estado de la bomba
            Serial.println("auto Bomba encendida");
            if(tempdif <= 2.5 && tempdif >=0){ //Si la diferencia de la temperatura es menor o igual a 2.5 y la misma diferencia es mayor a 1.
              digitalWrite(mAlto, 0); //Desactiva la velocidad alta.
              digitalWrite(mBajo, 1); //Activa la velocidad baja
              //Se establece el estado de las velocidades.
              estatusMotorAlto = false;
              estatusMotorBajo = true;
              Serial.println("auto Motor bajo encendido");
            } else { //Si la diferencia de la temperatura es mayor a 2.5
              digitalWrite(mBajo, 0); //Se desactiva la velocidad baja.
              digitalWrite(mAlto, 1); //Se activa la velocidad alta
              //Se establece el estado de las velocidades
              estatusMotorAlto = true;
              estatusMotorBajo = false;
              Serial.println("auto Motor alto encendido");
            }
          }
        }
      } else { //Si la temperatura actual es menor a la temperatura deseada.
         digitalWrite(Bomba, 0); //Se apaga la bomba
         digitalWrite(mAlto, 0); //Se apaga la velocidad alta
         digitalWrite(mBajo, 0); //Se apaga la velocidad baja
         //Se estable el estado de las velocidades y de la bomba
         estatusMotorAlto = false;
         estatusMotorBajo = false;
         estatusBomba = false;
      }
    } else { //Si la temperatura actual no es un numero
      Serial.println("Error: Sensor de temperatura desconectado"); //Envia al monitor serial que el sensor de temperatura esta desconectado.
    }
} else if(autoMode == false) { //Si el modo automatico esta desactivado
  if(estatusBomba == true){ //Si el estado de la bomba es verdadero
    digitalWrite(Bomba, 1); //Se activa la bomba
    Serial.println("manual Bomba encendida");
  } else { //Si el estado de la bomba es falso
    digitalWrite(Bomba, 0); //Se desactiva la bomba
    Serial.println("manual Bomba apagada");
  }
  if(estatusMotorAlto == true){ //Si el estado de la velocidad alta es verdadero
    digitalWrite(mBajo, 0); //Se desactiva la velocidad baja
    digitalWrite(mAlto, 1); //Se activa la velocidad alta.
    Serial.println("manual Motor alto encendido");
  } else if(estatusMotorBajo == true){ //de lo contrario, si el estado de la velocidad baja es falto
    digitalWrite(mAlto, 0); //Se desactiva la velocidad alta
    digitalWrite(mBajo, 1); //Se activa la velocidad baja
    Serial.println("manual Motor bajo encendido");
  } else { //Si el estado de velocidad alta y baja es falsa
    digitalWrite(mAlto, 0); //Se desactiva la velocidad alta
    digitalWrite(mBajo, 0); //Se desactiva la velocidad baja
    Serial.println("manual Motor apagado");
  }
}
/*
 * Colores
 * Apagado ROJO -
 * Alto Azul -
 * Bajo Verde - 
 * Alto frio Azul fuerte -
 * Bajo frio Verde fuerte -
 * Bomba encendida amarillo
 * Conectando wifi Naranja -
*/
//Control de LED RGB
if(estatusMotorAlto == true && estatusBomba == true){ //Si el estado de la velocidad alta y el estado de la bomba es verdadero
  setColor(0, 0, 255);  //Se envia el color Azul
} else if(estatusMotorBajo == true && estatusBomba == true){ //Si el estado de la velocidad baja y el estado de la bomba es verdadero
  setColor(0, 255, 0); //Se envia el color Verde
} else if(estatusMotorAlto == true && estatusBomba == false){ //Si el estado de la velocidad alta es verdadero y el estado de la bomba es falso
  setColor(0, 255, 255); //Se envia el color Celeste
} else if(estatusMotorBajo == true && estatusBomba == false){ //Si el estado de la velocidad baja es verdadero y el estado de la bomba es falso
  setColor(191, 255, 0); //Se envia el color lima(verde bajo).
} else if(estatusMotorBajo == false && estatusMotorAlto == false && estatusBomba == false){ //Si el estado del motor es falso y el estado de la bomba es falso
   setColor(255, 0, 0); //Se envia el color Rojo
} else if(estatusMotorBajo == false && estatusMotorAlto == false && estatusBomba == true){ //Si solo el estado de la bomba es verdadero
   setColor(204, 255, 0); //Se envia el color amarillo
} 
delay(2000);
}

//Metodo utilizado en el metodo de getLogs()
int setAdjustmentTemp(int x){
  return temperatura_deseada = x; //Declara la temperatura deseada obtenido de la base de datos.
 }
//Metodo utilizado en el metodo de getLogs()
void setAutoMode(bool autom){
  autoMode = autom; //Declara el estado del modo automatico obtenido de la base de datos.
}
//Metodo utilizado en el metodo de getLogs()
bool setMHigh(bool mHigh){
  return estatusMotorAlto = mHigh; //Declara el estado de la velocidad alta de la base de datos.
}
//Metodo utilizado en el metodo de getLogs()
bool setMLow(bool mLow){
  return estatusMotorBajo = mLow; //Declara el estado de la velocidad baja de la base de datos.
}
//Metodo utilizado en el metodo de getLogs()
bool setPumb(bool pumb){
  return estatusBomba = pumb; //Declara el estado de la bomba de la base de datos.
}

//Conversor de color de Catodo a anado
//Metodo utilizado en void loop()
void setColor(int red, int green, int blue){
  red = 255 - red;
  green = 255 - green;
  blue = 255 - blue;
  analogWrite(R, red);
  analogWrite(G, green);
  analogWrite(B, blue);  
}

//Metodo utilizado en el metodo postTemp()
//Obtiene la temperatura del sensor y la retorna en un float.
float getTemp(){
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float temp = event.temperature;
  return temp;
}
//Metodo utilizado en el metodo postTemp()
//Obtiene la humedad del sensor y la retorn en un float..
float getHum(){
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float hum = event.relative_humidity;
  return hum;
}

//Metodo que obtiene los Logs de la base de datos
void getLogs(){
  HTTPClient http;
  http.begin("https://mongodbutch.herokuapp.com/api/logs/getLast"); //Indicamos el destino
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Preparamos el header text/plain si solo vamos a enviar texto plano sin un paradigma llave:valor
  int getLogs = http.GET(); //Envia y obtiene la peticion de los registros de la coleccion Logs
  if(getLogs>0){ //Si el codigo que retorna es mayor a 0
    Serial.println("Código HTTP ► " + String(getLogs));   //Imprime el codigo enviado
    if(getLogs == 200){ //Si el codigo es igual a 200
      DynamicJsonDocument doc(2048); //Se da memoria a la variable doc
      deserializeJson(doc, http.getStream()); //Se convierte el archivo JSON para trabajar con el.
      temperatura_deseada = doc[0]["adjustment_temperature"] //Se declara la temperatura deseada por el dato obtenido del JSON.
      setAdjustmentTemp(temperatura_deseada); //Se envia por uun metodo para trabajar con el en loop() 
      setMHigh(doc[0]["mHigh_state"]); //Se envia el dato obtenido de la base de datos mediante un metodo para trabajar con el en loop() 
      setMLow(doc[0]["mLow_state"]); //Se envia el dato obtenido de la base de datos mediante un metodo para trabajar con el en loop() 
      setPumb(doc[0]["pumb_state"]); //Se envia el dato obtenido de la base de datos mediante un metodo para trabajar con el en loop() 
      setAutoMode(doc[0]["autoMode_state"]); //Se envia el dato obtenido de la base de datos mediante un metodo para trabajar con el en loop() 
     }
   } else {
    Serial.print("Error enviando GET, código: ");
    Serial.println(getLogs);
   }
 http.end(); //Se liberan recursos
}

//Metodo que publica la temperatura y la humedad a la base de datos.
void postTemp(){
  HTTPClient http;
  http.begin("https://mongodbutch.herokuapp.com/api/temp/post"); //Indicamos el destino
  http.addHeader("Content-Type", "application/json");   //Preparamos el header text/plain si solo vamos a enviar texto plano sin un paradigma llave:valor. //Enviamos el post pasándole, los datos que queremos enviar. (esta función nos devuelve un código que guardamos en un int)
  float temp = getTemp(); //Se obtiene la temperatura del sensor.
  float hum = getHum(); //Se obtiene la humedad del sensor
  int date = 555;

  const int capacity = JSON_OBJECT_SIZE(3); //Se establece la capacidad del objeto JSON.
  StaticJsonDocument<200> doc; //Se da memoria a la variable doc.
  doc["datetime"]= date; //Se establece el valor a el objeto date a la variable doc.
  doc["current_temperature"]= temp; //Se establece el valor a el objeto current_temperature a la variable doc.
  doc["current_humidity"]= hum; //Se establece el valor a el objeto current_humidity a la variable doc.

  String output; //Se declara un string
  serializeJson(doc, output); //Se hace una conversion
  int postTemp = http.POST(output);//Hace la peticion post a la base de datos.
    
  if(postTemp>0){ //Si la temperatura es mayor a 0
    Serial.println("Código HTTP ► " + String(postTemp));   //Imprime el codigo recibido
  } else { 
    Serial.printf("Error occurred while sending HTTP POST: %s\n", http.errorToString(postTemp).c_str());   
  }
  http.end(); //Se liberan recursos.
}




  
