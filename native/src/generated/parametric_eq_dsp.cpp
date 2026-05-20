/* ------------------------------------------------------------
name: "parametric_eq"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
Compilation options: -lang cpp -i -fpga-mem-th 4 -ct 1 -cn Parametric_eqDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __Parametric_eqDSP_H__
#define  __Parametric_eqDSP_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS Parametric_eqDSP
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

static float Parametric_eqDSP_faustpower2_f(float value) {
	return value * value;
}

class Parametric_eqDSP : public dsp {
	
 private:
	
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	float fConst3;
	float fConst4;
	float fConst5;
	float fConst6;
	FAUSTFLOAT fEntry0;
	FAUSTFLOAT fEntry1;
	float fConst7;
	FAUSTFLOAT fEntry2;
	float fConst8;
	float fConst9;
	float fConst10;
	float fConst11;
	float fConst12;
	float fConst13;
	float fConst14;
	float fVec0[2];
	float fConst15;
	float fConst16;
	float fRec4[2];
	float fRec3[3];
	FAUSTFLOAT fEntry3;
	float fRec6[2];
	float fRec5[3];
	float fRec2[3];
	float fVec1[2];
	float fConst17;
	float fConst18;
	float fRec1[2];
	float fRec0[3];
	FAUSTFLOAT fEntry4;
	float fRec8[2];
	float fRec7[3];
	float fVec2[2];
	float fRec13[2];
	float fRec12[3];
	float fRec15[2];
	float fRec14[3];
	float fRec11[3];
	float fVec3[2];
	float fRec10[2];
	float fRec9[3];
	float fRec17[2];
	float fRec16[3];
	
 public:
	Parametric_eqDSP() {
	}
	
	Parametric_eqDSP(const Parametric_eqDSP&) = default;
	
	virtual ~Parametric_eqDSP() = default;
	
	Parametric_eqDSP& operator=(const Parametric_eqDSP&) = default;
	
	void metadata(Meta* m) { 
		m->declare("analyzers.lib/name", "Faust Analyzer Library");
		m->declare("analyzers.lib/version", "1.3.0");
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "1.22.0");
		m->declare("compile_options", "-lang cpp -i -fpga-mem-th 4 -ct 1 -cn Parametric_eqDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("filename", "parametric_eq.dsp");
		m->declare("filters.lib/filterbank:author", "Julius O. Smith III");
		m->declare("filters.lib/filterbank:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/filterbank:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/fir:author", "Julius O. Smith III");
		m->declare("filters.lib/fir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/fir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/highpass:author", "Julius O. Smith III");
		m->declare("filters.lib/highpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/highshelf:author", "Julius O. Smith III");
		m->declare("filters.lib/highshelf:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/highshelf:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/iir:author", "Julius O. Smith III");
		m->declare("filters.lib/iir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/iir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/low_shelf:author", "Julius O. Smith III");
		m->declare("filters.lib/low_shelf:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/low_shelf:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass0_highpass1:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowshelf:author", "Julius O. Smith III");
		m->declare("filters.lib/lowshelf:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowshelf:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/peak_eq:author", "Julius O. Smith III");
		m->declare("filters.lib/peak_eq:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/peak_eq:license", "MIT-style STK-4.3 license");
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
		m->declare("name", "parametric_eq");
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
		fConst1 = std::tan(25132.742f / fConst0);
		fConst2 = 1.0f / Parametric_eqDSP_faustpower2_f(fConst1);
		fConst3 = 2.0f * (1.0f - fConst2);
		fConst4 = 1.0f / fConst1;
		fConst5 = (fConst4 + -1.0f) / fConst1 + 1.0f;
		fConst6 = 1.0f / ((fConst4 + 1.0f) / fConst1 + 1.0f);
		fConst7 = 6.2831855f / fConst0;
		fConst8 = 3.1415927f / fConst0;
		fConst9 = std::tan(628.31854f / fConst0);
		fConst10 = 1.0f / Parametric_eqDSP_faustpower2_f(fConst9);
		fConst11 = 2.0f * (1.0f - fConst10);
		fConst12 = 1.0f / fConst9;
		fConst13 = (fConst12 + -1.0f) / fConst9 + 1.0f;
		fConst14 = 1.0f / ((fConst12 + 1.0f) / fConst9 + 1.0f);
		fConst15 = 1.0f - fConst12;
		fConst16 = 1.0f / (fConst12 + 1.0f);
		fConst17 = 1.0f - fConst4;
		fConst18 = 1.0f / (fConst4 + 1.0f);
	}
	
	virtual void instanceResetUserInterface() {
		fEntry0 = static_cast<FAUSTFLOAT>(0.0f);
		fEntry1 = static_cast<FAUSTFLOAT>(1e+03f);
		fEntry2 = static_cast<FAUSTFLOAT>(1.0f);
		fEntry3 = static_cast<FAUSTFLOAT>(0.0f);
		fEntry4 = static_cast<FAUSTFLOAT>(0.0f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fVec0[l0] = 0.0f;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fRec4[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 3; l2 = l2 + 1) {
			fRec3[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec6[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 3; l4 = l4 + 1) {
			fRec5[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 3; l5 = l5 + 1) {
			fRec2[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fVec1[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			fRec1[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 3; l8 = l8 + 1) {
			fRec0[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
			fRec8[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 3; l10 = l10 + 1) {
			fRec7[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fVec2[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			fRec13[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 3; l13 = l13 + 1) {
			fRec12[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
			fRec15[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 3; l15 = l15 + 1) {
			fRec14[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 3; l16 = l16 + 1) {
			fRec11[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2; l17 = l17 + 1) {
			fVec3[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec10[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 3; l19 = l19 + 1) {
			fRec9[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 2; l20 = l20 + 1) {
			fRec17[l20] = 0.0f;
		}
		for (int l21 = 0; l21 < 3; l21 = l21 + 1) {
			fRec16[l21] = 0.0f;
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
	
	virtual Parametric_eqDSP* clone() {
		return new Parametric_eqDSP(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("parametric_eq");
		ui_interface->addNumEntry("high_gain", &fEntry4, FAUSTFLOAT(0.0f), FAUSTFLOAT(-12.0f), FAUSTFLOAT(12.0f), FAUSTFLOAT(0.1f));
		ui_interface->addNumEntry("low_gain", &fEntry3, FAUSTFLOAT(0.0f), FAUSTFLOAT(-12.0f), FAUSTFLOAT(12.0f), FAUSTFLOAT(0.1f));
		ui_interface->addNumEntry("mid_freq", &fEntry1, FAUSTFLOAT(1e+03f), FAUSTFLOAT(1e+02f), FAUSTFLOAT(1e+04f), FAUSTFLOAT(1.0f));
		ui_interface->addNumEntry("mid_gain", &fEntry0, FAUSTFLOAT(0.0f), FAUSTFLOAT(-12.0f), FAUSTFLOAT(12.0f), FAUSTFLOAT(0.1f));
		ui_interface->addNumEntry("mid_q", &fEntry2, FAUSTFLOAT(1.0f), FAUSTFLOAT(0.1f), FAUSTFLOAT(1e+01f), FAUSTFLOAT(0.1f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* input1 = inputs[1];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = static_cast<float>(fEntry0);
		int iSlow1 = fSlow0 > 0.0f;
		float fSlow2 = static_cast<float>(fEntry1);
		float fSlow3 = static_cast<float>(fEntry2) * std::sin(fConst7 * fSlow2);
		float fSlow4 = fConst8 * (fSlow2 * std::pow(1e+01f, 0.05f * std::fabs(fSlow0)) / fSlow3);
		float fSlow5 = fConst8 * (fSlow2 / fSlow3);
		float fSlow6 = ((iSlow1) ? fSlow5 : fSlow4);
		float fSlow7 = std::tan(fConst8 * fSlow2);
		float fSlow8 = 1.0f / fSlow7;
		float fSlow9 = fSlow8 * (fSlow8 + fSlow6) + 1.0f;
		float fSlow10 = ((iSlow1) ? fSlow4 : fSlow5);
		float fSlow11 = fSlow8 * (fSlow8 - fSlow10) + 1.0f;
		float fSlow12 = 2.0f * (1.0f - 1.0f / Parametric_eqDSP_faustpower2_f(fSlow7));
		float fSlow13 = fSlow8 * (fSlow8 - fSlow6) + 1.0f;
		float fSlow14 = std::pow(1e+01f, 0.05f * static_cast<float>(fEntry3));
		float fSlow15 = fSlow8 * (fSlow8 + fSlow10) + 1.0f;
		float fSlow16 = fConst2 * std::pow(1e+01f, 0.05f * static_cast<float>(fEntry4));
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			float fTemp0 = fSlow12 * fRec2[1];
			float fTemp1 = static_cast<float>(input0[i0]);
			fVec0[0] = fTemp1;
			fRec4[0] = -(fConst16 * (fConst15 * fRec4[1] - (fTemp1 + fVec0[1])));
			fRec3[0] = fRec4[0] - fConst14 * (fConst13 * fRec3[2] + fConst11 * fRec3[1]);
			fRec6[0] = -(fConst16 * (fConst15 * fRec6[1] - fConst12 * (fTemp1 - fVec0[1])));
			fRec5[0] = fRec6[0] - fConst14 * (fConst13 * fRec5[2] + fConst11 * fRec5[1]);
			fRec2[0] = fConst14 * (fConst10 * (fRec5[2] + (fRec5[0] - 2.0f * fRec5[1])) + fSlow14 * (fRec3[2] + fRec3[0] + 2.0f * fRec3[1])) - (fRec2[2] * fSlow13 + fTemp0) / fSlow9;
			float fTemp2 = (fTemp0 + fRec2[0] * fSlow15 + fRec2[2] * fSlow11) / fSlow9;
			fVec1[0] = fTemp2;
			fRec1[0] = -(fConst18 * (fConst17 * fRec1[1] - fConst4 * (fTemp2 - fVec1[1])));
			fRec0[0] = fRec1[0] - fConst6 * (fConst5 * fRec0[2] + fConst3 * fRec0[1]);
			fRec8[0] = -(fConst18 * (fConst17 * fRec8[1] - (fTemp2 + fVec1[1])));
			fRec7[0] = fRec8[0] - fConst6 * (fConst5 * fRec7[2] + fConst3 * fRec7[1]);
			output0[i0] = static_cast<FAUSTFLOAT>(fConst6 * (fRec7[2] + fRec7[0] + 2.0f * fRec7[1] + fSlow16 * (fRec0[2] + (fRec0[0] - 2.0f * fRec0[1]))));
			float fTemp3 = fSlow12 * fRec11[1];
			float fTemp4 = static_cast<float>(input1[i0]);
			fVec2[0] = fTemp4;
			fRec13[0] = -(fConst16 * (fConst15 * fRec13[1] - (fTemp4 + fVec2[1])));
			fRec12[0] = fRec13[0] - fConst14 * (fConst13 * fRec12[2] + fConst11 * fRec12[1]);
			fRec15[0] = -(fConst16 * (fConst15 * fRec15[1] - fConst12 * (fTemp4 - fVec2[1])));
			fRec14[0] = fRec15[0] - fConst14 * (fConst13 * fRec14[2] + fConst11 * fRec14[1]);
			fRec11[0] = fConst14 * (fConst10 * (fRec14[2] + (fRec14[0] - 2.0f * fRec14[1])) + fSlow14 * (fRec12[2] + fRec12[0] + 2.0f * fRec12[1])) - (fSlow13 * fRec11[2] + fTemp3) / fSlow9;
			float fTemp5 = (fTemp3 + fRec11[0] * fSlow15 + fSlow11 * fRec11[2]) / fSlow9;
			fVec3[0] = fTemp5;
			fRec10[0] = -(fConst18 * (fConst17 * fRec10[1] - fConst4 * (fTemp5 - fVec3[1])));
			fRec9[0] = fRec10[0] - fConst6 * (fConst5 * fRec9[2] + fConst3 * fRec9[1]);
			fRec17[0] = -(fConst18 * (fConst17 * fRec17[1] - (fTemp5 + fVec3[1])));
			fRec16[0] = fRec17[0] - fConst6 * (fConst5 * fRec16[2] + fConst3 * fRec16[1]);
			output1[i0] = static_cast<FAUSTFLOAT>(fConst6 * (fRec16[2] + fRec16[0] + 2.0f * fRec16[1] + fSlow16 * (fRec9[2] + (fRec9[0] - 2.0f * fRec9[1]))));
			fVec0[1] = fVec0[0];
			fRec4[1] = fRec4[0];
			fRec3[2] = fRec3[1];
			fRec3[1] = fRec3[0];
			fRec6[1] = fRec6[0];
			fRec5[2] = fRec5[1];
			fRec5[1] = fRec5[0];
			fRec2[2] = fRec2[1];
			fRec2[1] = fRec2[0];
			fVec1[1] = fVec1[0];
			fRec1[1] = fRec1[0];
			fRec0[2] = fRec0[1];
			fRec0[1] = fRec0[0];
			fRec8[1] = fRec8[0];
			fRec7[2] = fRec7[1];
			fRec7[1] = fRec7[0];
			fVec2[1] = fVec2[0];
			fRec13[1] = fRec13[0];
			fRec12[2] = fRec12[1];
			fRec12[1] = fRec12[0];
			fRec15[1] = fRec15[0];
			fRec14[2] = fRec14[1];
			fRec14[1] = fRec14[0];
			fRec11[2] = fRec11[1];
			fRec11[1] = fRec11[0];
			fVec3[1] = fVec3[0];
			fRec10[1] = fRec10[0];
			fRec9[2] = fRec9[1];
			fRec9[1] = fRec9[0];
			fRec17[1] = fRec17[0];
			fRec16[2] = fRec16[1];
			fRec16[1] = fRec16[0];
		}
	}

};

#endif
