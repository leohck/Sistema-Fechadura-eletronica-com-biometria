#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

#include <LiquidCrystal.h>

#include <Keypad.h>


const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal ihm(rs, en, d4, d5, d6, d7);


const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {37, 35, 33, 31}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {29, 27, 25, 23}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad teclado = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// INSTANCIANDO OBJETOS
SoftwareSerial mySerial(11, 10);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

String senhaMaster = "0018";
String senhaAbertura = "0000";
bool tocarSom = false;
int maxTentativas = 3;
int segundosEspera = 30;
uint8_t id;

const byte buzzer = A15;
const byte trava = A8;

void setup()
{
  // set the data rate for the sensor serial port
  finger.begin(57600);
  ihm.begin(16, 2);
  pinMode(buzzer, OUTPUT);
  pinMode(trava, OUTPUT);
  ihm.clear();
}


//***********************************************************************


void loop() {
  int get_id = getFingerprintIDez();
  if ((get_id != -1) and (get_id != 99)) {
    ihm.print("Acesso Liberado");
    ihm.setCursor(0, 1); ihm.print("ID: #"); ihm.print(get_id);
    delay(1000);
    ihm.clear();
  } else {
    if (get_id == 99) {
      ihm.print("erro no leitor");
    } else {
      ihm.setCursor(0, 0); ihm.print("biometria");
      ihm.setCursor(0, 1); ihm.print("nao aceita");
    }
    delay(2000);
    ihm.clear();
    bool admin = false;
    String senha = digitarSenha();
    if (senha == senhaAbertura) {
      ihm.print("Acesso Liberado");
      destravar();
    } else if ( senha == senhaMaster) {
      ihm.print("Admin");
      admin = true;
    }
    delay(1000);
    ihm.clear();
    while (admin) {
      char tecla = teclado.getKey();
      switch (tecla) {
        default:
          ihm.setCursor(0, 0); ihm.print("C-Senha");
          ihm.setCursor(0, 1); ihm.print("D-Configuracoes");
          break;

        case 'C': // Segundo Menu para cadastrar nova senha
          ihm.clear();
          tecla = teclado.getKey();
          while (tecla != '*') {
            switch (tecla) {
              default:
                ihm.print("C - Numerica");
                ihm.setCursor(0, 1); ihm.print("D - Biometrica");
                break;
              case 'A':
                novaSenha();
                break;

              case 'B':
                ihm.clear();
                tecla = teclado.getKey();
                int id = 0;
                while (tecla != '*') {
                  switch (tecla) {
                    default:
                      ihm.print("A - 1 | B - 2");
                      ihm.setCursor(0, 1); ihm.print("C - 3 | D - 4");
                      break;
                    case 'A':
                      id = 1;
                      break;
                    case 'B':
                      id = 2;
                      break;
                    case 'C':
                      id = 3;
                      break;
                    case 'D':
                      id = 4;
                      break;
                  }
                }
            }
            cadastrarDigital(id);
            break;
          }
          break;

        case 'D': // Configurações
          ihm.clear();
          configuracoes();
          ihm.clear();
          break;

        case '*':
          admin = false;
          ihm.clear();
          break;
      }
    }
  }
}

void configuracoes() {
  ihm.setCursor(0, 0); ihm.print("C-Bateria");
  ihm.setCursor(0, 1); ihm.print("D-Som");
  char tecla = teclado.getKey();
  while (tecla != '*') {
    tecla = teclado.getKey();
    switch (tecla) {
      case 'C':
        ihm.clear();
        while (true) {
          tecla = teclado.getKey();
          if (tecla == '*') {
            break;
          }
          ihm.setCursor(0, 0); ihm.print("Nivel Bat.:");
          ihm.setCursor(12, 0); ihm.print(0.008797653 * analogRead(A1) + 0); //para 9V
          ihm.setCursor(0, 1); ihm.print("*-VOLTAR");
        }
        break;
      case 'D':
        ihm.clear();
        while (true) {
          tecla = teclado.getKey();
          if (tecla == '#') {
            tocarSom = (tocarSom == false) ? true : false;
            if (tocarSom) {
              tone(buzzer, 400, 100);
            }
            ihm.setCursor(0, 0); ihm.print("                ");
          }
          if (tecla == '*') {
            break;
          }
          ihm.setCursor(0, 0); ihm.print("SOM: ");
          if (tocarSom) {
            ihm.print("LIGADO");
          } else {
            ihm.print("DESLIGADO");
          }
          ihm.setCursor(0, 1); ihm.print("*-VOLTAR #-MUDAR");
        }
        break;
    }
  }
}

//************************************************************************************************
String novaSenha() {
novasenha:
  ihm.setCursor(0, 0); ihm.print("NOVA SENHA:");
  String senha1 = getSenha();
  ihm.clear();
  ihm.setCursor(0, 0); ihm.print("NOVAMENTE:");
  String senha2 = getSenha();
  if (senha1 == senha2) {
    ihm.clear();
    ihm.setCursor(0, 0); ihm.print("SENHA CADASTRADA");
    ihm.setCursor(0, 1); ihm.print("COM SUCESSO");
    senhaAbertura = senha2;
    delay(2000);
    ihm.clear();
  } else {
    ihm.clear();
    ihm.setCursor(0, 0); ihm.print("AS SENHAS");
    ihm.setCursor(0, 1); ihm.print("NAO COINCIDEM");
    ihm.clear();
    delay(2000);
    goto novasenha;
  }
}

String getSenha() {
  String senha = "";
  byte _cursor = 4;
  ihm.setCursor(0, 1); ihm.print("-> ");
  ihm.setCursor(_cursor, 1);
  ihm.blink();

  while (senha.length() < 4) {
    char tecla = teclado.getKey();
    if (tecla != NO_KEY) {
      if (isDigit(tecla)) {
        senha = senha + tecla;
        ihm.print(tecla);
        delay(200);
        ihm.setCursor(_cursor, 1); ihm.print('*');
        _cursor++;
      }
    }
  }
  ihm.noBlink();
  delay(1000);
  return senha;
}

String digitarSenha() {
  byte senhaErrada = 0;
digitar:
  ihm.setCursor(0, 0); ihm.print("DIGITE SUA SENHA");
  String senha = getSenha();
  ihm.clear();
  if ((senha == senhaMaster) or (senha == senhaAbertura)) {
    if (tocarSom) {
      tone(buzzer, 400, 100);
    }
    delay(1000);
    senhaErrada = 0;
    return senha;
  } else {
    senhaErrada++;
    ihm.print("Incorreta");
    ihm.setCursor(0, 1); ihm.print("Tentativas: ");
    ihm.print(maxTentativas - senhaErrada);
    delay(1000);
    if (senhaErrada < maxTentativas) {
      ihm.clear();
      goto digitar;
    } else {
      telaAcessoBloqueado();
      senhaErrada = 0;
      return "erro";
    }
  }
}

void telaAcessoBloqueado() {
  ihm.clear();
  ihm.print("Acesso Bloqueado");
  ihm.setCursor(0, 1); ihm.print("Aguarde:");
  for (int i = 0; i <= segundosEspera; i++) {
    if ((segundosEspera - i) >= 10) {
      ihm.setCursor(9, 1); ihm.print(segundosEspera - i);
    } else {
      ihm.setCursor(9, 1); ihm.print("0"); ihm.print(segundosEspera - i);
    }
    delay(1000);
  }
  ihm.clear();
}

void destravar() {
  if (tocarSom) {
    tone(buzzer, 200, 100);
    delay(200);
    tone(buzzer, 200, 100);
  }
  digitalWrite(trava, HIGH);
  delay(1000);
  digitalWrite(trava, LOW);
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {

  if (testeSensorBiometrico() == false) return 99;

  uint8_t p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  //  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  //  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

bool testeSensorBiometrico() {
  if (finger.verifyPassword()) {
    return true;
  } else {
    return false;
  }
}

void cadastrarDigital(int id) {
  ihm.clear();
  ihm.print("Usuario - #"); ihm.print(id);
  delay(1000);
  while (!  getFingerprintEnroll(id) );
}

uint8_t getFingerprintEnroll(int id) {
  ihm.clear();
  ihm.print("Posicione o dedo");
  ihm.setCursor(0, 1); ihm.print("No sensor");
  String response = "";
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        response = "Leitura concluída";
        break;
      case FINGERPRINT_NOFINGER:
        response = ".";
        delay(200);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        response = "Erro comunicação";
        break;
      case FINGERPRINT_IMAGEFAIL:
        response = "Erro de leitura";
        break;
      default:
        response = "Erro desconhecido";
        break;
    }
  }
  // OK successo!
  delay(1000);
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      response = "Leitura convertida";
      break;
    case FINGERPRINT_IMAGEMESS:
      response = "Leitura suja";
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      response = "Erro de comunicação";
      return p;
    case FINGERPRINT_FEATUREFAIL:
      response = "Não foi possível encontrar propriedade da digital";
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      response = "Não foi possível encontrar propriedade da digital";
      return p;
    default:
      response = "Erro desconhecido";
      return p;
  }

  ihm.clear();
  ihm.print("Remova o dedo");
  delay(2000);

  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  ihm.clear();
  ihm.print("Posicione o dedo");
  ihm.setCursor(0, 1); ihm.print("Novamente");
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        response = "Leitura concluída";
        break;
      case FINGERPRINT_NOFINGER:
        response = ".";
        delay(200);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        response = "Erro de comunicação";
        break;
      case FINGERPRINT_IMAGEFAIL:
        response = "Erro de Leitura";
        break;
      default:
        response = "Erro desconhecido";
        break;
    }
  }

  // OK successo!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      response = "Leitura convertida";
      break;
    case FINGERPRINT_IMAGEMESS:
      response = "Leitura suja";
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      response = "Erro de comunicação";
      return p;
    case FINGERPRINT_FEATUREFAIL:
      response = "Não foi possível encontrar as propriedades da digital";
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      response = "Não foi possível encontrar as propriedades da digital";
      return p;
    default:
      response = "Erro desconhecido";
      return p;
  }

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    response = "batem!";
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    response = "Erro de comunicação";
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    response = "nao batem";
  } else {
    response = "Erro desconhecido";
    return p;
  }
  delay(1000);

  ihm.clear();
  ihm.print("As digitais");
  ihm.setCursor(0, 1); ihm.print(response);
  delay(1000);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    response = "Armazenado!";
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    response = "Erro de comunicação";
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    response = "Não foi possível gravar neste local da memória";
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    response = "Erro durante escrita na memória flash";
    return p;
  } else {
    response = "Erro desconhecido";
    return p;
  }
  ihm.clear();
  ihm.print("Biometria Salva");
  ihm.setCursor(0, 1); ihm.print("Usuario: #"); ihm.print(id);
}
