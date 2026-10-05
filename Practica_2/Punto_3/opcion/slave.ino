#include "driver/spi_slave.h"
#include "esp_system.h"

#define PIN_MISO 19
#define PIN_MOSI 23
#define PIN_SCLK 18
#define PIN_SS   5

uint8_t sendData = 0;
uint8_t recvData = 0;

void setup() {
  Serial.begin(115200);

  spi_bus_config_t buscfg = {
    .mosi_io_num = PIN_MOSI,
    .miso_io_num = PIN_MISO,
    .sclk_io_num = PIN_SCLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1
  };

  spi_slave_interface_config_t slvcfg = {
    .spics_io_num = PIN_SS, // 👉 PRIMERO
    .flags = 0,             // 👉 SEGUNDO
    .queue_size = 3,        // 👉 TERCERO
    .mode = 0               // 👉 CUARTO
  };

  spi_slave_initialize(VSPI_HOST, &buscfg, &slvcfg, 1);
}

void loop() {
  spi_slave_transaction_t t;
  memset(&t, 0, sizeof(t));

  t.length = 8;
  t.tx_buffer = &sendData;
  t.rx_buffer = &recvData;

  spi_slave_transmit(VSPI_HOST, &t, portMAX_DELAY);

  Serial.print("Slave recibió: ");
  Serial.println(recvData);

  sendData = recvData + 1;
}
