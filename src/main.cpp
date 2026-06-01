// ============================================================
// VIRTUALFRIEND - Prototipo Nivel 1
// Compañero virtual para el coche
// Placa: K128 (ESP32)
// ============================================================
//
// Este es el archivo principal del proyecto.
// Cuando subes el programa a la placa, este archivo es
// el que controla todo.
//
// En Arduino/ESP32, todo programa tiene DOS funciones
// obligatorias:
//
//   setup()  → Se ejecuta UNA SOLA VEZ al encender la placa.
//              Aquí se prepara todo: puertos, pantalla, etc.
//
//   loop()   → Se ejecuta continuamente en un bucle infinito
//              después de setup(). Es el "corazón" del programa.
//              Cada vez que termina, vuelve a empezar desde
//              el principio.
//
// Imagina que el programa es una persona de guardia:
//   - Al llegar (setup): se pone el uniforme, enciende las
//     luces, anota que ha empezado el turno.
//   - Mientras trabaja (loop): mira alrededor, detecta si
//     pasa algo, reacciona, y vuelve a mirar.
// ============================================================

// ---- Incluir nuestros archivos de código ----
// El símbolo #include "archivo.h" le dice al compilador
// que pegue el contenido de ese archivo aquí.
// Con comillas ("") buscamos en la carpeta del proyecto.
// Con ángulos (<>) buscamos en las librerías del sistema.
#include <Arduino.h>     // Funciones base de Arduino: Serial, delay, random, etc.
#include "eventos.h"     // Define EventoSistema (el enum de eventos)
#include "estados.h"     // Define EstadoCompanero (el enum de estados)
#include "expresiones.h" // Define mostrarExpresion()
#include "sonidos.h"     // Define las funciones de sonido
#include "frases.h"      // Define elegirFrase() y los arrays de frases
#include "simulador.h"   // Define leerEventoSimuladoRaw() y numeroAEvento()

// ============================================================
// VARIABLES GLOBALES
// ============================================================
//
// ¿Qué es una variable global?
// Una variable declarada fuera de cualquier función.
// Puede ser leída y modificada desde cualquier parte del código.
//
// Se usan con cuidado: demasiadas variables globales hacen
// el código difícil de seguir. Aquí solo usamos las
// imprescindibles para que el programa "recuerde" el estado
// actual entre una iteración del loop y la siguiente.
// ============================================================

// Estado actual del compañero virtual
// Empieza en INICIO y cambia según los eventos que lleguen.
EstadoCompanero estadoActual = EstadoCompanero::INICIO;

// Último evento procesado
// Lo guardamos para detectar si el evento ha CAMBIADO.
// Si el usuario sigue enviando el mismo número, no queremos
// que el personaje repita la misma frase una y otra vez.
EventoSistema ultimoEvento = EventoSistema::NORMAL;

// ¿Es la primera iteración del loop?
// Se usa para mostrar el mensaje de bienvenida solo una vez.
bool primerArranque = true;

// ============================================================
// PROTOTIPOS DE FUNCIONES
// ============================================================
//
// ¿Qué es un prototipo?
// En C++, antes de usar una función, el compilador necesita
// saber que existe. Si la definición está más abajo en el
// archivo, ponemos el "prototipo" aquí arriba: solo el nombre,
// los parámetros y el tipo de retorno, sin el cuerpo.
//
// Esto permite ordenar el código de forma lógica:
// las funciones importantes (setup, loop) arriba,
// los detalles de implementación abajo.
// ============================================================
void iniciarSistema();
EventoSistema leerEvento();
void reaccionarAEvento(EventoSistema evento);
void cambiarEstado(EstadoCompanero nuevoEstado, const char* nombreEstado);
void mostrarMensaje(const char* mensaje);

// ============================================================
// setup() — SE EJECUTA UNA SOLA VEZ AL ENCENDER
// ============================================================
void setup() {
    // Iniciar la comunicación con el ordenador a través del USB
    // El número 115200 es la "velocidad" en baudios (bits por segundo).
    // Debe coincidir con la velocidad configurada en el Monitor Serie.
    // Si no coincide, el texto aparece como símbolos extraños.
    Serial.begin(115200);

    // Esperar un momento para que el Monitor Serie esté listo.
    // delay(ms) pausa el programa el número de milisegundos indicado.
    // 1000ms = 1 segundo.
    delay(1000);

    // Semilla aleatoria: para que random() no dé siempre los mismos
    // resultados. analogRead(0) lee ruido del pin 0, que es diferente
    // cada vez, y se usa como semilla del generador pseudoaleatorio.
    randomSeed(analogRead(0));

    // Llamar a la función que prepara todo el sistema
    iniciarSistema();
}

// ============================================================
// loop() — SE EJECUTA CONTINUAMENTE
// ============================================================
//
// Este es el bucle principal del programa.
// Cada vez que termina, vuelve a empezar.
// La placa lo ejecuta miles de veces por segundo.
//
// delay(500) al final hace que el bucle espere medio segundo
// antes de volver a empezar. Esto evita que el programa
// procese demasiado rápido y que el personaje reaccione
// varias veces al mismo evento por accidente.
// ============================================================
void loop() {
    // En la primera iteración, mostrar el menú de simulación
    if (primerArranque) {
        mostrarMenuSimulador();
        primerArranque = false;
    }

    // Leer el evento actual (desde simulación o sensores reales)
    EventoSistema eventoActual = leerEvento();

    // Solo reaccionar si el evento ha CAMBIADO respecto al anterior.
    // Si el usuario no ha escrito nada nuevo, eventoActual será
    // igual a ultimoEvento y el personaje no repetirá su reacción.
    if (eventoActual != ultimoEvento) {
        reaccionarAEvento(eventoActual);
        ultimoEvento = eventoActual;
    }

    // Pausa de medio segundo antes de la siguiente comprobación
    delay(500);
}

// ============================================================
// iniciarSistema()
// ============================================================
// Se llama una sola vez desde setup().
// Prepara el sistema, muestra la expresión inicial y
// reproduce el sonido y la frase de arranque.
// ============================================================
void iniciarSistema() {
    Serial.println();
    Serial.println("============================================");
    Serial.println("  [BOOT] Iniciando VirtualFriend...");
    Serial.println("============================================");

    // Mostrar expresión de "dormido" durante el arranque
    mostrarExpresion("Dormido");

    // Reproducir sonido de inicio
    sonidoInicio();

    // Elegir y mostrar una frase de inicio
    // frasesInicio está definida en frases.h
    int indiceAleatorio = random(NUM_FRASES_INICIO);
    Serial.print("[FRASE] ");
    Serial.println(frasesInicio[indiceAleatorio]);

    // Pasar al estado normal después del arranque
    cambiarEstado(EstadoCompanero::NORMAL, "NORMAL");
}

// ============================================================
// leerEvento()
// ============================================================
// Función "puente" entre el simulador y los sensores reales.
// El resto del programa llama SIEMPRE a leerEvento(),
// nunca directamente al simulador ni a los sensores.
//
// Si MODO_SIMULACION está activado, lee del Monitor Serie.
// Si no, leería de los sensores reales de la K128.
//
// ¿Por qué este diseño?
// Porque cuando conectemos los sensores reales, SOLO hay
// que cambiar esta función. El resto del programa no sabe
// ni le importa de dónde vienen los eventos.
// ============================================================
EventoSistema leerEvento() {
    if (MODO_SIMULACION) {
        int raw = leerEventoSimuladoRaw();

        // Si no hay dato nuevo, devolver el último evento conocido
        // para que el loop() no detecte "cambio" y no reaccione.
        if (raw == -1) {
            return ultimoEvento;
        }

        // Convertir el número recibido en un EventoSistema
        return numeroAEvento(raw);
    } else {
        // FUTURO: aquí irá la llamada al sensor real de la K128
        // return leerEventoRealK128();
        return EventoSistema::NORMAL;
    }
}

// ============================================================
// reaccionarAEvento(evento)
// ============================================================
// Decide cómo reacciona el personaje ante un evento.
// Para cada evento:
//   1. Cambia el estado
//   2. Muestra la expresión adecuada
//   3. Reproduce el sonido adecuado
//   4. Elige y muestra una frase
//
// ¿Qué es un parámetro de función?
// Es un valor que le pasamos a la función cuando la llamamos.
// Aquí, "evento" recibe el EventoSistema que llegó.
// La función trabaja con ese valor internamente.
// ============================================================
void reaccionarAEvento(EventoSistema evento) {
    Serial.println();

    // Imprimir el nombre del evento detectado
    Serial.print("[EVENTO] ");

    switch (evento) {
        case EventoSistema::NORMAL:
            Serial.println("Normal");
            mostrarExpresion("Normal");
            sonidoConfirmacion();
            cambiarEstado(EstadoCompanero::NORMAL, "NORMAL");
            break;

        case EventoSistema::FRENAZO_SUAVE:
            Serial.println("Frenazo suave");
            mostrarExpresion("Sarcastico");
            sonidoConfirmacion();
            cambiarEstado(EstadoCompanero::SARCASTICO, "SARCASTICO");
            break;

        case EventoSistema::FRENAZO_FUERTE:
            Serial.println("Frenazo fuerte");
            mostrarExpresion("Asustado");
            sonidoAlerta();
            cambiarEstado(EstadoCompanero::ASUSTADO, "ASUSTADO");
            break;

        case EventoSistema::VOLANTAZO:
            Serial.println("Volantazo");
            mostrarExpresion("Asustado");
            sonidoAlerta();
            cambiarEstado(EstadoCompanero::ASUSTADO, "ASUSTADO");
            break;

        case EventoSistema::IMPACTO:
            Serial.println("Impacto");
            mostrarExpresion("Alerta");
            sonidoAlerta();
            cambiarEstado(EstadoCompanero::ALERTA, "ALERTA");
            break;

        case EventoSistema::REPOSO:
            Serial.println("Reposo");
            mostrarExpresion("Dormido");
            sonidoReposo();
            cambiarEstado(EstadoCompanero::REPOSO, "REPOSO");
            break;

        case EventoSistema::DESCONOCIDO:
        default:
            Serial.println("Desconocido");
            mostrarExpresion("Error");
            sonidoError();
            cambiarEstado(EstadoCompanero::ERROR_, "ERROR");
            break;
    }

    // Elegir y mostrar la frase correspondiente al evento
    const char* frase = elegirFrase(evento);
    Serial.print("[FRASE] ");
    Serial.println(frase);
    Serial.println();
}

// ============================================================
// cambiarEstado(nuevoEstado, nombreEstado)
// ============================================================
// Actualiza la variable global "estadoActual" y lo imprime
// por el Monitor Serie para que podamos ver el estado actual.
//
// Recibe dos parámetros:
//   - nuevoEstado: el valor del enum EstadoCompanero
//   - nombreEstado: el nombre como texto, para imprimirlo
//
// ¿Por qué pasar el nombre como texto si ya tenemos el enum?
// Porque imprimir el valor de un enum en Arduino no da el
// nombre, sino el número. Pasarlo como texto es la forma
// más sencilla en el Prototipo Nivel 1.
// ============================================================
void cambiarEstado(EstadoCompanero nuevoEstado, const char* nombreEstado) {
    estadoActual = nuevoEstado;
    Serial.print("[ESTADO] ");
    Serial.println(nombreEstado);
}

// ============================================================
// mostrarMensaje(mensaje)
// ============================================================
// Función de utilidad para imprimir cualquier mensaje
// genérico por el Monitor Serie.
// ============================================================
void mostrarMensaje(const char* mensaje) {
    Serial.println(mensaje);
}
