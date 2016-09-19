/*
    MultiClick.cpp - One Button, Multiple Events
    Created by Anton Gerasimov <me@zyxmasta.com>.

    Based on http://pastebin.com/raw/hmSEJgDN
    4-Way Button:  Click, Double-Click, Press+Hold, and Press+Long-Hold Test Sketch
    By Jeff Saltzman
    Oct. 13, 2009
*/

#include "Arduino.h"
#include "MultiClick.h"

// номер пина, дребезг, интервал между кликами, интервал длинного клика (остальное умолчания)
MultiClick::MultiClick( byte pin, int debounce, int gap, int hold )
    : _pin( pin ), _debounceTime( debounce ), _gapTime( gap ), _holdTime( hold ) {

    defaults();
}

// номер пина, дребезг, интервал между кликами (остальное умолчания)
MultiClick::MultiClick( byte pin, int debounce, int gap )
    : _pin( pin ), _debounceTime( debounce ), _gapTime( gap ) {

    _holdTime       = 3000;

    defaults();
}

// номер пина, дребезг (остальное умолчания)
MultiClick::MultiClick( byte pin, int debounce )
    : _pin( pin ), _debounceTime( debounce ) {

    _gapTime        = 250;
    _holdTime       = 3000;

    defaults();
}

// только номер пина (остальное умолчания)
MultiClick::MultiClick( byte pin )
    : _pin( pin ) {

    _debounceTime   = 20;
    _gapTime        = 250;
    _holdTime       = 3000;

    defaults();
}

// умолчания
void MultiClick::defaults() {
    pinMode( _pin, INPUT );

#ifdef MULTICLICK_INVERT
    _direct = false;
#else
    _direct = true;
#endif

    _now            = hi();     // текущее состояние кнопки
    _old            = hi();     // прошлое состояние кнопки

    _tm_pressed     = -1;       // момент времени нажатия кнопки
    _tm_released    = -1;       // момент времени отжатия кнопки

    _DCwaiting = false;         // whether we're waiting for a double click (down)
    _DConUp = false;            // whether to register a double click on next release, or whether to wait and click
    _singleOK = true;           // whether it's OK to do a single click
    _ignoreUp = false;          // whether to ignore the button release because the click+hold was triggered
    _waitForUp = false;        // when held, whether to wait for the up event
}

// опрос состояния кнопки
int MultiClick::poll() {
    int clicks = 0;

    _now = digitalRead( _pin );

    // кнопку нажали
    if ( pressed() && ( millis() - _tm_released ) > _debounceTime ) {
        _tm_pressed = millis();
        _ignoreUp   = false;
        _waitForUp  = false;
        _singleOK   = true;

        if ( ( millis() - _tm_released ) < _gapTime && !_DConUp && _DCwaiting ) {
            _DConUp = true;
        }
        else {
            _DConUp = false;
        }

        _DCwaiting = false;
    }
    // кнопку отжали
    else if ( released() && ( millis() - _tm_pressed ) > _debounceTime ) {
        if ( !_ignoreUp ) {
            _tm_released = millis();
            if ( _DConUp == false ) {
                _DCwaiting = true;
            }
            else {
                clicks = 2;
                _DConUp = false;
                _DCwaiting = false;
                _singleOK = false;
            }
        }
    }

    // Test for normal click event: DCgap expired
    if ( _now == hi() && ( millis() - _tm_released ) >= _gapTime && _DCwaiting && !_DConUp && _singleOK && clicks != 2 ) {
        clicks = 1;
        _DCwaiting = false;
    }

    // удержание кнопки
    if ( _now == lo() && ( millis() - _tm_pressed ) >= _holdTime ) {
        _tm_pressed = millis();
        clicks = -1;

        _waitForUp = true;
        _ignoreUp = true;
        _DConUp = false;
        _DCwaiting = false;
    }

    _old = _now;

    return clicks;
}

// кнопку нажали?
bool MultiClick::pressed() {
    return _now == lo() && _old == hi();
}

// кнопку отпустили?
bool MultiClick::released() {
    return _now == hi() && _old == lo();
}

// высокий уровень кнопки
bool MultiClick::hi() {
    return !_direct ? HIGH : LOW;
}

// низкий уровень кнопки
bool MultiClick::lo() {
    return !_direct ? LOW : HIGH;
}
