# AIR_BOREAS

Sistema de medición de la calidad del aire en interior, se instalará en macetas autorregantes @Medialab-Prado

## Funcionamiento

El sistema recoge medidas de la cantidad de partículas volátiles en el aire, PM2.5 y PM10 y devuelve por pantalla estos valores. Un anillo de LEDS que "respira" traduce estos datos a valores de color que reflejan los límites recomendados por la OMS: 

- Verde - [PM2.5 && PM10 < 10 ug/m3]
- Naranja - [25 ug/m3 <= PM2.5 && PM10 > 10 ug/m3]
- Rojo - [PM2.5 && PM10 >25  ug/m3]



## Componentes

- Arduino MEGA 2560 
  https://store.arduino.cc/arduino-mega-2560-rev3
- Sensor de Partículas Volátiles: PMS5003
  https://www.adafruit.com/product/3686
- Módulo de memoria de reloj RTC
  https://es.aliexpress.com/item/32952354500.html
- Adaptardor de tarjeta Micro SD
  https://es.aliexpress.com/item/32907897214.html
- Anillo de LEDs NeoPixel Ring
  https://www.adafruit.com/product/1643
- Pantalla LCD con I2C
  https://es.aliexpress.com/item/32974217935.html
