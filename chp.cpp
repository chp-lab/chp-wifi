#include "chp.h"

unsigned long last_wake_up = 0;
unsigned long time_elapsed = 0;
unsigned long sync_time = SYNC_TIME;
unsigned long last_sync = 0;
bool start_up = true;
unsigned long schedule = INTERVAL;
bool rt_req = false;

WiFiClient client;
PubSubClient mqtt(client);
byte mac[32]; 
float E[NUM_PHASE];
String client_name = "default";
String meter_id = INFLUX_DOOR;
String location = LOCATION;
int timezone = 7 * 3600;
int dst = 0;
int num_prepare = NUM_PREPARE;
int prepare = PREPARE;

String rt_key = RT_TOPIC;

String mqtt_server = MQTT_SERVER;
int mqtt_port = MQTT_PORT;
String mqtt_username = MQTT_USERNAME;
String mqtt_password = MQTT_PASSWORD;
int mqtt_max_reconnect = MQTT_MAX_RECONNECT;

bool clid_flag = false;
String ota_pwd = OTA_PWD;

void callback(char* topic, byte* payload, unsigned int length) 
{
	payload[length] = '\0';
	String topic_str = topic, payload_str = (char*)payload;
	Serial.println("### [" + topic_str + "]: " + payload_str);
	String rt_topic = rt_key + "/" + device_id();
	if(topic_str == rt_topic)
	{
		rt_req = true;
	}
}

void pubData(String payload, String topic)
{
    char msg1[255];
    char ch_topic[32];
    
    payload.toCharArray(msg1, payload.length() + 1);
    topic.toCharArray(ch_topic, topic.length() + 1);
//    Serial.print("Publish message: ");
//	  Serial.println(msg1);
//    Serial.println("-----");
    
    mqtt.publish(ch_topic, msg1);
    delay(100);
}

void mqtt_connect()
{
  int count = 0;
  mqtt.setServer(mqtt_server.c_str(), mqtt_port);
  mqtt.setCallback(callback);
  Serial.println("Connecting to mqtt broker");
  
  while(!mqtt.connect(client_name.c_str(), mqtt_username.c_str(), mqtt_password.c_str()))
  {
  	set_mqtt_flag(false);
  	chp_wifi_handle();
    Serial.print("*.");
    count = count + 1;
    if(count > mqtt_max_reconnect)
    {
        Serial.println("Internet connection failed, restarting device...");
        sudo_reboot();
    }
    delay(1000);
  }
  set_mqtt_flag(true);
  Serial.println("Mqtt connected");
  mqtt.subscribe(TOPIC);
  String rt_topic = rt_key + "/" + device_id();
  Serial.println("rt_topic=" + rt_topic);
  sub_data(rt_topic);
}

void giantOta()
{
    Serial.println("Starting ota...");
    ArduinoOTA.setHostname(get_client_name().c_str());
    ArduinoOTA.setPassword(ota_pwd.c_str());
    ArduinoOTA.onStart([]() 
    {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
      {
        type = "sketch";
      }
      else
      { // U_FS
        type = "filesystem";
      }

      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]()
    {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
    {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) 
    {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) 
      {
        Serial.println("Auth Failed");
      } 
      else if (error == OTA_BEGIN_ERROR) 
      {
        Serial.println("Begin Failed");
      } 
      else if (error == OTA_CONNECT_ERROR) 
      {
        Serial.println("Connect Failed");
      }
      else if (error == OTA_RECEIVE_ERROR)
      {
        Serial.println("Receive Failed");
      }
      else if (error == OTA_END_ERROR)
      {
        Serial.println("End Failed");
      }
    });
    ArduinoOTA.begin();
    Serial.println("ota started");
}

String get_client_name()
{
  String tmp_name = "device";
  int mac_len = 6;
//  char buf[mac_len];

  WiFi.macAddress(mac);
  
  Serial.print("MAC: ");
  for(int i = 0; i < mac_len; i++ )
  {
    char buf[6];
    sprintf(buf, "%2X", mac[i]);
    tmp_name = tmp_name + "_" + buf;
  }

  Serial.println(tmp_name);
  return tmp_name;
}

String randomMessage()
{
//  String json_str = "{\"V\":[218.75,220.30,213.8],\"I\":[10.01,11.13,12.22],\"P\":[1313.81,1716.35,1698.21],\"E\":[1000,1001,1002],\"f\":[49.52,48.9,50.11],\"pf\":[0.6,0.7,0.65]}"
  
  float V[NUM_PHASE], I[NUM_PHASE], P[NUM_PHASE], f[NUM_PHASE], pf[NUM_PHASE];
  int i;

  // Voltage
  String json_str = "{\"V\":[";
  for(i = 0; i < NUM_PHASE; i++)
  {
    V[i] = 210 + random(10);
    json_str = json_str + V[i];
    if(i < NUM_PHASE - 1)
    {
      json_str = json_str + ",";
    }
//    Serial.println("V[" + String(i) + "]= " + String(V[i]) + " V");
  }

  // Current
  json_str = json_str + "],\"I\":[";
  for(i = 0; i < NUM_PHASE; i++)
  {
    I[i] = 10 + 3*random(0, 100)/100.00;
    json_str = json_str + I[i];
    if(i < NUM_PHASE - 1)
    {
      json_str = json_str + ",";
    }
//    Serial.println(I[i]);
  }


  // Power factor
  json_str = json_str + "],\"pf\":[";
  for(i = 0; i < NUM_PHASE; i++)
  {
    pf[i] = random(70, 100)/100.00;
    json_str = json_str + pf[i];
    if(i < NUM_PHASE - 1)
    {
      json_str = json_str + ",";
    }
//    Serial.println(pf[i]);
  }


  // Power in Watt
  json_str = json_str + "],\"P\":[";
  for(i = 0; i < NUM_PHASE; i++)
  {
    P[i] = I[i]*V[i]*pf[i];
    json_str = json_str + P[i];
    if(i < NUM_PHASE - 1)
    {
      json_str = json_str + ",";
    }
//    Serial.println(P[i]);
  }

  // frequency in Hz
  json_str = json_str + "],\"f\":[";
  for(i = 0; i < NUM_PHASE; i++)
  {
    f[i] = random(48, 50);
    json_str = json_str + f[i];
    if(i < NUM_PHASE - 1)
    {
      json_str = json_str + ",";
    }
//    Serial.println(f[i]);
  }

  // Energy in kWhr
  json_str = json_str + "],\"E\":[";
  for(i = 0; i < NUM_PHASE; i ++)
  {
    float interval = schedule/1000.00;
    interval = interval/(60*60.00);
    E[i] = E[i] + interval*P[i]/1000.00;
    json_str = json_str + E[i];
    if(i < NUM_PHASE - 1)
    {
      json_str = json_str + ",";
    }
//    Serial.println(E[i]);
  }
  json_str = json_str + "]}";
//  Serial.println(json_str);
  return json_str;
}

#ifdef DEBUG_MODE
String uart_read()
{
  return randomMessage();
}
#else
String uart_read()
{
  int incoming_msg;
  String buf_msg;
  while (Serial.available() > 0) {
    // read the incoming byte:
    incoming_msg = Serial.read();
    buf_msg = buf_msg + char(incoming_msg);
  }
//  Serial.println(buf_msg);
  return buf_msg;
}
#endif

String influx_inline(String j_str)
{
	// deserialize json data from uart
//  StaticJsonDocument<2000> doc;
  DynamicJsonDocument doc(2000);
  
  DeserializationError error = deserializeJson(doc, j_str);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return "falied";
  }
  /*
   * 1. V
   * 2. I
   * 3. pf
   * 4. P,
   * 5. f
   * 6. E
   */
  float V[NUM_PHASE];
  float I[NUM_PHASE];
  float pf[NUM_PHASE];
  float P[NUM_PHASE];
  float f[NUM_PHASE];
  float E[NUM_PHASE];
  int i;

  // V
  for(i = 0; i < NUM_PHASE; i++)
  {
    V[i] = doc["V"][i];
//    Serial.println("V[" + String(i) + "]=" + String(V[i]));
  }

  // I
  for(i = 0; i < NUM_PHASE; i++)
  {
    I[i] = doc["I"][i];
//    Serial.println("I[" + String(i) + "]=" + String(I[i]));
  }

  // pf
  for(i = 0; i < NUM_PHASE; i++)
  {
    pf[i] = doc["pf"][i];
//    Serial.println("pf[" + String(i) + "]=" + String(pf[i]));
  }

  // P
  for(i = 0; i < NUM_PHASE; i++)
  {
    P[i] = doc["P"][i];
//    Serial.println("P[" + String(i) + "]=" + String(P[i]));
  }

  // f
  for(i = 0; i < NUM_PHASE; i++)
  {
    f[i] = doc["f"][i];
//    Serial.println("f[" + String(i) + "]=" + String(f[i]));
  }

  // E
  for(i = 0; i < NUM_PHASE; i++)
  {
    E[i] = doc["E"][i];
//    Serial.println("E[" + String(i) + "]=" + String(E[i]));
  }

  String ts = get_time();
  String influx_msg = meter_id + ",location=" + location + " V0=" + String(V[0]) + ",V1=" + String(V[1]) + ",V2=" + String(V[2]) + ",I0=" + String(I[0]) + ",I1=" + String(I[1]) + ",I2=" + String(I[2]) + \
  ",pf0=" + String(pf[0]) + ",pf1=" + String(pf[1]) + ",pf2=" + String(pf[2]) + ",P0=" + String(P[0]) + ",P1=" + String(P[1]) + ",P2=" + String(P[2]) + ",f0=" + String(f[0]) + ",f1=" + String(f[1]) + ",f2=" + String(f[2]) + \
  ",E0=" + String(E[0]) + ",E1=" + String(E[1]) + ",E2=" + String(E[2]) + " " + ts;
//  Serial.println("influx_msg=" + influx_msg);
  return influx_msg;
}

String get_time()
{
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  String cur_ts = String(now) + "000000000";
  Serial.println("cur_ts=" + cur_ts);
  return cur_ts;
}

void chp_init(bool en_log)
{

	randomSeed(analogRead(0));
  
	Serial.println(MODEL_NAME);
	Serial.println(FW_VERSION);
  
  	if(!clid_flag)
  	{
  		client_name = get_client_name();
  		Serial.println("Use default client name!");
	}
  	
  	if(!en_log)
  	{
  		Serial.println("Log disabled");
  		meter_id = "";
	}
  	meter_id = meter_id + client_name;
  	Serial.println("client_name=" + client_name);
  	Serial.println("device_id=" + meter_id);

    giantOta();
    mqtt_connect();

  	last_wake_up =  millis();
  	Serial.println("Sync time ***");
  	configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  	while (!time(nullptr)) {
    	Serial.print(".");
    	delay(1000);
  	}
  	last_sync = millis();
  	Serial.println("Preparing...");
  	Serial.println("ts=" + get_time());
  	int i;
  	// Clear uart
  	for(i=0; i < num_prepare; i++)
  	{
    	String msg = uart_read();
    	delay(prepare);
//    	Serial.println("ts=" + String(i) + " " + get_time());
  	}
  	Serial.println("Success, waiting for schedule");
}

bool time_to_sync()
{
	if((millis() - last_sync) > sync_time)
	{
//		Serial.println("Sync time");
    	configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
    	last_sync = millis();
		return true;
	}
	else
	{
		return false;
	}
}

void listen_for_fw()
{
	ArduinoOTA.handle();
}

void chp_loop()
{
	time_elapsed = millis() - last_wake_up;
    if(!mqtt.connected())
    {
      	Serial.println("On idel state, mqtt client not connect");
      	mqtt_connect();
    }
    mqtt.loop();
}

bool time_to_send()
{
	if((abs(time_elapsed) > schedule) || start_up)
	{
		start_up = false;
//		Serial.println("Time elapsed=" + String(time_elapsed));
    	last_wake_up = millis();
    	time_elapsed = 0;
    	if(!mqtt.connected())
    	{
      		Serial.println("mqtt client not connect");
      		mqtt_connect();
    	}
    	mqtt.loop();
		return true;	
	}
	else
	{
		return false;
	}
}

String device_id()
{
	return meter_id;
}

unsigned long get_sync_time()
{
	return sync_time;
}

void set_sync_time(unsigned long sync_t)
{
	sync_time = sync_t;
}

unsigned long get_interval()
{
	return schedule;
}

void set_interval(unsigned long sch_t)
{
	schedule = sch_t;
}

void sub_data(String topic)
{
  mqtt.subscribe(topic.c_str());
}

bool real_time_req()
{
	bool tmp_rt_req = rt_req;
	rt_req = false;
	return tmp_rt_req;
}

void set_mqtt(String m_server, int m_port, String m_clid, String m_uname, String m_pwd)
{
	mqtt_server = m_server;
	client_name = m_clid;
	mqtt_username = m_uname;
	mqtt_password = m_pwd;
	mqtt_port = m_port;
	
	clid_flag = true;
}

void use_saved_config()
{
	Mqtt_config s_mqtt_config = get_mqtt_config();
	
	mqtt_server = s_mqtt_config.mqtt_server;
	client_name = s_mqtt_config.client_name;
	mqtt_username = s_mqtt_config.mqtt_username;
	mqtt_password = s_mqtt_config.mqtt_password;
	mqtt_port = s_mqtt_config.mqtt_port;
	schedule = s_mqtt_config.interval*1000;
	
	Serial.println("### schedule=" + String(schedule));
	
	clid_flag = true;
}

void set_ota_pwd(String my_ota_pwd)
{
	ota_pwd = my_ota_pwd;
}

