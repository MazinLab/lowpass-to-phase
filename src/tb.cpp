#include "tophase.hpp"
#include "hls_dsp.h"
#include <cmath>
#include <iostream>

using namespace std;

#define NLANE 8
#define NPLANE 4
#define SAMPLES 8
const int INTEGER_BITS = 3;

const int InputWidth=16;
const int OutputWidth=16;


void gen_samples(float samples[2][SAMPLES]) {
	  for (int sample=0; sample<SAMPLES; sample++) {
	    const float re = 0.5;
	    const float im = 0.5;
	    float scale_re = 1;
	    float scale_im = 1;
	    if (sample <= SAMPLES/4) {
	      if (sample % 2 == 0) {
	        scale_re = 1;
	        scale_im = 1;
	      } else {
	        scale_re = 1;
	        scale_im = 0;
	      }
	    } else if (sample > SAMPLES/4 && sample <= SAMPLES/2) {
	      if (sample % 2 == 0) {
	        scale_re = -1;
	        scale_im = 1;
	      } else {
	        scale_re = 0;
	        scale_im = 1;
	      }
	    } else if (sample > SAMPLES/2 && sample <= 3*SAMPLES/4) {
	      if (sample % 2 == 0) {
	        scale_re = -1;
	        scale_im = -1;
	      } else {
	        scale_re = -1;
	        scale_im = 0;
	      }
	    } else {
	      if (sample % 2 == 0) {
	        scale_re = 1;
	        scale_im = -1;
	      } else {
	        scale_re = 0;
	        scale_im = -1;
	      }
	    }
	    samples[0][sample]= re*scale_re;
	    samples[1][sample]= im*scale_im;
	  }
}



int main (void){

	float TOLERANCE = pow(2,-(OutputWidth-(INTEGER_BITS+1)));
	const phase_t ap_TOLERANCE = TOLERANCE;// Equivalent to setting second LSB

	bool fail=false;
	float iq[2][SAMPLES];
	gen_samples(iq);

//	for (int i=0;i<SAMPLES;i++){
//		cout<<i<<" "<<iq[0][i]<<", "<<iq[1][i]<<endl;
//	}

	for (int i=0;i<SAMPLES;i++) {
		in_t in;

		//load
		for (int j=0;j<NLANE;j++) {
			in.data[j].range(31,16)=sample_t(iq[1][i]).range(15,0);
			in.data[j].range(15,0)=sample_t(iq[0][i]).range(15,0);
		}
		in.last=i==SAMPLES-1;
		in.user=i;

		//Call twice and check output
		for (int iter=0;iter<2;iter++) {
			phaseout_t phase[N_PHASE];
			phasesout_t phases;

			if (iter==1) {
				for (int j=0;j<NLANE;j++) in.data[j]=0;
				in.last=0;
				in.user=0;
			}

			//call
			lowpass_to_phase(in, phase, phases);

			//check
			for (int j=0;j<NPLANE;j++) { //
				if (phase[j].last && !(i==SAMPLES-1) && !iter==1) {
					fail=true;
					cout<<"FAIL: Last incorrect for lane "<<j<<" cycle "<<i<<endl;
				}
				if (phase[j].user != 2*i+iter) {
					fail=true;
					cout<<"FAIL: Group incorrect for lane "<<j<<" cycle "<<i<<" group: "<<phase[j].user<<endl;
				}
				if (phases.user!=phase[j].user) {
					fail=true;
					cout<<"FAIL: Phases group differs.\n";
				}
				if (phases.last!=phase[j].last) {
					fail=true;
					cout<<"FAIL: Phases last differs.\n";
				}
				if (phases.data[j]!=phase[j].data) {
					fail=true;
					cout<<"FAIL: Phases phase differs.\n";
				}


				atanin_t reference_x_val;
				phase_t expected_atanX, difference;

				reference_x_val.cartesian.real(iq[0][i]);
				reference_x_val.cartesian.imag(iq[1][i]);

				// Take local copy of function output
				phase_t atanX_val_tmp = phase[j].data;

				// Get reference value, scaled to output precision in cast
				expected_atanX = atan2(reference_x_val.cartesian.imag().to_float(),reference_x_val.cartesian.real().to_float())/M_PI;

				// Compensate for implementation differences
				if (reference_x_val.cartesian.imag() == 0 && reference_x_val.cartesian.real() < 0) {
				  if (-atanX_val_tmp == expected_atanX) {
					atanX_val_tmp *= -1;
					std::cout << "Ignoring sign difference between -pi and pi when on Y axis" << std::endl;
				  }
				}

				// Compare results for comparison against tolerance
				difference = atanX_val_tmp - expected_atanX;

				if (abs(difference.to_float()) > TOLERANCE) {
				  fail =true;
				  std::cout << "ERROR: atan2 output mismatch versus expected" << std::endl;
				  std::cout << "X axis input " << reference_x_val.cartesian.real() << std::endl;
				  std::cout << "Y axis input " << reference_x_val.cartesian.imag() << std::endl;
				  std::cout << "Expected " << expected_atanX << std::endl;
				  std::cout << "Actual  " << phase[j].data << std::endl;
				  std::cout << "Difference " << difference << std::endl;
				  std::cout << "Tolerance " << TOLERANCE << std::endl << std::endl;
				} else {
				  std::cout << "X axis input " << reference_x_val.cartesian.real() << std::endl;
				  std::cout << "Y axis input " << reference_x_val.cartesian.imag() << std::endl;
				  std::cout << "Expected " << expected_atanX << std::endl;
				  std::cout << "Actual " << phase[j].data << std::endl;
				  std::cout << "Difference " << difference << std::endl;
				  std::cout << "Tolerance " << TOLERANCE << std::endl << std::endl;
				}
			}
		}
	}

  if (fail) {
	std::cout << "Test failed" << std::endl;
  } else {
	std::cout << "Test passed" << std::endl;
  }


  return(fail);
}
