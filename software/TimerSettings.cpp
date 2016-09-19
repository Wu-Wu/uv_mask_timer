/*
    TimerSettings.cpp - Library for manage setting of the UV Timer.
    Created by Anton Gerasimov <me@zyxmasta.com>.
*/

#include "Arduino.h"
#include "TimerSettings.h"

TimerSettings::TimerSettings () {
    _mem_base   = DEFAULT_MEM_BASE;
    _mem_size   = DEFAULT_MEM_SIZE;
    _mem_writes = DEFAULT_MEM_WRITES;

    _max_profiles = DEFAULT_PROFILES;

    _allocate_storage();
}

TimerSettings::TimerSettings ( int num ) {
    _mem_base   = DEFAULT_MEM_BASE;
    _mem_size   = DEFAULT_MEM_SIZE;
    _mem_writes = DEFAULT_MEM_WRITES;

    _max_profiles = num;

    _allocate_storage();
}

TimerSettings::~TimerSettings () {
    delete [] _profiles;
}

void TimerSettings::begin () {
    EEPROM.setMemPool( _mem_base, _mem_size );
    EEPROM.setMaxAllowedWrites( _mem_writes );

    _offset_selected    = EEPROM.getAddress( sizeof( byte ) );
    _offset_profiles    = EEPROM.getAddress( sizeof( long ) * _max_profiles );
    _offset_effects     = EEPROM.getAddress( sizeof( byte ) );
    _offset_drift       = EEPROM.getAddress( sizeof( int ) * 2 );

    // читаем профили
    EEPROM.readBlock<unsigned long>( _offset_profiles, _profiles, _max_profiles );
    while ( !EEPROM.isReady() ) { delay( 1 );  }

    for ( int i = 0; i < _max_profiles; i++ ) {
        if ( _profiles[i] == 0xFFFFFFFF ) _profiles[i] = 0;
    }

    // читаем выбранный профиль
    _selected = EEPROM.readByte( _offset_selected );
    while ( !EEPROM.isReady() ) { delay( 1 );  }

    if ( _selected == 0xFF )    _selected = -1;

    // установки звуковых эффектов
    _effects = EEPROM.readByte( _offset_effects );
    while ( !EEPROM.isReady() ) { delay( 1 );  }

    // установки дрифта
    EEPROM.readBlock<unsigned int>( _offset_drift, _drift, 2 );
    while ( !EEPROM.isReady() ) { delay( 1 );  }
    // для памяти, которая не записывалась ещё
    if ( _drift[0] == 0xFFFF )  _drift[0] = 1;  // знак
    if ( _drift[1] == 0xFFFF )  _drift[1] = 0;  // значение
}

// преобразование из формата профиля в минуты и секунды
void TimerSettings::to_mmss ( const int profile_id, int *minutes, int *seconds ) {
    *minutes = (int)( _profiles[ profile_id ] / 60 / 1000 );
    *seconds = (int)( ( _profiles[ profile_id ] / 1000 ) % 60 );
}

void TimerSettings::to_mmss ( const unsigned long value, int *minutes, int *seconds ) {
    *minutes = (int)( value / 60 / 1000 );
    *seconds = (int)( ( value / 1000 ) % 60 );
}

// преобразование в формат профиля из минут и секунд
void TimerSettings::from_mmss ( const int profile_id, int minutes, int seconds ) {
    _profiles[ profile_id ] = (unsigned long)( minutes * 60 + seconds ) * 1000;
}

// выбранный номер профиля
int TimerSettings::selected () {
    return _selected;
}

// выбранный номер профиля
int TimerSettings::selected ( int profile_id ) {
    return _selected = profile_id;
}

// выделение памяти под данные профилей
void TimerSettings::_allocate_storage () {
    _profiles = new unsigned long [_max_profiles];
}

// сохранение профилей в энергонезависимой памяти
int TimerSettings::update_profiles () {
    int updated = 0;

    updated = EEPROM.updateBlock<unsigned long>( _offset_profiles, _profiles, _max_profiles );
    while ( !EEPROM.isReady() ) { delay( 1 );  }

    return updated;
}

// сохранение выбранного профилея в энергонезависимой памяти
bool TimerSettings::update_selected () {
    bool result = false;

    result = EEPROM.updateByte( _offset_selected, _selected == -1 ? 0xFF : _selected );
    while ( !EEPROM.isReady() ) { delay( 1 );  }

    return result;
}

// интервал времени выбранного профиля
unsigned long TimerSettings::time_limit () {
    return _profiles[ _selected ];
}

// сброс настроек
bool TimerSettings::erase () {
    // сброс значения выбранного профиля
    EEPROM.writeByte( _offset_selected, 0xFF );
    while ( !EEPROM.isReady() ) { delay( 1 );  }

    _selected = -1;

    // сброс значений профилей
    for ( int j = 0; j < _max_profiles; j++ ) {
        for ( int k = 0; k < 4; k++ ) {
            int addr = _offset_profiles + 4 * j + k;
            EEPROM.writeByte( addr, 0xFF );
            while ( !EEPROM.isReady() ) { delay( 1 ); }
        }

        _profiles[ j ] = 0L;
    }

    // сброс дрифта
    for ( int j = 0; j < 2; j++ ) {
        for ( int k = 0; k < 2; k++ ) {
            int addr = _offset_drift + 2 * j + k;
            EEPROM.writeByte( addr, 0xFF );
            while ( !EEPROM.isReady() ) { delay( 1 ); }
        }

        _drift[ j ] = 0;
    }

    // сброс эффектов
    EEPROM.writeByte( _offset_effects, 0xFF );
    while ( !EEPROM.isReady() ) { delay( 1 );  }

    return true;
}

// формирование значение дрифта
int TimerSettings::drift () {
    return _drift[1] * ( _drift[0] ? 1 : -1 );
}

// обновление значения дрифта
int TimerSettings::update_drift ( int drift ) {
    int updated = 0;

    _drift[0] = drift < 0 ? 0 : 1;                  // 0 = "-"; 1 = "+"
    _drift[1] = drift * ( _drift[0] ? 1 : -1 );     // эквивалент abs()

    updated = EEPROM.updateBlock<unsigned int>( _offset_drift, _drift, 2 );
    while ( !EEPROM.isReady() ) { delay( 1 );  }

    return updated;
}

// текущее состояние эффекта
bool TimerSettings::effect ( byte bit ) {
    if ( bit > 7 ) return false;

    return _effects & ( 1 << bit );
}

// обновление состояния эффекта
bool TimerSettings::update_effect ( byte bit, bool val ) {
    if ( bit > 7 ) return false;

    if ( val ) {
        // установка бита в 1
        _effects |= ( 1 << bit );
    }
    else {
        // установка бита в 0
        _effects &= ~( 1 << bit );
    }

    // обновляем бит
    return EEPROM.updateBit( _offset_effects, bit, val );
}

// инвертирование состояния эффекта
bool TimerSettings::toggle_effect ( byte bit ) {
    return update_effect( bit, !effect( bit ) );
}
