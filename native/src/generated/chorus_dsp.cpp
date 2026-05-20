/* ------------------------------------------------------------
name: "chorus"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
Compilation options: -lang cpp -i -fpga-mem-th 4 -ct 1 -cn ChorusDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __ChorusDSP_H__
#define  __ChorusDSP_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS ChorusDSP
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


class ChorusDSP : public dsp {
	
 private:
	
	int iVec0[2];
	FAUSTFLOAT fEntry0;
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fRec0[2];
	float fRec1[2];
	FAUSTFLOAT fEntry1;
	float fConst2;
	int IOTA0;
	float fVec1[2048];
	float fRec2[2];
	float fRec3[2];
	float fVec2[2048];
	
 public:
	ChorusDSP() {
	}
	
	ChorusDSP(const ChorusDSP&) = default;
	
	virtual ~ChorusDSP() = default;
	
	ChorusDSP& operator=(const ChorusDSP&) = default;
	
	void metadata(Meta* m) { 
		m->declare("compile_options", "-lang cpp -i -fpga-mem-th 4 -ct 1 -cn ChorusDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.2.0");
		m->declare("filename", "chorus.dsp");
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
		m->declare("name", "chorus");
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
		fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, static_cast<float>(fSampleRate)));
		fConst1 = 6.2831855f / fConst0;
		fConst2 = 0.5f * fConst0;
	}
	
	virtual void instanceResetUserInterface() {
		fEntry0 = static_cast<FAUSTFLOAT>(1.5f);
		fEntry1 = static_cast<FAUSTFLOAT>(0.002f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iVec0[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fRec0[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec1[l2] = 0.0f;
		}
		IOTA0 = 0;
		for (int l3 = 0; l3 < 2048; l3 = l3 + 1) {
			fVec1[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fRec2[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fRec3[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2048; l6 = l6 + 1) {
			fVec2[l6] = 0.0f;
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
	
	virtual ChorusDSP* clone() {
		return new ChorusDSP(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("chorus");
		ui_interface->addNumEntry("depth", &fEntry1, FAUSTFLOAT(0.002f), FAUSTFLOAT(0.0f), FAUSTFLOAT(0.01f), FAUSTFLOAT(0.0001f));
		ui_interface->addNumEntry("rate", &fEntry0, FAUSTFLOAT(1.5f), FAUSTFLOAT(0.1f), FAUSTFLOAT(1e+01f), FAUSTFLOAT(0.1f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* input1 = inputs[1];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = static_cast<float>(fEntry0);
		float fSlow1 = fConst1 * fSlow0;
		float fSlow2 = std::cos(fSlow1);
		float fSlow3 = std::sin(fSlow1);
		float fSlow4 = fConst2 * static_cast<float>(fEntry1);
		float fSlow5 = fConst1 * (fSlow0 + 0.1f);
		float fSlow6 = std::cos(fSlow5);
		float fSlow7 = std::sin(fSlow5);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iVec0[0] = 1;
			fRec0[0] = fSlow3 * fRec1[1] + fSlow2 * fRec0[1];
			float fTemp0 = static_cast<float>(1 - iVec0[1]);
			fRec1[0] = fTemp0 + fSlow2 * fRec1[1] - fSlow3 * fRec0[1];
			float fTemp1 = fSlow4 * (fRec0[0] + 1.0f);
			float fTemp2 = fTemp1 + 1e+01f;
			float fTemp3 = std::floor(fTemp2);
			float fTemp4 = static_cast<float>(input0[i0]);
			fVec1[IOTA0 & 2047] = fTemp4;
			int iTemp5 = static_cast<int>(fTemp2);
			output0[i0] = static_cast<FAUSTFLOAT>(0.5f * ((fTemp1 + (1e+01f - fTemp3)) * fVec1[(IOTA0 - std::min<int>(1025, std::max<int>(0, iTemp5 + 1))) & 2047] + fTemp4 + fVec1[(IOTA0 - std::min<int>(1025, std::max<int>(0, iTemp5))) & 2047] * (fTemp3 + (-9.0f - fTemp1))));
			fRec2[0] = fSlow7 * fRec3[1] + fSlow6 * fRec2[1];
			fRec3[0] = fTemp0 + fSlow6 * fRec3[1] - fSlow7 * fRec2[1];
			float fTemp6 = fSlow4 * (fRec2[0] + 1.0f);
			float fTemp7 = fTemp6 + 1e+01f;
			float fTemp8 = std::floor(fTemp7);
			float fTemp9 = static_cast<float>(input1[i0]);
			fVec2[IOTA0 & 2047] = fTemp9;
			int iTemp10 = static_cast<int>(fTemp7);
			output1[i0] = static_cast<FAUSTFLOAT>(0.5f * ((fTemp6 + (1e+01f - fTemp8)) * fVec2[(IOTA0 - std::min<int>(1025, std::max<int>(0, iTemp10 + 1))) & 2047] + fTemp9 + fVec2[(IOTA0 - std::min<int>(1025, std::max<int>(0, iTemp10))) & 2047] * (fTemp8 + (-9.0f - fTemp6))));
			iVec0[1] = iVec0[0];
			fRec0[1] = fRec0[0];
			fRec1[1] = fRec1[0];
			IOTA0 = IOTA0 + 1;
			fRec2[1] = fRec2[0];
			fRec3[1] = fRec3[0];
		}
	}

};

#endif
