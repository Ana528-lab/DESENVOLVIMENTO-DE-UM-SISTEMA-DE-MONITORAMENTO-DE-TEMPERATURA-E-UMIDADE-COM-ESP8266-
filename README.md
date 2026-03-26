# DESENVOLVIMENTO-DE-UM-SISTEMA-DE-MONITORAMENTO-DE-TEMPERATURA-E-UMIDADE-COM-ESP8266-
COMPONENTES UTILIZADOS: 
- Módulo ESP-12E (ESP8266)
- Sensor de temperatura e umidade DHT22
- Módulo de Relógio de Tempo Real (RTC DS3231)
- Módulo de Armazenamento em Cartão SD 

Este projeto consiste em um sistema automático para monitorar condições ambientais utilizando um sensor DHT22 integrado ao microcontrolador ESP8266. O objetivo principal é garantir que as medições de temperatura e umidade sejam registradas de forma segura e contínua, realizando leituras a cada 5 minutos.
​Para maior confiabilidade, o sistema trabalha com armazenamento duplo: as informações são enviadas em tempo real para uma planilha online no Google Sheets via Wi-Fi e, simultaneamente, gravadas em um cartão SD em formato CSV. O uso de um módulo RTC DS3231 permite que cada registro contenha a data e o horário exatos da medição, garantindo um histórico preciso mesmo que a conexão com a internet oscile. 
