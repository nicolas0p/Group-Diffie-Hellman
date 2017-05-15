// EPOS YYYY Display Mediator Declarations

#ifndef __yyyy_display_h
#define __yyyy_display_h

#include <display.h>

__BEGIN_SYS

// YYYY Physical Display Controller
class CHIP_YYYY_Dislay
{
};

class YYYY_Display: public Display_Common, private CHIP_YYYY_Dislay
{
private:
    static const unsigned int FB = Traits<YYYY_Display>::FRAME_BUFFER_ADDRESS;
    static const int LINES = Traits<YYYY_Display>::LINES;
    static const int COLUMNS = Traits<YYYY_Display>::COLUMNS;
    static const int TAB_SIZE = Traits<YYYY_Display>::TAB_SIZE;

public:
    // Frame Buffer
    typedef unsigned short Cell;
    typedef Cell * Frame_Buffer;

    // Cell Attributes
    typedef Cell Attribute;
    enum {
        NORMAL = 0x0700
    };

public:
    YYYY_Display() {}

    static void remap(unsigned int fb = FB) {
        _frame_buffer = reinterpret_cast<Frame_Buffer>(fb);
    }

    static void putc(char c) {
        unsigned int pos = CHIP_YYYY_Dislay::position();

        switch(c) {
        case '\n':
            pos = (pos + COLUMNS) / COLUMNS * COLUMNS;
            break;
        case '\t':
            pos = (pos + TAB_SIZE) / TAB_SIZE * TAB_SIZE;
            break;
        default:
            _frame_buffer[pos++] = NORMAL | c;
        }
        if(pos >= LINES * COLUMNS) {
            scroll();
            pos-= COLUMNS;
        }
        CHIP_YYYY_Dislay::position(pos);
    }

    static void puts(const char * s) {
        while(*s != '\0')
            putc(*s++);
    }

    static void clear() { 
        for(unsigned int i = 0; i < LINES * COLUMNS; i++)
            _frame_buffer[i] = NORMAL | ' ';
        CHIP_YYYY_Dislay::position(0);
    }

    static void position(int * line, int * column) {
        unsigned int pos = CHIP_YYYY_Dislay::position();
        *column = pos % COLUMNS;
        *line = pos / COLUMNS;
    }

    static void position(int line, int column) {
        if(line > LINES)
            line = LINES;
        if(column > COLUMNS)
            column = COLUMNS;
        if((line < 0) || (column < 0)) {
            int old_line, old_column;
            position(&old_line, &old_column);
            if(column < 0)
        	column = old_column;
            if(line < 0)
        	line = old_line;
        }
        CHIP_YYYY_Dislay::position(line * COLUMNS + column);
    }

    static void geometry(int * lines, int * columns) {
        *lines = LINES;
        *columns = COLUMNS;
    }

private:
    static void scroll() {
        for(unsigned int i = 0; i < (LINES - 1) * COLUMNS; i++)
            _frame_buffer[i] = _frame_buffer[i + COLUMNS];
        for(unsigned int i = (LINES - 1) * COLUMNS; i < LINES * COLUMNS; i++)
            _frame_buffer[i] = NORMAL | ' ';
    }

private:
    static Frame_Buffer _frame_buffer;
};

__END_SYS

#endif
