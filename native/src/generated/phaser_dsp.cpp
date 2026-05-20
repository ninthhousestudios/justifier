/* ------------------------------------------------------------
name: "phaser"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
Compilation options: -lang cpp -i -fpga-mem-th 4 -ct 1 -cn PhaserDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __PhaserDSP_H__
#define  __PhaserDSP_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS PhaserDSP
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

static float PhaserDSP_faustpower2_f(float value) {
	return value * value;
}

class PhaserDSP : public dsp {
	
 private:
	
	int iVec0[2];
	FAUSTFLOAT fEntry0;
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fRec1[2];
	float fRec2[2];
	float fConst2;
	float fConst3;
	float fConst4;
	float fConst5;
	float fConst6;
	float fConst7;
	float fConst8;
	FAUSTFLOAT fEntry1;
	float fRec6[3];
	float fRec5[3];
	float fRec4[3];
	float fRec3[3];
	float fRec0[2];
	FAUSTFLOAT fEntry2;
	float fRec11[3];
	float fRec10[3];
	float fRec9[3];
	float fRec8[3];
	float fRec7[2];
	
 public:
	PhaserDSP() {
	}
	
	PhaserDSP(const PhaserDSP&) = default;
	
	virtual ~PhaserDSP() = default;
	
	PhaserDSP& operator=(const PhaserDSP&) = default;
	
	void metadata(Meta* m) { 
		m->declare("compile_options", "-lang cpp -i -fpga-mem-th 4 -ct 1 -cn PhaserDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("filename", "phaser.dsp");
		m->declare("filters.lib/fir:author", "Julius O. Smith III");
		m->declare("filters.lib/fir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/fir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/iir:author", "Julius O. Smith III");
		m->declare("filters.lib/iir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/iir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/nlf2:author", "Julius O. Smith III");
		m->declare("filters.lib/nlf2:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/nlf2:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf2:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/version", "1.7.1");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.9.0");
		m->declare("name", "phaser");
		m->declare("oscillators.lib/name", "Faust Oscillator Library");
		m->declare("oscillators.lib/version", "1.7.0");
		m->declare("phaflangers.lib/name", "Faust Phaser and Flanger Library");
		m->declare("phaflangers.lib/version", "1.1.0");
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
		fConst2 = 5.0625f / fConst0;
		fConst3 = 3.375f / fConst0;
		fConst4 = 2.25f / fConst0;
		fConst5 = 1.5f / fConst0;
		fConst6 = std::exp(-(157.07964f / fConst0));
		fConst7 = PhaserDSP_faustpower2_f(fConst6);
		fConst8 = 2.0f * fConst6;
	}
	
	virtual void instanceResetUserInterface() {
		fEntry0 = static_cast<FAUSTFLOAT>(0.5f);
		fEntry1 = static_cast<FAUSTFLOAT>(0.7f);
		fEntry2 = static_cast<FAUSTFLOAT>(0.7f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iVec0[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fRec1[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec2[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 3; l3 = l3 + 1) {
			fRec6[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 3; l4 = l4 + 1) {
			fRec5[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 3; l5 = l5 + 1) {
			fRec4[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 3; l6 = l6 + 1) {
			fRec3[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			fRec0[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 3; l8 = l8 + 1) {
			fRec11[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 3; l9 = l9 + 1) {
			fRec10[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 3; l10 = l10 + 1) {
			fRec9[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 3; l11 = l11 + 1) {
			fRec8[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			fRec7[l12] = 0.0f;
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
	
	virtual PhaserDSP* clone() {
		return new PhaserDSP(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("phaser");
		ui_interface->addNumEntry("depth", &fEntry2, FAUSTFLOAT(0.7f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("fb", &fEntry1, FAUSTFLOAT(0.7f), FAUSTFLOAT(0.0f), FAUSTFLOAT(0.95f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("speed", &fEntry0, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.01f), FAUSTFLOAT(1e+01f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* input1 = inputs[1];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = fConst1 * static_cast<float>(fEntry0);
		float fSlow1 = std::cos(fSlow0);
		float fSlow2 = std::sin(fSlow0);
		float fSlow3 = static_cast<float>(fEntry1);
		float fSlow4 = 0.5f * static_cast<float>(fEntry2);
		float fSlow5 = 1.0f - fSlow4;
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iVec0[0] = 1;
			fRec1[0] = fSlow2 * fRec2[1] + fSlow1 * fRec1[1];
			fRec2[0] = static_cast<float>(1 - iVec0[1]) + fSlow1 * fRec2[1] - fSlow2 * fRec1[1];
			float fTemp0 = 11938.052f * (1.0f - fRec1[0]) + 1256.6371f;
			float fTemp1 = std::cos(fConst2 * fTemp0);
			float fTemp2 = fRec3[1] * fTemp1;
			float fTemp3 = std::cos(fConst3 * fTemp0);
			float fTemp4 = fRec4[1] * fTemp3;
			float fTemp5 = std::cos(fConst4 * fTemp0);
			float fTemp6 = fRec5[1] * fTemp5;
			float fTemp7 = std::cos(fConst5 * fTemp0);
			float fTemp8 = fRec6[1] * fTemp7;
			float fTemp9 = static_cast<float>(input0[i0]);
			fRec6[0] = fTemp9 + fSlow3 * fRec0[1] + fConst8 * fTemp8 - fConst7 * fRec6[2];
			fRec5[0] = fRec6[2] + fConst7 * (fRec6[0] - fRec5[2]) - fConst8 * (fTemp8 - fTemp6);
			fRec4[0] = fRec5[2] + fConst7 * (fRec5[0] - fRec4[2]) - fConst8 * (fTemp6 - fTemp4);
			fRec3[0] = fRec4[2] + fConst7 * (fRec4[0] - fRec3[2]) - fConst8 * (fTemp4 - fTemp2);
			fRec0[0] = fRec3[2] + fConst7 * fRec3[0] - fConst8 * fTemp2;
			output0[i0] = static_cast<FAUSTFLOAT>(fSlow5 * fTemp9 + fSlow4 * fRec0[0]);
			float fTemp10 = fTemp1 * fRec8[1];
			float fTemp11 = fTemp3 * fRec9[1];
			float fTemp12 = fTemp5 * fRec10[1];
			float fTemp13 = fTemp7 * fRec11[1];
			float fTemp14 = static_cast<float>(input1[i0]);
			fRec11[0] = fTemp14 + fSlow3 * fRec7[1] + fConst8 * fTemp13 - fConst7 * fRec11[2];
			fRec10[0] = fRec11[2] + fConst7 * (fRec11[0] - fRec10[2]) - fConst8 * (fTemp13 - fTemp12);
			fRec9[0] = fRec10[2] + fConst7 * (fRec10[0] - fRec9[2]) - fConst8 * (fTemp12 - fTemp11);
			fRec8[0] = fRec9[2] + fConst7 * (fRec9[0] - fRec8[2]) - fConst8 * (fTemp11 - fTemp10);
			fRec7[0] = fRec8[2] + fConst7 * fRec8[0] - fConst8 * fTemp10;
			output1[i0] = static_cast<FAUSTFLOAT>(fSlow5 * fTemp14 + fSlow4 * fRec7[0]);
			iVec0[1] = iVec0[0];
			fRec1[1] = fRec1[0];
			fRec2[1] = fRec2[0];
			fRec6[2] = fRec6[1];
			fRec6[1] = fRec6[0];
			fRec5[2] = fRec5[1];
			fRec5[1] = fRec5[0];
			fRec4[2] = fRec4[1];
			fRec4[1] = fRec4[0];
			fRec3[2] = fRec3[1];
			fRec3[1] = fRec3[0];
			fRec0[1] = fRec0[0];
			fRec11[2] = fRec11[1];
			fRec11[1] = fRec11[0];
			fRec10[2] = fRec10[1];
			fRec10[1] = fRec10[0];
			fRec9[2] = fRec9[1];
			fRec9[1] = fRec9[0];
			fRec8[2] = fRec8[1];
			fRec8[1] = fRec8[0];
			fRec7[1] = fRec7[0];
		}
	}

};

#endif
