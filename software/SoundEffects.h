/*
    SoundEffects.h - Library for manage sound effects of the UV Timer.
    Created by Anton Gerasimov <me@zyxmasta.com>.
*/

#ifndef SoundEffects_h
#define SoundEffects_h

#include "Arduino.h"
#include "TimerSettings.h"

#define NOTE_C6  1047
#define NOTE_FS6 1480
#define NOTE_A4  440
#define NOTE_C4  262
#define NOTE_GS4 415
#define NOTE_G3  196
#define NOTE_A3  220
#define NOTE_B3  247
#define NOTE_FS5 740
#define NOTE_D5  587
#define NOTE_DS5 622

class SoundEffects {
    public:
        SoundEffects( byte buzzer );

        void begin( TimerSettings* settings );

        // нажатия
        void ClickSingle ();
        void ClickDouble ();
        void ClickLong ();

        // вращение
        void TurnCW ();
        void TurnCCW ();
        void TurnFirst ();
        void TurnLast ();
        void TurnAny ( int direction, int previous, int current );

        // состояния
        void HoldOn ();
        void HoldOff ();
        void Finish ();

    private:
        byte _buzzer;
        TimerSettings* _settings;
};

#endif
