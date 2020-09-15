#include "ap_int.h"
#include "ap_fixed.h"
#include "hls_dsp.h"

#define N_PHASE 4
typedef ap_uint<32> iq_t;
typedef ap_uint<8> group_t;
typedef ap_uint<9> pgroup_t;
typedef ap_fixed<16,1,AP_TRN_ZERO, AP_WRAP> sample_t;


typedef struct in_t {
	iq_t data[8];
	group_t user;
	bool last;
} in_t;

typedef struct out_t {
	iq_t data[4];
	pgroup_t user;
	bool last;
} out_t;



typedef hls::atan2_input<16>::cartesian atanin_t;
typedef hls::atan2_output<16>::phase atanout_t;
typedef ap_fixed<16,3,AP_TRN_ZERO,AP_WRAP,0> phase_t;


typedef struct phasesout_t {
	phase_t data[4];
	pgroup_t user;
	bool last;
} phasesout_t;

typedef struct phaseout_t {
	phase_t data;
	pgroup_t user;
	bool last;
} phaseout_t;



void lowpass_to_phase(in_t &instream, phaseout_t phase[N_PHASE], phasesout_t &phases);
