/*
    TimerSettings.h - Library for manage setting of the UV Timer.
    Created by Anton Gerasimov <me@zyxmasta.com>.
*/

#ifndef TimerSettings_h
#define TimerSettings_h

#include "EEPROMex.h"
#include "Arduino.h"

#define DEFAULT_MEM_WRITES      2048
#define DEFAULT_MEM_BASE        100
#define DEFAULT_MEM_SIZE        EEPROMSizeATmega328
#define DEFAULT_PROFILES        4

class TimerSettings {
    public:
        TimerSettings();
        TimerSettings(int);
        ~TimerSettings();

        void begin();

        int update_profiles();
        bool update_selected();

        void to_mmss( const int, int*, int * );
        void to_mmss( const unsigned long, int*, int * );
        void from_mmss( const int, int, int );

        unsigned long time_limit();

        int selected();
        int selected( int );

        int drift ();
        int update_drift ( int );

        bool effect ( byte );
        bool update_effect ( byte, bool );
        bool toggle_effect ( byte );

        bool erase();

    private:
        int _offset_profiles;
        int _offset_selected;
        int _offset_effects;
        int _offset_drift;

        unsigned long* _profiles;
        int _selected;
        unsigned int _drift[2];
        byte _effects;

        int _mem_writes;
        int _mem_base;
        int _mem_size;
        int _max_profiles;

        void _allocate_storage();
};

#endif
