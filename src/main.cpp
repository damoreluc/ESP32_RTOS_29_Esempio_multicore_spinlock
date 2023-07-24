/*
 * FreeRTOS Esercizio 29: Esempio illustrativo di task e spinlock/sezioni critiche in sistemi multicore
 *
 * Lo spinlock in FreeRTOS costringe la CPU a bloccare l'esecuzione degli interrupt 
 * e quindi dello scheduler del sistema operativo.
 * Nel porting di FreeRTOS operato da Espressif, lo spinlock opera su ogni core che lo interroga.
 *
 * Il programma di esempio analizza l'effetto di uno spinlock condiviso tra task 
 * che girano su due core distinti della ESP32.
 * 
 * Il Task0, sul core0, commuta il led sul pin_0 ogni task_0_delay ms. 
 * Inizialmente non si impiega lo spinlock in questo task.
 *
 * Il Task1 sul Core1 impiega lo spinlock per proteggere una sezione critica di lunga durata. 
 * Il Task1 entra nella sezione critica, commuta il led sul pin_1 e poi simula una lunga operazione 
 * non interrompibile e infine rilascia lo spinlock: per tutto questo tempo lo scheduler sul Core1 
 * resta bloccato.
 * 
 * Analizzare con l'oscilloscopio le temporizzazioni dei segnali su pin_0 e pin_1
 * nei due casi:
 * a) senza lo spinlock in Task0;
 * b) con lo spinlock attivato in entrambi i task.
 *
 * Nota: nel file soc.h sono definiti i riferimenti ai due core della ESP32:
 *   #define PRO_CPU_NUM (0)
 *   #define APP_CPU_NUM (1)
 *
 * Qui vengono adoperati entrambi i core della CPU.
 *
 */

#include <Arduino.h>

// Impostazioni  ***************************************************************
// il core1 viene monopolizzato dal Task1 per time_hog ms
static const TickType_t time_hog = 1;
// Tempo di pausa (ms) del Task 0
static const TickType_t task_0_delay = 30;
// Tempo di pausa (ms) del Task 1
static const TickType_t task_1_delay = 100;

// Pin  ************************************************************************

// pin del led gestito dal task 0 sul Core0
static const int pin_0 = 22;
// pin del led gestito dal task 1 sul Core1
static const int pin_1 = 23;

// Variabili Globali  **********************************************************

// spinlock di protezione delle sezioni critiche
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

//******************************************************************************
// Funzioni ausiliarie

// monopolizza il processore. Temporizzazione accurata per tempi <= 1 secondo
static void hog_delay(uint32_t ms)
{
  for (uint32_t i = 0; i < ms; i++)
  {
    for (uint32_t j = 0; j < 40000; j++)
    {
      asm("nop"); // istruzione NO-OPERATION della CPU
    }
  }
}

//*****************************************************************************
// Tasks

// task sul core 0
void doTask0(void *parameters)
{

  // Configura il pin del led
  pinMode(pin_0, OUTPUT);

  // Ciclo infinito del task
  while (1)
  {

    // Commuta il pin del led (provare a chiuderlo in una sezione critica con lo spinlock)
    portENTER_CRITICAL(&spinlock);
    digitalWrite(pin_0, !digitalRead(pin_0));
    portEXIT_CRITICAL(&spinlock);

    // rilascia il processore per un po' di tempo
    vTaskDelay(pdMS_TO_TICKS(task_0_delay));
  }
}

// task sul core 1
void doTask1(void *parameters)
{

  // Configura il pin del led
  pinMode(pin_1, OUTPUT);

  // Ciclo infinito del task
  while (1)
  {

    // esegue una sezione critica di lunga durata (NON è mai una buona idea!)
    portENTER_CRITICAL(&spinlock);
    digitalWrite(pin_1, HIGH);
    hog_delay(time_hog);
    digitalWrite(pin_1, LOW);
    portEXIT_CRITICAL(&spinlock);

    // rilascia il processore per un po' di tempo
    vTaskDelay(pdMS_TO_TICKS(task_1_delay));
  }
}


//************************************************************************************
// Main (sul core 1, con priorità 1)

// configurazione del sistema
void setup()
{
  // Configurazione della seriale
  Serial.begin(115200);

  // breve pausa
  vTaskDelay(pdMS_TO_TICKS(1000));
  Serial.println();
  Serial.println("FreeRTOS Esempio di spinlock e multicore");

  // Crea e avvia i due task;

  // Start Task L (sul core 0)
  xTaskCreatePinnedToCore(doTask0,
                          "Task 0",
                          1024,
                          NULL,
                          1,
                          NULL,
                          PRO_CPU_NUM);

  // Start Task H (sul core 1)
  xTaskCreatePinnedToCore(doTask1,
                          "Task 1",
                          1024,
                          NULL,
                          1,
                          NULL,
                          APP_CPU_NUM);

  // Elimina il task con "Setup e Loop"
  vTaskDelete(NULL);
}

void loop()
{
  // lasciare vuoto
}
