# Reprodutor de Músicas com Arduino UNO

Reprodutor de músicas feito com Arduino UNO, display LCD 16x2 (I2C), buzzer, 4 botões e 2 LEDs. O usuário escolhe a música em um menu no LCD, e o Arduino toca a melodia no buzzer, mostrando o progresso em porcentagem.

Projeto da disciplina CCP020 – Digital Experience Ultimate.

Funcionalidades

- Menu com rolagem no LCD (seta -> indica a música selecionada)
- Reproduzir, pausar e retomar a música
- Parar a música e voltar ao menu
- Barra de status no LCD: estado (Reproduzindo / Pausado), nome da música e porcentagem concluída
- LED verde acende ao tocar, LED vermelho acende ao pausar
- Volta ao menu automaticamente quando a música termina
- Funciona no Tinkercad e em um Arduino real
- Modos de teste para conferir botões, LEDs e buzzer na montagem

Músicas disponíveis

| Música | Andamento (BPM)
| Harry Potter (Hedwig's Theme) | 144 | |
| Rei Leão (The Lion Sleeps Tonight) | 122 | |
| Star Wars (Tema principal) | 108 | |
| Take On Me (a-ha) | 140 | |
| Tubarão | 75 | Acelera 70% até o fim da música |

Componentes

- 1x Arduino UNO
- 1x LCD 16x2 com módulo I2C
- 1x Buzzer
- 4x Botões (push button)
- 1x LED verde e 1x LED vermelho (use resistores em série com os LEDs)
- Protoboard e jumpers

Ligações

| Componente | Pino do Arduino |

| LCD (SDA) | A4 |
| LCD (SCL) | A5 |
| LCD (VCC / GND) | 5V / GND |
| Botão UP | A0 |
| Botão DOWN | A1 |
| Botão PLAY/PAUSE | A2 |
| Botão STOP | A3 |
| LED verde (tocando) | D9 |
| LED vermelho (pausado) | D8 |
| Buzzer | D6 |

Os botões usam INPUT_PULLUP: um lado do botão vai no pino e o outro lado vai no GND (não precisa de resistor externo).

Bibliotecas

| Ambiente | Biblioteca |
| Tinkercad | Adafruit_LiquidCrystal (já incluída) |
| Arduino IDE | LiquidCrystal_I2C (instale pelo Gerenciador de Bibliotecas) |

Configuração

No início do código há duas constantes:

#define TINKERCAD 1   // 1 = Tinkercad, 0 = Arduino real
#define MODO_TESTE 0  // 0 = normal, 1 = testa botões, 2 = descobre pinos

Como rodar

No Tinkercad
1. Monte o circuito conforme a tabela de ligações.
2. Cole o código no editor, mantendo TINKERCAD 1.
3. Inicie a simulação.

No Arduino real
1. Mude para #define TINKERCAD 0.
2. Instale a biblioteca LiquidCrystal_I2C.
3. Selecione a placa Arduino UNO e a porta correta, e faça o upload.
4. Se o LCD não acender, troque o endereço I2C de 0x27 para 0x3F na linha LiquidCrystal_I2C lcd(0x27, 16, 2). Se o texto estiver fraco, ajuste o contraste no potenciômetro do módulo I2C.

Como usar

| Botão | No menu |
| UP | Música anterior |
| DOWN | Próxima música |
| PLAY/PAUSE | Toca a música selecionada | Pausa | Retoma (recomeça a nota atual) |
| STOP | – | Volta ao menu |

Exemplo do LCD durante a reprodução:

Reproduzindo  42%
Star Wars

Modos de teste

Altere MODO_TESTE para conferir a montagem:

- 1 – Teste dos botões: o LCD mostra U0 D0 P0 S0 (1 = apertado). O botão UP acende o LED verde, DOWN acende o LED vermelho, PLAY toca um Lá (440 Hz) e STOP toca um Lá agudo (880 Hz).
- 2 – Descoberta de pinos: testa os pinos D6, D7, D8, D9, D10 e D13 (um LED aceso e um som no buzzer em cada), mostrando no LCD qual pino está sendo testado. Serve para descobrir onde LEDs e buzzer realmente foram ligados.

Os eventos dos botões também são enviados pelo Monitor Serial (9600 baud).

Como o código funciona

Máquina de estados. O programa tem três estados: MENU, TOCANDO e PAUSADO. O loop() lê os botões e trata cada estado sem usar delay(), então os botões respondem durante a música.

Representação das músicas. Cada nota é uma "struct Nota" com a frequência em Hz (f) e o divisor da nota inteira (d):

cpp
{NOTE_G4, 4}    // sol, semínima (1/4)
{NOTE_AS4, 8}   // lá sustenido, colcheia (1/8)
{NOTE_G4, -4}   // sol, semínima pontuada (dura 1,5x)
{REST, 2}       // pausa de mínima

As melodias ficam na memória de programa (PROGMEM) para economizar a RAM do Arduino e são lidas com pgm_read_word / pgm_read_byte.

Duração das notas. A nota inteira dura 240000 / BPM ms. A duração de cada nota é inteira / |d|, multiplicada por 1,5 se d for negativo. O buzzer toca 90% desse tempo e fica 10% em silêncio, para separar as notas. Na música com aceleração (acel), a duração diminui gradualmente conforme a música avança.

Botões. São lidos com INPUT_PULLUP e debounce de 40 ms. Cada clique vale apenas por um ciclo do loop().

Progresso. A porcentagem é notaAtual x 100 / total de notas e só é redesenhada no LCD quando o valor muda.

Adicionando uma nova música

1. Crie o vetor de notas:
   const Nota minhaMusica[] PROGMEM = {
     {NOTE_C4, 4}, {NOTE_E4, 4}, {NOTE_G4, 2}
   };
   
3. Se precisar de alguma nota que não esteja na lista, adicione o #define NOTE_... com a frequência.
4. Adicione uma linha na tabela musicas[]:

   {"Minha Musica", minhaMusica, sizeof(minhaMusica) / sizeof(Nota), 120, 0},
   Os campos são: nome (até 14 caracteres), notas, quantidade de notas, BPM e % de aceleração.

O TOTAL de músicas é calculado automaticamente.

Estrutura do código

Pinos e frequências      → constantes e #define das notas
Músicas                  → vetores PROGMEM + tabela musicas[]
lerBotoes()              → leitura com debounce
linha() / desenhaMenu()  → funções do LCD
tocarNota() / playPause()→ reprodução
testeBotoes() / descobrePinos() → modos de teste
setup() / loop()         → inicialização e máquina de estados

Créditos

As notas das músicas são do repositório [arduino-songs](https://github.com/robsoncouto/arduino-songs), de Robson Couto (2019), adaptadas para PROGMEM e para este projeto.
