/* ------------------------------------------------------------
name: "delay"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
Compilation options: -lang cpp -i -fpga-mem-th 4 -ct 1 -cn DelayDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __DelayDSP_H__
#define  __DelayDSP_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS DelayDSP
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


class DelayDSP : public dsp {
	
 private:
	
	int IOTA0;
	int fSampleRate;
	float fConst0;
	int iConst1;
	FAUSTFLOAT fEntry0;
	FAUSTFLOAT fEntry1;
	float fVec0[2];
	FAUSTFLOAT fEntry2;
	float fConst2;
	float fRec1[2];
	float fRec0[524288];
	float fVec1[2];
	float fRec3[2];
	float fRec2[524288];
	
 public:
	DelayDSP() {
	}
	
	DelayDSP(const DelayDSP&) = default;
	
	virtual ~DelayDSP() = default;
	
	DelayDSP& operator=(const DelayDSP&) = default;
	
	void metadata(Meta* m) { 
		m->declare("compile_options", "-lang cpp -i -fpga-mem-th 4 -ct 1 -cn DelayDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.2.0");
		m->declare("filename", "delay.dsp");
		m->declare("filters.lib/lowpass0_highpass1", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass0_highpass1:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/tf1:author", "Julius O. Smith III");
		m->declare("filters.lib/tf1:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf1:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf1s:author", "Julius O. Smith III");
		m->declare("filters.lib/tf1s:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf1s:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/version", "1.7.1");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.9.0");
		m->declare("name", "delay");
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
		iConst1 = static_cast<int>(2.0f * fConst0);
		fConst2 = 3.1415927f / fConst0;
	}
	
	virtual void instanceResetUserInterface() {
		fEntry0 = static_cast<FAUSTFLOAT>(0.3f);
		fEntry1 = static_cast<FAUSTFLOAT>(0.4f);
		fEntry2 = static_cast<FAUSTFLOAT>(0.5f);
	}
	
	virtual void instanceClear() {
		IOTA0 = 0;
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fVec0[l0] = 0.0f;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fRec1[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 524288; l2 = l2 + 1) {
			fRec0[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fVec1[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fRec3[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 524288; l5 = l5 + 1) {
			fRec2[l5] = 0.0f;
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
	
	virtual DelayDSP* clone() {
		return new DelayDSP(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("delay");
		ui_interface->addNumEntry("damp", &fEntry2, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("delay_time", &fEntry0, FAUSTFLOAT(0.3f), FAUSTFLOAT(0.01f), FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("feedback", &fEntry1, FAUSTFLOAT(0.4f), FAUSTFLOAT(0.0f), FAUSTFLOAT(0.95f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* input1 = inputs[1];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		int iSlow0 = std::min<int>(iConst1, std::max<int>(0, static_cast<int>(fConst0 * static_cast<float>(fEntry0)))) + 1;
		float fSlow1 = static_cast<float>(fEntry1);
		float fSlow2 = 1.0f / std::tan(fConst2 * (1.2e+04f * (1.0f - static_cast<float>(fEntry2)) + 8e+02f));
		float fSlow3 = 1.0f - fSlow2;
		float fSlow4 = 1.0f / (fSlow2 + 1.0f);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			float fTemp0 = fSlow1 * fRec0[(IOTA0 - iSlow0) & 524287];
			fVec0[0] = fTemp0;
			fRec1[0] = -(fSlow4 * (fSlow3 * fRec1[1] - (fTemp0 + fVec0[1])));
			fRec0[IOTA0 & 524287] = static_cast<float>(input0[i0]) + fRec1[0];
			output0[i0] = static_cast<FAUSTFLOAT>(fRec0[IOTA0 & 524287]);
			float fTemp1 = fSlow1 * fRec2[(IOTA0 - iSlow0) & 524287];
			fVec1[0] = fTemp1;
			fRec3[0] = -(fSlow4 * (fSlow3 * fRec3[1] - (fTemp1 + fVec1[1])));
			fRec2[IOTA0 & 524287] = static_cast<float>(input1[i0]) + fRec3[0];
			output1[i0] = static_cast<FAUSTFLOAT>(fRec2[IOTA0 & 524287]);
			IOTA0 = IOTA0 + 1;
			fVec0[1] = fVec0[0];
			fRec1[1] = fRec1[0];
			fVec1[1] = fVec1[0];
			fRec3[1] = fRec3[0];
		}
	}

};

#endif
