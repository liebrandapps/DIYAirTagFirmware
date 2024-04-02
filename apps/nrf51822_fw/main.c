#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "nrf_log.h"
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "ble_stack.h"
#include "openhaystack.h"

/*
#define NRF_LOG_ENABLED 1
#define NRF_LOG_BACKEND_SERIAL_USES_UART 0
#define NRF_LOG_BACKEND_SERIAL_USES_RTT 1
*/

/** 
 * advertising interval in milliseconds 
 */
#define ADVERTISING_INTERVAL 5000

#define STATUS_FLAG_BATTERY_MASK           0b11000000
#define STATUS_FLAG_COUNTER_MASK           0b00111111
#define STATUS_FLAG_MEDIUM_BATTERY         0b01000000
#define STATUS_FLAG_LOW_BATTERY            0b10000000
#define STATUS_FLAG_CRITICALLY_LOW_BATTERY 0b11000000
#define STATUS_FLAG_MAINTAINED             0b00000100

#define APP_TIMER_PRESCALER              0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE          4  

/* Check Battery Status every 12 hours */
#define BATTERY_TIMER_PERIOD APP_TIMER_TICKS(12 * 3600 * 1000, APP_TIMER_PRESCALER) 
#define BUTTON_RELEASE_PERIOD APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)

APP_TIMER_DEF(battery_timer);
APP_TIMER_DEF(button_timer);

static char public_key[28] = "OFFLINEFINDINGPUBLICKEYHERE!"; 
static uint8_t status_flag = 0;
static uint8_t *raw_data;
static uint8_t data_len;
static uint8_t button_pressed = false;
static uint8_t button_counter = 0;
#define BUTTON_PIN 8


void update_battery_level(){
    uint8_t battery_level = get_current_level();
    /* NRF_LOG_DEBUG("battery level: %d\n", battery_level); */
    status_flag &= (~STATUS_FLAG_BATTERY_MASK);
    if(battery_level > 80){
        // do nothing
    }else if(battery_level > 50){
        status_flag |= STATUS_FLAG_MEDIUM_BATTERY;
    }else if(battery_level > 30){
        status_flag |= STATUS_FLAG_LOW_BATTERY;
    }else{
        status_flag |= STATUS_FLAG_CRITICALLY_LOW_BATTERY;
    }
}

void battery_timer_handler(void * context) {
    update_battery_level();

    //status_flag +=1;
    raw_data[6] = status_flag;
    setAdvertisementData(raw_data, data_len);
}


void blink() {
    nrf_gpio_cfg_output(10);
    nrf_gpio_pin_set(10);
    nrf_delay_ms(500);
    nrf_gpio_pin_clear(10);
    nrf_delay_ms(500);
    nrf_gpio_pin_set(10);
    nrf_delay_ms(500);
    nrf_gpio_pin_clear(10);
}

void button_handler(long unsigned int pin, nrf_gpiote_polarity_t polarity) {
    status_flag +=1;
    raw_data[6] = status_flag;
    setAdvertisementData(raw_data, data_len);
    if(button_pressed) {
        return;
    }
    blink();
    button_pressed = true;
    app_timer_start(button_timer, BUTTON_RELEASE_PERIOD, NULL);
    button_counter++;
    if(button_counter>63) {
        button_counter = 0;
    }
    status_flag &= (~STATUS_FLAG_COUNTER_MASK);
    status_flag |= (button_counter & STATUS_FLAG_COUNTER_MASK);
    raw_data[6] = status_flag;
    setAdvertisementData(raw_data, data_len);
}

void button_pressed_handler(void * context) {
    button_pressed = false;
}

void buttons_init(){
    ret_code_t err_code;
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_config_t in_config = {
        .sense = NRF_GPIOTE_POLARITY_HITOLO,
        .pull = NRF_GPIO_PIN_PULLUP,
        .hi_accuracy = false,
        .is_watcher = false
    };
    err_code = nrf_drv_gpiote_in_init(BUTTON_PIN, &in_config, button_handler);
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_event_enable(BUTTON_PIN, true);
}


void timer_init() {
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);

    app_timer_create(&battery_timer, APP_TIMER_MODE_REPEATED, battery_timer_handler);
    app_timer_create(&button_timer, APP_TIMER_MODE_SINGLE_SHOT, button_pressed_handler);
    /*app_timer_create(&led_blink_timer, APP_TIMER_MODE_REPEATED, led_timer_handler);*/

    app_timer_start(battery_timer, BATTERY_TIMER_PERIOD, NULL);
}


/**
 * main function
 */
int main(void) {
    // Variable to hold the data to advertise
    uint8_t *ble_address;
    /*uint8_t *raw_data;*/
 
    /*NRF_LOG_INIT();
    NRF_LOG_DEBUG("START");
    */
    //update_battery_level();
   blink();

    /*
    #define led 10
    #define Button 8
    nrf_gpio_cfg_output(led); //configure led pin as output
    nrf_gpio_cfg_input(Button,NRF_GPIO_PIN_PULLUP);// configures button as input Pin

    nrf_gpio_pin_set(led);  //Turn OFF the LED

    while (true)
    {
        if(nrf_gpio_pin_read(Button)==0)  //Read GPIO
        nrf_gpio_pin_clear(led);   //Turn ON the LED
      
        else
        nrf_gpio_pin_set(led);   //Turn OFF the LED
    }

    nrf_gpio_cfg_output(10);
    nrf_gpio_pin_set(10);
    nrf_delay_ms(1000);
    nrf_gpio_pin_clear(10);
    nrf_delay_ms(500);
    nrf_gpio_pin_set(10);
    nrf_delay_ms(1000);
    nrf_gpio_pin_clear(10);
    */

    buttons_init();
    timer_init();

    // Set key to be advertised
    data_len = setAdvertisementKey(public_key, &ble_address, &raw_data);

    // Init BLE stack
    init_ble();

    // Set bluetooth address
    setMacAddress(ble_address);

    update_battery_level();

    // Set advertisement data
    setAdvertisementData(raw_data, data_len);

    // Start advertising
    startAdvertisement(ADVERTISING_INTERVAL);

    // Go to low power mode
    while (1) {
        power_manage();
    }
}
