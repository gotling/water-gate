char message[100];
String clientId = "ESP32Watergate";

void callback(char* topic, byte* payload, unsigned int length) {
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");
  payload[length] = '\0';

  if (strcmp("gotling/feeds/karnhult.button", topic) == 0) {
    Serial.print("Topic: button Value: ");
    Serial.println(atoi((char *)payload));
    switch (atoi((char *)payload)) {
      case 11:
        Serial.println("Remote Action: Start pump");
        pump(true);
        break;
      case 12:
        Serial.println("Remote Action: Enable nutrition");
        actionNut = true;
        digitalWrite(LED_ACTION, HIGH);
        break;
    }
  }
}

void setupWiFi() {  
  //myDisplay.setFont(nullptr);
  Serial.print("Connecting to: " + String(WIFI_NAME));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);


  //esp_wifi_set_mode(WIFI_MODE_STA);
  //esp_wifi_start();
  //Serial.println(esp_wifi_start());
  //WiFi.disconnect();
  //WiFi.reconnect();

  short tick = 0;
  short timeout_counter = 0;

  led(true);
  while(WiFi.status() != WL_CONNECTED) 
  {
    delay(500);

    switch(tick % 2) {
      case 0:
        led(false);
        break;
      case 1:
        led(true);
        break;
    }
    Serial.print(".");
    tick++;

    timeout_counter++;
    if(timeout_counter >= 120){
      break;
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("\nConnected! IP address: "));
    Serial.println(WiFi.localIP());
    led(false);
  } else {
    Serial.println("");
    Serial.println("Failed to connect to WiFi");
    //ESP.restart();
  }
  
  // WiFi & Config
  //WiFi.mode(WIFI_STA);
  //wm.resetSettings();
  // wm.setConfigPortalBlocking(false);
  // wm.setConfigPortalTimeout(120);
  // //wm.setConnectTimeout(30);
  // //wm.setParamsPage(true);
  // wm.setConnectTimeout(10);
  // wm.setConnectRetries(6);
  // //if (wm.getWiFiIsSaved())
  // //  wm.setEnableConfigPortal(false);

  // // invert theme, dark
  // wm.setDarkMode(true);
  // // set Hostname
  // //wm.setHostname("Watergate");
  // wm.setBreakAfterConfig(true);

  // if (wm.autoConnect("Watergate")) {
  //     Serial.println("Connected to WiFi");
  //     ledBlink(0);
  // } else {
  //     Serial.println("Configuration portal running");
  //     ledBlink(2000);
  // }
  //wm.startConfigPortal("Watergate");

  clientId = "ESP32Client-";
  clientId += String(random(0xffffff), HEX);
  // Serial.print("Client ID: ");
  // Serial.println(clientId.c_str());
  // Serial.println("DEBUG MQTT");
  // Serial.println(MQTT_USER);
  // Serial.println("END DEBUG MQTT");

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);
}

bool mqttConnect() {
  client.setInsecure();

  if (mqtt.connected()) {
    return true;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, skipping MQTT");
    return false;
  }

  Serial.print("Connecting to MQTT... ");
  if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
    Serial.println("MQTT Connected!");
    mqtt.subscribe("gotling/feeds/karnhult.button");
    return true;
  } else {
    Serial.print("MQTT connection failed, rc=");
    Serial.println(mqtt.state());
    return false;
  }
}

void mqttSend() {
  sprintf(message, "temperature,%.1f\nhumidity,%d\nvoltage,%.1f\nhyg1,%.1f\nhyg2,%.1f\nhyg3,%.1f\nsoil_temperature,%.1f\nlevel,%d", temperature, humidity, voltage, hyg1, hyg2, hyg3, soilTemperature, level);

  if (mqttConnect()) {
    if (mqtt.publish(MQTT_TOPIC, message)) {
      //Serial.println("MQTT send OK!");
    } else {
      Serial.println("MQTT send Failed"); 
    }
  }
}

void mqttSendEvent(Event event, short value) {
  switch (event) {
    case ACTION:
      sprintf(message, "button,%d\n", value);
      break;
    case LED:
      sprintf(message, "led,%d\n", value);
      break;
    case NUT:
      sprintf(message, "nut,%d\n", value);
      break;
    case PUMP:
      sprintf(message, "pump,%d\n", value);
      break;
    case LEVEL:
      sprintf(message, "level,%d\n", value);
      break;
    case TIMER:
      sprintf(message, "timer,%d\n", value);
      break;
    default:
      Serial.println("MQTT send event: Unknown Event");      
  }
  
  if (mqttConnect()) {
    if (mqtt.publish(MQTT_TOPIC, message)) {
      Serial.print("MQTT send event OK! ");
      Serial.print(message);      
    } else {
      Serial.println("MQTT send event Failed"); 
    }
  }
}