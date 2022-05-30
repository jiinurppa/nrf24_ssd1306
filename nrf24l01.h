#ifndef NRF24L01_H
#define NRF24L01_H

#include <stdint.h>
#include <RF24.h>

#define NRF23_SPI_PORT spi0
#define NRF23_SPI_SCK 18
#define NRF23_SPI_TX 19
#define NRF23_SPI_RX 16
#define NRF24_CE_PIN 20
#define NRF24_CSN_PIN 17
#define NRF24_IRQ_PIN 21

#define NRF24_CHANNEL 101
#define NRF24_PAYLOAD_SIZE 21
#define NRF24_TX_PIPE 0
#define NRF24_RX_PIPE 1

static const uint8_t NRF24_TX_ADDRESS[6] = "ARDU1";
static const uint8_t NRF24_RX_ADDRESS[6] = "PICO1";
char nrf24_message_buffer[NRF24_PAYLOAD_SIZE] = { 0 };

RF24 radio(NRF24_CE_PIN, NRF24_CSN_PIN);

static void nrf24_init()
{
    radio.setChannel(NRF24_CHANNEL);
    //radio.setRadiation(RF24_PA_MAX, RF24_250KBPS, true); // PA_MAX for distance
    radio.setRadiation(RF24_PA_LOW, RF24_250KBPS, true); // PA_LOW for debug
    radio.setPayloadSize(NRF24_PAYLOAD_SIZE);
    radio.openWritingPipe(NRF24_TX_ADDRESS);
    radio.openReadingPipe(NRF24_RX_PIPE, NRF24_RX_ADDRESS);
    radio.startListening();
}

void nrf24_receive_message()
{
    uint8_t pipe;

    if (radio.available(&pipe))
    {
        uint8_t bytes = radio.getPayloadSize();
        radio.read(&nrf24_message_buffer, NRF24_PAYLOAD_SIZE);
    }
}

#endif /* NRF24L01_H */
