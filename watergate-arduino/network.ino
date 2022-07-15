char message[100];
String clientId = "ESP32Watergate";

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  payload[length] = '\0';

  if (strcmp("gotling/feeds/karnhult.button", topic) == 0) {
    Serial.println("PAYLOAD");
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
  // WiFi & Config
  WiFi.mode(WIFI_STA);
  //wm.resetSettings();
  wm.setConfigPortalBlocking(false);
  wm.setConfigPortalTimeout(60);
  //wm.setParamsPage(true);

  // invert theme, dark
  wm.setDarkMode(true);
  // set Hostname
  //wm.setHostname("Watergate");
  wm.setBreakAfterConfig(true);

  if (wm.autoConnect("Watergate")) {
      Serial.println("Connected to WiFi");
  }
  else {
      Serial.println("Configuration portal running");
  }
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
  sprintf(message, "temperature,%.1f\nhumidity,%d\nvoltage,%.1f\nhyg1,%.1f\nhyg2,%.1f\nhyg3,%.1f\nsoil_temperature,%.1f", temperature, humidity, voltage, hyg1, hyg2, hyg3, soilTemperature);

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
    case BUTTON:
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