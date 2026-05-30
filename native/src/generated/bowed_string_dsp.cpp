/* ------------------------------------------------------------
name: "bowed_string"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
Compilation options: -lang cpp -i -fpga-mem-th 4 -ct 1 -cn Bowed_stringDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __Bowed_stringDSP_H__
#define  __Bowed_stringDSP_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS Bowed_stringDSP
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

static float Bowed_stringDSP_faustpower2_f(float value) {
	return value * value;
}

class Bowed_stringDSP : public dsp {
	
 private:
	
	FAUSTFLOAT fEntry0;
	float fVec0[2];
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
	int iRec16[2];
	float fConst9;
	float fConst10;
	float fConst11;
	float fConst12;
	float fConst13;
	float fConst14;
	int IOTA0;
	float fConst15;
	FAUSTFLOAT fEntry8;
	float fRec29[2];
	FAUSTFLOAT fEntry9;
	float fConst16;
	float fRec26[2];
	float fRec30[4];
	float fRec31[4096];
	float fConst17;
	float fVec1[2];
	float fVec2[4096];
	float fRec23[4096];
	float fRec25[2];
	float fRec22[4];
	float fRec20[3];
	float fConst18;
	int iRec12[2];
	float fRec8[4096];
	float fRec6[2];
	float fRec5[3];
	float fRec4[3];
	FAUSTFLOAT fEntry10;
	float fRec32[2];
	
 public:
	Bowed_stringDSP() {
	}
	
	Bowed_stringDSP(const Bowed_stringDSP&) = default;
	
	virtual ~Bowed_stringDSP() = default;
	
	Bowed_stringDSP& operator=(const Bowed_stringDSP&) = default;
	
	void metadata(Meta* m) { 
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "1.22.0");
		m->declare("compile_options", "-lang cpp -i -fpga-mem-th 4 -ct 1 -cn Bowed_stringDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("delays.lib/fdelay4:author", "Julius O. Smith III");
		m->declare("delays.lib/fdelayltv:author", "Julius O. Smith III");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.2.0");
		m->declare("envelopes.lib/adsr:author", "Yann Orlarey and Andrey Bundin");
		m->declare("envelopes.lib/author", "GRAME");
		m->declare("envelopes.lib/copyright", "GRAME");
		m->declare("envelopes.lib/license", "LGPL with exception");
		m->declare("envelopes.lib/name", "Faust Envelope Library");
		m->declare("envelopes.lib/version", "1.3.0");
		m->declare("filename", "bowed_string.dsp");
		m->declare("filters.lib/fir:author", "Julius O. Smith III");
		m->declare("filters.lib/fir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/fir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/iir:author", "Julius O. Smith III");
		m->declare("filters.lib/iir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/iir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass0_highpass1:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:author", "Julius O. Smith III");
		m->declare("filters.lib/lowpass:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/lowpass:license", "MIT-style STK-4.3 license");
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
		m->declare("name", "bowed_string");
		m->declare("physmodels.lib/name", "Faust Physical Models Library");
		m->declare("physmodels.lib/version", "1.2.0");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("routes.lib/name", "Faust Signal Routing Library");
		m->declare("routes.lib/version", "1.3.0");
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
		fConst4 = std::tan(9424.778f / fConst0);
		fConst5 = 2.0f * (1.0f - 1.0f / Bowed_stringDSP_faustpower2_f(fConst4));
		fConst6 = 1.0f / fConst4;
		fConst7 = (fConst6 + -1.4142135f) / fConst4 + 1.0f;
		fConst8 = 1.0f / ((fConst6 + 1.4142135f) / fConst4 + 1.0f);
		fConst9 = std::tan(1570.7964f / fConst0);
		fConst10 = 2.0f * (1.0f - 1.0f / Bowed_stringDSP_faustpower2_f(fConst9));
		fConst11 = 1.0f / fConst9;
		fConst12 = (fConst11 + -0.5f) / fConst9 + 1.0f;
		fConst13 = (fConst11 + 0.5f) / fConst9 + 1.0f;
		fConst14 = 1.0f / fConst13;
		fConst15 = 0.020588236f * fConst0;
		fConst16 = 0.00125f * fConst0;
		fConst17 = 0.00022058823f * fConst0;
		fConst18 = 1.0f / (fConst9 * fConst13);
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
			fVec0[l0] = 0.0f;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			iRec0[l1] = 0;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec1[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec2[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fRec3[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			iRec16[l5] = 0;
		}
		IOTA0 = 0;
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fRec29[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			fRec26[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 4; l8 = l8 + 1) {
			fRec30[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 4096; l9 = l9 + 1) {
			fRec31[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			fVec1[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 4096; l11 = l11 + 1) {
			fVec2[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 4096; l12 = l12 + 1) {
			fRec23[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 2; l13 = l13 + 1) {
			fRec25[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 4; l14 = l14 + 1) {
			fRec22[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 3; l15 = l15 + 1) {
			fRec20[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 2; l16 = l16 + 1) {
			iRec12[l16] = 0;
		}
		for (int l17 = 0; l17 < 4096; l17 = l17 + 1) {
			fRec8[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec6[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 3; l19 = l19 + 1) {
			fRec5[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 3; l20 = l20 + 1) {
			fRec4[l20] = 0.0f;
		}
		for (int l21 = 0; l21 < 2; l21 = l21 + 1) {
			fRec32[l21] = 0.0f;
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
	
	virtual Bowed_stringDSP* clone() {
		return new Bowed_stringDSP(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("bowed_string");
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
		float fSlow13 = 3.4e+02f / std::pow(2.0f, 0.00083333335f * static_cast<float>(fEntry9));
		int iSlow14 = iSlow7 >= 3;
		float fSlow15 = fConst1 * static_cast<float>(fEntry10);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			fVec0[0] = fSlow0;
			iRec0[0] = iSlow1 * (iRec0[1] + 1);
			fRec1[0] = fSlow0 + fRec1[1] * static_cast<float>(fVec0[1] >= fSlow0);
			fRec2[0] = fSlow10 + fConst2 * fRec2[1];
			float fTemp0 = std::tan(fConst3 * fRec2[0]);
			fRec3[0] = fSlow11 + fConst2 * fRec3[1];
			float fTemp1 = 1.0f / (19.293f * fRec3[0] + 0.707f);
			float fTemp2 = 1.0f / fTemp0;
			float fTemp3 = (fTemp2 + fTemp1) / fTemp0 + 1.0f;
			iRec16[0] = 0;
			int iRec17 = iRec16[1];
			float fRec21 = static_cast<float>(iRec12[1]) - 0.98926467f * (0.6f * fRec22[2] + 0.2f * (fRec22[1] + fRec22[3]));
			fRec29[0] = fSlow12 + fConst2 * fRec29[1];
			float fTemp4 = fSlow13 / fRec29[0] + -0.08f;
			float fTemp5 = fConst16 * fTemp4;
			float fTemp6 = fTemp5 + -1.499995f;
			int iTemp7 = static_cast<int>(fTemp6);
			int iTemp8 = static_cast<int>(std::min<float>(fConst15, static_cast<float>(std::max<int>(0, iTemp7 + 4))));
			float fTemp9 = std::floor(fTemp6);
			float fTemp10 = fTemp5 + (-3.0f - fTemp9);
			float fTemp11 = fTemp5 + (-2.0f - fTemp9);
			float fTemp12 = fTemp5 + (-1.0f - fTemp9);
			float fTemp13 = fTemp5 - fTemp9;
			float fTemp14 = fTemp13 * fTemp12;
			float fTemp15 = fTemp14 * fTemp11;
			float fTemp16 = fTemp15 * fTemp10;
			int iTemp17 = static_cast<int>(std::min<float>(fConst15, static_cast<float>(std::max<int>(0, iTemp7 + 3))));
			int iTemp18 = static_cast<int>(std::min<float>(fConst15, static_cast<float>(std::max<int>(0, iTemp7 + 2))));
			int iTemp19 = static_cast<int>(std::min<float>(fConst15, static_cast<float>(std::max<int>(0, iTemp7 + 1))));
			int iTemp20 = static_cast<int>(std::min<float>(fConst15, static_cast<float>(std::max<int>(0, iTemp7))));
			float fTemp21 = fTemp5 + (-4.0f - fTemp9);
			fRec26[0] = fTemp21 * (fTemp10 * (fTemp11 * (0.041666668f * fRec8[(IOTA0 - (iTemp20 + 1)) & 4095] * fTemp12 - 0.16666667f * fTemp13 * fRec8[(IOTA0 - (iTemp19 + 1)) & 4095]) + 0.25f * fTemp14 * fRec8[(IOTA0 - (iTemp18 + 1)) & 4095]) - 0.16666667f * fTemp15 * fRec8[(IOTA0 - (iTemp17 + 1)) & 4095]) + 0.041666668f * fTemp16 * fRec8[(IOTA0 - (iTemp8 + 1)) & 4095];
			fRec30[0] = fRec6[1];
			fRec31[IOTA0 & 4095] = -(0.99880147f * (0.8f * fRec30[2] + 0.1f * (fRec30[1] + fRec30[3])));
			float fTemp22 = fConst17 * fTemp4;
			float fTemp23 = fTemp22 + -1.499995f;
			int iTemp24 = static_cast<int>(fTemp23);
			int iTemp25 = static_cast<int>(std::min<float>(fConst15, static_cast<float>(std::max<int>(0, iTemp24 + 4))));
			float fTemp26 = std::floor(fTemp23);
			float fTemp27 = fTemp22 + (-3.0f - fTemp26);
			float fTemp28 = fTemp22 + (-2.0f - fTemp26);
			float fTemp29 = fTemp22 + (-1.0f - fTemp26);
			float fTemp30 = fTemp22 - fTemp26;
			float fTemp31 = fTemp30 * fTemp29;
			float fTemp32 = fTemp31 * fTemp28;
			float fTemp33 = fTemp32 * fTemp27;
			int iTemp34 = static_cast<int>(std::min<float>(fConst15, static_cast<float>(std::max<int>(0, iTemp24 + 3))));
			int iTemp35 = static_cast<int>(std::min<float>(fConst15, static_cast<float>(std::max<int>(0, iTemp24 + 2))));
			int iTemp36 = static_cast<int>(std::min<float>(fConst15, static_cast<float>(std::max<int>(0, iTemp24 + 1))));
			int iTemp37 = static_cast<int>(std::min<float>(fConst15, static_cast<float>(std::max<int>(0, iTemp24))));
			float fTemp38 = fTemp22 + (-4.0f - fTemp26);
			fVec1[0] = fTemp38 * (fTemp27 * (fTemp28 * (0.041666668f * fRec31[(IOTA0 - (iTemp37 + 2)) & 4095] * fTemp29 - 0.16666667f * fTemp30 * fRec31[(IOTA0 - (iTemp36 + 2)) & 4095]) + 0.25f * fTemp31 * fRec31[(IOTA0 - (iTemp35 + 2)) & 4095]) - 0.16666667f * fTemp32 * fRec31[(IOTA0 - (iTemp34 + 2)) & 4095]) + 0.041666668f * fTemp33 * fRec31[(IOTA0 - (iTemp25 + 2)) & 4095];
			float fTemp39 = 0.2f - (fRec26[1] + fVec1[1]);
			float fTemp40 = fTemp39 * std::min<float>(1.0f, std::pow(std::fabs(3.0f * fTemp39) + 0.75f, -4.0f));
			float fRec27 = fRec26[1] + fTemp40;
			float fTemp41 = fVec1[1] + fTemp40;
			fVec2[IOTA0 & 4095] = fTemp41;
			float fRec28 = fTemp21 * (fTemp10 * (fTemp11 * (0.041666668f * fTemp12 * fVec2[(IOTA0 - iTemp20) & 4095] - 0.16666667f * fTemp13 * fVec2[(IOTA0 - iTemp19) & 4095]) + 0.25f * fTemp14 * fVec2[(IOTA0 - iTemp18) & 4095]) - 0.16666667f * fTemp15 * fVec2[(IOTA0 - iTemp17) & 4095]) + 0.041666668f * fTemp16 * fVec2[(IOTA0 - iTemp8) & 4095];
			fRec23[IOTA0 & 4095] = fRec27;
			float fRec24 = fTemp38 * (fTemp27 * (fTemp28 * (0.041666668f * fTemp29 * fRec23[(IOTA0 - (iTemp37 + 1)) & 4095] - 0.16666667f * fTemp30 * fRec23[(IOTA0 - (iTemp36 + 1)) & 4095]) + 0.25f * fTemp31 * fRec23[(IOTA0 - (iTemp35 + 1)) & 4095]) - 0.16666667f * fTemp32 * fRec23[(IOTA0 - (iTemp34 + 1)) & 4095]) + 0.041666668f * fTemp33 * fRec23[(IOTA0 - (iTemp25 + 1)) & 4095];
			fRec25[0] = fRec28;
			fRec22[0] = fRec25[1];
			fRec20[0] = fRec22[1] - fConst14 * (fConst12 * fRec20[2] + fConst10 * fRec20[1]);
			float fTemp42 = fConst18 * (fRec20[0] - fRec20[2]);
			float fRec18 = fTemp42;
			float fRec19 = fTemp42;
			iRec12[0] = iRec17;
			float fRec13 = fRec21;
			float fRec14 = fRec18;
			float fRec15 = fRec19;
			fRec8[IOTA0 & 4095] = fRec13;
			float fRec9 = fRec24;
			float fRec10 = fRec14;
			float fRec11 = fRec15;
			fRec6[0] = fRec9;
			float fRec7 = fRec11;
			fRec5[0] = fRec7 - fConst8 * (fConst7 * fRec5[2] + fConst5 * fRec5[1]);
			float fTemp43 = fConst8 * (fRec5[2] + fRec5[0] + 2.0f * fRec5[1]);
			fRec4[0] = fTemp43 - (fRec4[2] * ((fTemp2 - fTemp1) / fTemp0 + 1.0f) + 2.0f * fRec4[1] * (1.0f - 1.0f / Bowed_stringDSP_faustpower2_f(fTemp0))) / fTemp3;
			float fTemp44 = (fRec4[2] + fRec4[0] + 2.0f * fRec4[1]) / fTemp3;
			float fTemp45 = (fRec4[0] - fRec4[2]) / (fTemp0 * fTemp3);
			fRec32[0] = fSlow15 + fConst2 * fRec32[1];
			output0[i0] = static_cast<FAUSTFLOAT>(fRec32[0] * ((iSlow8) ? ((iSlow14) ? fTemp43 - fTemp45 : fTemp45) : ((iSlow9) ? fTemp43 - fTemp44 : fTemp44)) * std::max<float>(0.0f, std::min<float>(fSlow4 * fRec1[0], std::max<float>(fSlow6 * (fSlow3 - fRec1[0]) + 1.0f, fSlow5)) * (1.0f - fSlow2 * static_cast<float>(iRec0[0]))));
			fVec0[1] = fVec0[0];
			iRec0[1] = iRec0[0];
			fRec1[1] = fRec1[0];
			fRec2[1] = fRec2[0];
			fRec3[1] = fRec3[0];
			iRec16[1] = iRec16[0];
			IOTA0 = IOTA0 + 1;
			fRec29[1] = fRec29[0];
			fRec26[1] = fRec26[0];
			for (int j0 = 3; j0 > 0; j0 = j0 - 1) {
				fRec30[j0] = fRec30[j0 - 1];
			}
			fVec1[1] = fVec1[0];
			fRec25[1] = fRec25[0];
			for (int j1 = 3; j1 > 0; j1 = j1 - 1) {
				fRec22[j1] = fRec22[j1 - 1];
			}
			fRec20[2] = fRec20[1];
			fRec20[1] = fRec20[0];
			iRec12[1] = iRec12[0];
			fRec6[1] = fRec6[0];
			fRec5[2] = fRec5[1];
			fRec5[1] = fRec5[0];
			fRec4[2] = fRec4[1];
			fRec4[1] = fRec4[0];
			fRec32[1] = fRec32[0];
		}
	}

};

#endif
