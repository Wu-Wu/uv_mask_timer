/*
    MultiClick.cpp - One Button, Multiple Events
    Created by Anton Gerasimov <me@zyxmasta.com>.

    Based on http://pastebin.com/raw/hmSEJgDN
    4-Way Button:  Click, Double-Click, Press+Hold, and Press+Long-Hold Test Sketch
    By Jeff Saltzman
    Oct. 13, 2009
*/

#ifndef MultiClick_h
#define MultiClick_h

#include "Arduino.h"

class MultiClick {
    public:
        MultiClick( byte, int, int, int );
        MultiClick( byte, int, int );
        MultiClick( byte, int );
        MultiClick( byte );

        int poll();

    private:
        byte _pin;
        int _debounceTime;
        int _gapTime;
        int _holdTime;

        bool _now;
        bool _old;
        bool _DCwaiting;
        bool _DConUp;
        bool _singleOK;

        long _tm_pressed;
        long _tm_released;

        bool _ignoreUp;
        bool _waitForUp;

        bool _direct;

        bool hi();
        bool lo();

        void defaults();

        bool pressed();
        bool released();
};

#endif
