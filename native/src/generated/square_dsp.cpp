/* ------------------------------------------------------------
name: "square"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
Compilation options: -lang cpp -i -fpga-mem-th 4 -ct 1 -cn SquareDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __SquareDSP_H__
#define  __SquareDSP_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS SquareDSP
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

static float SquareDSP_faustpower2_f(float value) {
	return value * value;
}

class SquareDSP : public dsp {
	
 private:
	
	int iVec0[2];
	FAUSTFLOAT fEntry0;
	float fVec1[2];
	int iRec0[2];
	FAUSTFLOAT fEntry1;
	int fSampleRate;
	float fConst0;
	float fRec1[2];
	FAUSTFLOAT fEntry2;
	FAUSTFLOAT fEntry3;
	FAUSTFLOAT fEntry4;
	FAUSTFLOAT fEntry5;
	float fConst1;
	float fConst2;
	FAUSTFLOAT fEntry6;
	float fRec2[2];
	float fConst3;
	FAUSTFLOAT fEntry7;
	float fRec3[2];
	FAUSTFLOAT fEntry8;
	float fRec5[2];
	FAUSTFLOAT fEntry9;
	float fConst4;
	float fRec6[2];
	float fVec2[2];
	int IOTA0;
	float fVec3[4096];
	float fConst5;
	float fConst6;
	float fRec4[3];
	FAUSTFLOAT fEntry10;
	float fRec7[2];
	
 public:
	SquareDSP() {
	}
	
	SquareDSP(const SquareDSP&) = default;
	
	virtual ~SquareDSP() = default;
	
	SquareDSP& operator=(const SquareDSP&) = default;
	
	void metadata(Meta* m) { 
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "1.22.0");
		m->declare("compile_options", "-lang cpp -i -fpga-mem-th 4 -ct 1 -cn SquareDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("envelopes.lib/adsr:author", "Yann Orlarey and Andrey Bundin");
		m->declare("envelopes.lib/author", "GRAME");
		m->declare("envelopes.lib/copyright", "GRAME");
		m->declare("envelopes.lib/license", "LGPL with exception");
		m->declare("envelopes.lib/name", "Faust Envelope Library");
		m->declare("envelopes.lib/version", "1.3.0");
		m->declare("filename", "square.dsp");
		m->declare("filters.lib/fir:author", "Julius O. Smith III");
		m->declare("filters.lib/fir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/fir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/iir:author", "Julius O. Smith III");
		m->declare("filters.lib/iir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/iir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/resonbp:author", "Julius O. Smith III");
		m->declare("filters.lib/resonbp:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/resonbp:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/resonhp:author", "Julius O. Smith III");
		m->declare("filters.lib/resonhp:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/resonhp:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/resonlp:author", "Julius O. Smith III");
		m->declare("filters.lib/resonlp:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/resonlp:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf2:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf2s:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2s:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2s:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/version", "1.7.1");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.9.0");
		m->declare("name", "square");
		m->declare("oscillators.lib/lf_sawpos:author", "Bart Brouns, revised by Stéphane Letz");
		m->declare("oscillators.lib/lf_sawpos:licence", "STK-4.3");
		m->declare("oscillators.lib/name", "Faust Oscillator Library");
		m->declare("oscillators.lib/sawN:author", "Julius O. Smith III");
		m->declare("oscillators.lib/sawN:license", "STK-4.3");
		m->declare("oscillators.lib/version", "1.7.0");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("signals.lib/name", "Faust Routing Library");
		m->declare("signals.lib/version", "1.6.0");
	}

	virtual int getNumInputs() {
		return 0;
	}
	virtual int getNumOutputs() {
		return 1;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, static_cast<float>(fSampleRate)));
		fConst1 = 44.1f / fConst0;
		fConst2 = 1.0f - fConst1;
		fConst3 = 3.1415927f / fConst0;
		fConst4 = 1.0f / fConst0;
		fConst5 = 0.5f * fConst0;
		fConst6 = 0.25f * fConst0;
	}
	
	virtual void instanceResetUserInterface() {
		fEntry0 = static_cast<FAUSTFLOAT>(0.0f);
		fEntry1 = static_cast<FAUSTFLOAT>(2.0f);
		fEntry2 = static_cast<FAUSTFLOAT>(0.05f);
		fEntry3 = static_cast<FAUSTFLOAT>(0.3f);
		fEntry4 = static_cast<FAUSTFLOAT>(0.8f);
		fEntry5 = static_cast<FAUSTFLOAT>(0.0f);
		fEntry6 = static_cast<FAUSTFLOAT>(2e+04f);
		fEntry7 = static_cast<FAUSTFLOAT>(0.0f);
		fEntry8 = static_cast<FAUSTFLOAT>(4.4e+02f);
		fEntry9 = static_cast<FAUSTFLOAT>(0.0f);
		fEntry10 = static_cast<FAUSTFLOAT>(0.0f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iVec0[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fVec1[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			iRec0[l2] = 0;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec1[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fRec2[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fRec3[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fRec5[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			fRec6[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
			fVec2[l8] = 0.0f;
		}
		IOTA0 = 0;
		for (int l9 = 0; l9 < 4096; l9 = l9 + 1) {
			fVec3[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 3; l10 = l10 + 1) {
			fRec4[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fRec7[l11] = 0.0f;
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
	
	virtual SquareDSP* clone() {
		return new SquareDSP(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("square");
		ui_interface->addNumEntry("amp", &fEntry10, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.001f));
		ui_interface->addNumEntry("attack", &fEntry2, FAUSTFLOAT(0.05f), FAUSTFLOAT(0.001f), FAUSTFLOAT(5.0f), FAUSTFLOAT(0.001f));
		ui_interface->addNumEntry("decay", &fEntry3, FAUSTFLOAT(0.3f), FAUSTFLOAT(0.001f), FAUSTFLOAT(5.0f), FAUSTFLOAT(0.001f));
		ui_interface->addNumEntry("detune", &fEntry9, FAUSTFLOAT(0.0f), FAUSTFLOAT(-1e+02f), FAUSTFLOAT(1e+02f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("filter_cutoff", &fEntry6, FAUSTFLOAT(2e+04f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(2e+04f), FAUSTFLOAT(1.0f));
		ui_interface->addNumEntry("filter_res", &fEntry7, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("filter_type", &fEntry5, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(3.0f), FAUSTFLOAT(1.0f));
		ui_interface->addNumEntry("freq", &fEntry8, FAUSTFLOAT(4.4e+02f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(2e+04f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("gate", &fEntry0, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(1.0f));
		ui_interface->addNumEntry("release", &fEntry1, FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f), FAUSTFLOAT(3e+01f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("sustain", &fEntry4, FAUSTFLOAT(0.8f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.001f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		float fSlow0 = static_cast<float>(fEntry0);
		int iSlow1 = fSlow0 == 0.0f;
		float fSlow2 = 1.0f / std::max<float>(1.0f, fConst0 * static_cast<float>(fEntry1));
		float fSlow3 = std::max<float>(1.0f, fConst0 * static_cast<float>(fEntry2));
		float fSlow4 = 1.0f / fSlow3;
		float fSlow5 = static_cast<float>(fEntry4);
		float fSlow6 = (1.0f - fSlow5) / std::max<float>(1.0f, fConst0 * static_cast<float>(fEntry3));
		int iSlow7 = static_cast<int>(static_cast<float>(fEntry5));
		int iSlow8 = iSlow7 >= 2;
		int iSlow9 = iSlow7 >= 1;
		float fSlow10 = fConst1 * static_cast<float>(fEntry6);
		float fSlow11 = fConst1 * static_cast<float>(fEntry7);
		float fSlow12 = fConst1 * static_cast<float>(fEntry8);
		float fSlow13 = std::pow(2.0f, 0.00083333335f * static_cast<float>(fEntry9));
		int iSlow14 = iSlow7 >= 3;
		float fSlow15 = fConst1 * static_cast<float>(fEntry10);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iVec0[0] = 1;
			fVec1[0] = fSlow0;
			iRec0[0] = iSlow1 * (iRec0[1] + 1);
			fRec1[0] = fSlow0 + fRec1[1] * static_cast<float>(fVec1[1] >= fSlow0);
			fRec2[0] = fSlow10 + fConst2 * fRec2[1];
			float fTemp0 = std::tan(fConst3 * fRec2[0]);
			fRec3[0] = fSlow11 + fConst2 * fRec3[1];
			float fTemp1 = 1.0f / (19.293f * fRec3[0] + 0.707f);
			float fTemp2 = 1.0f / fTemp0;
			float fTemp3 = (fTemp2 + fTemp1) / fTemp0 + 1.0f;
			fRec5[0] = fSlow12 + fConst2 * fRec5[1];
			float fTemp4 = std::max<float>(fSlow13 * fRec5[0], 23.44895f);
			float fTemp5 = std::max<float>(2e+01f, std::fabs(fTemp4));
			float fTemp6 = ((1 - iVec0[1]) ? 0.0f : fRec6[1] + fConst4 * fTemp5);
			fRec6[0] = fTemp6 - std::floor(fTemp6);
			float fTemp7 = SquareDSP_faustpower2_f(2.0f * fRec6[0] + -1.0f);
			fVec2[0] = fTemp7;
			float fTemp8 = static_cast<float>(iVec0[1]) * (fTemp7 - fVec2[1]) / fTemp5;
			fVec3[IOTA0 & 4095] = fTemp8;
			float fTemp9 = std::max<float>(0.0f, std::min<float>(2047.0f, fConst5 / fTemp4));
			int iTemp10 = static_cast<int>(fTemp9);
			float fTemp11 = std::floor(fTemp9);
			float fTemp12 = fConst6 * (fTemp8 - fVec3[(IOTA0 - iTemp10) & 4095] * (fTemp11 + (1.0f - fTemp9)) - (fTemp9 - fTemp11) * fVec3[(IOTA0 - (iTemp10 + 1)) & 4095]);
			fRec4[0] = fTemp12 - (fRec4[2] * ((fTemp2 - fTemp1) / fTemp0 + 1.0f) + 2.0f * fRec4[1] * (1.0f - 1.0f / SquareDSP_faustpower2_f(fTemp0))) / fTemp3;
			float fTemp13 = (fRec4[2] + fRec4[0] + 2.0f * fRec4[1]) / fTemp3;
			float fTemp14 = (fRec4[0] - fRec4[2]) / (fTemp0 * fTemp3);
			fRec7[0] = fSlow15 + fConst2 * fRec7[1];
			output0[i0] = static_cast<FAUSTFLOAT>(fRec7[0] * ((iSlow8) ? ((iSlow14) ? fTemp12 - fTemp14 : fTemp14) : ((iSlow9) ? fTemp12 - fTemp13 : fTemp13)) * std::max<float>(0.0f, std::min<float>(fSlow4 * fRec1[0], std::max<float>(fSlow6 * (fSlow3 - fRec1[0]) + 1.0f, fSlow5)) * (1.0f - fSlow2 * static_cast<float>(iRec0[0]))));
			iVec0[1] = iVec0[0];
			fVec1[1] = fVec1[0];
			iRec0[1] = iRec0[0];
			fRec1[1] = fRec1[0];
			fRec2[1] = fRec2[0];
			fRec3[1] = fRec3[0];
			fRec5[1] = fRec5[0];
			fRec6[1] = fRec6[0];
			fVec2[1] = fVec2[0];
			IOTA0 = IOTA0 + 1;
			fRec4[2] = fRec4[1];
			fRec4[1] = fRec4[0];
			fRec7[1] = fRec7[0];
		}
	}

};

#endif
