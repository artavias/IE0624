#include <TensorFlowLite.h>
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "CircularBuffer.h"
#include "model.h"

// Constantes globales
namespace {
  const int TAMANO_VENTANA = 10; // Numero de intervalos requeridos por el modelo
  constexpr int kTensorArenaSize = 2 * 1024; // Memoria para TensorFlow Lite
  uint8_t tensor_arena[kTensorArenaSize]; // Espacio de memoria para los tensores

  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
}

// Clase para detectar picos
class DetectorPicos {
private:
    int threshold; // Umbral para determinar un pico
    bool ultimo_encima_de_umbral; // Indica si el ultimo valor estaba por encima del umbral

public:
    DetectorPicos(int threshold) : threshold(threshold), ultimo_encima_de_umbral(false) {}

    bool detectar(int valor) {
        bool es_pico = (valor > threshold && !ultimo_encima_de_umbral);
        ultimo_encima_de_umbral = (valor > threshold);
        return es_pico;
    }
};

// Clase para calcular media móvil
template<typename T, typename S, int tamanoBuffer>
class FiltroMediaMovil {
private:
    CircularBuffer<T, tamanoBuffer> buffer;
    S suma;

public:
    FiltroMediaMovil() : suma(0) {}

    void agregar(T valor) {
        if (buffer.isFilled()) {
            suma -= buffer[0];
        }
        suma += valor;
        buffer.push(valor);
    }

    float promedio() const {
        return static_cast<float>(suma) / buffer.size();
    }

    bool estaLleno() const {
        return buffer.isFilled();
    }
};

// Instancias globales
FiltroMediaMovil<int, int, 10> filtro; // Filtro
CircularBuffer<int, TAMANO_VENTANA> buffer_intervalos_picos; // Buffer para intervalos de picos
DetectorPicos detector_picos(750); // Detector de picos con umbral 750

// Funcion para controlar LEDs segun la prediccion
void encender_leds(float prediccion) {
  digitalWrite(LEDG, HIGH); // Apagar LED verde
  digitalWrite(LEDR, HIGH); // Apagar LED rojo

  if (prediccion > 0.5f) {
    digitalWrite(LEDR, LOW); // Encender LED rojo
  } else {
    digitalWrite(LEDG, LOW); // Encender LED verde
  }
}

void setup() {
  // Configuración inicial
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  digitalWrite(LEDR, HIGH); // LEDs apagados
  digitalWrite(LEDG, HIGH);

  // Configurar TensorFlow Lite
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  model = tflite::GetModel(model_tflite);

  static tflite::MicroMutableOpResolver<3> op_resolver;
  op_resolver.AddFullyConnected();
  op_resolver.AddLogistic();
  op_resolver.AddRelu();

  static tflite::MicroInterpreter static_interpreter(
      model, op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  interpreter->AllocateTensors();
}

void loop() {
  static long ultimo_tiempo_pico = millis(); // Tiempo del ultimo pico detectado
  int muestra = analogRead(A0); // Leer senal analogica del pin A0

  // Filtrar senal con media movil
  filtro.agregar(muestra);
  int valor_filtrado = filtro.promedio();

  // Mostrar valor filtrado en el monitor serie
  Serial.println(valor_filtrado);

  // Detectar picos
  if (detector_picos.detectar(valor_filtrado)) {
    long tiempo_actual = millis();
    int intervalo = tiempo_actual - ultimo_tiempo_pico;
    ultimo_tiempo_pico = tiempo_actual;

    // Guardar intervalo en el buffer de picos
    buffer_intervalos_picos.push(intervalo);

    // Si el buffer de intervalos ests lleno realizar inferencia
    if (buffer_intervalos_picos.isFilled()) {
      for (int i = 0; i < TAMANO_VENTANA; ++i) {
        interpreter->input(0)->data.f[i] = static_cast<float>(buffer_intervalos_picos[i]) / 1000.0; // Normalizacion
      }

      // Ejecutar el modelo
      if (interpreter->Invoke() == kTfLiteOk) {
        float prediccion = interpreter->output(0)->data.f[0];
        encender_leds(prediccion); // Controlar LEDs según la predicción
      }
    }
  }

  delay(10); // Retardo para evitar lecturas excesivas
}