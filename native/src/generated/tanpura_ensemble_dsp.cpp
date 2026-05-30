/* ------------------------------------------------------------
name: "tanpura_ensemble"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
Compilation options: -lang cpp -i -fpga-mem-th 4 -ct 1 -cn Tanpura_ensembleDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __Tanpura_ensembleDSP_H__
#define  __Tanpura_ensembleDSP_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS Tanpura_ensembleDSP
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

static float Tanpura_ensembleDSP_faustpower2_f(float value) {
	return value * value;
}

class Tanpura_ensembleDSP : public dsp {
	
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
	float fConst4;
	float fConst5;
	float fConst6;
	float fConst7;
	float fConst8;
	float fConst9;
	float fConst10;
	float fConst11;
	float fConst12;
	float fConst13;
	int iRec9[2];
	float fRec8[3];
	float fConst14;
	float fConst15;
	float fRec11[2];
	float fConst16;
	float fRec10[2];
	FAUSTFLOAT fEntry8;
	float fRec13[2];
	FAUSTFLOAT fEntry9;
	int IOTA0;
	float fConst17;
	float fRec12[2];
	float fRec17[2];
	float fRec16[2];
	float fRec15[2];
	float fRec14[2];
	float fRec7[16384];
	float fRec19[2];
	float fRec20[2];
	float fRec24[2];
	float fRec23[2];
	float fRec22[2];
	float fRec21[2];
	float fRec18[16384];
	float fRec26[2];
	float fRec27[2];
	float fRec31[2];
	float fRec30[2];
	float fRec29[2];
	float fRec28[2];
	float fRec25[16384];
	float fRec33[2];
	FAUSTFLOAT fEntry10;
	float fRec35[2];
	float fRec34[2];
	float fRec39[2];
	float fRec38[2];
	float fRec37[2];
	float fRec36[2];
	float fRec32[16384];
	float fVec2[2];
	float fRec6[2];
	float fRec5[3];
	float fRec4[3];
	FAUSTFLOAT fEntry11;
	float fRec40[2];
	
 public:
	Tanpura_ensembleDSP() {
	}
	
	Tanpura_ensembleDSP(const Tanpura_ensembleDSP&) = default;
	
	virtual ~Tanpura_ensembleDSP() = default;
	
	Tanpura_ensembleDSP& operator=(const Tanpura_ensembleDSP&) = default;
	
	void metadata(Meta* m) { 
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "1.22.0");
		m->declare("compile_options", "-lang cpp -i -fpga-mem-th 4 -ct 1 -cn Tanpura_ensembleDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.2.0");
		m->declare("envelopes.lib/adsr:author", "Yann Orlarey and Andrey Bundin");
		m->declare("envelopes.lib/author", "GRAME");
		m->declare("envelopes.lib/copyright", "GRAME");
		m->declare("envelopes.lib/license", "LGPL with exception");
		m->declare("envelopes.lib/name", "Faust Envelope Library");
		m->declare("envelopes.lib/version", "1.3.0");
		m->declare("filename", "tanpura_ensemble.dsp");
		m->declare("filters.lib/allpassnn:author", "Julius O. Smith III");
		m->declare("filters.lib/allpassnn:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/allpassnn:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/dcblocker:author", "Julius O. Smith III");
		m->declare("filters.lib/dcblocker:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/dcblocker:license", "MIT-style STK-4.3 license");
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
		m->declare("filters.lib/pole:author", "Julius O. Smith III");
		m->declare("filters.lib/pole:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/pole:license", "MIT-style STK-4.3 license");
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
		m->declare("filters.lib/zero:author", "Julius O. Smith III");
		m->declare("filters.lib/zero:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/zero:license", "MIT-style STK-4.3 license");
		m->declare("instruments.lib/author", "Romain Michon (rmichon@ccrma.stanford.edu)");
		m->declare("instruments.lib/copyright", "Romain Michon");
		m->declare("instruments.lib/licence", "STK-4.3");
		m->declare("instruments.lib/name", "Faust-STK Tools Library");
		m->declare("instruments.lib/version", "1.0.0");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.9.0");
		m->declare("name", "tanpura_ensemble");
		m->declare("noises.lib/name", "Faust Noise Generator Library");
		m->declare("noises.lib/version", "1.5.0");
		m->declare("oscillators.lib/lf_sawpos:author", "Bart Brouns, revised by Stéphane Letz");
		m->declare("oscillators.lib/lf_sawpos:licence", "STK-4.3");
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
		fConst4 = std::tan(8796.459f / fConst0);
		fConst5 = 2.0f * (1.0f - 1.0f / Tanpura_ensembleDSP_faustpower2_f(fConst4));
		fConst6 = 1.0f / fConst4;
		fConst7 = (fConst6 + -1.4142135f) / fConst4 + 1.0f;
		fConst8 = 1.0f / ((fConst6 + 1.4142135f) / fConst4 + 1.0f);
		fConst9 = std::tan(3141.5928f / fConst0);
		fConst10 = 2.0f * (1.0f - 1.0f / Tanpura_ensembleDSP_faustpower2_f(fConst9));
		fConst11 = 1.0f / fConst9;
		fConst12 = (fConst11 + -1.4142135f) / fConst9 + 1.0f;
		fConst13 = 1.0f / ((fConst11 + 1.4142135f) / fConst9 + 1.0f);
		fConst14 = std::exp(-(28.571428f / fConst0));
		fConst15 = 0.208f / fConst0;
		fConst16 = 1.0f - fConst14;
		fConst17 = 2.0f * fConst0;
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
		fEntry8 = static_cast<FAUSTFLOAT>(2.2e+02f);
		fEntry9 = static_cast<FAUSTFLOAT>(0.0f);
		fEntry10 = static_cast<FAUSTFLOAT>(1.5f);
		fEntry11 = static_cast<FAUSTFLOAT>(0.0f);
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
			iRec9[l6] = 0;
		}
		for (int l7 = 0; l7 < 3; l7 = l7 + 1) {
			fRec8[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
			fRec11[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
			fRec10[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			fRec13[l10] = 0.0f;
		}
		IOTA0 = 0;
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fRec12[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			fRec17[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 2; l13 = l13 + 1) {
			fRec16[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
			fRec15[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fRec14[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 16384; l16 = l16 + 1) {
			fRec7[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2; l17 = l17 + 1) {
			fRec19[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec20[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 2; l19 = l19 + 1) {
			fRec24[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 2; l20 = l20 + 1) {
			fRec23[l20] = 0.0f;
		}
		for (int l21 = 0; l21 < 2; l21 = l21 + 1) {
			fRec22[l21] = 0.0f;
		}
		for (int l22 = 0; l22 < 2; l22 = l22 + 1) {
			fRec21[l22] = 0.0f;
		}
		for (int l23 = 0; l23 < 16384; l23 = l23 + 1) {
			fRec18[l23] = 0.0f;
		}
		for (int l24 = 0; l24 < 2; l24 = l24 + 1) {
			fRec26[l24] = 0.0f;
		}
		for (int l25 = 0; l25 < 2; l25 = l25 + 1) {
			fRec27[l25] = 0.0f;
		}
		for (int l26 = 0; l26 < 2; l26 = l26 + 1) {
			fRec31[l26] = 0.0f;
		}
		for (int l27 = 0; l27 < 2; l27 = l27 + 1) {
			fRec30[l27] = 0.0f;
		}
		for (int l28 = 0; l28 < 2; l28 = l28 + 1) {
			fRec29[l28] = 0.0f;
		}
		for (int l29 = 0; l29 < 2; l29 = l29 + 1) {
			fRec28[l29] = 0.0f;
		}
		for (int l30 = 0; l30 < 16384; l30 = l30 + 1) {
			fRec25[l30] = 0.0f;
		}
		for (int l31 = 0; l31 < 2; l31 = l31 + 1) {
			fRec33[l31] = 0.0f;
		}
		for (int l32 = 0; l32 < 2; l32 = l32 + 1) {
			fRec35[l32] = 0.0f;
		}
		for (int l33 = 0; l33 < 2; l33 = l33 + 1) {
			fRec34[l33] = 0.0f;
		}
		for (int l34 = 0; l34 < 2; l34 = l34 + 1) {
			fRec39[l34] = 0.0f;
		}
		for (int l35 = 0; l35 < 2; l35 = l35 + 1) {
			fRec38[l35] = 0.0f;
		}
		for (int l36 = 0; l36 < 2; l36 = l36 + 1) {
			fRec37[l36] = 0.0f;
		}
		for (int l37 = 0; l37 < 2; l37 = l37 + 1) {
			fRec36[l37] = 0.0f;
		}
		for (int l38 = 0; l38 < 16384; l38 = l38 + 1) {
			fRec32[l38] = 0.0f;
		}
		for (int l39 = 0; l39 < 2; l39 = l39 + 1) {
			fVec2[l39] = 0.0f;
		}
		for (int l40 = 0; l40 < 2; l40 = l40 + 1) {
			fRec6[l40] = 0.0f;
		}
		for (int l41 = 0; l41 < 3; l41 = l41 + 1) {
			fRec5[l41] = 0.0f;
		}
		for (int l42 = 0; l42 < 3; l42 = l42 + 1) {
			fRec4[l42] = 0.0f;
		}
		for (int l43 = 0; l43 < 2; l43 = l43 + 1) {
			fRec40[l43] = 0.0f;
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
	
	virtual Tanpura_ensembleDSP* clone() {
		return new Tanpura_ensembleDSP(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("tanpura_ensemble");
		ui_interface->addNumEntry("amp", &fEntry11, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.001f));
		ui_interface->addNumEntry("attack", &fEntry2, FAUSTFLOAT(0.05f), FAUSTFLOAT(0.001f), FAUSTFLOAT(5.0f), FAUSTFLOAT(0.001f));
		ui_interface->addNumEntry("decay", &fEntry3, FAUSTFLOAT(0.3f), FAUSTFLOAT(0.001f), FAUSTFLOAT(5.0f), FAUSTFLOAT(0.001f));
		ui_interface->addNumEntry("detune", &fEntry9, FAUSTFLOAT(0.0f), FAUSTFLOAT(-1e+02f), FAUSTFLOAT(1e+02f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("filter_cutoff", &fEntry6, FAUSTFLOAT(2e+04f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(2e+04f), FAUSTFLOAT(1.0f));
		ui_interface->addNumEntry("filter_res", &fEntry7, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("filter_type", &fEntry5, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(3.0f), FAUSTFLOAT(1.0f));
		ui_interface->addNumEntry("freq", &fEntry8, FAUSTFLOAT(2.2e+02f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(2e+04f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("gate", &fEntry0, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(1.0f));
		ui_interface->addNumEntry("release", &fEntry1, FAUSTFLOAT(2.0f), FAUSTFLOAT(0.01f), FAUSTFLOAT(3e+01f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("sustain", &fEntry4, FAUSTFLOAT(0.8f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.001f));
		ui_interface->addNumEntry("tone1_ratio", &fEntry10, FAUSTFLOAT(1.5f), FAUSTFLOAT(0.25f), FAUSTFLOAT(4.0f), FAUSTFLOAT(0.001f));
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
		float fSlow12 = fConst13 * fSlow0;
		float fSlow13 = fConst1 * static_cast<float>(fEntry8);
		float fSlow14 = std::pow(2.0f, 0.00083333335f * static_cast<float>(fEntry9));
		float fSlow15 = 1.5e-07f * fSlow14;
		float fSlow16 = fConst17 / fSlow14;
		float fSlow17 = fConst0 / fSlow14;
		float fSlow18 = 1.0995574f * fSlow0;
		float fSlow19 = 3e-07f * fSlow14;
		float fSlow20 = fConst1 * static_cast<float>(fEntry10);
		int iSlow21 = iSlow7 >= 3;
		float fSlow22 = fConst1 * static_cast<float>(fEntry11);
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
			iRec9[0] = 1103515245 * iRec9[1] + 12345;
			fRec8[0] = 4.656613e-10f * static_cast<float>(iRec9[0]) - fConst13 * (fConst12 * fRec8[2] + fConst10 * fRec8[1]);
			float fTemp4 = fRec8[2] + fRec8[0] + 2.0f * fRec8[1];
			float fTemp5 = ((1 - iVec0[1]) ? 0.0f : fConst15 + fRec11[1]);
			fRec11[0] = fTemp5 - std::floor(fTemp5);
			float fTemp6 = fRec11[0] + (0.75f - std::floor(fRec11[0] + 0.75f));
			fRec10[0] = fConst16 * std::exp(-(12.0f * fTemp6)) + fConst14 * fRec10[1];
			fRec13[0] = fSlow13 + fConst2 * fRec13[1];
			float fTemp7 = fSlow16 / fRec13[0];
			float fTemp8 = fTemp7 + -1.0f;
			int iTemp9 = static_cast<int>(fTemp8);
			float fTemp10 = std::floor(fTemp8);
			float fTemp11 = fSlow17 / fRec13[0];
			fRec12[0] = 0.65f * (fRec7[(IOTA0 - (std::min<int>(8193, std::max<int>(0, iTemp9)) + 1)) & 16383] * (fTemp10 + 2.0f * (1.0f - fTemp11)) + (fTemp7 + (-1.0f - fTemp10)) * fRec7[(IOTA0 - (std::min<int>(8193, std::max<int>(0, iTemp9 + 1)) + 1)) & 16383]) * std::min<float>(0.9996f, fSlow15 * fRec13[0] + 0.9994f) + 0.35f * fRec12[1];
			float fTemp12 = fSlow18 * fRec12[0] * std::exp(-(1.664f * fTemp6));
			float fTemp13 = std::cos(fTemp12);
			float fTemp14 = std::sin(fTemp12);
			float fTemp15 = fRec12[0] * fTemp13 - fTemp14 * fRec14[1];
			float fTemp16 = fTemp13 * fTemp15 - fTemp14 * fRec15[1];
			float fTemp17 = fTemp13 * fTemp16 - fTemp14 * fRec16[1];
			fRec17[0] = fTemp13 * fTemp17 - fTemp14 * fRec17[1];
			fRec16[0] = fTemp14 * fTemp17 + fTemp13 * fRec17[1];
			fRec15[0] = fTemp14 * fTemp16 + fTemp13 * fRec16[1];
			fRec14[0] = fTemp14 * fTemp15 + fTemp13 * fRec15[1];
			fRec7[IOTA0 & 16383] = 0.65f * fRec12[0] + 0.35f * (fRec12[0] * fTemp14 + fRec14[1] * fTemp13 + fSlow12 * fRec10[0] * fTemp4);
			float fTemp18 = fRec11[0] + (0.5f - std::floor(fRec11[0] + 0.5f));
			fRec19[0] = fConst16 * std::exp(-(12.0f * fTemp18)) + fConst14 * fRec19[1];
			float fTemp19 = fTemp11 + -1.0f;
			int iTemp20 = static_cast<int>(fTemp19);
			int iTemp21 = std::min<int>(8193, std::max<int>(0, iTemp20 + 1)) + 1;
			float fTemp22 = std::floor(fTemp19);
			float fTemp23 = fTemp11 + (-1.0f - fTemp22);
			int iTemp24 = std::min<int>(8193, std::max<int>(0, iTemp20)) + 1;
			float fTemp25 = fTemp22 + (2.0f - fTemp11);
			float fTemp26 = std::min<float>(0.9996f, fSlow19 * fRec13[0] + 0.9994f);
			fRec20[0] = 0.65f * fTemp26 * (fTemp25 * fRec18[(IOTA0 - iTemp24) & 16383] + fTemp23 * fRec18[(IOTA0 - iTemp21) & 16383]) + 0.35f * fRec20[1];
			float fTemp27 = fSlow18 * fRec20[0] * std::exp(-(1.664f * fTemp18));
			float fTemp28 = std::cos(fTemp27);
			float fTemp29 = std::sin(fTemp27);
			float fTemp30 = fRec20[0] * fTemp28 - fTemp29 * fRec21[1];
			float fTemp31 = fTemp28 * fTemp30 - fTemp29 * fRec22[1];
			float fTemp32 = fTemp28 * fTemp31 - fTemp29 * fRec23[1];
			fRec24[0] = fTemp28 * fTemp32 - fTemp29 * fRec24[1];
			fRec23[0] = fTemp29 * fTemp32 + fTemp28 * fRec24[1];
			fRec22[0] = fTemp29 * fTemp31 + fTemp28 * fRec23[1];
			fRec21[0] = fTemp29 * fTemp30 + fTemp28 * fRec22[1];
			fRec18[IOTA0 & 16383] = 0.65f * fRec20[0] + 0.35f * (fRec20[0] * fTemp29 + fRec21[1] * fTemp28 + fSlow12 * fRec19[0] * fTemp4);
			float fTemp33 = fRec11[0] + (0.25f - std::floor(fRec11[0] + 0.25f));
			fRec26[0] = fConst16 * std::exp(-(12.0f * fTemp33)) + fConst14 * fRec26[1];
			fRec27[0] = 0.65f * (fRec25[(IOTA0 - iTemp24) & 16383] * fTemp25 + fTemp23 * fRec25[(IOTA0 - iTemp21) & 16383]) * fTemp26 + 0.35f * fRec27[1];
			float fTemp34 = fSlow18 * fRec27[0] * std::exp(-(1.664f * fTemp33));
			float fTemp35 = std::cos(fTemp34);
			float fTemp36 = std::sin(fTemp34);
			float fTemp37 = fRec27[0] * fTemp35 - fTemp36 * fRec28[1];
			float fTemp38 = fTemp35 * fTemp37 - fTemp36 * fRec29[1];
			float fTemp39 = fTemp35 * fTemp38 - fTemp36 * fRec30[1];
			fRec31[0] = fTemp35 * fTemp39 - fTemp36 * fRec31[1];
			fRec30[0] = fTemp36 * fTemp39 + fTemp35 * fRec31[1];
			fRec29[0] = fTemp36 * fTemp38 + fTemp35 * fRec30[1];
			fRec28[0] = fTemp36 * fTemp37 + fTemp35 * fRec29[1];
			fRec25[IOTA0 & 16383] = 0.65f * fRec27[0] + 0.35f * (fRec27[0] * fTemp36 + fRec28[1] * fTemp35 + fSlow12 * fRec26[0] * fTemp4);
			float fTemp40 = fRec11[0] - std::floor(fRec11[0]);
			fRec33[0] = fConst16 * std::exp(-(12.0f * fTemp40)) + fConst14 * fRec33[1];
			fRec35[0] = fSlow20 + fConst2 * fRec35[1];
			float fTemp41 = fRec13[0] * fRec35[0];
			float fTemp42 = fSlow17 / fTemp41;
			float fTemp43 = fTemp42 + -1.0f;
			int iTemp44 = static_cast<int>(fTemp43);
			float fTemp45 = std::floor(fTemp43);
			fRec34[0] = 0.65f * (fRec32[(IOTA0 - (std::min<int>(8193, std::max<int>(0, iTemp44)) + 1)) & 16383] * (fTemp45 + (2.0f - fTemp42)) + (fTemp42 + (-1.0f - fTemp45)) * fRec32[(IOTA0 - (std::min<int>(8193, std::max<int>(0, iTemp44 + 1)) + 1)) & 16383]) * std::min<float>(0.9996f, fSlow19 * fTemp41 + 0.9994f) + 0.35f * fRec34[1];
			float fTemp46 = fSlow18 * fRec34[0] * std::exp(-(1.664f * fTemp40));
			float fTemp47 = std::cos(fTemp46);
			float fTemp48 = std::sin(fTemp46);
			float fTemp49 = fRec34[0] * fTemp47 - fTemp48 * fRec36[1];
			float fTemp50 = fTemp47 * fTemp49 - fTemp48 * fRec37[1];
			float fTemp51 = fTemp47 * fTemp50 - fTemp48 * fRec38[1];
			fRec39[0] = fTemp47 * fTemp51 - fTemp48 * fRec39[1];
			fRec38[0] = fTemp48 * fTemp51 + fTemp47 * fRec39[1];
			fRec37[0] = fTemp48 * fTemp50 + fTemp47 * fRec38[1];
			fRec36[0] = fTemp48 * fTemp49 + fTemp47 * fRec37[1];
			fRec32[IOTA0 & 16383] = 0.65f * fRec34[0] + 0.35f * (fRec34[0] * fTemp48 + fRec36[1] * fTemp47 + fSlow12 * fRec33[0] * fTemp4);
			float fTemp52 = fRec32[IOTA0 & 16383] + fRec25[IOTA0 & 16383] + fRec18[IOTA0 & 16383] + fRec7[IOTA0 & 16383];
			fVec2[0] = fTemp52;
			fRec6[0] = 0.995f * fRec6[1] + 0.5f * (fTemp52 - fVec2[1]);
			fRec5[0] = fRec6[0] - fConst8 * (fConst7 * fRec5[2] + fConst5 * fRec5[1]);
			float fTemp53 = fConst8 * (fRec5[2] + fRec5[0] + 2.0f * fRec5[1]);
			fRec4[0] = fTemp53 - (fRec4[2] * ((fTemp2 - fTemp1) / fTemp0 + 1.0f) + 2.0f * fRec4[1] * (1.0f - 1.0f / Tanpura_ensembleDSP_faustpower2_f(fTemp0))) / fTemp3;
			float fTemp54 = (fRec4[2] + fRec4[0] + 2.0f * fRec4[1]) / fTemp3;
			float fTemp55 = (fRec4[0] - fRec4[2]) / (fTemp0 * fTemp3);
			fRec40[0] = fSlow22 + fConst2 * fRec40[1];
			output0[i0] = static_cast<FAUSTFLOAT>(fRec40[0] * ((iSlow8) ? ((iSlow21) ? fTemp53 - fTemp55 : fTemp55) : ((iSlow9) ? fTemp53 - fTemp54 : fTemp54)) * std::max<float>(0.0f, std::min<float>(fSlow4 * fRec1[0], std::max<float>(fSlow6 * (fSlow3 - fRec1[0]) + 1.0f, fSlow5)) * (1.0f - fSlow2 * static_cast<float>(iRec0[0]))));
			iVec0[1] = iVec0[0];
			fVec1[1] = fVec1[0];
			iRec0[1] = iRec0[0];
			fRec1[1] = fRec1[0];
			fRec2[1] = fRec2[0];
			fRec3[1] = fRec3[0];
			iRec9[1] = iRec9[0];
			fRec8[2] = fRec8[1];
			fRec8[1] = fRec8[0];
			fRec11[1] = fRec11[0];
			fRec10[1] = fRec10[0];
			fRec13[1] = fRec13[0];
			IOTA0 = IOTA0 + 1;
			fRec12[1] = fRec12[0];
			fRec17[1] = fRec17[0];
			fRec16[1] = fRec16[0];
			fRec15[1] = fRec15[0];
			fRec14[1] = fRec14[0];
			fRec19[1] = fRec19[0];
			fRec20[1] = fRec20[0];
			fRec24[1] = fRec24[0];
			fRec23[1] = fRec23[0];
			fRec22[1] = fRec22[0];
			fRec21[1] = fRec21[0];
			fRec26[1] = fRec26[0];
			fRec27[1] = fRec27[0];
			fRec31[1] = fRec31[0];
			fRec30[1] = fRec30[0];
			fRec29[1] = fRec29[0];
			fRec28[1] = fRec28[0];
			fRec33[1] = fRec33[0];
			fRec35[1] = fRec35[0];
			fRec34[1] = fRec34[0];
			fRec39[1] = fRec39[0];
			fRec38[1] = fRec38[0];
			fRec37[1] = fRec37[0];
			fRec36[1] = fRec36[0];
			fVec2[1] = fVec2[0];
			fRec6[1] = fRec6[0];
			fRec5[2] = fRec5[1];
			fRec5[1] = fRec5[0];
			fRec4[2] = fRec4[1];
			fRec4[1] = fRec4[0];
			fRec40[1] = fRec40[0];
		}
	}

};

#endif
