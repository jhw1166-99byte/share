#include "mqtt.h"
#include "string.h"
#include "usart.h"
#include "esp8266.h"
#include "main.h"
#include <stdio.h>

/* ========== 文件作用域静态变量 ========== */

/* CONNECT 消息缓存 */
static uint8_t  mqtt_connect_data[200];
static uint16_t mqtt_connect_len   = 0;
static uint8_t  mqtt_connect_ready = 0;

/* PUBLISH 消息缓存 */
static uint8_t  mqtt_pub_data[256];
static uint16_t mqtt_pub_len   = 0;
static uint8_t  mqtt_pub_ready = 0;

/* SUBSCRIBE 消息缓存 */
static uint8_t  mqtt_sub_data[256];
static uint16_t mqtt_sub_len   = 0;
static uint8_t  mqtt_sub_ready = 0;

/* ========== 预处理函数（构建数据并返回总字节数） ========== */

/**
 * @brief  预构建 CONNECT 报文，返回报文总字节数
 * @retval 报文长度（用于 AT+CIPSEND）
 */
void mqtt_start(void)
{
	mqtt_prepare_connect();
	mqtt_prepare_publish();
	mqtt_prepare_publish();

}
uint16_t mqtt_prepare_connect(void)
{
    if (mqtt_connect_ready)
        return mqtt_connect_len;

    uint16_t num = 0;
    uint8_t  len;
    uint8_t  msg_len;

    /* ---------- 固定头 ---------- */
    mqtt_connect_data[num++] = 0x10;

    /* ---------- 剩余长度 ---------- */
    msg_len = 2 + strlen(Client_ID)
            + 2 + strlen(User_Name)
            + 2 + strlen(Password)
            + 10;

    if (msg_len < 128) {
        mqtt_connect_data[num++] = msg_len;
    } else {
        mqtt_connect_data[num++] = msg_len % 128 + 128;
        mqtt_connect_data[num++] = msg_len / 128;
    }

    /* ---------- 协议名长度 + 协议名 ---------- */
    mqtt_connect_data[num++] = 0x00;
    mqtt_connect_data[num++] = 0x04;
    mqtt_connect_data[num++] = 0x4D;  // M
    mqtt_connect_data[num++] = 0x51;  // Q
    mqtt_connect_data[num++] = 0x54;  // T
    mqtt_connect_data[num++] = 0x54;  // T

    /* ---------- 协议版本 ---------- */
    mqtt_connect_data[num++] = 0x04;

    /* ---------- 连接标志位 ---------- */
    mqtt_connect_data[num++] = 0xC2;

    /* ---------- 保活时间 ---------- */
    mqtt_connect_data[num++] = 0x0B;
    mqtt_connect_data[num++] = 0xB8;

    /* ---------- 客户端 ID ---------- */
    len = strlen(Client_ID);
    mqtt_connect_data[num++] = 0x00;
    mqtt_connect_data[num++] = len;
    for (int j = 0; j < len; j++)
        mqtt_connect_data[num++] = Client_ID[j];

    /* ---------- 用户名 ---------- */
    len = strlen(User_Name);
    mqtt_connect_data[num++] = 0x00;
    mqtt_connect_data[num++] = len;
    for (int j = 0; j < len; j++)
        mqtt_connect_data[num++] = User_Name[j];

    /* ---------- 密码 ---------- */
    len = strlen(Password);
    mqtt_connect_data[num++] = 0x00;
    mqtt_connect_data[num++] = len;
    for (int j = 0; j < len; j++)
        mqtt_connect_data[num++] = Password[j];

    mqtt_connect_len   = num;
    mqtt_connect_ready = 1;
    return mqtt_connect_len;
}

/**
 * @brief  预构建 PUBLISH 报文，返回报文总字节数
 * @retval 报文长度（用于 AT+CIPSEND）
 */
uint16_t mqtt_prepare_publish(void)
{
    if (mqtt_pub_ready)
        return mqtt_pub_len;

    uint16_t num = 0;

    /* ---------- 固定报头 ---------- */
    mqtt_pub_data[num++] = 0x30;   // PUBLISH, QoS0

    /* ---------- 剩余长度 ---------- */
    mqtt_pub_data[num++] = 0x06;   // 2(TopicLen) + 3(Topic) + 2(Payload)

    /* ---------- Topic 长度 ---------- */
    mqtt_pub_data[num++] = 0x00;
    mqtt_pub_data[num++] = 0x03;

    /* ---------- Topic ---------- */
    mqtt_pub_data[num++] = 'j';
    mqtt_pub_data[num++] = 'z';
    mqtt_pub_data[num++] = 'd';

    /* ---------- Payload ---------- */
    mqtt_pub_data[num++] = 'B';

    mqtt_pub_len   = num;
    mqtt_pub_ready = 1;
    return mqtt_pub_len;
}

/**
 * @brief  预构建 SUBSCRIBE 报文，返回报文总字节数
 * @retval 报文长度（用于 AT+CIPSEND）
 */
uint16_t mqtt_prepare_subscribe(void)
{
    if (mqtt_sub_ready)
        return mqtt_sub_len;

    uint16_t num = 0;

    /* ---------- 固定报头：SUBSCRIBE (0x82) ---------- */
    mqtt_sub_data[num++] = 0x82;

    mqtt_sub_data[num++] = 0x08;

    /* ---------- 消息 ID ---------- */
    mqtt_sub_data[num++] = 0x00;
    mqtt_sub_data[num++] = 0x01;

    /* ---------- Topic 长度 ---------- */
    mqtt_sub_data[num++] = 0x00;
    mqtt_sub_data[num++] = 0x03;

    /* ---------- Topic ---------- */
    mqtt_sub_data[num++] = 'j';
    mqtt_sub_data[num++] = 'z';
    mqtt_sub_data[num++] = 'd';

    /* ---------- Requested QoS ---------- */
    mqtt_sub_data[num++] = 0x00;

    mqtt_sub_len   = num;
    mqtt_sub_ready = 1;
    return mqtt_sub_len;
}

/* ========== 发送函数（直接发送已构建好的数据） ========== */

/**
 * @brief  发送 CONNECT 报文（需要先调用 mqtt_prepare_connect）
 */
void mqtt_send(void)
{
    mqtt_prepare_connect();  // 如果还没构建则先构建
    HAL_UART_Transmit(&huart2, mqtt_connect_data, mqtt_connect_len, 1000);
}

/**
 * @brief  发送 PUBLISH 报文（需要先调用 mqtt_prepare_publish）
 */
void mqtt_publish_data(void)
{
    mqtt_prepare_publish();  // 如果还没构建则先构建
    HAL_UART_Transmit(&huart2, mqtt_pub_data, mqtt_pub_len, 1000);
}

/**
 * @brief  发送 SUBSCRIBE 报文（需要先调用 mqtt_prepare_subscribe）
 */
void mqtt_subscribe(void)
{
    mqtt_prepare_subscribe();  // 如果还没构建则先构建
    HAL_UART_Transmit(&huart2, mqtt_sub_data, mqtt_sub_len, 1000);
}

/* ========== 按键处理函数（封装完整操作流程） ========== */

/**
 * @brief  按键 CONNECT 处理：设置LED → 构建报文 → AT+CIPSEND → 发送
 */
void mqtt_handle_connect(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
    uint16_t len = mqtt_prepare_connect();
    char at_cmd[32];
    sprintf(at_cmd, "AT+CIPSEND=%d\r\n", len);
    esp8266_send_cmd(at_cmd, "OK", 8000);
    mqtt_send();
}

/**
 * @brief  按键 SUBSCRIBE 处理：设置LED → 构建报文 → AT+CIPSEND → 发送 → 检查应答
 */
void mqtt_handle_subscribe(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);
    uint16_t len = mqtt_prepare_subscribe();
    char at_cmd[32];
    sprintf(at_cmd, "AT+CIPSEND=%d\r\n", len);
    esp8266_send_cmd(at_cmd, "OK", 8000);
    mqtt_subscribe();
    HAL_Delay(1000);
    char *p = esp8266_getIPD();
    if (*p == 0x90)
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)"mqtt_subscribe_ok\r\n", 19, HAL_MAX_DELAY);
    }
}

/**
 * @brief  按键 PUBLISH 处理：设置LED → 构建报文 → AT+CIPSEND → 发送
 */
void mqtt_handle_publish(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);
    uint16_t len = mqtt_prepare_publish();
    char at_cmd[32];
    sprintf(at_cmd, "AT+CIPSEND=%d\r\n", len);
    esp8266_send_cmd(at_cmd, "OK", 8000);
    mqtt_publish_data();
}
