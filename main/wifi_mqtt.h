#ifndef WIFI_MQTT_H
#define WIFI_MQTT_H

void wifi_init_sta(void);
void mqtt_app_start(void);
void mqtt_publish_status(const char* message);

#endif