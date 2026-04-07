#ifndef INC_PRAMP_H
#define INC_PRAMP_H

#include <cmath> // abs, fmod

// Periodic ramp with unit phase

// Output is a linearly increasing ramp from 0 to 1.
// A triangle wave mapping is also provided to create
// oscillating motions.
// 
// Lance Putnam, 2022
class PRamp{
public:

	// Set frequency, in Hertz
	PRamp& freq(float v) { mFreq = v; return *this; }
	// Get frequency, in Hertz
	float freq() const { return mFreq; }

	// Set period, in seconds
	PRamp& period(float v) { return freq(1./v); }

	// Set phase, in [0,1)
	PRamp& phase(float v){ mPhase=v; return *this; }
	// Get phase, in [0,1)
	float phase() const { return mPhase; }
	// Get phase, in [0, 2pi)
	float phaseRad() const { return mPhase * 6.283185307179586; }

	// Update phase
	PRamp& update(float deltaSec){
		mPhase = std::fmod(mPhase + mFreq*deltaSec, 1.f);
		return *this;
	}

	// Map current phase to triangle wave, in [0,1]
	float tri() const {
		// https://www.desmos.com/calculator/ojpjy7ib0w
		return 1. - std::abs(2.*mPhase-1.);
	}

	// Map current phase to parabolic wave, in [0,1]
	float para() const {
		auto saw = 2.*mPhase-1.;
		return 1. - saw*saw;
	}

private:
	float mPhase = 0.; // current phase of ramp
	float mFreq = 1.; // 1/period
};

#endif // include guard
