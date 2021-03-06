#include <ESP8266WiFi.h>
#include <PubSubClient.h>
 
const char* ssid = "CLARO_2GF47564"; //nome do wifi
const char* password =  "FAF47564"; // senha do wifi
const char* mqttServer = "ioticos.org"; //Server disponibilizado pela instância do CloudMQTT
const int mqttPort =  1883; //Porta disponibilizado pela instância do CloudMQTT
const char* mqttUser = "GOHPn44W1RTBlyj"; //Usuário disponibilizado pela instância do CloudMQTT
const char* mqttPassword = "MKaal5c6sLmW3bN"; //Senha disponibilizado pela instância do CloudMQTT
const char* mqttTopicSub ="nRC4GZ8LVaDPMYn";   //Tópico voce pode escolher um nome

#define psom    5 //sensor de som
#define prele    4 //modulo relé
boolean rele = HIGH;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
 
  Serial.begin(115200);
  pinMode(psom, INPUT); //Coloca o pino do sensor de som como entrada
  pinMode(prele, OUTPUT);//Coloca o pino do módulo relé como saida
  digitalWrite(prele, rele);
  //Conexão 
  
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #ifdef DEBUG
    Serial.println("Conectando ao WiFi..");
    #endif
  }
  #ifdef DEBUG
  Serial.println("Conectado na rede WiFi");
  #endif
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    #ifdef DEBUG
    Serial.println("Conectando ao Broker MQTT...");
    #endif
 
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
      #ifdef DEBUG
      Serial.println("Conectado");  
      #endif
 
    } else {
      #ifdef DEBUG 
      Serial.print("falha estado  ");
      Serial.print(client.state());
      #endif
      delay(2000);
 
    }
  }

  //subscreve no tópico
  client.subscribe(mqttTopicSub);
 
}
 
void callback(char* topic, byte* payload, unsigned int length) {

  //armazena msg recebida em uma sring
  payload[length] = '\0';
  String strMSG = String((char*)payload);

  #ifdef DEBUG
  Serial.print("Mensagem chegou do tópico: ");
  Serial.println(topic);
  Serial.print("Mensagem:");
  Serial.print(strMSG);
  Serial.println();
  Serial.println("-----------------------");
  #endif

  //aciona saída conforme msg recebida 
  if (strMSG == "on" or strMSG == "off"){ 
     rele = !rele; //muda o estado do relé 
     digitalWrite(prele, rele);  
  }
  
}

//função pra reconectar ao servido MQTT
void reconect() {
  //Enquanto estiver desconectado
  while (!client.connected()) {
    #ifdef DEBUG
    Serial.print("Tentando conectar ao servidor MQTT");
    #endif
     
    bool conectado = strlen(mqttUser) > 0 ?
                     client.connect("ESP32Client", mqttUser, mqttPassword) :
                     client.connect("ESP32Client");

    if(conectado) {
      #ifdef DEBUG
      Serial.println("Conectado!");
      #endif
      //subscreve no tópico
      client.subscribe(mqttTopicSub, 1); //nivel de qualidade: QoS 1
    } else {
      #ifdef DEBUG
      Serial.println("Falha durante a conexão.Code: ");
      Serial.println( String(client.state()).c_str());
      Serial.println("Tentando novamente em 10 s");
      #endif
      //Aguarda 10 segundos 
      delay(10000);
    }
  }
}
 
void loop() {
  int s = digitalRead(psom); //Verifica se a saida do sensor esta ativa
   if(s){ // Se houver captura de som
    rele = !rele; //muda o estado do relé, se estiver acessa devido ao mqtt, com a palma da mão ele apaga, vice e versa.
    digitalWrite(prele, rele); // Manda o valor da variavel para o rele
    delay(1000); // 
   }
  if (!client.connected()) {
    reconect();
  }
  client.loop();
}
