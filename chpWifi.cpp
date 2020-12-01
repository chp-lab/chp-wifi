#include "chpWifi.h"

#if defined(ARDUINO_ARCH_ESP8266)
ESP8266WebServer Server;
#elif defined(ARDUINO_ARCH_ESP32)
WebServer Server;
#endif

AutoConnect       Portal(Server);
AutoConnectConfig Config;       // Enable autoReconnect supported on v0.9.4
AutoConnectAux    Timezone;
int index_server_url = 0;    int index_server_url_end = 50;
int index_client_id  = 50;   int index_client_id_end = 100;
int index_token      = 100;  int index_token_end = 150;
int index_secret     = 150;  int index_secret_end = 200;
int index_port       = 200;  int index_port_end = 250;
int index_interval   = 250;  int index_interval_end = 300;
bool __eprm_flag = false;

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
  String  content =
    "<html>"
    "<head>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
    "<script type=\"text/javascript\">"
    "setTimeout(\"location.reload()\", 10000);"
    "</script>"
    "</head>"
    "<body>"
    "<h2 align=\"center\" style=\"color:blue;margin:20px;\">GIANT AP</h2>"
    "<h3 align=\"center\" style=\"color:gray;margin:10px;\">{{DateTime}}</h3>"
    "<p style=\"text-align:center;\">Reload the page to update the time.</p>"
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

void chp_wifi_begin()
{
	// Enable saved past credential by autoReconnect option,
	// even once it is disconnected.
	Config.autoReconnect = true;
	Portal.config(Config);
	if(!__eprm_flag)
	{
		EEPROM.begin(512);
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
	if (Portal.begin()) 
	{
		Serial.println("WiFi connected: " + WiFi.localIP().toString());
	}
}

void chp_wifi_handle()
{
	Portal.handleClient();
}

void EEPROM_write(int index, String text) 
{
  for (int i = index; i < text.length() + index; ++i) 
  {
    EEPROM.put(i, text[i - index]);
  }
    EEPROM.commit();
//    Serial.println("length value: "+String(text.length() + 1));
//   return text.length() + 1;
}

String EEPROM_read(int index, int length) 
{
  String text = "";
  char ch = 1;

  for (int i = index; (i < (index + length)) && ch; ++i) {
    if (ch = EEPROM.read(i)) {
      text.concat(ch);
    }
  }
  return text;
}

void Reset_EEPROM(boolean RESET_EEPROM)
{
  if ( RESET_EEPROM ) {
    for (int i = 0; i < 512; i++) {
        EEPROM.put(i, 0);
    }
    if(EEPROM.commit()){
      Serial.println("Reset pass");
    }
    delay(500);
  }
}

void dvSetup()
{
  String new_server_url = Server.arg("server_uri");
  String new_client_id = Server.arg("clid");
  String new_token = Server.arg("token");
  String new_secret = Server.arg("secret");
  String new_port = Server.arg("port");
  String new_interval = Server.arg("interval");

  Server.send(200, "text/html", String(F(
                                         "<html>"
                                         "<head>"
                                         "<meta name='viewport' content='width=device-width,initial-scale=1.0'>"
                                         "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\" integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\" crossorigin=\"anonymous\">"
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

  Reset_EEPROM(true);
  EEPROM_write(index_server_url, new_server_url);
  EEPROM_write(index_client_id, new_client_id);
  EEPROM_write(index_token, new_token);
  EEPROM_write(index_secret, new_secret);
  EEPROM_write(index_port, new_port);
  EEPROM_write(index_interval, new_interval);

//  Serial.println("server_uri=" + new_server_url);
//  Serial.println("client_id=" + new_client_id);
//  Serial.println("token=" + new_token);
//  Serial.println("secret=" + new_secret);
//  Serial.println("port=" + new_port);
//  Serial.println("interval=" + new_interval);
//  
//  Serial.println("server_uri read: " + EEPROM_read(index_server_url, index_server_url_end));
//  Serial.println("client_id read: " + EEPROM_read(index_client_id, index_client_id_end));
//  Serial.println("token read: " + EEPROM_read(index_token, index_token_end));
//  Serial.println("secret read: " + EEPROM_read(index_secret, index_secret_end));
//  Serial.println("port read: " + EEPROM_read(index_port, index_port_end));
//  Serial.println("interval read: " + EEPROM_read(index_interval, index_interval_end));
}

void handleNetpie() 
{
  String server_url = EEPROM_read(index_server_url, index_server_url_end);
  String Client_id = EEPROM_read(index_client_id, index_client_id_end);
  String username = EEPROM_read(index_token, index_token_end);
  String password = EEPROM_read(index_secret, index_secret_end);
  String port = EEPROM_read(index_port, index_port_end);
  String interval = EEPROM_read(index_interval, index_interval_end);
  
  if(server_url == "" ){server_url = "broker.netpie.io";}
  if(Client_id == "" ){Client_id = "-";}
  if(username == "" ){username = "-";}
  if(password == "" ){password = "-";}
  if(port == "" ){port = "1883";}
  if(interval == "" ){interval = "60";}

  Server.send(200, "text/html", String(
                                         "<html>"
                                         "<head>"
                                         "<meta name='viewport' content='width=device-width,initial-scale=1.0'>"
                                         "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\" integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\" crossorigin=\"anonymous\">"
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
                                                                            "<input type=\"text\" id=\"server_uri\" name=\"server_uri\" value="+server_url+" class=\"form-control\" id=\"exampleFormControlInput1\" maxlength=\"50\" required>"
                                                                       "</div>"
                                                                       "<div class=\"form-group\">\n"
                                                                                "<label for=\"exampleFormControlInput1\">Client ID</label>\n"
                                                                                "<input type=\"text\" id=\"clid\" name=\"clid\" value="+Client_id+" class=\"form-control\" aria-label=\"Sizing example input\" aria-describedby=\"inputGroup-sizing-sm\" maxlength=\"50\" required>"
                                                                       "</div>"
                                                                       "<div class=\"form-group\">\n"
                                                                               "<label for=\"exampleFormControlInput1\">Username(token)</label>\n"
                                                                               "<input type=\"text\" id=\"token\" name=\"token\" value="+username+" class=\"form-control\" aria-label=\"Sizing example input\" aria-describedby=\"inputGroup-sizing-sm\" maxlength=\"50\" required>"
                                                                       "</div>"
                                                                       "<div class=\"form-group\">\n"
                                                                               "<label for=\"exampleFormControlInput1\">Password(secret)</label>\n"
                                                                               "<input type=\"text\" id=\"secret\" name=\"secret\"  value="+password+" class=\"form-control\" aria-label=\"Sizing example input\" aria-describedby=\"inputGroup-sizing-sm\" maxlength=\"50\" required>"
                                                                       "</div>"
                                                                       "<div class=\"form-group\">\n"
                                                                              "<label for=\"exampleFormControlInput1\">Port</label>\n"
                                                                              "<input type=\"text\" id=\"port\" name=\"port\" value="+port+" class=\"form-control\" aria-label=\"Sizing example input\" aria-describedby=\"inputGroup-sizing-sm\" maxlength=\"50\" required>"
                                                                      "</div>"
                                                                      "<div class=\"form-group\">\n"
                                                                             "<label for=\"exampleFormControlInput1\">Interval(s)</label>\n"
                                                                              "<input type=\"text\" id=\"interval\" name=\"interval\" value="+interval+" class=\"form-control\" aria-label=\"Sizing example input\" aria-describedby=\"inputGroup-sizing-sm\" maxlength=\"50\" required>"
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
                                         "</html>"
                                       ));
}

Mqtt_config get_mqtt_config()
{	
	Mqtt_config mqtt_config;
	
	if(!__eprm_flag)
	{
		EEPROM.begin(512);
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


