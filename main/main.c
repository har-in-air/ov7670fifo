#include <string.h>
#include <errno.h>
#include <sys/fcntl.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"


#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_attr.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"
#include "tcpip_adapter.h"
#include "rom/uart.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "spiffs_vfs.h"

#include "sdkconfig.h"

#include "wiring.h"
#include "sccb.h"
#include "twi.h"
#include "ov7670.h"
#include "ov7670reg.h"

static const char TAG[] = "CamFifo";

#define gpioLED GPIO_NUM_21

// uncomment this for testing camera capture over serial port for debug purposes
// interfaces with python script camview_*.py on pc host to download and display 
// the captured frame.
//#define CAMERA_DEBUG

#ifdef CAMERA_DEBUG
static uint8_t hextoi(char* hexbuf);
static const char hex[] = "0123456789abcdef";
static uint8_t rcvbuf[16];
static int rcvbufpos;
static void handleSerialEvent();


static uint8_t hextoi(char* hexbuf) {
    int i;
    uint8_t res  = 0;
    i = 0;
    while (i < 16) {
      if (hexbuf[0] == hex[i++]) break;
      }
    res = i;
    i = 0;
    while (i < 16) {
      if (hexbuf[1] == hex[i++]) break;
      }
    res += (16*i);
    return res;
    }


	
static void handleSerialEvent() {
	uint8_t c;
	STATUS sts = uart_rx_one_char(&c);
	if (sts == OK){
		if ((c >= 32) && (c <= 126)) {
			rcvbuf[rcvbufpos++] = c;
			}
		else
		if (c == 13) {
			rcvbuf[rcvbufpos++] = 0;
			rcvbufpos = 0;
			if (strncmp((char*)rcvbuf,"reg",3) == 0) {
				char regaddr[3] = {0};
				char regdata[3] = {0};
				regaddr[0] = rcvbuf[4];
				regaddr[1] = rcvbuf[5];
				regdata[0] = rcvbuf[7];
				regdata[1] = rcvbuf[8];
				uint8_t adr = hextoi(regaddr);
				uint8_t dta = hextoi(regdata);
				SCCB_Write(OV7670_I2C_ADDR, adr,dta);
				}
			else       
			if (strcmp((char *) rcvbuf, "cap") == 0) {
				OV7670_captureFrame();
				printf("OK\r\n");
				}
			else
			if (strcmp((char *) rcvbuf, "rrst") == 0) {
				OV7670_rrst();
				printf("OK\r\n");
				}
			else
			if (strlen((char *) rcvbuf) > 5 && strncmp((char *) rcvbuf, "read ", 5) == 0) {
				OV7670_readLine();
				 //uart_write_bytes(UART_NUM_0, (const char *)imageLine, LINE_BYTES);
				for (int inx = 0; inx < LINE_BYTES; inx++) {
					uart_tx_one_char(imageLine[inx]);
					}
				}         
			}
		}
	}	  

#else
	
static void wifiInitStation();
static void wifiInitAccessPoint();
static void serveImage(struct netconn *conn);
static void serveWebPage(struct netconn *conn, char *fname);
static void http_server_netconn_serve(struct netconn *conn);
static void http_server(void *pvParameters);

	


static esp_err_t esp32_wifi_eventHandler(void *ctx, system_event_t *event) {
	tcpip_adapter_ip_info_t ip_info;
	switch(event->event_id) {
		case SYSTEM_EVENT_AP_START: // Handle the AP start event
			tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip_info);
			ESP_LOGI(TAG, "Now an access point with SSID = ESP32Cam")
			ESP_LOGI(TAG, "Connect to http://" IPSTR, IP2STR(&ip_info.ip));
			break;

		case SYSTEM_EVENT_STA_DISCONNECTED: 
			ESP_LOGI(TAG, "Station disconnected, init as AP");
			wifiInitAccessPoint();
			break;

		case SYSTEM_EVENT_STA_GOT_IP: 
			ESP_LOGI(TAG, "Now a station on %s network", CONFIG_WIFI_SSID);
			ESP_LOGI(TAG, "Connect to http://" IPSTR, IP2STR(&event->event_info.got_ip.ip_info.ip));
			break;

		default: 
			break;
		} 

	return ESP_OK;
	}




static void serveWebPage(struct netconn *conn, char *fname) {
	int res;
	char *buf;
	buf = calloc(4096, 1);
	if (buf == NULL) {
    	ESP_LOGE(TAG,"Error allocating read buffer\"\r\n");
    	return;
		}

	FILE *fd = fopen(fname, "rb");
    if (fd == NULL) {
    	ESP_LOGE(TAG,"Error opening file %s\r\n", fname);
    	free(buf);
    	return;
		}
	
	char szbuf[35];
	sprintf(szbuf,"HTTP/1.1 200 OK\r\n");
	netconn_write(conn, szbuf, strlen(szbuf), NETCONN_COPY);
  
	sprintf(szbuf,"Content-Type: text/html\r\n");
	netconn_write(conn, szbuf, strlen(szbuf), NETCONN_COPY);
	
    int nbytes = 999;
	nbytes = fread(buf, 1, 4096, fd);
	if (nbytes <= 0) {
		ESP_LOGE(TAG,"Error reading file %s\r\n", fname);
		res = fclose(fd);	
		if (res) {
			ESP_LOGE(TAG,"Error closing file %s\r\n", fname);
			}
		free(buf);
		return;
 		}
	else {
		ESP_LOGI(TAG,"Read %d bytes", nbytes);
		buf[nbytes] = '\0';
		
		sprintf(szbuf,"Content-Length: %d\r\n", nbytes);
		netconn_write(conn, szbuf, strlen(szbuf), NETCONN_COPY);

		sprintf(szbuf,"Connection: Keep-Alive\r\n");
		netconn_write(conn, szbuf, strlen(szbuf), NETCONN_COPY);

		sprintf(szbuf,"Access-Control-Allow-Origin: *\r\n");
		netconn_write(conn, szbuf, strlen(szbuf), NETCONN_COPY);

		sprintf(szbuf,"\r\n");
		netconn_write(conn, szbuf, strlen(szbuf), NETCONN_COPY);
		
		netconn_write(conn, buf, nbytes, NETCONN_NOCOPY);
		
		free(buf);
		res = fclose(fd);
		if (res) {
			ESP_LOGE(TAG,"Error closing file %s\r\n", fname);
			}
		}
	}



static void serveImage(struct netconn *conn) {
  OV7670_captureFrame();
  OV7670_rrst();
  
  char szbuf[35];
  sprintf(szbuf,"HTTP/1.1 200 OK\r\n");
  netconn_write(conn, szbuf, strlen(szbuf), NETCONN_COPY);
  
  sprintf(szbuf,"Content-Type: application/octet-stream\r\n");
  netconn_write(conn, szbuf, strlen(szbuf), NETCONN_COPY);

  sprintf(szbuf,"Content-Length: %d\r\n",NUM_LINES*LINE_BYTES);
  netconn_write(conn, szbuf, strlen(szbuf), NETCONN_COPY);

  sprintf(szbuf,"Connection: close\r\n");
  netconn_write(conn, szbuf, strlen(szbuf), NETCONN_COPY);

  sprintf(szbuf,"Access-Control-Allow-Origin: *\r\n");
  netconn_write(conn, szbuf, strlen(szbuf), NETCONN_COPY);

  sprintf(szbuf,"\r\n");
  netconn_write(conn, szbuf, strlen(szbuf), NETCONN_COPY);

  for (int row = 0; row < NUM_LINES; row++) {
    OV7670_readLine();
	netconn_write(conn, imageLine, LINE_BYTES, NETCONN_COPY);
    }
}


static void http_server_netconn_serve(struct netconn *conn) {
    struct netbuf *inbuf;
    char *buf;
    uint16_t buflen;
    err_t err;
    // Read the data from the port, blocking if nothing yet there.
    // We assume the request (the part we care about) is in one netbuf 
    err = netconn_recv(conn, &inbuf);
    if (err == ERR_OK) {
        netbuf_data(inbuf, (void**) &buf, &buflen);
		buf[buflen] = 0;
		ESP_LOGI(TAG,"%s", buf);

		if (strncmp(buf,"GET /image",10) == 0) {
	        gpio_set_level(gpioLED, 1);
			serveImage(conn);
	        gpio_set_level(gpioLED, 0);
			}
		else 
		if (strncmp(buf,"GET /",5) == 0) {	
			serveWebPage(conn, "/spiffs/qvgayuv.html");
			}
        }
    
    // Close the connection (server closes in HTTP) 
    netconn_close(conn);
    // Delete the buffer (netconn_recv gives us ownership,
    // so we have to make sure to deallocate the buffer) 
    netbuf_delete(inbuf);
}


static void http_server(void *pvParameters){
    struct netconn *conn, *newconn;
    err_t err;
    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, NULL, 80);
    netconn_listen(conn);
    do {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK) {
            http_server_netconn_serve(newconn);
            netconn_delete(newconn);
        }
    } while (err == ERR_OK);
    netconn_close(conn);
    netconn_delete(conn);
}


static void wifiInitStation() {
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .bssid_set = false
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );
} 


static void wifiInitAccessPoint() {
	tcpip_adapter_ip_info_t ipInfo;

	inet_pton(AF_INET, "192.168.4.1", &ipInfo.ip);
	inet_pton(AF_INET, "192.168.4.1", &ipInfo.gw);
	inet_pton(AF_INET, "255.255.255.0", &ipInfo.netmask);
	tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);	
    
	ESP_LOGD(TAG, "Starting access point ESP32Cam at 192.168.4.1...");
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	wifi_config_t apConfig = {
		.ap = {
			.ssid="ESP32Cam",
			.ssid_len=0,
			.password="klik",
			.channel=0,
			.authmode=WIFI_AUTH_OPEN,
			.ssid_hidden=0,
			.max_connection=4,
			.beacon_interval=100
		}
	};
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &apConfig));
	ESP_ERROR_CHECK(esp_wifi_start());
} 
#endif


int app_main(void) {
	nvs_flash_init();

	OV7670_init();
    gpio_set_direction(gpioLED, GPIO_MODE_OUTPUT);
	
#ifndef CAMERA_DEBUG
	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(esp32_wifi_eventHandler, NULL));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

	// try to connect to existing network as station first
	wifiInitStation();
    vfs_spiffs_register();
	vTaskDelay(1000 / portTICK_RATE_MS);
    ESP_LOGI(TAG, "Starting http_server task...");
    xTaskCreatePinnedToCore(&http_server, "http_server", 4096, NULL, 5, NULL,1);
#endif

	
    while (true) {
#ifdef CAMERA_DEBUG
		handleSerialEvent();		
#endif		
        vTaskDelay(10 / portTICK_PERIOD_MS);
		}
    return 0;
}

