#include "chpWifi.h"

//static const char PAGE_AUTH[] PROGMEM = R"(
//{
//  "uri": "/auth",
//  "title": "Auth",
//  "menu": true,
//  "element": [
//    {
//      "name": "text",
//      "type": "ACText",
//      "value": "AutoConnect has authorized",
//      "style": "font-family:Arial;font-size:18px;font-weight:400;color:#191970"
//    }
//  ]
//}
//)";

#if defined(ARDUINO_ARCH_ESP8266)
ESP8266WebServer Server;
#elif defined(ARDUINO_ARCH_ESP32)
WebServer Server;
#endif

const char* ap_ssid = "ESP-AP";
const char* ap_pwd = AP_PWD_FK;
const unsigned long ap_timeout = 10*60*1000;
String ap_prefix = "ESP-AP-";

static const char OTA[] PROGMEM = R"(
[
  {
    "title": "Update",
    "uri": "/update",
    "menu": true,
    "element": [
      {
        "name": "chk",
        "type": "ACSubmit",
        "value": "Check Update",
        "uri": "/chk"
      }
    ]
  },
  {
    "title": "Update",
    "uri": "/chk",
    "menu": false,
    "element": [
      {
        "name": "res",
        "type": "ACText"
      }
    ]
  }
]
)";

AutoConnect       Portal(Server);
AutoConnectConfig Config(ap_ssid, ap_pwd, ap_timeout);       // Enable autoReconnect supported on v0.9.4
char* updateServerUrl = ota_server_uri;
const int   updateServerPort = ota_port;
char* updateFirmware = ota_fw_name;
AutoConnectUpdate update(updateServerUrl, updateServerPort);
AutoConnectAux    Timezone;

int index_server_url = URL_START;    
int index_server_url_end = index_server_url + URL_LEN;

int index_client_id  = index_server_url_end;  
int index_client_id_end = index_client_id + CLID_LEN;

int index_token      = index_client_id_end;  
int index_token_end = index_token + TOKEN_LEN;

int index_secret     = index_token_end;  
int index_secret_end = index_secret + SECRET_LEN;

int index_port       = index_secret_end;  
int index_port_end = index_port + PORT_LEN;

int index_interval   = index_port_end;  
int index_interval_end = index_interval + INTERVAL_LEN;
int index_room_num = index_interval_end;
int index_room_num_end = index_room_num + ROOM_NUM_LEN;

bool __eprm_flag = false;
bool __mqtt_flag = false;

int mode_pin = MODE_PIN;
int wifi_led_pin = 10;

AutoConnectAux    netpie("/netpie", "MQTT");   // Step #1 as the above procedure

static const char AUX_TIMEZONE[] PROGMEM = R"(
{
  "title": "TimeZone",
  "uri": "/timezone",
  "menu": true,
  "element": [
    {
      "name": "caption",
      "type": "ACText",
      "value": "Sets the time zone to get the current local time.",
      "style": "font-family:Arial;font-weight:bold;text-align:center;margin-bottom:10px;color:DarkSlateBlue"
    },
    {
      "name": "timezone",
      "type": "ACSelect",
      "label": "Select TZ name",
      "option": [],
      "selected": 10
    },
    {
      "name": "newline",
      "type": "ACElement",
      "value": "<br>"
    },
    {
      "name": "start",
      "type": "ACSubmit",
      "value": "OK",
      "uri": "/start"
    }
  ]
}
)";

typedef struct 
{
	const char* zone;
	const char* ntpServer;
	int8_t      tzoff;
} Timezone_t;

static const Timezone_t TZ[] = 
{
	{ "Europe/London", "europe.pool.ntp.org", 0 },
	{ "Europe/Berlin", "europe.pool.ntp.org", 1 },
	{ "Europe/Helsinki", "europe.pool.ntp.org", 2 },
	{ "Europe/Moscow", "europe.pool.ntp.org", 3 },
	{ "Asia/Dubai", "asia.pool.ntp.org", 4 },
	{ "Asia/Karachi", "asia.pool.ntp.org", 5 },
	{ "Asia/Dhaka", "asia.pool.ntp.org", 6 },
	{ "Asia/Jakarta", "asia.pool.ntp.org", 7 },
	{ "Asia/Manila", "asia.pool.ntp.org", 8 },
	{ "Asia/Tokyo", "asia.pool.ntp.org", 9 },
	{ "Australia/Brisbane", "oceania.pool.ntp.org", 10 },
	{ "Pacific/Noumea", "oceania.pool.ntp.org", 11 },
	{ "Pacific/Auckland", "oceania.pool.ntp.org", 12 },
	{ "Atlantic/Azores", "europe.pool.ntp.org", -1 },
	{ "America/Noronha", "south-america.pool.ntp.org", -2 },
	{ "America/Araguaina", "south-america.pool.ntp.org", -3 },
	{ "America/Blanc-Sablon", "north-america.pool.ntp.org", -4},
	{ "America/New_York", "north-america.pool.ntp.org", -5 },
	{ "America/Chicago", "north-america.pool.ntp.org", -6 },
	{ "America/Denver", "north-america.pool.ntp.org", -7 },
	{ "America/Los_Angeles", "north-america.pool.ntp.org", -8 },
	{ "America/Anchorage", "north-america.pool.ntp.org", -9 },
	{ "Pacific/Honolulu", "north-america.pool.ntp.org", -10 },
	{ "Pacific/Samoa", "oceania.pool.ntp.org", -11 }
};

void rootPage() 
{
  String __home_status = "<span class=\"ant-avatar ant-avatar-circle\" style=\"width: 20px; height: 20px; line-height: 20px; font-size: 18px; background-color:#909b8e;\"></span></span> Offline";;

  if(__mqtt_flag)
  {
  	__home_status = "<span class=\"ant-avatar ant-avatar-circle\" style=\"width: 20px; height: 20px; line-height: 20px; font-size: 18px; background-color:#20d406;\"></span> Online";
  
  }
  
  String  content =
    "<html>"
      "<head>"
          "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
          "<title>Dashboard</title>"
          "<link rel=\"stylesheet\" type=\"text/css\" href=\"https://portal.netpie.io/static/css/2.d4320d25.chunk.css\">"
          "<link rel=\"styleshee\" type=\"text/css\" href=\"https://portal.netpie.io/static/css/2.d4320d25.chunk.css\">"
          "<link rel=\"shortcut icon\" href=\"https://portal.nexpie.io/nexpie_logo_icon.png\" data-rh=\"true\">"
          "<script type=\"text/javascript\">"
//            "setTimeout(\"location.reload()\", 60000);"
          "</script>"
      "</head>"
      "<body>"
          "<h2 align=\"center\" style=\"color:blue;margin:20px;\">WiFi Setup V" WIFI_CONF_V "</h2>"
          "<h3 align=\"center\" style=\"color:gray;margin:10px;\">{{DateTime}}</h3>"
          "<p style=\"text-align:center;\">"+ __home_status +"</p>"
			"<form action=\"/_ac/connect\" align=\"center\" style=\"color:blue;margin:20px;>"
			  "<label for=\"SSID\">SSID</label><br>"
			  "<input type=\"text\" id=\"SSID\" name=\"SSID\" value=\"\"><br>"
			  "<label for=\"Passphrase\">Password</Passphrase><br>"
			  "<input type=\"text\" id=\"Passphrase\" name=\"Passphrase\" value=\"\"><br><br>"
			  "<input type=\"submit\" value=\"Connect\">"
			"</form>"
			"<p></p><p style=\"padding-top:15px;text-align:center\">" AUTOCONNECT_LINK(COG_24) "</p>"
      "</body>"
    "</html>";
  static const char *wd[7] = { "Sun","Mon","Tue","Wed","Thr","Fri","Sat" };
  struct tm *tm;
  time_t  t;
  char    dateTime[26];

  t = time(NULL);
  tm = localtime(&t);
  sprintf(dateTime, "%04d/%02d/%02d(%s) %02d:%02d:%02d.",
    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
    wd[tm->tm_wday],
    tm->tm_hour, tm->tm_min, tm->tm_sec);
  content.replace("{{DateTime}}", String(dateTime));
  Server.send(200, "text/html", content);
}

void startPage() 
{
	// Retrieve the value of AutoConnectElement with arg function of WebServer class.
	// Values are accessible with the element name.
	String  tz = Server.arg("timezone");
	
	for (uint8_t n = 0; n < sizeof(TZ) / sizeof(Timezone_t); n++) 
	{
		String  tzName = String(TZ[n].zone);
		if (tz.equalsIgnoreCase(tzName)) 
		{
		  configTime(TZ[n].tzoff * 3600, 0, TZ[n].ntpServer);
		  Serial.println("Time zone: " + tz);
		  Serial.println("ntp server: " + String(TZ[n].ntpServer));
		  break;
		}
	}
	
	// The /start page just constitutes timezone,
	// it redirects to the root page without the content response.
	Server.sendHeader("Location", String("http://") + Server.client().localIP().toString() + String("/"));
	Server.send(302, "text/plain", "");
	Server.client().flush();
	Server.client().stop();
}

void connect_to_myap(String my_ssid, String my_password)
{
	#if defined(ARDUINO_ARCH_ESP32)
	Serial.println("Connecting to new ap...");
	HTTPClient http;
	String serverPath = "http://localhost/_ac/connect";
	http.begin(serverPath.c_str());
	http.addHeader("Content-Type", "application/x-www-form-urlencoded");
	String httpRequestData = "SSID=" + my_ssid + "&Passphrase=" + my_password + "&dhcp=en";
	int httpResponseCode = http.POST(httpRequestData);
	Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
	// Free resources
	http.end();
	#elif defined(ARDUINO_ARCH_ESP8266)
	Serial.println("esp8266 not support connect_to_myap!!");
	#endif
}

void chp_wifi_begin()
{
	// Enable saved past credential by autoReconnect option,
	// even once it is disconnected.
	Config.autoReconnect = true;
//	Config.portalTimeout = 3000;
	/*Http Authentication*/
//	Config.auth = AC_AUTH_DIGEST;
//	Config.authScope = AC_AUTHSCOPE_PORTAL;
//	Config.username = "admin";
//	Config.password = "123454";
//	String ble_wifi_config = EEPROM_read(450, 500);
//	if(ble_wifi_config != "")
//	{
//		Serial.println("!!! ble wifi config found!");
//	}
//	// Check any setting available
	
	Config.apid = ap_prefix + get_ap_name();
	Config.autoRise = true; // It's the default, no setting is needed explicitly.
	Config.retainPortal = true;
//	Config.password = "farmkids";
	
//	Serial.println("AP SSID:" + String(Config.apid));
	
	Portal.config(Config);
//	Portal.load(FPSTR(PAGE_AUTH));
	if(!__eprm_flag)
	{
		EEPROM.begin(EEPROM_SIZE);
		__eprm_flag = true;	
	}
	
	// Load aux. page
	Timezone.load(AUX_TIMEZONE);
	// Retrieve the select element that holds the time zone code and
	// register the zone mnemonic in advance.
	AutoConnectSelect&  tz = Timezone["timezone"].as<AutoConnectSelect>();
	for (uint8_t n = 0; n < sizeof(TZ) / sizeof(Timezone_t); n++) 
	{
		tz.add(String(TZ[n].zone));
	}
	
	Portal.join({ Timezone });        // Register aux. page
	Portal.join({ netpie });
	
	// Behavior a root path of ESP8266WebServer.
	Server.on("/", rootPage);
	Server.on("/start", startPage);   // Set NTP server trigger handler
	Server.on("/netpie", handleNetpie);
  	Server.on("/dvsetup", dvSetup);
	
	// Establish a connection with an autoReconnect option.
	Serial.println("Connecting to WiFi...");
	
	// change_to_scanner(true);
	
	digitalWrite(wifi_led_pin, LOW);
	
	if (Portal.begin()) 
	{	
		update.attach(Portal);
		Portal.on("/chk", onUpdate);
		Serial.println("WiFi connected: " + WiFi.localIP().toString());
		digitalWrite(wifi_led_pin, HIGH);
		String tmp_md_name;
		int mac_len = 6;
		byte tmp_mac[mac_len]; 
		WiFi.macAddress(tmp_mac);
  		Serial.print("MAC: ");
  		
		/*
  		for(int i = 0; i < mac_len; i++ )
  		{
    		char tmp_buf[mac_len];
			//    Serial.println("mac[" + String(i) + "]=" + String(mac[i]));
    		sprintf(tmp_buf, "%2X", tmp_mac[i]);
    		tmp_md_name = tmp_md_name + tmp_buf;
    		tmp_md_name.replace(" ", "0");
  		}
		*/
		tmp_md_name = get_ap_name();
		Serial.println("MDNS Started with name:" + tmp_md_name);
		
  		Serial.println(tmp_md_name);
		if (MDNS.begin(tmp_md_name.c_str())) 
		{
			MDNS.addService("http", "tcp", 80);
		}
		else
		{
			Serial.println("### MDNS failed");
		}
	}
	else
	{
		Serial.println("Portal begin failed, end the portal!");
		Portal.end();
		sudo_reboot();
	}
//	digitalWrite(10, HIGH);
}

void chp_wifi_handle()
{
	Portal.handleClient();
}

void EEPROM_write(int index, String text) 
{
  for (int i = index; i < text.length() + index; i++) 
  {
    EEPROM.put(i, text[i - index]);
  }
    EEPROM.commit();
}

String EEPROM_read(int index, int read_end) 
{
  String text = "";
  char ch;
  for (int i = index; i < read_end; i++) {
    if (ch = EEPROM.read(i)) {
      text.concat(ch);
    }
  }
  text.replace("�", "");
  return text;
}

void Reset_EEPROM()
{
    for (int i = URL_START; i < CHP_EEPROM_SCOPE; i++) {
        EEPROM.put(i, 0);
    }
    if(EEPROM.commit()){
      Serial.println("Reset pass");
    }
    delay(500);
}

void dvSetup()
{
  String new_server_url = Server.arg("server_uri");
  String new_client_id = Server.arg("clid");
  String new_token = Server.arg("token");
  String new_secret = Server.arg("secret");
  String new_port = Server.arg("port");
  String new_interval = Server.arg("interval");
  String room_num = Server.arg("room_num");

  Server.send(200, "text/html", String(F(
                                         "<html>"
                                         "<head>"
                                         "<meta name='viewport' content='width=device-width,initial-scale=1.0'>"
                                         "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\" integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\" crossorigin=\"anonymous\">"
                                         "<link rel=\"stylesheet\" type=\"text/css\" href=\"https://portal.netpie.io/static/css/2.d4320d25.chunk.css\">"
										 "<link rel=\"styleshee\" type=\"text/css\" href=\"https://portal.netpie.io/static/css/2.d4320d25.chunk.css\">"
										 "<link rel=\"shortcut icon\" href=\"https://portal.nexpie.io/nexpie_logo_icon.png\" data-rh=\"true\">"
										 "</head>"
                                         "<body>"
                                             "<div class=\"container\">\n"
                                              "<div class=\"row\">"
                                                "<div class=\"col-3\"></div>"
                                                    "<div class=\"col-6\">"
                                                        "<br><h3 style=\"color:#6633FF;margin:20px; text-align: center;\">Configuration saved, please restart your device...</h3>"
                                                       "<div class=\"d-flex justify-content-center\">"
                                                          "<form action=\"/_ac\">"
                                                          "<input type=\"submit\" value=\"AP configuration\" class=\"btn btn-primary\">"
                                                          "</form>"
                                                       "</dev>"
                                                    "</div>"
                                               "<div class=\"col-3\"></div>"
                                            "</div>"
                                         "</body>"
                                         "</html>"
                                       )));

  Reset_EEPROM();
  EEPROM_write(index_server_url, new_server_url);
  EEPROM_write(index_client_id, new_client_id);
  EEPROM_write(index_token, new_token);
  EEPROM_write(index_secret, new_secret);
  EEPROM_write(index_port, new_port);
  EEPROM_write(index_interval, new_interval);
  EEPROM_write(index_room_num, room_num);
}

void handleNetpie() 
{
  String server_url = EEPROM_read(index_server_url, index_server_url_end);
  String Client_id = EEPROM_read(index_client_id, index_client_id_end);
  String username = EEPROM_read(index_token, index_token_end);
  String password = EEPROM_read(index_secret, index_secret_end);
  String port = EEPROM_read(index_port, index_port_end);
  String interval = EEPROM_read(index_interval, index_interval_end);
  String room_num = get_room_num();
  
//  if(server_url == "" ){server_url = "broker.netpie.io";}
//  if(Client_id == "" ){Client_id = "-";}
//  if(username == "" ){username = "-";}
//  if(password == "" ){password = "-";}
//  if(port == "" ){port = "1883";}
//  if(interval == "" ){interval = "60";}

String tmp_netpie_str = "<html>"
                                         "<head>"
                                         "<meta name='viewport' content='width=device-width,initial-scale=1.0'>"
                                         "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\" integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\" crossorigin=\"anonymous\">"
                                         "<link rel=\"stylesheet\" type=\"text/css\" href=\"https://portal.netpie.io/static/css/2.d4320d25.chunk.css\">"
          								 "<link rel=\"styleshee\" type=\"text/css\" href=\"https://portal.netpie.io/static/css/2.d4320d25.chunk.css\">"
          								 "<link rel=\"shortcut icon\" href=\"https://portal.nexpie.io/nexpie_logo_icon.png\" data-rh=\"true\">"
										 "</head>"
                                         "<script type=\"text/javascript\">"
                                          "console.log(\"test\");"
                                          "function confirmSubmit() {"
                                            "if (confirm(\"Save configuration?\")) {"
                                              "document.getElementById(\"frm_setup\").submit();}"
                                            "return false;}"
                                          "</script>"
                                         "<body>"
                                         "<div class=\"container\">\n"
                                                "<div class=\"row\">"
                                                        "<div class=\"col-3\"></div>"
                                                            "<div class=\"col-6\">"
                                                              "<br><h2 style=\"color:#6633FF;margin:20px; text-align: center;\">MQTT Configurations</h2>"
                                                                   "<form action=\"/dvsetup\" method=\"post\" id=\"frm_setup\">"
                                                                      "<div class=\"form-group\">\n"
                                                                              "<label for=\"exampleFormControlInput1\">Server Uri</label>\n"
                                                                            "<input type=\"text\" id=\"server_uri\" name=\"server_uri\" value=\""+server_url+"\" class=\"form-control\" id=\"exampleFormControlInput1\" maxlength=\"50\" required>"
                                                                       "</div>"
                                                                       "<div class=\"form-group\">\n"
                                                                                "<label for=\"exampleFormControlInput1\">Client ID</label>\n"
                                                                                "<input type=\"text\" id=\"clid\" name=\"clid\" value=\""+Client_id+"\" class=\"form-control\" aria-label=\"Sizing example input\" aria-describedby=\"inputGroup-sizing-sm\" maxlength=\"50\" required>"
                                                                       "</div>"
                                                                       "<div class=\"form-group\">\n"
                                                                               "<label for=\"exampleFormControlInput1\">Username(token)</label>\n"
                                                                               "<input type=\"text\" id=\"token\" name=\"token\" value=\""+username+"\" class=\"form-control\" aria-label=\"Sizing example input\" aria-describedby=\"inputGroup-sizing-sm\" maxlength=\"50\" required>"
                                                                       "</div>"
                                                                       "<div class=\"form-group\">\n"
                                                                               "<label for=\"exampleFormControlInput1\">Password(secret)</label>\n"
                                                                               "<input type=\"text\" id=\"secret\" name=\"secret\"  value=\""+password+"\" class=\"form-control\" aria-label=\"Sizing example input\" aria-describedby=\"inputGroup-sizing-sm\" maxlength=\"50\" required>"
                                                                       "</div>"
                                                                       "<div class=\"form-group\">\n"
                                                                              "<label for=\"exampleFormControlInput1\">Port</label>\n"
                                                                              "<input type=\"text\" id=\"port\" name=\"port\" value=\""+port+"\" class=\"form-control\" aria-label=\"Sizing example input\" aria-describedby=\"inputGroup-sizing-sm\" maxlength=\"50\" required>"
                                                                      "</div>"
                                                                      "<div class=\"form-group\">\n"
                                                                             "<label for=\"exampleFormControlInput1\">Interval(s)</label>\n"
                                                                             "<input type=\"text\" id=\"interval\" name=\"interval\" value=\""+interval+"\" class=\"form-control\" aria-label=\"Sizing example input\" aria-describedby=\"inputGroup-sizing-sm\" maxlength=\"50\" required>"
                                                                      "</div>"
                                                                      "<div class=\"form-group\">\n"
                                                                             "<label for=\"exampleFormControlInput1\">Room number</label>\n"
                                                                             "<input type=\"text\" id=\"room_num\" name=\"room_num\" value=\""+room_num+"\" class=\"form-control\" aria-label=\"Sizing example input\" aria-describedby=\"inputGroup-sizing-sm\" maxlength=\"50\" required>"
                                                                      "</div>"
                                                                      "<div class=\"d-flex justify-content-center\">"
                                                                          "<input type=\"button\" value=\"Save\" onclick=\"confirmSubmit()\" class=\"btn btn-primary\">"
                                                                      "</div>"
                                                                 "</form>"
                                                        "</div>"
                                                    "<div class=\"col-3\"></div>"
                                              "</div>"
                                        "</div>"
                                         "</body>"
                                         "</html>";

  Server.send(200, "text/html", tmp_netpie_str);
}

Mqtt_config get_mqtt_config()
{	
	Mqtt_config mqtt_config;
	
	if(!__eprm_flag)
	{
		EEPROM.begin(EEPROM_SIZE);
		__eprm_flag = true;
	}
	
	mqtt_config.mqtt_server = EEPROM_read(index_server_url, index_server_url_end);
	mqtt_config.client_name = EEPROM_read(index_client_id, index_client_id_end);
	mqtt_config.mqtt_username = EEPROM_read(index_token, index_token_end);
	mqtt_config.mqtt_password = EEPROM_read(index_secret, index_secret_end);
	
	String mqtt_port_str = EEPROM_read(index_port, index_port_end);
	String mqtt_interval_str = EEPROM_read(index_interval, index_interval_end);
	
	mqtt_config.mqtt_port = mqtt_port_str.toInt();
	mqtt_config.interval = mqtt_interval_str.toInt();

	
	if(mqtt_config.mqtt_port == 0)
	{
		mqtt_config.mqtt_port = 1883;
		EEPROM_write(index_port, String(mqtt_config.mqtt_port));
	}
	
	if(mqtt_config.interval == 0)
	{
		mqtt_config.interval = 60;
		EEPROM_write(index_interval, String(mqtt_config.interval));
	}
	
	return mqtt_config;	
}

void set_mqtt_flag(bool __my_flag)
{
	__mqtt_flag = __my_flag;
}

String get_room_num()
{
	String tmp_r_num = EEPROM_read(index_room_num, index_room_num_end);
	return tmp_r_num;
}

void set_broker_connection(String my_server_url, String my_client_id, String my_token, String my_secret, int my_port, int my_interval, String room_num)
{
	Reset_EEPROM();
	EEPROM_write(index_server_url, my_server_url);
	EEPROM_write(index_client_id, my_client_id);
	EEPROM_write(index_token, my_token);
	EEPROM_write(index_secret, my_secret);
	EEPROM_write(index_port, String(my_port));
	EEPROM_write(index_interval, String(my_interval));
	EEPROM_write(index_room_num, room_num);
}

void change_to_scanner(bool cnt_mode)
{
	/*
	pinMode(mode_pin, OUTPUT);
	// if cnt_mode is true, switch to scanner
	if(cnt_mode)
	{
		digitalWrite(mode_pin, LOW);
		Serial.println("!!! Change to finger scan, mode pin= " + String(mode_pin));
	}
	else
	{
		digitalWrite(mode_pin, HIGH);
		Serial.println("!!! Change to control box, mode pin= " + String(mode_pin));
	}
	*/
}

void set_mode_pin(int this_mode_pin)
{
	mode_pin = this_mode_pin;
	Serial.println("set mode_pin to " + String(mode_pin));
}

String get_ap_name()
{
	String tmp_name;
	int obset = 4;
	int mac_len = 6;
	byte this_mac[mac_len]; 
	WiFi.macAddress(this_mac);

	Serial.print("APP Name: ");
	for(int i = obset; i < mac_len; i++ )
	{
		char buf[6];
		//    Serial.println("mac[" + String(i) + "]=" + String(mac[i]));
		sprintf(buf, "%2X", this_mac[i]);
		tmp_name = tmp_name + buf;
		tmp_name.replace(" ", "0");
	}
	Serial.println(tmp_name);
	return tmp_name;
}

String get_mac()
{
	String tmp_name;
	int mac_len = 6;
	byte this_mac[mac_len]; 
	//  char buf[mac_len];
	WiFi.macAddress(this_mac);

	// Serial.println("GET MAC: ");
	for(int i = 0; i < mac_len; i++ )
	{
		char buf[6];
		// Serial.println("this_mac[" + String(i) + "]=" + String(this_mac[i], HEX));
		sprintf(buf, "%2X", this_mac[i]);
		tmp_name = tmp_name + buf;
		tmp_name.replace(" ", "0");
		if((i + 1 < mac_len))
		{
			tmp_name = tmp_name + ":";
		}
	}
	tmp_name.toLowerCase();
	// Serial.println(tmp_name);
	return tmp_name;
}

void eeprom_init() {
	if(!__eprm_flag)
	{
		EEPROM.begin(EEPROM_SIZE);
		Serial.println("eeprom_init...");
		__eprm_flag = true;
	}
}

void reset_wifi_con() {
	Serial.println("Disconnect old WiFi...");
	delay(500);
	
	WiFi.disconnect(true);
	deleteAllCredentials();
	WiFi.begin("0","0");
//	Serial.print("Old ssid:");
//	Serial.println( WiFi.SSID() );
//	Serial.print("Old password:");
//	Serial.println( WiFi.psk() );
	delay(500);
	Serial.println("WiFi disconnected.");
	
	Serial.println("Factory reset complete!");
	delay(500);
	ESP.restart();
    delay(1000);
}

void deleteAllCredentials() {
  AutoConnectCredential credential;
  station_config_t config;
  uint8_t ent = credential.entries();

  Serial.println("AutoConnectCredential deleting");
  if (ent)
    Serial.printf("Available %d entries.\n", ent);
  else {
    Serial.println("No credentials saved.");
    return;
  }

  while (ent) {
    credential.load((int8_t)0, &config);
//	Serial.println(String((const char*)config.ssid) + " record detected");
//	Serial.printf("%s record.\n", (const char*)config.ssid);
	
   if (credential.del((const char*)&config.ssid[0])) {
      Serial.printf("%s deleted.\n", (const char*)config.ssid);
	}
    else {
		Serial.printf("%s failed to delete.\n", (const char*)config.ssid);
	}
	ent = ent - 1;
  }
}

String onUpdate(AutoConnectAux& aux, PageArgument& args) {
  aux["res"].value = "Updating...";
  return String();
}

void apt_update() {
	WiFiClient  u_client;
	t_httpUpdate_return ret = httpUpdate.update(u_client, updateServerUrl, updateServerPort, updateFirmware);
	switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
    }
}

void apt_update(const char* uuri, const int uuport, const char* ufw_name) {
	WiFiClient  u_client;
	t_httpUpdate_return ret = httpUpdate.update(u_client, uuri, uuport, ufw_name);
	switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
    }
}

void set_ap_prefix(String this_prefix) {
	ap_prefix = this_prefix;
}




