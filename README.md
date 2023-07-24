# FreeRTOS Esercizio 29: Esempio illustrativo di task e spinlock/sezioni critiche in sistemi multicore

Lo _spinlock_ in FreeRTOS costringe la CPU a bloccare l'esecuzione degli interrupt e quindi dello scheduler del sistema operativo.
Nel porting di FreeRTOS operato da Espressif, lo spinlock opera __su ogni core__ che lo interroga.

Il programma di esempio analizza l'effetto di uno spinlock __condiviso__ tra task che girano su due core distinti della ESP32.

Il __Task0__ sul __Core0__ commuta il led sul __pin_0__ ogni `task_0_delay ms`. Inizialmente non si impiega lo spinlock in questo task.

Il __Task1__ sul __Core1__ impiega lo spinlock per proteggere una sezione critica di lunga durata. Il __Task1__ entra nella sezione critica, commuta il led sul __pin_1__ e poi simula una lunga operazione non interrompibile e infine rilascia lo spinlock: per tutto questo tempo lo scheduler sul __Core1__  resta bloccato.

Analizzare con l'oscilloscopio le temporizzazioni dei segnali su __pin_0__ e __pin_1__ nei due casi:

1. __senza__ lo spinlock in __Task0__;
2. __con__ lo spinlock attivato __in entrambi__ i task.
