#include "state_machine.h"

#include "macros.h"

unsigned char c = 0x00;


STATE machine(STATE s, SET_UA type, unsigned char input, int role){
    STATE state;     
    
    switch(s){
        case START:
            if(input == FLAG)
                state = FLAG_RCV;
            break;
        case FLAG_RCV:
            if(input == A_EMI || (input == A_REC && (type == UAFRAME || type == DISCFRAME)))
                state = A_RCV;
            else 
                state = START; 
            break;
        case A_RCV:
            if(type == SETFRAME){
                if(input == C_SET)
                    state = C_RCV;
                break;
            }
            else if(type == UAFRAME){
                if(input == C_UA)
                    state = C_RCV;
                break;
            }
            else if(type == ACK){
                if(input == C_RR0 || input == C_RR1 || input == C_REJ0 || input == C_REJ1){
                    state = C_RCV;
                    c = input;
                    break;
                }
            }
            else if(type == DISCFRAME){
                if(input == C_DISC){
                    state = C_RCV;
                    break;
                }
            }
            if(input == FLAG)
                state = FLAG_RCV;
            else
                state = START; 
            break;
        case C_RCV:
            if(type == SETFRAME){
                if(input == (A_EMI^C_SET)){
                    state = BCC_OK;
                    break;
                }
            }
            else if(type == UAFRAME){
                if((input == (A_EMI^C_UA) && role == 0) || (input == (A_REC^C_UA) && role == 1)){
                    state = BCC_OK;
                    break;
                }
            }
            else if(type == ACK){
                if(input == (A_EMI^c)){
                    state = BCC_OK;
                    break;
                }
            }
            else if(type == DISCFRAME){
                if((input == (A_EMI^C_DISC) && role == 1) || (input == (A_REC^C_DISC) && role == 0)){
                    state = BCC_OK;
                    break;
                }
            }
            if(input == FLAG)
                state = FLAG_RCV;     
            else
                state = START; 
            break;
        case BCC_OK:
            if(input == FLAG)
                state = STOP;
            else 
                state = START; 
            break;
        case STOP:
            state = STOP;
            break;
    }
    
    return state; 
}

STATE infoMachine(STATE s, unsigned char input, unsigned char *frame){
    static int i = 0;

    switch (s)
    {
    case START:
        i = 0;
        if(input == FLAG){
            s = FLAG_RCV;
            frame[i++] = FLAG;
        }
        break;
    case FLAG_RCV:
        if(input == A_EMI){
            frame[i++] = A_EMI;
            s = A_RCV;
        } 
        else if(input == FLAG)
            break;
        else 
            s = START;
        break;
    case A_RCV:
        if(input == C_I0){
            frame[i++] = C_I0;
            c = C_I0;
            s = C_RCV;
            break;
        }
        else if(input == C_I1){
            frame[i++] = C_I1;
            c = C_I1;
            s = C_RCV;
            break;
        }
        else if(input == FLAG)
            s = FLAG_RCV;
        else 
            s = START;
        break;
    case C_RCV:
        if(input == (A_EMI^c)){
            frame[i++] = input;
            s = BCC_OK;
            break;
        }
        else if(input == FLAG)
            s = FLAG_RCV;
        else 
            s = START;
        break;
    case BCC_OK:
        if(input == FLAG){
            s = STOP;
            frame[i++]=FLAG;
        }
        else 
            frame[i++] = input;
        break;
    case STOP:
        s = START;
        break;
    }

    return s; 
}