#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <SPI.h>
#include <SD.h>

#define FusoHorario -3 // Define o fuso-horario GMT -3, o padrão é brasília -3

#define GPS_RX 4 // Pino que foi definido pela posição do MicroJumper
#define GPS_TX 3 // Pino que foi definido pela posição do MicroJumper

SoftwareSerial GPS_Serial(GPS_RX, GPS_TX); // Definindo os pinos GPS_RX e GPS_TX como comunicação serial entre o Arduino e GPS
TinyGPS GPS; //Criando o objeto GPS

uint8_t chipSelect = 8; // O pino CS da shield GPS

void setup() {
  GPS_Serial.begin(9600); // // Inicializando as comunicações seriais com os Bauds definidos
  Serial.begin(9600);
  Serial.println("Inicializando o cartao MicroSD...");
  if (!SD.begin(chipSelect)) { // Se o cartão não estiver presente ou falhar....
    Serial.println("O MicroSD falhou ou nao esta presente");
    delay(1000);
  }
  Serial.println("O cartao foi inicializado corretamente.");
}

void loop() {
  bool conexao = false; // Indica se o GPS está conectado e recebendo dados do satelite
  int16_t ano; // Criação das variáveis  que guardará as informações de data
  uint8_t mes, dia, hora, minuto, segundo;

  while (GPS_Serial.available()) { // Ficará em loop até que consiga se conectar.
    char cIn = GPS_Serial.read();
    conexao = GPS.encode(cIn);
  }
  // Se saiu do loop, signifca que conseguiu conectar e está pronto para mostrar os dados
  if (conexao) {
    File dataFile = SD.open("GPSlog.txt", FILE_WRITE); // Associará o objeto dataFile ao arquivo GPSlog.txt. Se caso o arquivo não exista, será criado
    //O objeto dataFile foi setado como escrita, mas dá para utilizar de outras formas.
    
    Serial.println(" \n ----------------------------------------"); 
    dataFile.println(" \n ----------------------------------------"); //Este comando salva a linha no cartão MicroSD
    
    // Se caso não conseguir abrir o arquivo por qualquer razão, irá avisar no MonitorSerial
    if (!dataFile) Serial.println("Erro ao abrir o arquivo GPSlog.txt");  

    //Latitude e Longitude
    long latitude, longitude;
    GPS.get_position(&latitude, &longitude); // obtem a   latitude e longitude

    // se a latitude for algo valido ela será impressa no MicroSD e no MonitorSerial
    if ((latitude != TinyGPS::GPS_INVALID_F_ANGLE)) {
      Serial.print("Latitude: ");
      Serial.println(float(latitude) / 1000000, 6);
      dataFile.print("Latitude: ");
      dataFile.println(float(latitude) / 1000000, 6);

    }
    // se a longitude for algo valido ela será impressa no MicroSD e no MonitorSerial
    if (longitude != TinyGPS::GPS_INVALID_F_ANGLE) {
      Serial.print("Longitude: ");
      Serial.println(float(longitude) / 1000000, 6);
      dataFile.print("Longitude: ");
      dataFile.println(float(longitude) / 1000000, 6);
    }

    // Chamará a função para converter o horário recebido pelo satelite GMT, e converte para o fuso escolhido
    HorarioFuso(&ano, &mes, &dia, &hora, &minuto, &segundo); 
    // imprimindo os dados no monitor serial
    Serial.print("Data (GMT "); Serial.print(FusoHorario); Serial.println(")");
    Serial.print(dia);
    Serial.print("/");
    Serial.print(mes);
    Serial.print("/");
    Serial.println(ano);
    Serial.print("Horario (GMT "); Serial.print(FusoHorario); Serial.println(")");
    Serial.print(hora);
    Serial.print(":");
    Serial.print(minuto);
    Serial.print(":");
    Serial.println(segundo);
    // imprimindo os dados no cartão MicroSD
    dataFile.print("Data (GMT "); dataFile.print(FusoHorario); dataFile.println(")");
    dataFile.print(dia);
    dataFile.print("/");
    dataFile.print(mes);
    dataFile.print("/");
    dataFile.println(ano);
    dataFile.print("Horario (GMT "); dataFile.print(FusoHorario); dataFile.println(")");
    dataFile.print(hora);
    dataFile.print(":");
    dataFile.print(minuto);
    dataFile.print(":");
    dataFile.println(segundo);
    dataFile.close();
  }
}

void HorarioFuso(int16_t *ano_, uint8_t *mes_, uint8_t *dia_, uint8_t *hora_, uint8_t *minuto_, uint8_t *segundo_) {
  uint8_t QntDiasMes[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  int16_t ano;
  int8_t mes, dia, hora, minuto, segundo;
  GPS.crack_datetime(&ano, &mes, &dia, &hora, &minuto, &segundo); //obtendo a data e horário do satelite no padrão GMT
  
  hora += FusoHorario;
  
  if ((ano % 4) == 0) QntDiasMes[1] = 29; // Ano Bissexto
  if (hora < 0) {
    hora += 24;
    dia -= 1;
    if (dia < 1) {
      if (mes == 1) { // Jan 1
        mes = 12;
        ano -= 1;
      } else {
        mes -= 1;
      }
      dia = QntDiasMes[mes - 1];
    }
  }
  if (hora >= 24) {
    hora -= 24;
    dia += 1;
    if (dia > QntDiasMes[mes - 1]) {
      dia = 1;
      mes += 1;
      if (mes > 12) { // Jan 1
        ano += 1;
        mes = 1;
      }
    }
  }
  *ano_ = ano;
  *mes_ = mes;
  *dia_ = dia;
  *hora_ = hora;
  *minuto_ = minuto;
  *segundo_ = segundo;
}