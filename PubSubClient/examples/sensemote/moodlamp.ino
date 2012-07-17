#include <PubSubClient.h>

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <avr/pgmspace.h>

#define WIDGET_NAME "MyMoodLamp" // No spaces allowed

#define RED_PIN 6
#define BLUE_PIN 3
#define GREEN_PIN 5

#define MQTT_SERVER "test.mosquitto.org"

byte MAC_ADDRESS[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x04 };
PubSubClient client;

// This is the HTML which provides the frontend on the web
char PROGMEM html_progmem[] =
"<html>"
"<h1>" WIDGET_NAME "</h1>"
"<form>"
"    <label>RGB</label><input type='textarea' value='' id='rgbhex'/>"
"    <button class='btn' type='button' id='publish'>Set</button>"
"</form>"
"<script type='text/javascript' src='/jquery.min.js'></script>"
"<script type='text/javascript' src='/api2.js'></script>"
"<script type='text/javascript'>"
"var WIDGET_NAME = '" WIDGET_NAME "';"
"var rgbTopic = 'sensemote/widgets/'+WIDGET_NAME+'/rgb';"
"var timer;"
"function reconnect()"
"{"
"    clearTimeout(timer);"
"    timer = setTimeout(function(){startStreaming();}, 1000);"
"}"
"function startStreaming()"
"{"
"    sensemote.register({"
"        data: function(topic, msg) {"
"            if (topic == rgbTopic) {"
"                $('#rgbhex').val(msg);"
"            }"
"        },"
"        error: function() {reconnect();}"
"    });"
"    sensemote.subscribe(rgbTopic, {success: sensemote.stream});"
"}"
"$(window).load(function()"
"{"
"    $('#publish').click(function() {"
"        sensemote.publish(rgbTopic, $('#rgbhex').val());"
"    });"
"    startStreaming();"
"});"
"</script>"
"</body>"
"</html>";

int htoi(char c)
{
    if (c<='9')
        return c-'0';
    if (c<='F')
        return c-'A'+10;
    if (c<='f')
        return c-'a'+10;
    return 0;
}

void callback(char* topic, uint8_t* payload, unsigned int length)
{
    byte r, g, b;

    if (length == 7 && payload[0] == '#')    // #rrggbb
    {
        r = htoi(payload[1]) * 16 + htoi(payload[2]);
        g = htoi(payload[3]) * 16 + htoi(payload[4]);
        b = htoi(payload[5]) * 16 + htoi(payload[6]);
        
        analogWrite(RED_PIN, r);
        analogWrite(GREEN_PIN, g);
        analogWrite(BLUE_PIN, b);
    }
}

void setup()
{
    Serial.begin(9600);

    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);

    if (Ethernet.begin(MAC_ADDRESS) == 0)
    {
        Serial.println("Failed to configure Ethernet using DHCP");
        return;
    }

    client = PubSubClient(MQTT_SERVER, 1883, callback);
}

void loop()
{
    if (!client.connected())
    {
        client.connect(WIDGET_NAME, "sensemote/widgets/" WIDGET_NAME "/presence", 1, 1, "");
        client.publish("sensemote/widgets/" WIDGET_NAME "/presence", (uint8_t *)"1", 2, true);
        client.publish_P("sensemote/widgets/" WIDGET_NAME "/data", (uint8_t PROGMEM *)html_progmem, strlen_P(html_progmem), true);
        client.subscribe("sensemote/widgets/" WIDGET_NAME "/rgb");
    }
    client.loop();
}

