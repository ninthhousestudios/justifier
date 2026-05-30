/* ------------------------------------------------------------
name: "lfnoise2"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
Compilation options: -lang cpp -i -fpga-mem-th 4 -ct 1 -cn Lfnoise2DSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __Lfnoise2DSP_H__
#define  __Lfnoise2DSP_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS Lfnoise2DSP
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

static float Lfnoise2DSP_faustpower2_f(float value) {
	return value * value;
}

class Lfnoise2DSP : public dsp {
	
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
	float fRec13[2];
	float fRec14[2];
	int iRec15[2];
	float fRec12[2];
	float fRec11[2];
	float fRec10[2];
	float fRec9[2];
	float fRec8[2];
	float fRec7[2];
	float fRec6[3];
	float fRec4[3];
	FAUSTFLOAT fEntry10;
	float fRec16[2];
	
 public:
	Lfnoise2DSP() {
	}
	
	Lfnoise2DSP(const Lfnoise2DSP&) = default;
	
	virtual ~Lfnoise2DSP() = default;
	
	Lfnoise2DSP& operator=(const Lfnoise2DSP&) = default;
	
	void metadata(Meta* m) { 
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "1.22.0");
		m->declare("compile_options", "-lang cpp -i -fpga-mem-th 4 -ct 1 -cn Lfnoise2DSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("envelopes.lib/adsr:author", "Yann Orlarey and Andrey Bundin");
		m->declare("envelopes.lib/author", "GRAME");
		m->declare("envelopes.lib/copyright", "GRAME");
		m->declare("envelopes.lib/license", "LGPL with exception");
		m->declare("envelopes.lib/name", "Faust Envelope Library");
		m->declare("envelopes.lib/version", "1.3.0");
		m->declare("filename", "lfnoise2.dsp");
		m->declare("filters.lib/fir:author", "Julius O. Smith III");
		m->declare("filters.lib/fir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/fir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/iir:author", "Julius O. Smith III");
		m->declare("filters.lib/iir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/iir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/nlf2:author", "Julius O. Smith III");
		m->declare("filters.lib/nlf2:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/nlf2:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/resonbp:author", "Julius O. Smith III");
		m->declare("filters.lib/resonbp:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/resonbp:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/resonhp:author", "Julius O. Smith III");
		m->declare("filters.lib/resonhp:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/resonhp:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/resonlp:author", "Julius O. Smith III");
		m->declare("filters.lib/resonlp:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/resonlp:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf1:author", "Julius O. Smith III");
		m->declare("filters.lib/tf1:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf1:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf1s:author", "Julius O. Smith III");
		m->declare("filters.lib/tf1s:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf1s:license", "MIT-style STK-4.3 license");
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
		m->declare("name", "lfnoise2");
		m->declare("noises.lib/name", "Faust Noise Generator Library");
		m->declare("noises.lib/version", "1.5.0");
		m->declare("oscillators.lib/name", "Faust Oscillator Library");
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
		fConst4 = 6.2831855f / fConst0;
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
			fRec13[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
			fRec14[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
			iRec15[l9] = 0;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			fRec12[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fRec11[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			fRec10[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 2; l13 = l13 + 1) {
			fRec9[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
			fRec8[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fRec7[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 3; l16 = l16 + 1) {
			fRec6[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 3; l17 = l17 + 1) {
			fRec4[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec16[l18] = 0.0f;
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
	
	virtual Lfnoise2DSP* clone() {
		return new Lfnoise2DSP(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("lfnoise2");
		ui_interface->addNumEntry("amp", &fEntry10, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.001f));
		ui_interface->addNumEntry("attack", &fEntry2, FAUSTFLOAT(0.05f), FAUSTFLOAT(0.001f), FAUSTFLOAT(5.0f), FAUSTFLOAT(0.001f));
		ui_interface->addNumEntry("decay", &fEntry3, FAUSTFLOAT(0.3f), FAUSTFLOAT(0.001f), FAUSTFLOAT(5.0f), FAUSTFLOAT(0.001f));
		ui_interface->addNumEntry("detune", &fEntry9, FAUSTFLOAT(0.0f), FAUSTFLOAT(-1e+02f), FAUSTFLOAT(1e+02f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("filter_cutoff", &fEntry6, FAUSTFLOAT(2e+04f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(2e+04f), FAUSTFLOAT(1.0f));
		ui_interface->addNumEntry("filter_res", &fEntry7, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("filter_type", &fEntry5, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(3.0f), FAUSTFLOAT(1.0f));
		ui_interface->addNumEntry("freq", &fEntry8, FAUSTFLOAT(4.4e+02f), FAUSTFLOAT(0.1f), FAUSTFLOAT(2e+04f), FAUSTFLOAT(0.01f));
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
		float fSlow14 = fConst4 * fSlow13;
		float fSlow15 = fConst3 * fSlow13;
		int iSlow16 = iSlow7 >= 3;
		float fSlow17 = fConst1 * static_cast<float>(fEntry10);
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
			float fTemp4 = fSlow14 * fRec5[0];
			float fTemp5 = std::tan(fTemp4);
			float fTemp6 = 1.0f / fTemp5;
			float fTemp7 = (fTemp6 + 1.4142135f) / fTemp5 + 1.0f;
			float fTemp8 = 1.0f / std::tan(fSlow15 * fRec5[0]);
			float fTemp9 = fTemp8 + 1.0f;
			float fTemp10 = std::cos(fTemp4);
			float fTemp11 = std::sin(fTemp4);
			fRec13[0] = fRec14[1] * fTemp11 + fRec13[1] * fTemp10;
			fRec14[0] = static_cast<float>(1 - iVec0[1]) + fRec14[1] * fTemp10 - fTemp11 * fRec13[1];
			int iTemp12 = (fRec13[1] <= 0.0f) & (fRec13[0] > 0.0f);
			iRec15[0] = 1103515245 * iRec15[1] + 12345;
			fRec12[0] = fRec12[1] * static_cast<float>(1 - iTemp12) + 4.656613e-10f * static_cast<float>(iRec15[0]) * static_cast<float>(iTemp12);
			float fTemp13 = 1.0f - fTemp8;
			fRec11[0] = -((fTemp13 * fRec11[1] - (fRec12[0] + fRec12[1])) / fTemp9);
			fRec10[0] = -((fTemp13 * fRec10[1] - (fRec11[0] + fRec11[1])) / fTemp9);
			fRec9[0] = -((fTemp13 * fRec9[1] - (fRec10[0] + fRec10[1])) / fTemp9);
			fRec8[0] = -((fTemp13 * fRec8[1] - (fRec9[0] + fRec9[1])) / fTemp9);
			fRec7[0] = -((fRec7[1] * fTemp13 - (fRec8[0] + fRec8[1])) / fTemp9);
			fRec6[0] = fRec7[0] - (fRec6[2] * ((fTemp6 + -1.4142135f) / fTemp5 + 1.0f) + 2.0f * fRec6[1] * (1.0f - 1.0f / Lfnoise2DSP_faustpower2_f(fTemp5))) / fTemp7;
			float fTemp14 = (fRec6[2] + fRec6[0] + 2.0f * fRec6[1]) / fTemp7;
			fRec4[0] = fTemp14 - (fRec4[2] * ((fTemp2 - fTemp1) / fTemp0 + 1.0f) + 2.0f * fRec4[1] * (1.0f - 1.0f / Lfnoise2DSP_faustpower2_f(fTemp0))) / fTemp3;
			float fTemp15 = (fRec4[2] + fRec4[0] + 2.0f * fRec4[1]) / fTemp3;
			float fTemp16 = (fRec4[0] - fRec4[2]) / (fTemp0 * fTemp3);
			fRec16[0] = fSlow17 + fConst2 * fRec16[1];
			output0[i0] = static_cast<FAUSTFLOAT>(fRec16[0] * ((iSlow8) ? ((iSlow16) ? fTemp14 - fTemp16 : fTemp16) : ((iSlow9) ? fTemp14 - fTemp15 : fTemp15)) * std::max<float>(0.0f, std::min<float>(fSlow4 * fRec1[0], std::max<float>(fSlow6 * (fSlow3 - fRec1[0]) + 1.0f, fSlow5)) * (1.0f - fSlow2 * static_cast<float>(iRec0[0]))));
			iVec0[1] = iVec0[0];
			fVec1[1] = fVec1[0];
			iRec0[1] = iRec0[0];
			fRec1[1] = fRec1[0];
			fRec2[1] = fRec2[0];
			fRec3[1] = fRec3[0];
			fRec5[1] = fRec5[0];
			fRec13[1] = fRec13[0];
			fRec14[1] = fRec14[0];
			iRec15[1] = iRec15[0];
			fRec12[1] = fRec12[0];
			fRec11[1] = fRec11[0];
			fRec10[1] = fRec10[0];
			fRec9[1] = fRec9[0];
			fRec8[1] = fRec8[0];
			fRec7[1] = fRec7[0];
			fRec6[2] = fRec6[1];
			fRec6[1] = fRec6[0];
			fRec4[2] = fRec4[1];
			fRec4[1] = fRec4[0];
			fRec16[1] = fRec16[0];
		}
	}

};

#endif
