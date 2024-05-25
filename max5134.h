/*
 * max5134.h
 *
 * Created: 13/04/2024
 *  Author: GuavTek
 */ 


#ifndef MAX5134_H_
#define MAX5134_H_

#include <stdint.h>
#include "communication_base.h"

class max5134_c : public com_driver_c {
	public:
		void update(); // Try to update DAC registers if needed
        void set(uint16_t value, uint8_t output, uint8_t write_thru);
        void set(uint16_t value[4], uint8_t write_thru);
        void set_powered(uint8_t outputs, uint8_t ready_en); // outputs - power down outputs by setting bit to 1, ready_en - use ready pin on IC
		void set_outputs(uint8_t outputs);	// Move the set values to DAC outputs
		uint8_t optimize_linearity(uint8_t optimize); // Optimize must be set for 10ms to guarantee linearity
        void reset();	// Send a RESET command to the MAX5134
		virtual void com_cb();	// The callback called when the com object is done transmitting data
		using com_driver_c::com_driver_c;
	protected:
		uint16_t dac_value[4];
		uint8_t need_update;	// DAC values needing update
		uint8_t pending_reset;	// Reset is pending
		uint8_t power_state;	// State of the power control
		char com_buff[3];
		void write_to_dac(uint8_t output);
		void set_outputs();
		void set_powered();
};

#endif /* MAX5134_H_ */