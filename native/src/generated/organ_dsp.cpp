/* ------------------------------------------------------------
name: "organ"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
Compilation options: -lang cpp -i -fpga-mem-th 4 -ct 1 -cn OrganDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __OrganDSP_H__
#define  __OrganDSP_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS OrganDSP
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

class OrganDSPSIG0 {
	
  private:
	
	int iVec2[2];
	int iRec8[2];
	int fSampleRate;
	
  public:
	
	int getNumInputsOrganDSPSIG0() {
		return 0;
	}
	int getNumOutputsOrganDSPSIG0() {
		return 1;
	}
	
	void instanceInitOrganDSPSIG0(int sample_rate) {
		fSampleRate = sample_rate;
		for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
			iVec2[l9] = 0;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			iRec8[l10] = 0;
		}
	}
	
	void fillOrganDSPSIG0(int count, float* table) {
		for (int i1 = 0; i1 < count; i1 = i1 + 1) {
			iVec2[0] = 1;
			iRec8[0] = (iVec2[1] + iRec8[1]) % 65536;
			table[i1] = std::sin(9.58738e-05f * static_cast<float>(iRec8[0]));
			iVec2[1] = iVec2[0];
			iRec8[1] = iRec8[0];
		}
	}

};

static OrganDSPSIG0* newOrganDSPSIG0() { return (OrganDSPSIG0*)new OrganDSPSIG0(); }
static void deleteOrganDSPSIG0(OrganDSPSIG0* dsp) { delete dsp; }

static float OrganDSP_faustpower2_f(float value) {
	return value * value;
}
static float ftbl0OrganDSPSIG0[65536];

class OrganDSP : public dsp {
	
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
	int iRec6[2];
	float fRec5[3];
	float fConst9;
	float fConst10;
	float fConst11;
	float fConst12;
	float fConst13;
	float fConst14;
	float fRec7[3];
	float fConst15;
	float fConst16;
	float fRec9[2];
	float fConst17;
	float fRec11[2];
	FAUSTFLOAT fEntry8;
	float fRec12[2];
	FAUSTFLOAT fEntry9;
	float fConst18;
	float fRec10[2];
	float fConst19;
	float fRec13[2];
	float fConst20;
	float fRec15[2];
	float fConst21;
	float fRec14[2];
	float fConst22;
	float fRec16[2];
	float fConst23;
	float fRec18[2];
	float fConst24;
	float fRec17[2];
	float fConst25;
	float fRec19[2];
	float fConst26;
	float fRec21[2];
	float fConst27;
	float fRec20[2];
	float fConst28;
	float fRec22[2];
	float fConst29;
	float fRec24[2];
	float fConst30;
	float fRec23[2];
	float fConst31;
	float fRec26[2];
	float fConst32;
	float fRec25[2];
	float fConst33;
	float fRec27[2];
	float fConst34;
	float fRec29[2];
	float fConst35;
	float fRec28[2];
	float fConst36;
	float fRec30[2];
	float fConst37;
	float fRec32[2];
	float fConst38;
	float fRec31[2];
	float fRec4[3];
	FAUSTFLOAT fEntry10;
	float fRec33[2];
	
 public:
	OrganDSP() {
	}
	
	OrganDSP(const OrganDSP&) = default;
	
	virtual ~OrganDSP() = default;
	
	OrganDSP& operator=(const OrganDSP&) = default;
	
	void metadata(Meta* m) { 
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "1.22.0");
		m->declare("compile_options", "-lang cpp -i -fpga-mem-th 4 -ct 1 -cn OrganDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("envelopes.lib/adsr:author", "Yann Orlarey and Andrey Bundin");
		m->declare("envelopes.lib/author", "GRAME");
		m->declare("envelopes.lib/copyright", "GRAME");
		m->declare("envelopes.lib/license", "LGPL with exception");
		m->declare("envelopes.lib/name", "Faust Envelope Library");
		m->declare("envelopes.lib/version", "1.3.0");
		m->declare("filename", "organ.dsp");
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
		m->declare("name", "organ");
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
		OrganDSPSIG0* sig0 = newOrganDSPSIG0();
		sig0->instanceInitOrganDSPSIG0(sample_rate);
		sig0->fillOrganDSPSIG0(65536, ftbl0OrganDSPSIG0);
		deleteOrganDSPSIG0(sig0);
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, static_cast<float>(fSampleRate)));
		fConst1 = 44.1f / fConst0;
		fConst2 = 1.0f - fConst1;
		fConst3 = 3.1415927f / fConst0;
		fConst4 = std::tan(628.31854f / fConst0);
		fConst5 = 2.0f * (1.0f - 1.0f / OrganDSP_faustpower2_f(fConst4));
		fConst6 = 1.0f / fConst4;
		fConst7 = (fConst6 + -1.4142135f) / fConst4 + 1.0f;
		fConst8 = 1.0f / ((fConst6 + 1.4142135f) / fConst4 + 1.0f);
		fConst9 = std::tan(6911.504f / fConst0);
		fConst10 = 2.0f * (1.0f - 1.0f / OrganDSP_faustpower2_f(fConst9));
		fConst11 = 1.0f / fConst9;
		fConst12 = (fConst11 + -0.6666667f) / fConst9 + 1.0f;
		fConst13 = (fConst11 + 0.6666667f) / fConst9 + 1.0f;
		fConst14 = 1.0f / fConst13;
		fConst15 = 1.0f / (fConst9 * fConst13);
		fConst16 = 0.6f / fConst0;
		fConst17 = 0.28f / fConst0;
		fConst18 = 8.0f / fConst0;
		fConst19 = 0.53f / fConst0;
		fConst20 = 0.25f / fConst0;
		fConst21 = 6.0f / fConst0;
		fConst22 = 0.46f / fConst0;
		fConst23 = 0.22f / fConst0;
		fConst24 = 5.0f / fConst0;
		fConst25 = 0.39f / fConst0;
		fConst26 = 0.19f / fConst0;
		fConst27 = 4.0f / fConst0;
		fConst28 = 0.32f / fConst0;
		fConst29 = 0.16f / fConst0;
		fConst30 = 3.0f / fConst0;
		fConst31 = 0.13f / fConst0;
		fConst32 = 2.0f / fConst0;
		fConst33 = 0.18f / fConst0;
		fConst34 = 0.1f / fConst0;
		fConst35 = 1.0f / fConst0;
		fConst36 = 0.11f / fConst0;
		fConst37 = 0.07f / fConst0;
		fConst38 = 0.5f / fConst0;
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
			iRec6[l6] = 0;
		}
		for (int l7 = 0; l7 < 3; l7 = l7 + 1) {
			fRec5[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 3; l8 = l8 + 1) {
			fRec7[l8] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fRec9[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			fRec11[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 2; l13 = l13 + 1) {
			fRec12[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
			fRec10[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fRec13[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 2; l16 = l16 + 1) {
			fRec15[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2; l17 = l17 + 1) {
			fRec14[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec16[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 2; l19 = l19 + 1) {
			fRec18[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 2; l20 = l20 + 1) {
			fRec17[l20] = 0.0f;
		}
		for (int l21 = 0; l21 < 2; l21 = l21 + 1) {
			fRec19[l21] = 0.0f;
		}
		for (int l22 = 0; l22 < 2; l22 = l22 + 1) {
			fRec21[l22] = 0.0f;
		}
		for (int l23 = 0; l23 < 2; l23 = l23 + 1) {
			fRec20[l23] = 0.0f;
		}
		for (int l24 = 0; l24 < 2; l24 = l24 + 1) {
			fRec22[l24] = 0.0f;
		}
		for (int l25 = 0; l25 < 2; l25 = l25 + 1) {
			fRec24[l25] = 0.0f;
		}
		for (int l26 = 0; l26 < 2; l26 = l26 + 1) {
			fRec23[l26] = 0.0f;
		}
		for (int l27 = 0; l27 < 2; l27 = l27 + 1) {
			fRec26[l27] = 0.0f;
		}
		for (int l28 = 0; l28 < 2; l28 = l28 + 1) {
			fRec25[l28] = 0.0f;
		}
		for (int l29 = 0; l29 < 2; l29 = l29 + 1) {
			fRec27[l29] = 0.0f;
		}
		for (int l30 = 0; l30 < 2; l30 = l30 + 1) {
			fRec29[l30] = 0.0f;
		}
		for (int l31 = 0; l31 < 2; l31 = l31 + 1) {
			fRec28[l31] = 0.0f;
		}
		for (int l32 = 0; l32 < 2; l32 = l32 + 1) {
			fRec30[l32] = 0.0f;
		}
		for (int l33 = 0; l33 < 2; l33 = l33 + 1) {
			fRec32[l33] = 0.0f;
		}
		for (int l34 = 0; l34 < 2; l34 = l34 + 1) {
			fRec31[l34] = 0.0f;
		}
		for (int l35 = 0; l35 < 3; l35 = l35 + 1) {
			fRec4[l35] = 0.0f;
		}
		for (int l36 = 0; l36 < 2; l36 = l36 + 1) {
			fRec33[l36] = 0.0f;
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
	
	virtual OrganDSP* clone() {
		return new OrganDSP(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("organ");
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
		float fSlow14 = fConst18 * fSlow13;
		float fSlow15 = fConst21 * fSlow13;
		float fSlow16 = fConst24 * fSlow13;
		float fSlow17 = fConst27 * fSlow13;
		float fSlow18 = fConst30 * fSlow13;
		float fSlow19 = fConst32 * fSlow13;
		float fSlow20 = fConst35 * fSlow13;
		float fSlow21 = fConst38 * fSlow13;
		int iSlow22 = iSlow7 >= 3;
		float fSlow23 = fConst1 * static_cast<float>(fEntry10);
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
			iRec6[0] = 1103515245 * iRec6[1] + 12345;
			float fTemp4 = static_cast<float>(iRec6[0]);
			fRec5[0] = 3.7252905e-12f * fTemp4 - fConst8 * (fConst7 * fRec5[2] + fConst5 * fRec5[1]);
			fRec7[0] = 2.7939677e-11f * fTemp4 - fConst14 * (fConst12 * fRec7[2] + fConst10 * fRec7[1]);
			int iTemp5 = 1 - iVec0[1];
			float fTemp6 = ((iTemp5) ? 0.0f : fConst16 + fRec9[1]);
			fRec9[0] = fTemp6 - std::floor(fTemp6);
			float fTemp7 = ((iTemp5) ? 0.0f : fConst17 + fRec11[1]);
			fRec11[0] = fTemp7 - std::floor(fTemp7);
			fRec12[0] = fSlow12 + fConst2 * fRec12[1];
			float fTemp8 = ((iTemp5) ? 0.0f : fRec10[1] + fSlow14 * fRec12[0] * (0.0003f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec11[0]), 65535))] + 1.0f));
			fRec10[0] = fTemp8 - std::floor(fTemp8);
			float fTemp9 = ((iTemp5) ? 0.0f : fConst19 + fRec13[1]);
			fRec13[0] = fTemp9 - std::floor(fTemp9);
			float fTemp10 = ((iTemp5) ? 0.0f : fConst20 + fRec15[1]);
			fRec15[0] = fTemp10 - std::floor(fTemp10);
			float fTemp11 = ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec15[0]), 65535))];
			float fTemp12 = ((iTemp5) ? 0.0f : fRec14[1] + fSlow15 * fRec12[0] * (0.0003f * fTemp11 + 1.0f));
			fRec14[0] = fTemp12 - std::floor(fTemp12);
			float fTemp13 = ((iTemp5) ? 0.0f : fConst22 + fRec16[1]);
			fRec16[0] = fTemp13 - std::floor(fTemp13);
			float fTemp14 = ((iTemp5) ? 0.0f : fConst23 + fRec18[1]);
			fRec18[0] = fTemp14 - std::floor(fTemp14);
			float fTemp15 = ((iTemp5) ? 0.0f : fRec17[1] + fSlow16 * fRec12[0] * (0.0003f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec18[0]), 65535))] + 1.0f));
			fRec17[0] = fTemp15 - std::floor(fTemp15);
			float fTemp16 = ((iTemp5) ? 0.0f : fConst25 + fRec19[1]);
			fRec19[0] = fTemp16 - std::floor(fTemp16);
			float fTemp17 = ((iTemp5) ? 0.0f : fConst26 + fRec21[1]);
			fRec21[0] = fTemp17 - std::floor(fTemp17);
			float fTemp18 = ((iTemp5) ? 0.0f : fRec20[1] + fSlow17 * fRec12[0] * (0.0003f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec21[0]), 65535))] + 1.0f));
			fRec20[0] = fTemp18 - std::floor(fTemp18);
			float fTemp19 = ((iTemp5) ? 0.0f : fConst28 + fRec22[1]);
			fRec22[0] = fTemp19 - std::floor(fTemp19);
			float fTemp20 = ((iTemp5) ? 0.0f : fConst29 + fRec24[1]);
			fRec24[0] = fTemp20 - std::floor(fTemp20);
			float fTemp21 = ((iTemp5) ? 0.0f : fRec23[1] + fSlow18 * fRec12[0] * (0.0003f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec24[0]), 65535))] + 1.0f));
			fRec23[0] = fTemp21 - std::floor(fTemp21);
			float fTemp22 = ((iTemp5) ? 0.0f : fConst31 + fRec26[1]);
			fRec26[0] = fTemp22 - std::floor(fTemp22);
			float fTemp23 = ((iTemp5) ? 0.0f : fRec25[1] + fSlow19 * fRec12[0] * (0.0003f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec26[0]), 65535))] + 1.0f));
			fRec25[0] = fTemp23 - std::floor(fTemp23);
			float fTemp24 = ((iTemp5) ? 0.0f : fConst33 + fRec27[1]);
			fRec27[0] = fTemp24 - std::floor(fTemp24);
			float fTemp25 = ((iTemp5) ? 0.0f : fConst34 + fRec29[1]);
			fRec29[0] = fTemp25 - std::floor(fTemp25);
			float fTemp26 = ((iTemp5) ? 0.0f : fRec28[1] + fSlow20 * fRec12[0] * (0.0003f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec29[0]), 65535))] + 1.0f));
			fRec28[0] = fTemp26 - std::floor(fTemp26);
			float fTemp27 = ((iTemp5) ? 0.0f : fConst36 + fRec30[1]);
			fRec30[0] = fTemp27 - std::floor(fTemp27);
			float fTemp28 = ((iTemp5) ? 0.0f : fConst37 + fRec32[1]);
			fRec32[0] = fTemp28 - std::floor(fTemp28);
			float fTemp29 = ((iTemp5) ? 0.0f : fRec31[1] + fSlow21 * fRec12[0] * (0.0003f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec32[0]), 65535))] + 1.0f));
			fRec31[0] = fTemp29 - std::floor(fTemp29);
			float fTemp30 = 0.30487806f * (0.5f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec31[0]), 65535))] * (0.015f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec30[0]), 65535))] + 1.0f) + ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec28[0]), 65535))] * (0.015f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec27[0]), 65535))] + 1.0f) + 0.7f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec25[0]), 65535))] * (0.015f * fTemp11 + 1.0f) + 0.45f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec23[0]), 65535))] * (0.015f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec22[0]), 65535))] + 1.0f) + 0.3f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec20[0]), 65535))] * (0.015f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec19[0]), 65535))] + 1.0f) + 0.15f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec17[0]), 65535))] * (0.015f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec16[0]), 65535))] + 1.0f) + 0.1f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec14[0]), 65535))] * (0.015f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec13[0]), 65535))] + 1.0f) + 0.08f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec10[0]), 65535))] * (0.015f * ftbl0OrganDSPSIG0[std::max<int>(0, std::min<int>(static_cast<int>(65536.0f * fRec9[0]), 65535))] + 1.0f)) + fConst15 * (fRec7[0] - fRec7[2]) + fConst8 * (fRec5[2] + fRec5[0] + 2.0f * fRec5[1]);
			fRec4[0] = fTemp30 - (fRec4[2] * ((fTemp2 - fTemp1) / fTemp0 + 1.0f) + 2.0f * fRec4[1] * (1.0f - 1.0f / OrganDSP_faustpower2_f(fTemp0))) / fTemp3;
			float fTemp31 = fRec4[2] + fRec4[0] + 2.0f * fRec4[1];
			float fTemp32 = fTemp30 * (fRec4[0] - fRec4[2]) / (fTemp0 * fTemp3);
			fRec33[0] = fSlow23 + fConst2 * fRec33[1];
			output0[i0] = static_cast<FAUSTFLOAT>(fRec33[0] * ((iSlow8) ? ((iSlow22) ? fTemp30 - fTemp32 : fTemp32) : ((iSlow9) ? -(fTemp30 * (fTemp31 / fTemp3 - fTemp30)) : fTemp30 * fTemp31 / fTemp3)) * std::max<float>(0.0f, std::min<float>(fSlow4 * fRec1[0], std::max<float>(fSlow6 * (fSlow3 - fRec1[0]) + 1.0f, fSlow5)) * (1.0f - fSlow2 * static_cast<float>(iRec0[0]))));
			iVec0[1] = iVec0[0];
			fVec1[1] = fVec1[0];
			iRec0[1] = iRec0[0];
			fRec1[1] = fRec1[0];
			fRec2[1] = fRec2[0];
			fRec3[1] = fRec3[0];
			iRec6[1] = iRec6[0];
			fRec5[2] = fRec5[1];
			fRec5[1] = fRec5[0];
			fRec7[2] = fRec7[1];
			fRec7[1] = fRec7[0];
			fRec9[1] = fRec9[0];
			fRec11[1] = fRec11[0];
			fRec12[1] = fRec12[0];
			fRec10[1] = fRec10[0];
			fRec13[1] = fRec13[0];
			fRec15[1] = fRec15[0];
			fRec14[1] = fRec14[0];
			fRec16[1] = fRec16[0];
			fRec18[1] = fRec18[0];
			fRec17[1] = fRec17[0];
			fRec19[1] = fRec19[0];
			fRec21[1] = fRec21[0];
			fRec20[1] = fRec20[0];
			fRec22[1] = fRec22[0];
			fRec24[1] = fRec24[0];
			fRec23[1] = fRec23[0];
			fRec26[1] = fRec26[0];
			fRec25[1] = fRec25[0];
			fRec27[1] = fRec27[0];
			fRec29[1] = fRec29[0];
			fRec28[1] = fRec28[0];
			fRec30[1] = fRec30[0];
			fRec32[1] = fRec32[0];
			fRec31[1] = fRec31[0];
			fRec4[2] = fRec4[1];
			fRec4[1] = fRec4[0];
			fRec33[1] = fRec33[0];
		}
	}

};

#endif
