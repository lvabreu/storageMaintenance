#include <Arduino.h>

#define RXD2 16
#define TXD2 17

// Definição dos pinos de controle do MDI-3100-SR
#define TRIG_PIN 4 
#define WAKE_PIN 5 // Novo pino de controle de energia e mira

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  // Configuração inicial dos pinos (HIGH = Desativado, devido ao Pull-up interno)
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, HIGH); 
  
  pinMode(WAKE_PIN, OUTPUT);
  digitalWrite(WAKE_PIN, HIGH); 

  Serial.println("Bridge Serial ESP32 <-> Scanner V2");
}

void loop() {
  // 1. LEITURA DE RETORNO DO SCANNER
// 1. LEITURA DE RETORNO DO SCANNER
  if (Serial2.available()) {
    String barcodeData = Serial2.readStringUntil('\n');
    barcodeData.trim(); // Limpa lixo residual do buffer
    
    if(barcodeData.length() > 0){
      Serial.print("[SCANNER] LIDO BRUTO: ");
      Serial.println(barcodeData);

      // Regra de Extração:
      // O dado útil (31041) começa no índice 8 e termina no índice 13.
      // Ajuste os índices caso crachás de outros funcionários tenham tamanhos diferentes.
      if(barcodeData.length() >= 13) {
        String matricula = barcodeData.substring(8, 13); 
        Serial.print("[SISTEMA] Matrícula Extraída: ");
        Serial.println(matricula);
        
        // Aqui você pode inserir a lógica para liberar a trava do Smart Locker
        // ou validar a matrícula em um banco de dados.
      } else {
        Serial.println("[SISTEMA] Formato de código incompatível.");
      }
    }
  }
  // 2. ENVIO DE COMANDOS DO PC PARA O MÓDULO
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    
    // O .trim() remove '\r', '\n' e espaços, garantindo o match correto
    command.trim(); 
    
    if(command == "TRIGGER") {
       Serial.println("[ESP32] Acordando módulo e disparando leitura...");
       
       // Passo A: Acorda o módulo do modo Low Power (Transição para Standby/AIM)
       digitalWrite(WAKE_PIN, LOW); // L: Recover from Low Power state / Aiming LED on
       delay(50); // Atraso tático para estabilização de estado interno
       
       // Passo B: Dispara o gatilho de leitura (Transição para Read)
       digitalWrite(TRIG_PIN, LOW); 
       delay(50); // Pulso de ativação
       digitalWrite(TRIG_PIN, HIGH);
       
       // Passo C: Mantém o módulo acordado e a mira ativada por 2 segundos para tentar ler
       delay(2000); 
       
       // Passo D: Libera o módulo para voltar ao estado de Low Power
       digitalWrite(WAKE_PIN, HIGH); 
       Serial.println("[ESP32] Ciclo de leitura finalizado.");
       
    } else if (command.length() > 0) {
       // Repassa comandos seriais nativos
       Serial.println("[ESP32] TX -> " + command);
       Serial2.print(command);
       Serial2.print("\r"); 
    }
  }
}
