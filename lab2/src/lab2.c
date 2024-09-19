#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

// Variables globales
int intr_count = 0;
int sec = 0;
int msec = 0;
int color = 0;
int estado;
int guess_length = 4;
int sequence_index = 0;
int j;
int tiempo = 10;
int counter = 0;
int helper;
int intb6_mask=0;
int inta0_mask=0;
int limite = 61; // 1/2 segundo
int int_boton = 0;
int parpadeos = 0;
int secuencia_ingresada [10] = {};
int secuencia_actual[10] = {};
int secuencia[10] = {};
int nivel = 4;
int delay = 120;
int p = 0;
int boton = 0;


//Definicion de estados para FSM
#define e1 1 // secuencia inicial
#define e2 2 // imprime secuencia
#define e3 3 // lee botones
#define e4 4 // cambio de nivel
#define e5 5 // Game finished
#define e6 6 //Waiting Time

int estado; // Se define el estado base o default.


// GENERADOR DE NUMEROS RANDOM
uint8_t rotl(const uint8_t x, int k) {
    return (x << k) | (x >> (8 - k));
}
//semilla
uint8_t s[2] = { 0, 0xF };

uint8_t next(void) {
    uint8_t s0 = s[0];
    uint8_t s1 = s[1];
    uint8_t result = s0 + s1;

    s1 ^= s0;
    s[0] = rotl(s0, 6) ^ s1 ^ (s1 << 1);
    s[1] = rotl(s1, 3);

    return result;
}
// Acota el numero entre 0 y 3
int rand(void) {
    int randi = 0;
    while (1) {
        randi = next();
        if (randi < 5 && randi >= 1) {
           return randi;
        }
    }

}

void encender(int led){
  if(led==1){
    PORTB = 0x8;
  }
  else if(led==2){
    PORTB = 0x1;
  }
  else if(led==3){
    PORTB = 0x4;
  }
  else if(led==4){
    PORTB = 0x2;
  }
}

void espera(int delay)
{
  int i=0;
  while(i<=delay){
    while((TIFR & (1 << OCF0A))==0){
      ;
    }
    TIFR|=(1<<OCF0A);    
    i++;               
  }
}

void setup() {//funciones para configurar puertos de semaforo
  DDRB = 0x3F; //Configuracion del puerto
  //DEFINICIÓN DE ENTRADAS CON INTERRUPCIONES 
  GIMSK |= (1<<INT0);
  GIMSK |= (1<<INT1); // interrupciones externas en D2 y D3
  GIMSK |= (1<<PCIE1);  // interrupciones por cambio de pin en A0 
  GIMSK |= (1<<PCIE2);    // y D0
  MCUCR |= (1<<ISC11);    // se configura con flanco negativo
  MCUCR |= (1<<ISC01);
  PCMSK2 |= 0b00000001; // Habilita interrupciones en pin D0
  PCMSK1 |= 0b00000001; // Habilita interrupciones en pin A0
  TCCR0A=0x00;
  TCCR0B=0x00;
  TCCR0B |= (1<<CS00)|(1<<CS02);   //prescaling with 1024
  TCCR0A|=(1<<WGM01);//toggle mode and compare match  mode
  OCR0A= 255; //compare value
  TCNT0=0;
  sei(); 
}

int main(void)
{
  setup();
  while(p<=10){
    secuencia[p] = rand();
    p++;
  }
  p=0;
  estado = e1;
  while (1) {
    switch (estado){
      case e1: //esperando iniciar
        if (color > 0){  //secuencia de inicio
            PORTB |= (1<<PB4);
            PORTB = 0xF;
            espera(40);
            PORTB = 0x0;
            espera(40);
            PORTB = 0xF;
            espera(40);
            PORTB = 0x0;
            estado = e2;
        }
        else if (color==0) {
          estado=e1;
        }
        break;

      case e2: //imprime secuencia
        espera(10);
        while(p<nivel){
          secuencia_actual[p] = secuencia[p];
          espera(delay);
          encender(secuencia[p]);
          espera(delay);
          PORTB = 0x0;
          p++;
        }
        p=0;
        boton=0;
        j=0;
        estado = e3;
        break;

      case e3:
        while(j<nivel){
          PORTB = 0x10;
          if(boton){
            boton=0;
            encender(color);
            espera(10);
            if(secuencia[j]==color){
              j++;
            }
            else{
              color=0;
              PORTB = 0xF;
              espera(20);
              PORTB = 0x0;
              espera(20);
              PORTB = 0xF;
              espera(20);
              PORTB = 0x0;
              espera(20);
              PORTB = 0xF;
              espera(20);
              PORTB = 0x0;
              nivel=4;
              p=0;
              j=0;
              estado=e1;
              break;
            }
          }
        }
        if(j==nivel){
          estado=e4;
        }
        break;
      
      case e4:
        PORTB = 0x30;
        espera(100);
        PORTB = 0x00;
        if (nivel>10){
          estado = e5;
          break;
        }
        else{
          delay=delay-12;
          nivel++;
          estado=e2;
        }

        break;

      case e5:
        PORTB = 0x30;
        espera(10);
        PORTB = 0x10;
        PORTB = 0x30;
        espera(10);
        PORTB = 0x0;
        espera(10);
        PORTB = 0x30;
        espera(10);
        PORTB = 0x0;
        estado = e1;
        color = 0;
        nivel = 4;
        p=0;
        break;
      default:
        break;
    }
  }
}


ISR (TIMER0_OVF_vect){ 
  //msec++;
}

ISR (PCINT2_vect){        // Interrupcion por boton rojo
  intb6_mask = PIND & 0b1;
  if (intb6_mask){
    color = 1;
    boton=1;
  }
}

ISR (PCINT1_vect){        // Interrupcion por boton amarillo
  inta0_mask = PINA & 0b1;
  if (inta0_mask){
    color = 2;
    boton=1;
  }
}

ISR (INT0_vect){        // Interrupcion por boton verde
  color = 3;
  boton=1;
} 

ISR (INT1_vect){        // Interrupcion por boton azul
  color = 4;
  boton=1;
}