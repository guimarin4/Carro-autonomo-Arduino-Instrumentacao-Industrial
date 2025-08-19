#include <Stepper.h>

// === MOTOR DE PASSO (RADAR) ===
const int stepsPerRevolution = 2048;
const float stepsPerDegree = stepsPerRevolution / 360.0;
Stepper myStepper = Stepper(stepsPerRevolution, 8, 4, 5, 3);

// === ULTRASSÔNICO ===
const int trigPin = 6;
const int echoPin = 7;

// === SENSOR IR ===
const int irSensorPin = 2;

// === MOTOR DC (Ponte H) ===
int m1a = 9;
int m1b = 10;
int m2a = 11;
int m2b = 12;

// === BLUETOOTH ===
char comando = 'S';

// === CONFIGURAÇÃO DE ANGULOS DO RADAR ===
const int angleStep = 20;
const int totalAngle = 180;

const int distanciaLimite = 20;  // Limite para considerar obstáculo (cm)

// === FUNÇÕES DE MOVIMENTO ===
void parar() {
    digitalWrite(m1a, LOW);
    digitalWrite(m1b, LOW);
    digitalWrite(m2a, LOW);
    digitalWrite(m2b, LOW);
}

void frente() {
    digitalWrite(m1a, HIGH);
    digitalWrite(m1b, LOW);
    digitalWrite(m2a, HIGH);
    digitalWrite(m2b, LOW);
}

void re() {
    digitalWrite(m1a, LOW);
    digitalWrite(m1b, HIGH);
    digitalWrite(m2a, LOW);
    digitalWrite(m2b, HIGH);
}

void esquerda() {
    digitalWrite(m1a, LOW);
    digitalWrite(m1b, HIGH);
    digitalWrite(m2a, HIGH);
    digitalWrite(m2b, LOW);
}

void direita() {
    digitalWrite(m1a, HIGH);
    digitalWrite(m1b, LOW);
    digitalWrite(m2a, LOW);
    digitalWrite(m2b, HIGH);
}

// === FUNÇÃO ULTRASSÔNICO ===
long readDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 25000);
    if (duration == 0) return -1;
    return duration * 0.034 / 2;
}

// === SETUP ===
void setup() {
    Serial.begin(9600);

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(irSensorPin, INPUT);

    pinMode(m1a, OUTPUT);
    pinMode(m1b, OUTPUT);
    pinMode(m2a, OUTPUT);
    pinMode(m2b, OUTPUT);

    myStepper.setSpeed(10);

    parar();
}

// === LOOP ===
void loop() {
    bool linhaPreta = (digitalRead(irSensorPin) == HIGH);

    if (linhaPreta) {
        Serial.println("Linha preta detectada — Iniciando varredura!");

        int steps90 = round(stepsPerDegree * 90);
        int stepsPerAngle = round(stepsPerDegree * angleStep);

        // Vai para a posição inicial (90 graus horário)
        myStepper.step(steps90);
        Serial.println("Girou 90 graus horário — posição inicial.");
        delay(300);

        int stepsAccumulated = 0;

        int numPositions = (totalAngle / angleStep) + 1;
        long distancias[numPositions];

        for (int i = 0; i < numPositions; i++) {
            long distance = readDistance();
            distancias[i] = distance;

            Serial.print("Posicao ");
            Serial.print(i);
            Serial.print(" — Distancia: ");
            if (distance == -1) {
                Serial.println("Sem leitura");
            } else {
                Serial.print(distance);
                Serial.println(" cm");
            }

            if (i < numPositions - 1) {
                myStepper.step(-stepsPerAngle);  // Gira anti-horário
                stepsAccumulated += stepsPerAngle;
                delay(300);
            }
        }

        // Retorna para a posição inicial
        myStepper.step(stepsAccumulated - steps90);
        Serial.println("Retornou à posição inicial.");
        delay(500);

        // === Lógica de desvio ===
        long distFrente = distancias[numPositions / 2];    // Posição central
        long distDireita = distancias[0];                  // Posição inicial (direita)
        long distEsquerda = distancias[numPositions - 1];  // Última posição (esquerda)

        if ((distFrente > distanciaLimite) || (distFrente == -1)) {
            frente();
            Serial.println("Caminho livre — Indo para frente.");
        } else if ((distEsquerda > distDireita) &&
                   (distEsquerda > distanciaLimite || distEsquerda == -1)) {
            esquerda();
            Serial.println("Desviando para ESQUERDA.");
        } else if ((distDireita > distEsquerda) &&
                   (distDireita > distanciaLimite || distDireita == -1)) {
            direita();
            Serial.println("Desviando para DIREITA.");
        } else {
            parar();
            Serial.println("Obstáculo em todas as direções — Parando.");
        }

        delay(500);
    }

    else {
        // === CONTROLE VIA BLUETOOTH ===
        if (Serial.available() > 0) {
            comando = Serial.read();
            Serial.print("Comando recebido: ");
            Serial.println(comando);
        }

        if (comando == 'F') {
            frente();
        } else if (comando == 'B') {
            re();
        } else if (comando == 'L') {
            esquerda();
        } else if (comando == 'R') {
            direita();
        } else {
            parar();
        }
    }

    delay(50);
}
