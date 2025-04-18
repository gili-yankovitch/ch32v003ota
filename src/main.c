#include <ch32v003fun.h>
#include <ch32v003_GPIO_branchless.h>
#include <stdio.h>
#include <string.h>
#include <prot.h>
#include <aes.h>
#include <uart.h>
#include <ota.h>

static int readAnalog()
{
    return SysTick->CNT ^ GPIO_analogRead(GPIOv_from_PORT_PIN(GPIO_port_D, 3));
}

static int RNG(uint8_t *dest, unsigned size)
{
  // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of
  // random noise). This can take a long time to generate random data if the result of analogRead(0)
  // doesn't change very frequently.
  while (size) {
    uint8_t val = 0;
    for (unsigned i = 0; i < 8; ++i) {
      int init = readAnalog() ^ SysTick->CNT;
      int count = 0;
      while (readAnalog() == init) {
        ++count;
      }

      if (count == 0) {
         val = (val << 1) | (init & 0x01);
      } else {
         val = (val << 1) | (count & 0x01);
      }
    }
    *dest = val;
    ++dest;
    --size;
  }
  // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
  return 1;
}

static void aesTest()
{
    struct AES_ctx ctx;
    uint8_t iv[AES_BLOCKLEN] = { 0 };
    uint8_t key[AES_BLOCKLEN];
    uint8_t message[] = "Hello, world!!!!";
    size_t len = strlen((char *)message);

    // Randomize key
    RNG(key, AES_BLOCKLEN);

    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_encrypt_buffer(&ctx, (uint8_t *)message, len);

    printf("Encrypted message: %s\r\n", message);

    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_decrypt_buffer(&ctx, (uint8_t *)message, len);

    printf("Decrypted message: %s\r\n", message);
}

static void cryptoTest()
{
    aesTest();
}

int main()
{
    // Enable GPIOs
    funGpioInitAll();

    GPIO_ADCinit();

	GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_D, 3),
	    GPIO_pinMode_I_analog,
	    GPIO_Speed_In);

    // Optional: For blinking LED
    funPinMode(PC3, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
#if 0
    funPinMode(PC1, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(PC2, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(PC4, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(PD2, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(PD3, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
    funPinMode(PD4, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);
#endif
    cryptoTest();

    flashReadProtect();

    for (;;)
    {
#if 0
        printf("Flashing test...\r\n");

        Delay_Ms(1000);
#else
#if 0
        funDigitalWrite(PC1, FUN_HIGH);
        funDigitalWrite(PC2, FUN_HIGH);
        funDigitalWrite(PC4, FUN_HIGH);
        funDigitalWrite(PD2, FUN_HIGH);
        funDigitalWrite(PD3, FUN_HIGH);
        funDigitalWrite(PD4, FUN_HIGH);
#else
        funDigitalWrite(PC3, FUN_HIGH);
        Delay_Ms(100);
        funDigitalWrite(PC3, FUN_LOW);
        Delay_Ms(100);
#endif
#endif
    }

    return 0;
}
