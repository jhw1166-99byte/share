#ifndef __MQTT_H
#define __MQTT_H

#include "stm32f1xx_hal.h"

/* 定义数据数组 - 保持原有定义 */
//#define Client_ID     "oneplus"
//#define User_Name     "94f3yC7pGC"
//#define Password      "version=2018-10-31&res=products%2F94f3yC7pGC%2Fdevices%2Foneplus&et=1795272955&method=md5&sign=3MLldaAWdq%2FryIqb%2FSavQg%3D%3D"

#define Client_ID     "B2"
#define User_Name     "fancy"
#define Password      "123"//ID

/* 函数声明 */

/* 预处理函数：构建报文并返回总字节数（用于 AT+CIPSEND） */
void mqtt_start(void);
uint16_t mqtt_prepare_connect(void);    // 预构建 CONNECT 报文
uint16_t mqtt_prepare_publish(void);    // 预构建 PUBLISH 报文
uint16_t mqtt_prepare_subscribe(void);  // 预构建 SUBSCRIBE 报文

/* 发送函数：发送已构建好的报文 */
void mqtt_send(void);                   // 发送 CONNECT
void mqtt_publish_data(void);           // 发送 PUBLISH
void mqtt_subscribe(void);              // 发送 SUBSCRIBE

/* 按键处理函数：封装完整的按键操作流程 */
void mqtt_handle_connect(void);         // KEY2 - CONNECT 按键处理
void mqtt_handle_subscribe(void);       // KEY0 - SUBSCRIBE 按键处理
void mqtt_handle_publish(void);         // WKUP - PUBLISH 按键处理

#endif /* __MQTT_H */
