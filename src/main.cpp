#include <TFT_eSPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define TFT_BL 38 // Controle de brilho do display
#define bot_button 0
#define key_button 14

const char* ssid = "leds-ifes";       // SSID da sua rede Wi-Fi
const char* password = "Leds.cai@2023";  // Senha da sua rede Wi-Fi
const char* mqtt_server = "10.20.240.200";  // Endereço IP do seu broker MQTT
const int mqtt_port = 1883;           // Porta do broker MQTT
const char* mqtt_topic = "acelerometro";  // Tópico MQTT para enviar os dados

WiFiClient espClient;
PubSubClient client(espClient);

#define ANALOG_PIN A0


TFT_eSPI tft = TFT_eSPI(); // Criação do objeto da classe TFT_eSPI

const int ENDERECO_SENSOR = 0x68;
int acelX;
bool isCounting = false; // Variável para controlar o estado da contagem
bool bot_button_state = false;
bool lastbot_button_state = false;
unsigned long lastDebounceTimeBot = 0;

bool key_button_state = false;
bool lastkey_button_state = false;
unsigned long lastDebounceTimeKey = 0;
unsigned long debounceDelay = 50;

int opcaoSelecionada = 0;

//void medir_acelerometro();
void exibir_menu();
void executar_acao();
void retornar_ao_menu();
void vigiaBotao();

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado à rede WiFi");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando se conectar ao broker MQTT...");
    if (client.connect("ESP32S3Client")) {
      Serial.println("Conectado ao broker MQTT");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Falha na conexão. RC=");
      Serial.print(client.state());
      Serial.println("Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}




void setup() {

  pinMode(bot_button, INPUT_PULLUP);
  pinMode(key_button, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println("TFT_eSPI Example!");

  Wire.begin(18, 17, 400000);
  Wire.beginTransmission(ENDERECO_SENSOR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  tft.init(); // Inicialização do display
  tft.setRotation(2); // Rotação do display (opcional)
  tft.setSwapBytes(true);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH); // Ativa o controle de brilho do display
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);


  exibir_menu();
}

void loop() {

  void vigiaBotao();

  Wire.beginTransmission(ENDERECO_SENSOR);
  Wire.write(0x3B); 
  Wire.endTransmission(false);
  Wire.requestFrom(ENDERECO_SENSOR, 14, true);  

  acelX = Wire.read()<<8|Wire.read(); 

  
}

void exibir_menu() {
  tft.fillScreen(TFT_BLACK); // Limpa a tela

  tft.setCursor(50, 20);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.print("Menu:");

  tft.setCursor(50, 50);
  tft.setTextSize(2);
  tft.setTextColor(opcaoSelecionada == 0 ? TFT_YELLOW : TFT_WHITE);
  tft.print("1. Correr");

  tft.setCursor(50, 80);
  tft.setTextSize(2);
  tft.setTextColor(opcaoSelecionada == 1 ? TFT_YELLOW : TFT_WHITE);
  tft.print("2. Andar");

  tft.setCursor(50, 110);
  tft.setTextSize(2);
  tft.setTextColor(opcaoSelecionada == 2 ? TFT_YELLOW : TFT_WHITE);
  tft.print("3. Cair");
}

void executar_acao() {
  switch (opcaoSelecionada) {
    case 0:

      tft.fillScreen(TFT_BLACK);
      tft.setCursor(50, 80);
      tft.setTextSize(2);
      tft.setTextColor(TFT_WHITE);
      tft.print("deu certo");

     while (1){

      if (client.publish(mqtt_topic, String(acelX).c_str())) {
        Serial.print("Deu certinho certinho");
      } 
      else {
        tft.fillScreen(TFT_BLACK);
        tft.print("n deu certo");
      }
      
      isCounting = true;
      break;

    case 1:
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(50, 80);
      tft.setTextSize(2);
      tft.setTextColor(TFT_WHITE);
      tft.print("testando andar");
      isCounting = true;
      break;

    case 2:
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(50, 80);
      tft.setTextSize(2);
      tft.setTextColor(TFT_WHITE);
      tft.print("testando cair");
      isCounting = true;
      break;
  }
}

void retornar_ao_menu() {
  isCounting = false;
  opcaoSelecionada = 0;
  exibir_menu();
  delay(100);
}

void vigiaBotao(){
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  int reading_bot = digitalRead(bot_button);
  int reading_key = digitalRead(key_button);

  // Lógica para o botão "bot"
  if (reading_bot != lastbot_button_state) {
    lastDebounceTimeBot = millis();
  }

  if ((millis() - lastDebounceTimeBot) > debounceDelay) {
    if (reading_bot != bot_button_state) {
      bot_button_state = reading_bot;

      if (bot_button_state == LOW) {
        // Incrementa a opção selecionada
        opcaoSelecionada = (opcaoSelecionada + 1) % 3;
        exibir_menu(); // Atualiza a tela apenas quando a opção é alterada
        delay(100);
      }
    }
  }

  lastbot_button_state = reading_bot;

  // Lógica para o botão "key"
  if (reading_key != lastkey_button_state) {
    lastDebounceTimeKey = millis();
  }

  if ((millis() - lastDebounceTimeKey) > debounceDelay) {
    if (reading_key != key_button_state) {
      key_button_state = reading_key;

      if (key_button_state == LOW) {
        if (isCounting) {
          // Se já estiver contando, volte para o menu
          retornar_ao_menu();
        } else {
          // Senão, execute a ação correspondente à opção selecionada
          executar_acao();
        }

        delay(100);
      }
    }
  }

  lastkey_button_state = reading_key;
}