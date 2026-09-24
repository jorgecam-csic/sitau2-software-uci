#ifndef SRC_SITAU2_DYNAMIC_PHASE_H_
#define SRC_SITAU2_DYNAMIC_PHASE_H_

#define VCO_FREQ_MHZ	(62.5*16)

#define PS_CONTROL_REG		11
#define PS_TICK_COUNT_REG	12
#define PS_TICK_TARGET_REG	13

#define PHASE_SHIFT_DONE_BIT	0
#define PHASE_UP_BIT			1
#define PHASE_DOWN_BIT			2
#define CLEAR_COUNT_BIT			3
#define PHASE_SHIFT_TRIG		5

void set_dynamic_phase(u8 minibase,int phase_delay_ticks);

void set_dynamic_phase_ps(u8 minibase,int phase_delay_picoseconds);

#endif //SRC_SITAU2_DYNAMIC_PHASE_H_
