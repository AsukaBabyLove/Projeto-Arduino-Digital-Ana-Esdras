/*
  Reprodutor de musicas - Arduino UNO + LCD 16x2 I2C
  CCP020 - Digital Experience Ultimate

  AMBIENTE: mude TINKERCAD logo abaixo
    1 = Tinkercad (biblioteca Adafruit_LiquidCrystal)
    0 = Arduino real / Arduino IDE (biblioteca LiquidCrystal_I2C)

  Pinos conforme a montagem atual (CONFIRE buzzer e LEDs com MODO_TESTE 2):
    LCD (I2C): SDA=A4, SCL=A5, VCC=5V, GND=GND
    Botoes : UP=A0, DOWN=A1, PLAY/PAUSE=A2, STOP=A3 (INPUT_PULLUP, outro lado no GND)
    LED verde: D9 (tocando)    LED vermelho: D8 (pausado)
    Buzzer : D6

  MODO_TESTE: 0 = programa normal
              1 = testa os 4 botoes (U D P S, 1 = apertado)
              2 = descobre em qual pino estao LEDs e buzzer

  Notas das musicas: repositorio arduino-songs de Robson Couto, 2019
  (github.com/robsoncouto/arduino-songs), adaptadas para PROGMEM e para este projeto.
*/

#define TINKERCAD 1   // 1 = Tinkercad, 0 = Arduino real
#define MODO_TESTE 0

#if TINKERCAD
  #include <Adafruit_LiquidCrystal.h>
  Adafruit_LiquidCrystal lcd(0);          // LCD I2C do Tinkercad
#else
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(0x27, 16, 2);     // endereco I2C, colunas, linhas (se nao acender, tente 0x3F)
#endif

#include <avr/pgmspace.h>

// Pinos
const uint8_t PIN_UP = A0;
const uint8_t PIN_DOWN = A1;
const uint8_t PIN_PLAY = A2;
const uint8_t PIN_STOP = A3;
const uint8_t PIN_LED_VERDE = 9;
const uint8_t PIN_LED_VERM = 8;
const uint8_t PIN_BUZZER = 6;

// Frequencias (Hz)
#define REST      0
#define NOTE_C4   262
#define NOTE_CS4  277
#define NOTE_D4   294
#define NOTE_E4   330
#define NOTE_F4   349
#define NOTE_G4   392
#define NOTE_GS4  415
#define NOTE_A4   440
#define NOTE_AS4  466
#define NOTE_B4   494
#define NOTE_C5   523
#define NOTE_CS5  554
#define NOTE_D5   587
#define NOTE_DS5  622
#define NOTE_E5   659
#define NOTE_F5   698
#define NOTE_FS5  740
#define NOTE_G5   784
#define NOTE_GS5  831
#define NOTE_A5   880
#define NOTE_AS5  932
#define NOTE_B5   988
#define NOTE_C6   1047
#define NOTE_CS6  1109
#define NOTE_DS6  1245
#define NOTE_F6   1397

// d = divisor da nota inteira: 4 = quarto, 8 = oitavo, 16 = semicolcheia...
// d negativo = nota pontuada (dura 1,5x)
struct Nota {
  uint16_t f;
  int8_t d;
};

// Harry Potter - Hedwig's Theme
const Nota harryPotter[] PROGMEM = {
  {REST,2},{NOTE_D4,4},
  {NOTE_G4,-4},{NOTE_AS4,8},{NOTE_A4,4},
  {NOTE_G4,2},{NOTE_D5,4},
  {NOTE_C5,-2},
  {NOTE_A4,-2},
  {NOTE_G4,-4},{NOTE_AS4,8},{NOTE_A4,4},
  {NOTE_F4,2},{NOTE_GS4,4},
  {NOTE_D4,-1},
  {NOTE_D4,4},

  {NOTE_G4,-4},{NOTE_AS4,8},{NOTE_A4,4},
  {NOTE_G4,2},{NOTE_D5,4},
  {NOTE_F5,2},{NOTE_E5,4},
  {NOTE_DS5,2},{NOTE_B4,4},
  {NOTE_DS5,-4},{NOTE_D5,8},{NOTE_CS5,4},
  {NOTE_CS4,2},{NOTE_B4,4},
  {NOTE_G4,-1},
  {NOTE_AS4,4},

  {NOTE_D5,2},{NOTE_AS4,4},
  {NOTE_D5,2},{NOTE_AS4,4},
  {NOTE_DS5,2},{NOTE_D5,4},
  {NOTE_CS5,2},{NOTE_A4,4},
  {NOTE_AS4,-4},{NOTE_D5,8},{NOTE_CS5,4},
  {NOTE_CS4,2},{NOTE_D4,4},
  {NOTE_D5,-1},
  {REST,4},{NOTE_AS4,4},

  {NOTE_D5,2},{NOTE_AS4,4},
  {NOTE_D5,2},{NOTE_AS4,4},
  {NOTE_F5,2},{NOTE_E5,4},
  {NOTE_DS5,2},{NOTE_B4,4},
  {NOTE_DS5,-4},{NOTE_D5,8},{NOTE_CS5,4},
  {NOTE_CS4,2},{NOTE_AS4,4},
  {NOTE_G4,-1}
};

// Rei Leao - The Lion Sleeps Tonight
#define TREM(n) {n,-8},{n,16},{n,-8},{n,16},{n,-8},{n,16},{n,-8},{n,16}

#define LEAO_A \
  {NOTE_F4,4},{NOTE_G4,4},{NOTE_A4,8},{NOTE_G4,4},{NOTE_A4,8}, \
  {NOTE_AS4,4},{NOTE_A4,4},{NOTE_G4,8},{NOTE_F4,4},{NOTE_G4,8}, \
  {NOTE_A4,4},{NOTE_C4,8},{NOTE_C4,4},{NOTE_C4,8},{NOTE_C4,4}

#define LEAO_V \
  {NOTE_F4,4},{NOTE_G4,4},{NOTE_A4,8},{NOTE_G4,4},{NOTE_A4,8}, \
  {NOTE_AS4,4},{NOTE_A4,4},{NOTE_G4,8},{NOTE_F4,4},{NOTE_G4,8}, \
  {NOTE_A4,4},{NOTE_G4,4},{NOTE_F4,4},{NOTE_A4,4}, \
  {NOTE_G4,1}, \
  {NOTE_C5,4},{NOTE_A4,4},{NOTE_G4,8},{NOTE_A4,4},{NOTE_C5,8}, \
  {NOTE_AS4,4},{NOTE_A4,4},{NOTE_G4,8},{NOTE_F4,4},{NOTE_G4,8}, \
  {NOTE_A4,4},{NOTE_G4,4},{NOTE_F4,4},{NOTE_A4,4}, \
  {NOTE_G4,1}

#define LEAO_W \
  {NOTE_C5,1}, \
  {NOTE_C5,4},{NOTE_AS4,8},{NOTE_C5,8},{NOTE_AS4,2}, \
  {NOTE_A4,4},{NOTE_C4,8},{NOTE_C4,4},{NOTE_C4,8},{NOTE_C4,4}, \
  {NOTE_C4,1}, \
  {REST,4},{NOTE_A4,8},{NOTE_G4,8},{NOTE_F4,8},{NOTE_E4,8},{NOTE_D4,8},{NOTE_C4,8}, \
  {NOTE_D4,1}, \
  {REST,4},{NOTE_A4,8},{NOTE_G4,8},{NOTE_F4,8},{NOTE_E4,8},{NOTE_D4,8},{NOTE_C4,8}, \
  {NOTE_D4,1}

const Nota reiLeao[] PROGMEM = {
  LEAO_A, {NOTE_C4,1},
  LEAO_A, {NOTE_C4,-2},{REST,-8},{NOTE_A4,16},
  TREM(NOTE_A4), TREM(NOTE_AS4), TREM(NOTE_A4), TREM(NOTE_G4),
  TREM(NOTE_A4), TREM(NOTE_AS4), TREM(NOTE_A4), TREM(NOTE_G4),
  LEAO_V, LEAO_W, LEAO_V, LEAO_W,
  LEAO_A, {NOTE_C4,1},
  LEAO_V, LEAO_W,
  LEAO_A, {NOTE_C4,1}
};

// Star Wars - Tema principal
const Nota starWars[] PROGMEM = {
  {NOTE_AS4,8},{NOTE_AS4,8},{NOTE_AS4,8},
  {NOTE_F5,2},{NOTE_C6,2},
  {NOTE_AS5,8},{NOTE_A5,8},{NOTE_G5,8},{NOTE_F6,2},{NOTE_C6,4},
  {NOTE_AS5,8},{NOTE_A5,8},{NOTE_G5,8},{NOTE_F6,2},{NOTE_C6,4},
  {NOTE_AS5,8},{NOTE_A5,8},{NOTE_AS5,8},{NOTE_G5,2},{NOTE_C5,8},{NOTE_C5,8},{NOTE_C5,8},
  {NOTE_F5,2},{NOTE_C6,2},
  {NOTE_AS5,8},{NOTE_A5,8},{NOTE_G5,8},{NOTE_F6,2},{NOTE_C6,4},

  {NOTE_AS5,8},{NOTE_A5,8},{NOTE_G5,8},{NOTE_F6,2},{NOTE_C6,4},
  {NOTE_AS5,8},{NOTE_A5,8},{NOTE_AS5,8},{NOTE_G5,2},{NOTE_C5,-8},{NOTE_C5,16},
  {NOTE_D5,-4},{NOTE_D5,8},{NOTE_AS5,8},{NOTE_A5,8},{NOTE_G5,8},{NOTE_F5,8},
  {NOTE_F5,8},{NOTE_G5,8},{NOTE_A5,8},{NOTE_G5,4},{NOTE_D5,8},{NOTE_E5,4},{NOTE_C5,-8},{NOTE_C5,16},
  {NOTE_D5,-4},{NOTE_D5,8},{NOTE_AS5,8},{NOTE_A5,8},{NOTE_G5,8},{NOTE_F5,8},

  {NOTE_C6,-8},{NOTE_G5,16},{NOTE_G5,2},{REST,8},{NOTE_C5,8},
  {NOTE_D5,-4},{NOTE_D5,8},{NOTE_AS5,8},{NOTE_A5,8},{NOTE_G5,8},{NOTE_F5,8},
  {NOTE_F5,8},{NOTE_G5,8},{NOTE_A5,8},{NOTE_G5,4},{NOTE_D5,8},{NOTE_E5,4},{NOTE_C6,-8},{NOTE_C6,16},
  {NOTE_F6,4},{NOTE_DS6,8},{NOTE_CS6,4},{NOTE_C6,8},{NOTE_AS5,4},{NOTE_GS5,8},{NOTE_G5,4},{NOTE_F5,8},
  {NOTE_C6,1}
};

// Take On Me (a-ha)
#define TAKE_BLOCO \
  {NOTE_FS5,8},{NOTE_FS5,8},{NOTE_D5,8},{NOTE_B4,8},{REST,8},{NOTE_B4,8},{REST,8},{NOTE_E5,8}, \
  {REST,8},{NOTE_E5,8},{REST,8},{NOTE_E5,8},{NOTE_GS5,8},{NOTE_GS5,8},{NOTE_A5,8},{NOTE_B5,8}, \
  {NOTE_A5,8},{NOTE_A5,8},{NOTE_A5,8},{NOTE_E5,8},{REST,8},{NOTE_D5,8},{REST,8},{NOTE_FS5,8}, \
  {REST,8},{NOTE_FS5,8},{REST,8},{NOTE_FS5,8},{NOTE_E5,8},{NOTE_E5,8},{NOTE_FS5,8},{NOTE_E5,8}

const Nota takeOnMe[] PROGMEM = { TAKE_BLOCO, TAKE_BLOCO, TAKE_BLOCO };

// Tubarao
#define TUB_BLOCO \
  {NOTE_E4,4},{NOTE_F4,4},{NOTE_E4,4},{NOTE_F4,4},{NOTE_E4,4},{NOTE_F4,4},{NOTE_E4,4},{NOTE_F4,4}

const Nota tubarao[] PROGMEM = { TUB_BLOCO, TUB_BLOCO, TUB_BLOCO, TUB_BLOCO, TUB_BLOCO };

// Tabela de musicas
struct Musica {
  const char* nome;       // ate 14 caracteres (pra caber no LCD com a seta)
  const Nota* notas;
  uint16_t tamanho;       // quantidade de notas
  uint16_t tempo;         // batidas por minuto
  uint8_t acel;           // % de aceleracao ate o fim da musica
};

const Musica musicas[] = {
  {"Harry Potter", harryPotter, sizeof(harryPotter) / sizeof(Nota), 144,  0},
  {"Rei Leao",     reiLeao,     sizeof(reiLeao) / sizeof(Nota),     122,  0},
  {"Star Wars",    starWars,    sizeof(starWars) / sizeof(Nota),    108,  0},
  {"Take On Me",   takeOnMe,    sizeof(takeOnMe) / sizeof(Nota),    140,  0},
  {"Tubarao",      tubarao,     sizeof(tubarao) / sizeof(Nota),      75, 70}
};
const uint8_t TOTAL = sizeof(musicas) / sizeof(musicas[0]);

// Estado do programa
enum Estado { MENU, TOCANDO, PAUSADO };
Estado estado = MENU;

uint8_t selecionada = 0;
uint8_t topoMenu = 0;
uint16_t notaAtual = 0;
uint8_t ultimoPct = 255;
unsigned long inicioNota = 0;
unsigned long duracaoNota = 0;

// Botoes
const uint8_t pinosBtn[4] = {PIN_UP, PIN_DOWN, PIN_PLAY, PIN_STOP};
const char* const nomesBtn[4] = {"UP", "DOWN", "PLAY", "STOP"};
bool ultimoLeitura[4] = {HIGH, HIGH, HIGH, HIGH};
unsigned long ultimoTempo[4] = {0, 0, 0, 0};
bool clique[4] = {false, false, false, false};

void lerBotoes() {
  for (uint8_t i = 0; i < 4; i++) {
    clique[i] = false;
    bool leitura = digitalRead(pinosBtn[i]);
    if (leitura != ultimoLeitura[i] && millis() - ultimoTempo[i] > 40) {
      ultimoTempo[i] = millis();
      ultimoLeitura[i] = leitura;
      if (leitura == LOW) {
        clique[i] = true;
        Serial.println(nomesBtn[i]);
      }
    }
  }
}

// LCD
void linha(uint8_t l, const char* texto) {
  char buf[17];
  snprintf(buf, sizeof(buf), "%-16s", texto);
  lcd.setCursor(0, l);
  lcd.print(buf);
}

void desenhaMenu() {
  char buf[17];
  for (uint8_t l = 0; l < 2; l++) {
    uint8_t idx = topoMenu + l;
    if (idx < TOTAL) {
      snprintf(buf, sizeof(buf), "%s%s", (idx == selecionada) ? "->" : "  ", musicas[idx].nome);
      linha(l, buf);
    } else {
      linha(l, "");
    }
  }
}

void mostraPorcentagem(bool forcar) {
  uint8_t pct = (uint32_t)notaAtual * 100UL / musicas[selecionada].tamanho;
  if (!forcar && pct == ultimoPct) return;
  ultimoPct = pct;
  char buf[6];
  snprintf(buf, sizeof(buf), "%3u%%", pct);
  lcd.setCursor(12, 0);
  lcd.print(buf);
}

void desenhaReproducao() {
  linha(0, estado == TOCANDO ? "Reproduzindo" : "Pausado");
  linha(1, musicas[selecionada].nome);
  mostraPorcentagem(true);
}

void atualizaLeds() {
  digitalWrite(PIN_LED_VERDE, estado == TOCANDO);
  digitalWrite(PIN_LED_VERM, estado == PAUSADO);
}

// Reproducao
void tocarNota() {
  const Musica& m = musicas[selecionada];
  const Nota* p = &m.notas[notaAtual];
  uint16_t freq = pgm_read_word(&(p->f));
  int8_t div = (int8_t)pgm_read_byte(&(p->d));

  unsigned long inteira = 240000UL / m.tempo;      // duracao da nota inteira (ms)
  unsigned long dur = inteira / abs(div);
  if (div < 0) dur = dur * 3 / 2;                  // nota pontuada
  if (m.acel) {
    dur = dur * (100UL - (unsigned long)m.acel * notaAtual / m.tamanho) / 100UL;
  }
  duracaoNota = dur;
  inicioNota = millis();

  if (freq == 0) {
    noTone(PIN_BUZZER);
  } else {
    tone(PIN_BUZZER, freq, dur * 90UL / 100UL);    // 90% soando, 10% de silencio
  }
}

void irParaMenu() {
  noTone(PIN_BUZZER);
  estado = MENU;
  atualizaLeds();
  desenhaMenu();
}

void iniciarMusica() {
  notaAtual = 0;
  ultimoPct = 255;
  estado = TOCANDO;
  tocarNota();
  atualizaLeds();
  desenhaReproducao();
}

void playPause() {
  if (estado == MENU) {
    iniciarMusica();
  } else if (estado == TOCANDO) {
    noTone(PIN_BUZZER);
    estado = PAUSADO;
    atualizaLeds();
    desenhaReproducao();
  } else {
    estado = TOCANDO;
    tocarNota();                 // retoma recomecando a nota atual
    atualizaLeds();
    desenhaReproducao();
  }
}

// Modos de teste
#if MODO_TESTE == 1
void testeBotoes() {
  static uint8_t anterior = 255;
  uint8_t u = !digitalRead(PIN_UP);
  uint8_t d = !digitalRead(PIN_DOWN);
  uint8_t p = !digitalRead(PIN_PLAY);
  uint8_t s = !digitalRead(PIN_STOP);
  uint8_t atual = u | (d << 1) | (p << 2) | (s << 3);

  if (atual != anterior) {
    anterior = atual;
    char buf[17];
    snprintf(buf, sizeof(buf), "U%d D%d P%d S%d", u, d, p, s);   // 1 = apertado
    linha(1, buf);
    Serial.println(buf);
    digitalWrite(PIN_LED_VERDE, u);        // UP    acende o LED verde
    digitalWrite(PIN_LED_VERM, d);         // DOWN  acende o LED vermelho
    if (p) tone(PIN_BUZZER, 440);          // PLAY  toca um La
    else if (s) tone(PIN_BUZZER, 880);     // STOP  toca um La agudo
    else noTone(PIN_BUZZER);
  }
}
#endif

#if MODO_TESTE == 2
const uint8_t candidatos[] = {6, 7, 8, 9, 10, 13};

void descobrePinos() {
  for (uint8_t i = 0; i < sizeof(candidatos); i++) {
    uint8_t pino = candidatos[i];
    char buf[17];
    snprintf(buf, sizeof(buf), "Pino D%u", pino);
    Serial.println(buf);
    linha(0, buf);
    pinMode(pino, OUTPUT);

    linha(1, "LED aceso?");
    digitalWrite(pino, HIGH);
    delay(1200);
    digitalWrite(pino, LOW);

    linha(1, "Som no buzzer?");
    tone(pino, 440);
    delay(1200);
    noTone(pino);
    delay(300);
  }
}
#endif

// Setup e Loop
void setup() {
  Serial.begin(9600);
  for (uint8_t i = 0; i < 4; i++) pinMode(pinosBtn[i], INPUT_PULLUP);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_VERM, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

#if TINKERCAD
  lcd.begin(16, 2);
#else
  lcd.init();
  lcd.backlight();
#endif

#if MODO_TESTE == 1
  linha(0, "TESTE BOTOES");
  linha(1, "Aperte botoes");
#elif MODO_TESTE == 2
  linha(0, "TESTE DE PINOS");
  delay(1000);
#else
  linha(0, "Reprodutor");
  linha(1, "Arduino UNO");
  delay(1000);
  irParaMenu();
#endif
}

void loop() {
  lerBotoes();   // clique[0]=UP  [1]=DOWN  [2]=PLAY/PAUSE  [3]=STOP

#if MODO_TESTE == 1
  testeBotoes();
#elif MODO_TESTE == 2
  descobrePinos();
#else
  switch (estado) {
    case MENU:
      if (clique[0]) {
        selecionada = (selecionada + TOTAL - 1) % TOTAL;
        if (selecionada < topoMenu) topoMenu = selecionada;
        if (selecionada > topoMenu + 1) topoMenu = selecionada - 1;
        desenhaMenu();
      }
      if (clique[1]) {
        selecionada = (selecionada + 1) % TOTAL;
        if (selecionada < topoMenu) topoMenu = selecionada;
        if (selecionada > topoMenu + 1) topoMenu = selecionada - 1;
        desenhaMenu();
      }
      if (clique[2]) playPause();
      break;

    case TOCANDO:
      if (clique[3]) { irParaMenu(); break; }
      if (clique[2]) { playPause(); break; }
      if (millis() - inicioNota >= duracaoNota) {
        notaAtual++;
        if (notaAtual >= musicas[selecionada].tamanho) {
          irParaMenu();            // musica terminou
        } else {
          tocarNota();
          mostraPorcentagem(false);
        }
      }
      break;

    case PAUSADO:
      if (clique[3]) { irParaMenu(); break; }
      if (clique[2]) playPause();
      break;
  }
#endif
}