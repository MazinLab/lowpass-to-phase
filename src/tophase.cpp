#include "tophase.hpp"
#include <iostream>

using namespace std;

void comphase(iq_t xy, phase_t &phase) {
#pragma HLS INLINE
	atanin_t in;
	ap_fixed<16,1> x=0,y=0;
	x.range(15,0)=xy.range(15,0);
	y.range(15,0)=xy.range(31,16);
//	cout<<"in: "<<x<<", "<<y<<endl;
	in.cartesian.real(x);
	in.cartesian.imag(y);
	atanout_t atanx;
	hls::atan2<hls::CORDIC_FORMAT_SCA,16,16,hls::CORDIC_ROUND_TRUNCATE>(in, atanx);
	phase=atanx.phase;
}


void lowpass_to_phase(in_t &instream, phaseout_t phase[N_PHASE], phasesout_t &phases) {
#pragma HLS PIPELINE II=1
#pragma HLS INTERFACE axis port=instream
#pragma HLS INTERFACE axis port=phase
#pragma HLS INTERFACE axis port=phases
#pragma HLS ARRAY_PARTITION complete variable=phase
#pragma HLS INTERFACE ap_ctrl_none port=return

	static bool read=true;
	static in_t in;
	out_t out;

	if (read) {
		in=instream;

		for(int i=0;i<N_PHASE;i++) {
			out.data[i]=in.data[i];
//			if (i==0)cout<<"inE="<<in.data[i]<<" out="<<out.data[i]<<endl;
		}
		out.last=false;
		out.user=in.user*2;
		read=false;
	} else {
		for(int i=0;i<N_PHASE;i++) {
			out.data[i]=in.data[i+N_PHASE];
//			if (i==0) cout<<"inO="<<in.data[i+N_PHASE]<<" out="<<out.data[i]<<endl;
		}
		out.last=in.last;
		out.user=in.user*2+1;
		read=true;
	}

	phasesout_t tmpphases;
	for (int i=0;i<N_PHASE;i++) {
		phase_t x;
		iq_t cart=out.data[i];
		comphase(cart, x);
		tmpphases.data[i]=x;
		phase[i].data=x;
		phase[i].last=out.last;
		phase[i].user=out.user;
	}
	tmpphases.last=out.last;
	tmpphases.user=out.user;
	phases=tmpphases;
}

//void downsize(in_t &instream, out_t &outstream) {
//#pragma HLS PIPELINE II=1
//#pragma HLS INTERFACE axis port=instream
//#pragma HLS INTERFACE axis port=outstream
//#pragma HLS INTERFACE ap_ctrl_none port=return
//
//	static bool read=true;
//	static in_t in;
//	out_t out;
//
//	if (read) {
//		in=instream;
//		for(int i=0;i<N_PHASE;i++) out.data[i]=in.data[i];
//		out.last=false;
//		out.user=in.user*2;
//		read=false;
//	} else {
//		for(int i=0;i<N_PHASE;i++) out.data[i]=in.data[i+N_PHASE];
//		out.last=in.last;
//		out.user=in.user*2+1;
//		read=true;
//	}
//
//	outstream=out;
//
//}
