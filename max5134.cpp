/*
 * max5134.cpp
 *
 * Created: 13/04/2024
 *  Author: GuavTek
 */ 

#include "max5134.h"

void max5134_c::update(){
    if(com->Get_Status() != com_state_e::Idle){
        return;
    }
    if (pending_reset){
        reset();
    } else if (need_update & 0x0f){
        for (uint8_t i = 0; i < 4; i++){
            if (need_update & (1 << i)){
                write_to_dac(i);
                break;
            }
        }
    } else if (need_update & 0xf0){
        set_outputs();
    } else if (power_state & 0x80){
        set_powered();
    }
}

void max5134_c::set(uint16_t value, uint8_t output, uint8_t write_thru){
    dac_value[output] = value;
    need_update |= 1 << output;
    need_update &= ~(0b00010000 << output);
    need_update |= (write_thru * 0b00010000) << output;
}

void max5134_c::set(uint16_t value[4], uint8_t write_thru){
    for (uint8_t i = 0; i < 4; i++){
        dac_value[i] = value[i];
    }
    need_update = 0x0f;
    need_update |= 0xf0 * write_thru;
}

void max5134_c::set_powered(uint8_t outputs, uint8_t ready_en){
    power_state = 0b10000000 | outputs | (ready_en << 4);
    set_powered();
}

void max5134_c::set_powered(){
    com_buff[0] = 0b00000011;
    com_buff[1] = power_state & 0x0f;
    com_buff[2] = (power_state & 0b00010000) << 3;
    if (com->Get_Status() == com_state_e::Idle){
        Set_SS(1);
        com->Transfer(com_buff, 3, Tx);
        power_state &= 0x7f;
    }
}

void max5134_c::set_outputs(uint8_t outputs){
    need_update |= outputs << 4;
    set_outputs();
}

void max5134_c::set_outputs(){
    // Don't update outputs with pending values
    uint8_t updating = (need_update >> 4) & ~(need_update & 0xf);
    com_buff[0] = 0b00000001;
    com_buff[1] = updating;
    com_buff[2] = 0;
    if (com->Get_Status() == com_state_e::Idle){
        Set_SS(1);
        com->Transfer(com_buff, 3, Tx);
        need_update &= ~(updating << 4);
    }
}

void max5134_c::write_to_dac(uint8_t output){
    com_buff[0] = 0b00010000 | (1 << output);
    com_buff[0] |= ((need_update >> 4) << (5-output)) & 0b00100000; // Enable write-through?
    com_buff[1] = (dac_value[output] >> 8) & 0xff;
    com_buff[2] = dac_value[output] & 0xff;
    if (com->Get_Status() == com_state_e::Idle){
        Set_SS(1);
        com->Transfer(com_buff, 3, Tx);
        need_update &= ~(0b00010001 << output);
    }
}

uint8_t max5134_c::optimize_linearity(uint8_t optimize){
    com_buff[0] = 0b00000101;
    com_buff[1] = optimize << 1;
    com_buff[2] = 0;
    if(com->Get_Status() == com_state_e::Idle){
        Set_SS(1);
        com->Transfer(com_buff, 3, Tx);
        return 1;
    }
    return 0;
}

void max5134_c::reset(){
    com_buff[0] = 0b00000010;
    com_buff[1] = 0;
    com_buff[2] = 0;
    if(com->Get_Status() == com_state_e::Idle){
        Set_SS(1);
        com->Transfer(com_buff, 3, Tx);
        pending_reset = 0;
    } else {
        pending_reset = 1;
    }
}

void max5134_c::com_cb(){
    Set_SS(0);
    for (uint8_t i = 0; i < 4; i++){
        if (need_update & (1 << i)){
            write_to_dac(i);
            return;
        }
    }
    if (need_update & 0xf0){
        set_outputs();
    } else if (power_state & 0x80){
        set_powered();
    }
}