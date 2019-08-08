/* Mensagens de error enviadas para serial
E01 : Falha ao iniciar o cartão SD
E02 : Reset em 5 segundos...
E03 : Impossivel abrir arquivo
E04 : Falha escrever arquivo! Codigo congelado...
E05 : Arquivo pronto!
*/

/* Inclusão de todas as Bibliotecas */
#include <SD.h>
#include <SPI.h> // Biblioteca que permite a comunicação entre o microcontrolador e os dispositivos periféricos
#include <SoftwareSerial.h> // A biblioteca SoftwareSerial foi desenvolvida para permitir comunicação serial em outros pinos digitais do Arduino(?)
#include<Wire.h> // Esta Biblioteca permite a comunicação do microcontrolador com dispositivos I2C como o sensor MPU-6050
#include <Adafruit_GPS.h> // Biblioteca responsável por receber dados do módulos GPS, como ler os dados de fluxo contínuo em uma interrupção de fundo e analisá-los de maneira automática(?)
#include <avr/wdt.h> // Biblioteca responsável por configurar uma rotina de interrupção watchdog caso o sistema falhe depois de um tempo(?)

/* Inicialização da comunicação serial e escolha do canal de comunicação entre o microcontrolador e os componentes do projeto (Dúvida ?)*/
HardwareSerial mySerial = Serial1;
Adafruit_GPS GPS(&Serial1);

/* Criação de variável do tipo arquivo, para armazenar os dados no cartão SD */
File GPSData;

/* Pino CS do SD deve estar conectado no pino 10 do Arduino (Dúvida ?)*/
#define chipSelect 53

/* Variáveis do Acelerômetro e Giroscópio */
const int MPU = 0x68; // Endereço I2C do CI MPU-6050, só usa-se o endereço I2C 0x69 quando precisamos utilizar dois sensores MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; // Variável do tipo inteira com 16 bits de espaço(Qual intuito de usar esse tipo de variável?)
bool led_negate = false; // Variável do tipo bool com valores verdadeiro ou falso, como 0 e 1 para o LED começar apagado?
char conta_ciclos; // Variável do tipo caractere com o intuito de contar ciclos dentro do programa
uint32_t indexador = 0; // Variável do tipo inteira com 32 bits de espaço(Qual intuito de usar esse tipo de variável?)

/* Declara função reset, endereço 0 (Dúvida ?) */
void(* resetFunc) (void) = 0;

void setup() // A função será utilizada apenas uma vez quando o arduino for ligado e energizado, e devemos colocar apenas as funções que só podem e precisam ser executadas uma vez
{
  
 /* Inicializa e configura watchdog para 8 segundos(Vai ser sempre assim?)*/ 
 wdt_enable(WDTO_8S);
 
 /* Baud rate configurado em 9600 para cartão SD e GPS*/
 Serial.begin(9600); // Definição da taxa de comunicação desejada do tipo de comunicação serial, 9600 é a mais utilizada geralmente em simbolos por segundo
 GPS.begin(9600); // Definição da taxa de comunicação desejada do tipo de comunicação GPS, 9600 é a mais utilizada geralmente em simbolos por segundo
 
 /* Configura GPS para enviar dados da configuração mínima(Não entendi o que seria essa comunicação mínima?)*/
 GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
 
 /* Pino CS (45) deve ser configurado como saída(Não entendi por quê colocou o pino 49 e comentou o 45?) */
 //pinMode(10, OUTPUT); // Esta função permite configurar um pino específico para se comportar como um pino de entrada ou de saída. Por padrão, os pinos do Arduino tem a função de entrada, de modo que não é necessário declará-los neste caso. Já os pinos de saída devem ser configurados por meio dessa função, o que os leva a um estado de baixa impedância, de modo que eles podem fornecer corrente para outros circuitos.
 //digitalWrite(10, HIGH);
 /* Led piscando a cada meio segundo(o LED seria conectado ao pino 7? Como definiram o tempo em que o LED deve piscar?)*/
 //pinMode(7, OUTPUT); // Esta função permite configurar um pino específico para se comportar como um pino de entrada ou de saída. Por padrão, os pinos do Arduino tem a função de entrada, de modo que não é necessário declará-los neste caso. Já os pinos de saída devem ser configurados por meio dessa função, o que os leva a um estado de baixa impedância, de modo que eles podem fornecer corrente para outros circuitos.
 
 /* Inicia e liga o Acelerômetro e Giroscópio */
 Wire.begin(); // Inicia o uso da Biblioteca Wire que permite a comunicação do microcontrolador com dispositivos I2C como o sensor MPU-6050
 Wire.beginTransmission(MPU); // Inicia a transmissão dos dispositivos I2C que vem especificado como o endereço dentro do parêntesis
 Wire.write(0x6B); // Registrador PWR_MGMT_1(Não entendi para que serve?)
 Wire.write(0); // Armazenado valor 0 (Wake-up MPU-6050)(Não entendi para que serve?)
 Wire.endTransmission(true); // Termina a transmissão iniciada logo acima(Não entendi por quê começar e iniciar quase ao mesmo tempo?)
 Wire.beginTransmission(MPU); // Inicia a transmissão dos dispositivos I2C que vem especificado como o endereço dentro do parêntesis(Por quê voltar a iniciar novamente se acabou de encerrar?)

 Wire.write(0x1C); // Registrador MPU6050_ACCEL_CONFIG (Não entendi para que serve?)
 Wire.write(0b00011000); // Altera resolução Acelerômetro para 16G (Não entendi para que serve?)
 Wire.endTransmission(true); // Termina a transmissão iniciada logo acima(Não entendi por quê começar e iniciar quase ao mesmo tempo?)
 
 /* Verifica se o cartão SD está presente e pode ser inicializado */ 
 if (!SD.begin(chipSelect)) { //Por quê a função de iniciar o cartão SD foi usado pra identificar quando deu erro?
 Serial.println(F("E01")); // Escreve a mensagem de erro "E01 : Falha ao iniciar o cartão SD"
 // Aguarda 5 segundos e reinicia o código
 Serial.println(F("E02")); // Escreve a mensagem de erro "E02 : Reset em 5 segundos..."
 delay(5000); //Determina o tempo de espera para ocorrer a próxima função
 // Reinicia Arduino
 resetFunc(); // Função usada para reiniciar o microcontrolador
 }
 /* Abre ou Cria arquivo no cartão SD */
 GPSData = SD.open("log.txt", O_CREAT | O_WRITE | O_APPEND); // Função que abre um arquivo existente no cartão SD ou o cria caso ele ainda não exista(Não consigo entender os parâmetros)
 // Testa ponteiro do arquivo
 if ( ! GPSData ) { // Não entendi por quê usar a condição de criação do arquivo e colocar uma mensagem de erro caso ocorra a criação do arquivo
 int i = millis() + 5000; // 5 segundos Parou o código por 5 segundos, porém usando o millis, o microcontrolador ainda pode executar outras funções, diferentemente de quando utilizamos o delay(Não entendi a sintaxe)
 Serial.println(F("E03")); // Escreve a mensagem de erro "E03 : Impossivel abrir arquivo"
 do { // Não entendo qual a diferença entre else e do?
 } while ( i > millis() ); // Aguarda 5 segundos qual a função dessa sintaxe? se estiverem passado 5 segundos desde a última função, então ocorreu alguma falha ao criar o arquivo
 Serial.println(F("E04")); // Escreve a mensagem de erro "E04 : Falha escrever arquivo! Codigo congelado..."
 do; while (true); // Congela código Nãoo entendi por quê congela com apenas "While(True)"
 } else { // Caso não ocorra a primeira parte do código, executa a seguinte
 Serial.println(F("E05")); // Escreve a mensagem "E05 : Arquivo pronto!"
 }
 /* Interrupção dos sensores */
 OCR0A = 0xAF; // Não entendi a sintaxe e nem por quê se precisa interromper o funcionamento dos sensores
 TIMSK0 |= _BV(OCIE0A); // Não entendi a sintaxe e nem por quê se precisa interromper o funcionamento dos sensores
}
/* Interrupção a cada milisegundo, procura novos dados no GPS e
os armazena */
SIGNAL(TIMER0_COMPA_vect) { // Não entendi a sintaxe e nem por quê se precisa interromper o funcionamento dos sensores
 char c = GPS.read(); // Não entendi a sintaxe e nem pra quê serve
}
/* Inicia timer de refencia para LED e GPS */
uint32_t timer = millis(); // Cria e armazena o tempo de funcionamento decorrido até o momento para usar como referência para o LED (Qual a função?)
uint32_t timer2 = millis(); // Cria e armazena o tempo de funcionamento decorrido até o momento para usar como referência para o LED (Qual a função?)
/* Função para fechar e abrir o arquivo no cartão SD, usado para
'salvar' os dados */

void fechar_SD(void) { // Pode-se criar funções com diversos nomes?
 GPSData.close(); // Para que serve essa sintaxe?
 GPSData = SD.open("log.txt", FILE_WRITE); // Responsável por abrir o arquivo .txt?
}
/* Função que requisita dados para o chip dos sensores, salva os
dados em variáveis */
void requisitar_dados_accel(void) { // Pode-se criar funções com diversos nomes?
 int erro; // Para que serve essa sintaxe em termos práticos?
 AcX = 0; // Armazena o valor 0 no acelerômetro em X
 AcY = 0; // Armazena o valor 0 no acelerômetro em Y
 AcZ = 0; // Armazena o valor 0 no acelerômetro em Z
 Tmp = 0; // Armazena o valor 0 no termômetro
 GyX = 0; // Armazena o valor 0 no giroscópio em X
 GyY = 0; // Armazena o valor 0 no giroscópio em Y
 GyZ = 0; // Armazena o valor 0 no giroscópio em Z
 Wire.beginTransmission(MPU); // Inicia a transmissão dos dispositivos I2C que vem especificado como o endereço dentro do parêntesis
 Wire.write(0x3B); // Começa pelo endereço 0x3B (ACCEL_XOUT_H)(Não entendi para que serve?)
 if (Wire.endTransmission(false) != 0) { // Termina a transmissão iniciada logo acima?(Não entendi a ideia usada para o IF)
 resetFunc(); // Função usada para reiniciar o microcontrolador
 }
 
 Wire.requestFrom(MPU, 14, true); // Leitura de 14 registradores, requisitamento de bytes e dados de um dispositivo escravo, solicitando 14 bytes?
 AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L) // Leitura e armazenamento de acelerômetro em X(Não entendi a sintaxe)
 AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L) // Leitura e armazenamento de acelerômetro em Y(Não entendi a sintaxe)
 AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L) // Leitura e armazenamento de acelerômetro em Z(Não entendi a sintaxe)
 Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L) // Leitura e armazenamento do termômetro(Não entendi a sintaxe)
 GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L) // Leitura e armazenamento do giroscópio em X(Não entendi a sintaxe)
 GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L) // Leitura e armazenamento do giroscópio em Y(Não entendi a sintaxe)
 GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L) // Leitura e armazenamento do giroscópio em Z(Não entendi a sintaxe)
}

/* Função para gravar os dados dos sensores no cartão SD */
void gravar_SD_accel(void) { //Não entendi a condição nem a sintaxe
  
 GPSData.print(F("Acelerômetro em X: "));
 GPSData.print(AcX); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print(F(";")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 Serial.println();

 GPSData.print(F("Acelerômetro em Y: "));
 GPSData.print(AcY); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print(F(";")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 Serial.println();

 GPSData.print(F("Acelerômetro em Z: "));
 GPSData.print(AcZ); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print(F(";")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 Serial.println();

 GPSData.print(F("Giroscópio em X: "));
 GPSData.print(GyX); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print(F(";")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 Serial.println();

 GPSData.print(F("Giroscópio em Y: "));
 GPSData.print(GyY); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print(F(";")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 Serial.println();

 GPSData.print(F("Giroscópio em Z: "));
 GPSData.print(GyZ); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print(F(";")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns 
 Serial.println();

 GPSData.print(F("Temperatura: "));
 GPSData.print(Tmp / 340.00 + 36.53); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print(F(";")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 Serial.println();
 return;
}

/* Função para gravar os dados do GPS no cartão SD, se a condição
for verdadeira */
void gravar_SD_gps(bool a) { //Não entendi a condição nem a sintaxe
 if (a == true) { //Onde "a" assume o valor de verdadeiro ou falso?
  
 GPSData.print(F("Validação dos dados do GPS: '1' leitura válida e '0' leitura inválida: "));
 GPSData.print((int)GPS.fix); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print(F(";")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 Serial.println();
 
 GPSData.print(F("Latitude: "));
 GPSData.print(GPS.latitude, DEC); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print(F(";")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 Serial.println();

 GPSData.print(F("Longitude: "));
 GPSData.print(GPS.longitude, DEC); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print(F(";")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 Serial.println();
 
 GPSData.print(F("Altitude: "));
 GPSData.print(GPS.altitude, DEC); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print(F(";")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 Serial.println();

 GPSData.print(F("Velocidade: "));
 GPSData.print(GPS.speed, DEC); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print(F(";")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 Serial.println();
 
 GPSData.print(F("Hora: "));
 GPSData.print(GPS.hour, DEC); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print(F(";")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 
 GPSData.print(F("Minutos: "));
 GPSData.print(GPS.minute, DEC); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print(F(";")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns 

 GPSData.print(F("Segundos: "));
 GPSData.println(GPS.seconds, DEC); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 Serial.println();
 } else {
 GPSData.println(F(";No fix")); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 }
 return;
}

/*
================================================================
//
// === MAIN LOOP
=== //
//
================================================================
*/
void loop() //a função loop() faz precisamente o que seu nome indica: ela repete-se continuamente permitindo que seu programa funcione dinamicamente. É utilizada para controlar de forma ativa a placa Arduino.
{
 // Garante que a condição inicia em falsa
 bool caso = false; // a variável "caso" recebe o valor falso

// Avisa ao contador watchdog que o programa ainda esta rodando
 wdt_reset(); //Essa sintaxe é fixa? Ela dita o quê precisamente?

 // Caso o contador interno millis dê overflow, reinicia contador timer
 // o quê seria o contador dar overflow? Seria o número de dados de entrada exceder a capacidade disponível?
 if (timer > millis()) timer = millis(); //Não entendi direito a lógica da programação
 if (timer2 > millis()) timer2 = millis(); //Não entendi direito a lógica da programação
 
 // Quando uma mensagem NMEA for recebida, confere o checksum
 if (GPS.newNMEAreceived()) { //O que seriam essas mensagens NMEA?
 if (!GPS.parse(GPS.lastNMEA())) //O que a sintaxe significa?
 return; // Caso ocorra erro em processar uma mensagem, aguarda a proxima
 }
 // A cada 5 segundos, executa esta função
 if (millis() - timer > 5000) { //Se o tempo decorrido de execução do programa menos o valor da variável timer for maior que 5 segundos timer recebe millis(Fiquei um pouco confuso com a lógica)
 timer = millis(); // reinicia o timer
 // Incrementa conta_ciclos a cada 5 segundos.
 conta_ciclos++; //Para que serve no programa toda essa amarração de contadores?
 
 // Manda dados do GPS pela serial, se o arduino estiver conectado em um PC
 Serial.print(GPS.hour, DEC); Serial.print(':'); //Essas variáveis são caracteristicas do GPS? Para que serve o "DEC"?
 Serial.print(GPS.minute, DEC); Serial.print(':'); //Essas variáveis são caracteristicas do GPS? Para que serve o "DEC"?
 Serial.println(GPS.seconds, DEC); //Essas variáveis são caracteristicas do GPS? Para que serve o "DEC"?
 Serial.print(F("Fix: ")); Serial.println((int)GPS.fix);//Essas variáveis são caracteristicas do GPS? Para que serve o "Fix"?
 Serial.print(F("AcX: ")); Serial.println(AcX);//Por quê só usou o acelerômetro em X?
 Serial.print(F("Index: ")); Serial.println(indexador);//Para que serve um Indexador?
 // Permite que os dados do GPS sejam gravados no cartão SD(Por quê?)
 caso = true;
 // Zera contador de amostras dos sensores
 indexador = 0;
 }
 // Executa função de requisição de dados dos sensores(No caso antes do void loop só estavam sendo criadas as funções? Ou a ordem das funções interferem no funcionamento mesmo sem estar no void loop?)
 requisitar_dados_accel();

 indexador++; //Adicionou mais 1 no indexador(Não estou conseguindo compreender a lógica de programação no geral)
 GPSData.print(indexador); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 GPSData.print("&"); //Não consigo entender como ocorre a retirada dos dados do GPS e nem as sintaxes utilizadas são encontradas em fóruns
 // Grava dados dos sensores no cartão SD
 gravar_SD_accel(); //As funções estão sendo chamadas do void setup?
 // Grava dados do GPS no cartão SD, se condição verdadeira
 gravar_SD_gps(caso); //As funções estão sendo chamadas do void setup? Caso sempre vai ser verdadeira?
 // Controle de intervalo de aquisição de dados
 delay(40); //Tempo de intervalo para a aquisição de novos dados

 //A cada 15 segundos (3x5seg), fecha e abre arquivo no cartão
 if (conta_ciclos > 3) { //Por quê conta ciclos maior que 3? E por quê abrir e fechar o arquivo?
 Serial.println(F("gravou")); //Não entendo o uso desse "F"
 fechar_SD(); //As funções estão sendo chamadas do void setup?
 conta_ciclos = 0;
 
 //Caso ocorra erro na gravação, reinicia
 if (GPSData == 0) { //De onde vem esse valor de GPSData?
 Serial.println(F("RESET")); //Não entendo o uso desse "F"
 delay(100); //Tempo de intervalo para a aquisição de novos dados
 resetFunc(); //Na prática para que serve a ResetFunc?
 }
 }

 // Rotina de Piscar o LED (Para quê uma rotina de LED?)
 if (millis() - timer2 > 1000) { //Se a diferença do timer para o tempo de ocorrência do programa for maior que 1 segundo, então o timer recebe o millis
 timer2 = millis(); // reset timer
 if (led_negate == true) { //Essa variável led negate tem o valor retirado de onde?
 digitalWrite(7, HIGH); //as sintaxes utilizadas não são encontradas em fóruns
 led_negate = false; // Por quê a mudança de valores?
 } else {
 digitalWrite(7, LOW); //as sintaxes utilizadas não são encontradas em fóruns
 led_negate = true; // Por quê a mudança de valores?
 }
 }
// Fim
}
