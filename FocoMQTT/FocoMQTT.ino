#include <ESP8266WiFi.h>
#include <string.h>
#include <PubSubClient.h>
#include <gpio.h>

extern void esp_schedule();
extern void esp_yield();

#define wifi_ssid "CasaInteligente"
#define wifi_password "arduinoday2017"

#define mqtt_server "192.168.8.1"
#define mqtt_server_port 1883
#define tema "ARDUINODAY"
#define NombreDeCliente "FOCO1"


#define led 5

void InterpretaMsg(char* topic, byte* message, unsigned int length);

WiFiClient espClient;
PubSubClient client(espClient);

String Dispositivo = "FOCO1";

int InterpretaCMD(String comando) {
  unsigned int cmdVal = 0;
  int comandoInt = comando.toInt();
  switch (comandoInt) {
    case 0:
      cmdVal = 1023;
      break;
    case 1:
      cmdVal = 960;
      break;
    case 2:
      cmdVal = 956;
      break;
    case 3:
      cmdVal = 950;
      break;
    case 04:
      cmdVal = 941;
      break;
    case 5:
      cmdVal = 930;
      break;
    case 6:
      cmdVal = 915;
      break;
    case 7:
      cmdVal = 885;
      break;
    case 8:
      cmdVal = 860;
      break;
    case 9:
      cmdVal = 830;
      break;
    case 10:
      cmdVal = 800;
      break;
    case 11:
      cmdVal = 755;
      break;
    case 12:
      cmdVal = 710;
      break;
    case 13:
      cmdVal = 655;
      break;
    case 14:
      cmdVal = 600;
      break;
    case 15:
      cmdVal = 525;
      break;
    case 16:
      cmdVal = 450;
      break;
    case 17:
      cmdVal = 340;
      break;
    case 18:
      cmdVal = 220;
      break;
    case 19:
      cmdVal = 0;
      break;
  }
    
  return cmdVal;
}

void InterpretaMsg(char* topic, byte* message, unsigned int length){
  String mensaje = String((char*)message).substring(0,length);
  Serial.print("Mensaje recibido: ");
  Serial.print(mensaje);
  Serial.println(" de longitud " + length);
  

  if (length > 11)
  {
    String cmd = mensaje.substring(0,6);
    String Ncliente = mensaje.substring(7,12);
    int CmdVal = 0;
    String MensajeToPublish = "";
    
    Serial.println(cmd);
    Serial.println(Ncliente);
    if (Ncliente == Dispositivo)
    {
       if (cmd == "DIMMER")                                                        //este bloque if se puede eliminar si solo se controla encendido y apagado sin PWM ya que este bloque es el responsable de dicha funcion
       {                                                                           //
          CmdVal = InterpretaCMD(mensaje.substring(13,15));                        //
          analogWrite(led, CmdVal);                                                //
          MensajeToPublish = "DIMEADO " + Dispositivo + " con valor " + CmdVal;    //
          byte miArray[MensajeToPublish.length()];                                 //
          MensajeToPublish.getBytes(miArray, MensajeToPublish.length());           //
          client.publish(tema, miArray, MensajeToPublish.length());                //
          Serial.println(MensajeToPublish);                                        //
       }                                                                           //
       else                                                                        //
       if (cmd == "APAGAR")
       {
          CmdVal = InterpretaCMD("00");                                            //si se controla por PWM
          analogWrite(led, CmdVal);                                                //si se controla por PWM
          //digitalWrite(led, LOW);                                                //si solo se controla encendido y apagado
          MensajeToPublish = "APAGADO " + Dispositivo;
          byte miArray[MensajeToPublish.length()];
          MensajeToPublish.getBytes(miArray, MensajeToPublish.length());
          client.publish(tema, miArray, MensajeToPublish.length());
          Serial.println(MensajeToPublish);
       }
       else if (cmd == "PRENDE")
       {
          CmdVal = InterpretaCMD("19");                                            //si se controla por PWM
          analogWrite(led, CmdVal);                                                //si se controla por PWM
          //digitalWrite(led, HIGH);                                               //si solo se controla encendido y apagado
          MensajeToPublish = "PRENDIDO " + Dispositivo;
          byte miArray[MensajeToPublish.length()];
          MensajeToPublish.getBytes(miArray, MensajeToPublish.length());
          client.publish(tema, miArray, MensajeToPublish.length());
          Serial.println(MensajeToPublish);
       }
       else
       {
          Serial.println("CMD Desconocido");
       }
    }
    else
    {
      Serial.println("CMD para otro cliente");
    }
  }
  else
  {
    Serial.println("Mensaje Desconocido");
  }
  Serial.println();
  Serial.println();
}


void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a WIFI");

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("conexión WiFi realizada a ");
  Serial.println(wifi_ssid);
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());
}

void setup() {
  pinMode(led, OUTPUT);
  analogWrite(led,0);                                       //si se controla por PWM
  //digitalWrite(led,HIGH);                                 //Si solo se controla encendido y apagado
  Serial.begin(115200);
  setup_wifi();
  
  client.setServer(mqtt_server, mqtt_server_port);
  client.setCallback(InterpretaMsg);
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Conectando servidor MQTT");
    if (client.connect(NombreDeCliente)) {
      Serial.println("Conectado");
      delay(100);
      if (client.subscribe(tema,1)){
        delay(100);
        Serial.print("NodeMCU se ha suscrito al tema ");
        Serial.println(tema);
      }
    } else {
      Serial.print("Intento fallido, err =");
      Serial.print(client.state());
      Serial.println("volveremos a intentar reconexión en 5 seg.");
      abort();
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
