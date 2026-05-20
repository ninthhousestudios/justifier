/* ------------------------------------------------------------
name: "flanger"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
Compilation options: -lang cpp -i -fpga-mem-th 4 -ct 1 -cn FlangerDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __FlangerDSP_H__
#define  __FlangerDSP_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS FlangerDSP
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

#if defined(_WIN32)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif


class FlangerDSP : public dsp {
	
 private:
	
	int IOTA0;
	int iVec0[2];
	FAUSTFLOAT fEntry0;
	int fSampleRate;
	float fConst0;
	float fRec1[2];
	float fRec2[2];
	FAUSTFLOAT fEntry1;
	FAUSTFLOAT fEntry2;
	float fRec0[2048];
	float fRec3[2048];
	
 public:
	FlangerDSP() {
	}
	
	FlangerDSP(const FlangerDSP&) = default;
	
	virtual ~FlangerDSP() = default;
	
	FlangerDSP& operator=(const FlangerDSP&) = default;
	
	void metadata(Meta* m) { 
		m->declare("compile_options", "-lang cpp -i -fpga-mem-th 4 -ct 1 -cn FlangerDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.2.0");
		m->declare("filename", "flanger.dsp");
		m->declare("filters.lib/lowpass0_highpass1", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/nlf2:author", "Julius O. Smith III");
		m->declare("filters.lib/nlf2:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/nlf2:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/version", "1.7.1");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.9.0");
		m->declare("name", "flanger");
		m->declare("oscillators.lib/name", "Faust Oscillator Library");
		m->declare("oscillators.lib/version", "1.7.0");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
	}

	virtual int getNumInputs() {
		return 2;
	}
	virtual int getNumOutputs() {
		return 2;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = 6.2831855f / std::min<float>(1.92e+05f, std::max<float>(1.0f, static_cast<float>(fSampleRate)));
	}
	
	virtual void instanceResetUserInterface() {
		fEntry0 = static_cast<FAUSTFLOAT>(0.3f);
		fEntry1 = static_cast<FAUSTFLOAT>(0.5f);
		fEntry2 = static_cast<FAUSTFLOAT>(0.6f);
	}
	
	virtual void instanceClear() {
		IOTA0 = 0;
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iVec0[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fRec1[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec2[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 2048; l3 = l3 + 1) {
			fRec0[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 2048; l4 = l4 + 1) {
			fRec3[l4] = 0.0f;
		}
	}
	
	virtual void init(int sample_rate) {
		classInit(sample_rate);
		instanceInit(sample_rate);
	}
	
	virtual void instanceInit(int sample_rate) {
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
	}
	
	virtual FlangerDSP* clone() {
		return new FlangerDSP(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("flanger");
		ui_interface->addNumEntry("depth", &fEntry1, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("fb", &fEntry2, FAUSTFLOAT(0.6f), FAUSTFLOAT(0.0f), FAUSTFLOAT(0.95f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("rate", &fEntry0, FAUSTFLOAT(0.3f), FAUSTFLOAT(0.01f), FAUSTFLOAT(1e+01f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* input1 = inputs[1];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = fConst0 * static_cast<float>(fEntry0);
		float fSlow1 = std::cos(fSlow0);
		float fSlow2 = std::sin(fSlow0);
		float fSlow3 = 4.5f * static_cast<float>(fEntry1);
		float fSlow4 = static_cast<float>(fEntry2);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iVec0[0] = 1;
			fRec1[0] = fSlow2 * fRec2[1] + fSlow1 * fRec1[1];
			fRec2[0] = static_cast<float>(1 - iVec0[1]) + fSlow1 * fRec2[1] - fSlow2 * fRec1[1];
			float fTemp0 = fSlow3 * (fRec1[0] + 1.0f);
			float fTemp1 = fTemp0 + 1.0f;
			int iTemp2 = static_cast<int>(fTemp1);
			int iTemp3 = std::min<int>(1025, std::max<int>(0, iTemp2 + 1)) + 1;
			float fTemp4 = std::floor(fTemp1);
			float fTemp5 = fTemp0 + (1.0f - fTemp4);
			float fTemp6 = fTemp4 - fTemp0;
			int iTemp7 = std::min<int>(1025, std::max<int>(0, iTemp2)) + 1;
			float fTemp8 = static_cast<float>(input0[i0]);
			fRec0[IOTA0 & 2047] = fTemp8 + fSlow4 * (fRec0[(IOTA0 - iTemp7) & 2047] * fTemp6 + fTemp5 * fRec0[(IOTA0 - iTemp3) & 2047]);
			output0[i0] = static_cast<FAUSTFLOAT>(0.5f * (fTemp8 + fRec0[IOTA0 & 2047]));
			float fTemp9 = static_cast<float>(input1[i0]);
			fRec3[IOTA0 & 2047] = fTemp9 + fSlow4 * (fTemp6 * fRec3[(IOTA0 - iTemp7) & 2047] + fTemp5 * fRec3[(IOTA0 - iTemp3) & 2047]);
			output1[i0] = static_cast<FAUSTFLOAT>(0.5f * (fTemp9 + fRec3[IOTA0 & 2047]));
			IOTA0 = IOTA0 + 1;
			iVec0[1] = iVec0[0];
			fRec1[1] = fRec1[0];
			fRec2[1] = fRec2[0];
		}
	}

};

#endif
