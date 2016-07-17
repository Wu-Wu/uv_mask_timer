/*
 * таймер для управления матрицей УФ светодиодов
 * используется для засветки фоторезиста или паяльной маски
 * Anton Gerasimov <me@zyxmasta.com>, 2016
 */
#include <LiquidCrystal.h>
#include <QuadEncoder.h>
#include <TimerSettings.h>
#include <MultiClick.h>
#include "pitches.h"

// энкодер
#define ENC_CLK         9
#define ENC_DT          10
#define ENC_SW          2     // IRQ0 (pin 2)

// выход управления матрицей светодиодов
#define UV_MATRIX       13

// выход управления пищалкой
#define BUZZER          8

#define MAX_PROFILES    8

#define MENU_ADJUST     5
#define MENU_PROFILES   MAX_PROFILES + 2
#define MENU_SOUNDS     6

// ограничения при вводе времени
#define LIMIT_MINUTES   5 * 60
#define LIMIT_SECONDS   59

LiquidCrystal lcd( 12, 11, 7, 6, 5, 4 );

QuadEncoder encoder( ENC_CLK, ENC_DT );
MultiClick buttonR( ENC_SW );
TimerSettings settings( MAX_PROFILES );

char buf[9];
bool is_ready = false;

// заголовки (первые строки) интерфейса
const char *headers[9] = {
  "UV TIMER",     // splash
  "  OKAY  ",     // dashboard
  " ADJUST ",     // dashboard
  "EXPOSURE",     // exposure
  "  HOLD  ",     // exposure
  " CANCEL ",     // exposure
  " FINISH ",     // exposure
  " FLUSH  ",     // flushsettings
  " MEMORY ",     // flushsettings
};

void setup() {
  Serial.begin( 9600 );
  lcd.begin( 8, 2 );

  pinMode( BUZZER, OUTPUT );

  pinMode( UV_MATRIX, OUTPUT );
  MatrixOff();

  settings.begin();

  if ( settings.selected() != -1 )
    is_ready = true;

  Splash();
  // моргаем экраном
  FlashDisplay( 2 );
}

void loop() {
  switch ( buttonR.poll() ) {
    case -1:
      sfxClickLong();
      MenuAdjust();
      break;
    case 1:
      sfxClickSingle();
      if ( is_ready )
        Exposure();
      break;
    case 2:
      sfxClickDouble();
      Splash();
      break;
    default:
      Dashboard();
  }
}

// заставка
void Splash() {
  lcd.clear();
  lcd.print( headers[0] );

  for ( byte offset = 0; offset <= 4; offset++ ) {
    for ( byte posL = 0, posR = 7; posL < ( 4 - offset ), posR >= ( 4 + offset ); posL++, posR-- ) {
      lcd.setCursor( posL, 1 );
      lcd.print( char( 165 ) );

      if ( 0 <= posL - 1 ) {
        lcd.setCursor( posL - 1, 1 );
        lcd.print( char( 32 ) );
      }

      lcd.setCursor( posR, 1 );
      lcd.print( char( 165 ) );

      if ( 5 <= posR + 1 ) {
        lcd.setCursor( posR + 1, 1 );
        lcd.print( char( 32 ) );
      }
      delay( 250 );
    }
    delay( 100 );
  }
  delay( 1000 );
}

// засветка
void Exposure() {
  unsigned long tm_exposure = settings.time_limit();
  unsigned long tm_estimated = 0L;
  unsigned long tm_elapsed = 0L;
  unsigned long tm_hold_on = 0L;
  unsigned long delta = 0L;

  int minutes;
  int seconds;
  int clicks = 0;
  byte state;
  bool done = false;
  bool hold = false;

  MatrixOn();

  lcd.setCursor( 0, 0 );
  lcd.print( headers[3] );

  tm_elapsed = millis();
  tm_estimated = tm_elapsed + tm_exposure;

  while ( !done ) {
    // форматируем и выводим время экспонирования
    settings.to_mmss( tm_exposure, &minutes, &seconds );
    sprintf( buf, " %03d.%02d ", minutes, seconds );

    lcd.setCursor( 0, 1 );
    lcd.print( buf );

    switch ( clicks ) {
      case 1:
        // одиночный клик: приостановка/возобновление
        if ( !hold ) {
          // момент время приостановки
          tm_hold_on  = millis();

          // отключаем матрицу
          MatrixOff();
          sfxHoldOn();
        }
        else {
          // длительность приостановки
          delta = millis() - tm_hold_on;

          // пересчитываем время с учётом времени приостановки
          tm_estimated  += delta;
          tm_elapsed    += delta;

          // включаем матрицу
          MatrixOn();
          sfxHoldOff();
        }

        // инвертируем флаг приостановки
        hold = !hold;

        // меняем заголовок
        lcd.setCursor( 0, 0 );
        lcd.print( headers[ hold ? 4 : 3 ] );
        break;
      case 2:
        // двойной клик: отмена работы
        done = true;
        state = 5;
        break;
      default:
        ;;;
    }

    delay( 10 );

    if ( !hold ) {
      tm_exposure -= ( millis() - tm_elapsed );
      tm_elapsed = millis();
    }

    clicks = buttonR.poll();

    if ( tm_elapsed > tm_estimated ) {
      // нормальное завершение: время вышло
      done = true;
      state = 6;
    }
  }

  MatrixOff();

  // меняем заголовок
  lcd.setCursor( 0, 0 );
  lcd.print( headers[state] );

  switch ( state ) {
    case 6:
      sfxFinish();
      break;
    default:
      // none
      ;;;
  }

  // моргаем экраном
  FlashDisplay( 3 );

  delay( 1000 );
}

// включение матрицы светодиодов
void MatrixOn() {
  digitalWrite( UV_MATRIX, HIGH );
}

// выключение матрицы светодиодов
void MatrixOff() {
  digitalWrite( UV_MATRIX, LOW );
}

// моргаем экраном
void FlashDisplay( byte times ) {
  while ( times > 0 ) {
    delay( 600 );
    lcd.noDisplay();
    delay( 400 );
    lcd.display();
    times--;
  }
}

// дашборд
void Dashboard() {
  int minutes;
  int seconds;

  if ( settings.selected() != -1 ) {
    settings.to_mmss( settings.selected(), &minutes, &seconds );
    sprintf( buf, " %03d.%02d ", minutes, seconds );
    is_ready = true;
  }
  else {
    sprintf( buf, " %03s.%02s ", "---", "--" );
    is_ready = false;
  }

  lcd.setCursor( 0, 0 );
  lcd.print( headers[ is_ready ? 1 : 2 ] );
  lcd.setCursor( 0, 1 );
  lcd.print( buf );
}

// главное меню настроек
void MenuAdjust () {
  int current = 1;
  int previous = 0;

  lcd.clear();

  const char *items[] = {
    "ADJUST",
    "PROFILE",
    "SOUNDS",
    "RESET",
    "EXIT"
  };

  while ( buttonR.poll() != 1 ) {
    int turn = readEncoder();
    current += turn;

    if ( current < 1 ) current = 1;
    if ( current > MENU_ADJUST - 1 ) current = MENU_ADJUST - 1;

    previous = current - 1;

    if ( turn == 1 ) {
      current == MENU_ADJUST - 1
        ? sfxTurnLast()
        : sfxTurnCW();
    }

    if ( turn == -1 ) {
      current == 1
        ? sfxTurnFirst()
        : sfxTurnCCW();
    }

    sprintf( buf, "%c%-7s", (uint8_t)( previous == 0 ? 165 : 32 ), items[ previous ] );
    lcd.setCursor( 0, 0 );
    lcd.print( buf );

    sprintf( buf, "%c%-7s", (uint8_t)126, items[ current ] );
    lcd.setCursor( 0, 1 );
    lcd.print( buf );
  }

  sfxClickSingle();

  switch ( current ) {
    case 1:
      MenuProfiles( 1 );
      break;
    case 2:
      MenuSoundEffects();
      break;
    case 3:
      FlushSettings();
      break;
  }
}

// настройки звуковых эффектов
void MenuSoundEffects ( void ) {
  const char items[][12] = {
    "%cSOUNDS ",
    "%cDONE  %c",
    "%cTURN  %c",
    "%cHOLD  %c",
    "%cCLICK %c",
    "%cEXIT   "
  };

  int current = 0;
  int previous;

  bool done = false;

  lcd.clear();

  while ( !done ) {
    int turn  = readEncoder();
    int clicks = buttonR.poll();

    current += turn;

    if ( current < 1 )                  current = 1;
    if ( current > MENU_SOUNDS - 1 )    current = MENU_SOUNDS - 1;

    previous = current - 1;

    // текущий и предыдущий пункты меню являются звуковыми эффектами?
    bool is_fx_current  = current >= 1  && current < MENU_SOUNDS - 1;
    bool is_fx_previous = previous >= 1 && previous < MENU_SOUNDS - 1;

    // space (32)
    // letter Y (89)
    // letter N (78)
    // centered square dot (165)
    // right arrow (126)
    sprintf(
      buf,
      items[ previous ],
      (uint8_t)( previous == 0 ? 165 : 32 ),
      ( is_fx_previous ? (uint8_t)( FxOption( previous, false ) ? 89 : 78 ) : false )
    );
    lcd.setCursor( 0, 0 );
    lcd.print( buf );

    sprintf(
      buf,
      items[ current ],
      (uint8_t)( 126 ),
      ( is_fx_current ? (uint8_t)( FxOption( current, false ) ? 89 : 78 ) : false )
    );
    lcd.setCursor( 0, 1 );
    lcd.print( buf );

    if ( clicks ) {
      // если нажата кнопка
      if ( current != MENU_SOUNDS - 1 ) {
        // изменение опции текущего звука
        FxOption( current, true );
      }
      else {
        // выход из меню
        done = true;
      }
    }
  }
  // возврат в предыдущее меню
  MenuAdjust();
}

// получение/переключении опции эффекта
// TODO: использовать EEPROM
bool FxOption ( int fx, bool toggle ) {
  static bool options[] = {
    true,
    true,
    true,
    true
  };

  // если передан флаг инвертирования значения
  if ( toggle )
    options[ fx - 1 ] = !options[ fx - 1 ];

  return options[ fx - 1 ];
}

// настройки профилей
void MenuProfiles ( int current ) {
  char items[][9] = {
    "PROFILE",
    // изменяется динамически
    "",   // #0
    "",   // #1
    "",   // #2
    "",   // #3
    "",   // #4
    "",   // #5
    "",   // #6
    "",   // #7
    "EXIT"
  };

  int previous;
  int minutes;
  int seconds;

  lcd.clear();

  while ( buttonR.poll() != 1 ) {
    current += readEncoder();

    if ( current < 1 )                  current = 1;
    if ( current > MENU_PROFILES - 1 )  current = MENU_PROFILES - 1;

    previous = current - 1;
    if ( previous < 1 ) previous = 0;

    for ( int i = 1; i < MENU_PROFILES - 1; i++ ) {
      // считываем значения профиля
      settings.to_mmss( i - 1, &minutes, &seconds );
      // подставляем в пункты меню
      sprintf(
        items[i],
        "%03d.%02d%c",
        minutes, seconds,
        (uint8_t)( settings.selected() == i - 1 ? 235 : 32 )
      );
    }

    sprintf( buf, "%c%-7s", (uint8_t)( previous == 0 ? 165 : 32 ), items[ previous ] );
    lcd.setCursor( 0, 0 );
    lcd.print( buf );

    sprintf( buf, "%c%-7s", (uint8_t)126, items[ current ] );
    lcd.setCursor( 0, 1 );
    lcd.print( buf );
  }

  if ( current > 0 && current < MENU_PROFILES - 1 ) {
    // изменяем профиль
    EditProfile( current - 1 );
  }
  else {
    // возврат в предыдущее меню
    MenuAdjust();
  }
}

// редактирование профиля
void EditProfile ( int profile_id ) {
  int current = 0;

  int left;
  int right;

  // данные
  int minutes = 0;
  int seconds = 0;

  settings.to_mmss( profile_id, &minutes, &seconds );

  bool done      = false;
  bool mark_left = false;

  char element[][4] = {
    "",     // минуты
    "",     // секунды
    "OK",   // сохранить
    "USE"   // использовать
  };

  lcd.clear();

  // остаёмся тут, пока не выбран пункт сохранения
  while ( !done ) {
    // заголовок
    sprintf( buf, "%cEDIT %-2d", (uint8_t)165, profile_id );
    lcd.setCursor( 0, 0 );
    lcd.print( buf );

    while ( buttonR.poll() != 1 ) {
      current += readEncoder();

      // проверка выхода за пределы пунктов меню
      if ( current < 0 )  current = 3;
      if ( current > 4 )  current = 0;

      switch ( current ) {
        case 0:
        case 1:
          left      = 0;    // минуты
          right     = 1;    // секунды
          mark_left = current == 0 ? true : false;
          // форматируем числа минут и секунд в строку
          sprintf( element[0], "%03d", minutes );
          sprintf( element[1], "%02d", seconds );
          break;
        case 2:
        case 3:
          left      = 2;    // сохранить
          right     = 3;    // использовать
          mark_left = current == 2 ? true : false;
          break;
      }

      // прокручиваем и отображаем часть второй строки
      sprintf(
        buf,
        "%c%-3s%c%-3s",
        (uint8_t)( mark_left ? 126 : 32 ),
        element[ left ],
        (uint8_t)( mark_left ? 32 : 126 ),
        element[ right ]
      );

      lcd.setCursor( 0, 1 );
      lcd.print( buf );
    }

    switch ( current ) {
      case 0:
        InputTimeValue( "MINUTES", LIMIT_MINUTES, &minutes, 0 );
        break;
      case 1:
        InputTimeValue( "SECONDS", LIMIT_SECONDS, &seconds, 1 );
        break;
      case 2:
        done = true;
        settings.from_mmss( profile_id, minutes, seconds );
        settings.update_profiles();
        // текущий профиль выбран?
        if ( settings.selected() == profile_id ) {
          // обнулили значения и минут и секунд?
          if ( !minutes && !seconds ) {
            // делаем его не выбранным
            settings.selected( -1 );
            settings.update_selected();
          }
        }
        break;
      case 3:
        done = true;
        if ( (minutes && seconds) || (!minutes && seconds) || (minutes && !seconds) ) {
          // также обновляем значение
          settings.selected( profile_id );
          settings.from_mmss( profile_id, minutes, seconds );
          settings.update_profiles();
          settings.update_selected();
        }
        break;
    }
  }

  // возвращаемся в предыдущее меню на тот же пункт
  MenuProfiles( profile_id + 1 );
}

// ввода единиц времени
void InputTimeValue ( const char* header, const int limit, int *value, const int format_id ) {
  int clicks;

  // форматы
  const char formats[][10] = {
    "< %03d >",   // минуты
    " < %02d >"   // секунды
  };

  lcd.clear();

  while ( ( clicks = buttonR.poll() ) != 1 ) {
    *value += readEncoder();

    // двойной клик: сбрасываем значение
    if ( clicks == 2 )  *value = 0;
    // удержание: выставляем максимальное
    if ( clicks == -1 ) *value = limit;

    if ( *value < 0 )      *value = 0;
    if ( *value > limit )  *value = limit;

    // заголовок
    sprintf( buf, "%-8s", header );
    lcd.setCursor( 0, 0 );
    lcd.print( buf );

    // вводимое значение
    sprintf( buf, formats[ format_id ], *value );
    lcd.setCursor( 0, 1 );
    lcd.print( buf );
  }
}

// чтение смещения энкодера
int readEncoder () {
  switch ( encoder.tick() ) {
    case '>': return 1;
    case '<': return -1;
  }
  return 0;
}

// сброс всех настроек таймера
void FlushSettings() {
  lcd.clear();
  lcd.setCursor( 0, 0 );
  lcd.print( headers[7] );
  lcd.setCursor( 0, 1 );
  lcd.print( headers[8] );

  // моргаем экраном
  FlashDisplay( 5 );

  settings.erase();

  delay( 500 );
}

// поворот энкодера по часовой стрелке
void sfxTurnCW() {
  // TODO: проверка опций звука
  tone( BUZZER, NOTE_D5, 15 );
}

// поворот энкодера против часовой стрелки
void sfxTurnCCW() {
  // TODO: проверка опций звука
  tone( BUZZER, NOTE_A4, 15 );
}

// поворт энкодера выше первой позиции
void sfxTurnFirst() {
  // TODO: проверка опций звука
  tone( BUZZER, NOTE_GS4, 15 );
}

// поворт энкодера ниже последней позиции
void sfxTurnLast() {
  // TODO: проверка опций звука
  tone( BUZZER, NOTE_DS5, 15 );
}

// одиночное нажатие кнопки энкодера
void sfxClickSingle() {
  // TODO: проверка опций звука
  tone( BUZZER, NOTE_FS5, 15 );
}

// двойное нажатие кнопки энкодера
void sfxClickDouble() {
  // TODO: проверка опций звука
  tone( BUZZER, NOTE_FS5, 15 );
}

// долгое нажатие кнопки энкодера
void sfxClickLong() {
  // TODO: проверка опций звука
  tone( BUZZER, NOTE_FS5, 75 );
}

// переход засветки в режим паузы
void sfxHoldOn() {
  // TODO: проверка опций звука
  tone( BUZZER, NOTE_C6, 75 );
}

// переход засветки из режима паузы
void sfxHoldOff() {
  // TODO: проверка опций звука
  tone( BUZZER, NOTE_FS6, 75 );
}

// нормальное завершение засветки
void sfxFinish() {
  int melody[] = {
    NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
  };

  int noteDurations[] = {
    4, 8, 8, 4, 4, 4, 4, 4
  };

  for ( int thisNote = 0; thisNote < 8; thisNote++ ) {
    int noteDuration = 1000 / noteDurations[ thisNote ];
    tone( BUZZER, melody[ thisNote ], noteDuration );
    int pauseBetweenNotes = noteDuration * 1.30;
    delay( pauseBetweenNotes );
    noTone( BUZZER );
  }
}
