/*
    SoundEffects.cpp - Library for manage sound effects of the UV Timer.
    Created by Anton Gerasimov <me@zyxmasta.com>.
*/

#include "Arduino.h"
#include "SoundEffects.h"

SoundEffects::SoundEffects( byte buzzer ) {
    _buzzer = buzzer;
}

void SoundEffects::begin ( TimerSettings* settings ) {
    _settings = settings;
}

// переход засветки в режим паузы
void SoundEffects::HoldOn () {
    if ( !_settings->effect( 2 ) )
        return;

    tone( _buzzer, NOTE_C6, 75 );
}

// переход засветки из режима паузы
void SoundEffects::HoldOff () {
    if ( !_settings->effect( 2 ) )
        return;

    tone( _buzzer, NOTE_FS6, 75 );
}

// одиночное нажатие кнопки энкодера
void SoundEffects::ClickSingle () {
    if ( !_settings->effect( 3 ) )
        return;

    tone( _buzzer, NOTE_FS5, 15 );
}

// двойное нажатие кнопки энкодера
void SoundEffects::ClickDouble () {
    if ( !_settings->effect( 3 ) )
        return;

    tone( _buzzer, NOTE_FS5, 15 );
}

// долгое нажатие кнопки энкодера
void SoundEffects::ClickLong () {
    if ( !_settings->effect( 3 ) )
        return;

    tone( _buzzer, NOTE_FS5, 15 );
}

// поворот энкодера по часовой стрелке
void SoundEffects::TurnCW () {
    if ( !_settings->effect( 1 ) )
        return;

    tone( _buzzer, NOTE_D5, 15 );
}

// поворот энкодера против часовой стрелки
void SoundEffects::TurnCCW () {
    if ( !_settings->effect( 1 ) )
        return;

    tone( _buzzer, NOTE_A4, 15 );
}

// поворт энкодера выше первой позиции
void SoundEffects::TurnFirst () {
    if ( !_settings->effect( 1 ) )
        return;

    tone( _buzzer, NOTE_GS4, 15 );
}

// поворт энкодера ниже последней позиции
void SoundEffects::TurnLast () {
    if ( !_settings->effect( 1 ) )
        return;

    tone( _buzzer, NOTE_DS5, 15 );
}

// эффект вращения в любую сторону с отбивкой границ
void SoundEffects::TurnAny ( int direction, int previous, int current ) {
    if ( !_settings->effect( 1 ) )
        return;

    // по часовой стрелке
    if ( direction == 1 ) {
        previous == current     // значение совпадает с прошлым?
            ? TurnLast()        // отбивка верхней границы
            : TurnCW();         // стандартный эффект

        return;
    }

    // против часовой стрелке
    if ( direction == -1 ) {
        previous == current     // значение совпадает с прошлым?
            ? TurnFirst()       // отбивка нижней границы
            : TurnCCW();        // стандартный эффект

        return;
    }
}

// нормальное завершение засветки
void SoundEffects::Finish () {
    if ( !_settings->effect( 0 ) )
        return;

    int melody[] = {
        NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
    };

    int noteDurations[] = {
        4, 8, 8, 4, 4, 4, 4, 4
    };

    for ( int thisNote = 0; thisNote < 8; thisNote++ ) {
        int noteDuration = 1000 / noteDurations[ thisNote ];
        tone( _buzzer, melody[ thisNote ], noteDuration );
        int pauseBetweenNotes = noteDuration * 1.30;
        delay( pauseBetweenNotes );
        noTone( _buzzer );
    }
}
