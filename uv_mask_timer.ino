/*
 * таймер для управления матрицей УФ светодиодов
 * используется для засветки фоторезиста или паяльной маски
 * Anton Gerasimov <me@zyxmasta.com>, 2016
 */
#include <LiquidCrystal.h>
#include <QuadEncoder.h>
#include <TimerSettings.h>
#include <MultiClick.h>

// энкодер
#define ENC_CLK         9
#define ENC_DT          10
#define ENC_SW          2     // IRQ0 (pin 2)

// выход управления матрицей светодиодов
#define UV_MATRIX       13

#define MAX_PROFILES    8

#define MENU_BASIC      4
#define MENU_PROFILES   MAX_PROFILES + 2

// ограничения при вводе времени
#define LIMIT_MINUTES   5 * 60
#define LIMIT_SECONDS   59

LiquidCrystal lcd( 12, 11, 7, 6, 5, 4 );

QuadEncoder encoder( ENC_CLK, ENC_DT );
MultiClick buttonR( ENC_SW );
TimerSettings settings( MAX_PROFILES );

char buf[9];
bool is_ready = false;

void setup() {
  Serial.begin( 9600 );
  lcd.begin( 8, 2 );

  pinMode( UV_MATRIX, OUTPUT );
  digitalWrite( UV_MATRIX, LOW );

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
      MenuAdjust();
      break;
    case 1:
      if ( is_ready )
        Exposure();
      break;
    case 2:
      Splash();
      break;
    default:
      Dashboard();
  }
}

// заставка
void Splash() {
  lcd.clear();
  lcd.print( "UV TIMER" );

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
  unsigned long cycles = 0;

  int minutes;
  int seconds;
  byte state = 0;

  char *headers[9] = {
    "EXPOSURE",
    "  HOLD  ",   // TODO
    " FINISH ",
  };

  digitalWrite( UV_MATRIX, HIGH );

  lcd.setCursor( 0, 0 );
  lcd.print( headers[state] );

  tm_elapsed = millis();
  tm_estimated = tm_elapsed + tm_exposure;

  while ( tm_elapsed < tm_estimated ) {
    // форматируем и выводим время экспонирования
    settings.to_mmss( tm_exposure, &minutes, &seconds );
    // 46 "."
    // 32 " "
    sprintf( buf, " %03d%c%02d ", minutes, ( cycles % 8 ? char( 165 ) : char( 32 ) ), seconds );

    lcd.setCursor( 0, 1 );
    lcd.print( buf );

    delay( 250 );

    cycles++;
    tm_exposure -= ( millis() - tm_elapsed );
    tm_elapsed = millis();
  }

  digitalWrite( UV_MATRIX, LOW );

  // меняем заголовок
  state = 2;
  lcd.setCursor( 0, 0 );
  lcd.print( headers[state] );

  // моргаем экраном
  FlashDisplay( 3 );

  delay( 1000 );
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
  byte state;

  char *headers[9] = {
    "  OKAY  ",
    " ADJUST ",
  };

  if ( settings.selected() != -1 ) {
    settings.to_mmss( settings.selected(), &minutes, &seconds );
    sprintf( buf, " %03d.%02d ", minutes, seconds );
    state = 0;
    is_ready = true;
  }
  else {
    sprintf( buf, " %03s.%02s ", "---", "--" );
    state = 1;
    is_ready = false;
  }

  lcd.setCursor( 0, 0 );
  lcd.print( headers[state] );
  lcd.setCursor( 0, 1 );
  lcd.print( buf );
}

// главное меню настроек
void MenuAdjust () {
  byte current = 1;
  byte previous;

  lcd.clear();

  const char *items[] = {
    "ADJUST",
    "PROFILE",
    "SOUNDS",
    "EXIT"
  };

  while ( buttonR.poll() != 1 ) {
    current += readEncoder();

    if ( current < 1 ) current = 1;
    if ( current > MENU_BASIC - 1 ) current = MENU_BASIC - 1;

    previous = current - 1;
    if ( previous < 1 ) previous = 0;

    sprintf( buf, "%c%-7s", (uint8_t)( previous == 0 ? 165 : 32 ), items[ previous ] );
    lcd.setCursor( 0, 0 );
    lcd.print( buf );

    sprintf( buf, "%c%-7s", (uint8_t)126, items[ current ] );
    lcd.setCursor( 0, 1 );
    lcd.print( buf );
  }

  switch ( current ) {
    case 0:
      // todo...
      // currentEq0();
      break;
    case 1:
      MenuProfiles( 1 );
      break;
    // etc
  }
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
  // форматы
  const char formats[][10] = {
    "< %03d >",   // минуты
    " < %02d >"   // секунды
  };

  lcd.clear();

  int clicks = buttonR.poll();

  while ( clicks != 1 ) {
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

    clicks = buttonR.poll();
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
