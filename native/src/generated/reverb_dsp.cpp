/* ------------------------------------------------------------
name: "reverb"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
Compilation options: -lang cpp -i -fpga-mem-th 4 -ct 1 -cn ReverbDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __ReverbDSP_H__
#define  __ReverbDSP_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS ReverbDSP
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

static float ReverbDSP_faustpower2_f(float value) {
	return value * value;
}

class ReverbDSP : public dsp {
	
 private:
	
	FAUSTFLOAT fEntry0;
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	FAUSTFLOAT fEntry1;
	float fConst3;
	FAUSTFLOAT fEntry2;
	float fConst4;
	float fRec11[2];
	FAUSTFLOAT fEntry3;
	float fRec10[2];
	int IOTA0;
	float fVec0[65536];
	float fConst5;
	float fConst6;
	int iConst7;
	float fVec1[16384];
	FAUSTFLOAT fEntry4;
	float fConst8;
	float fVec2[8192];
	int iConst9;
	float fRec8[2];
	float fConst10;
	float fConst11;
	float fRec15[2];
	float fRec14[2];
	float fVec3[32768];
	float fConst12;
	float fConst13;
	int iConst14;
	float fVec4[4096];
	int iConst15;
	float fRec12[2];
	float fConst16;
	float fConst17;
	float fRec19[2];
	float fRec18[2];
	float fVec5[32768];
	float fConst18;
	float fConst19;
	int iConst20;
	float fVec6[8192];
	int iConst21;
	float fRec16[2];
	float fConst22;
	float fConst23;
	float fRec23[2];
	float fRec22[2];
	float fVec7[32768];
	float fConst24;
	float fConst25;
	int iConst26;
	float fVec8[4096];
	int iConst27;
	float fRec20[2];
	float fConst28;
	float fConst29;
	float fRec27[2];
	float fRec26[2];
	float fVec9[65536];
	float fConst30;
	float fConst31;
	int iConst32;
	float fVec10[16384];
	float fVec11[8192];
	int iConst33;
	float fRec24[2];
	float fConst34;
	float fConst35;
	float fRec31[2];
	float fRec30[2];
	float fVec12[65536];
	float fConst36;
	float fConst37;
	int iConst38;
	float fVec13[8192];
	int iConst39;
	float fRec28[2];
	float fConst40;
	float fConst41;
	float fRec35[2];
	float fRec34[2];
	float fVec14[65536];
	float fConst42;
	float fConst43;
	int iConst44;
	float fVec15[8192];
	int iConst45;
	float fRec32[2];
	float fConst46;
	float fConst47;
	float fRec39[2];
	float fRec38[2];
	float fVec16[65536];
	float fConst48;
	float fConst49;
	int iConst50;
	float fVec17[4096];
	int iConst51;
	float fRec36[2];
	float fRec0[3];
	float fRec1[3];
	float fRec2[3];
	float fRec3[3];
	float fRec4[3];
	float fRec5[3];
	float fRec6[3];
	float fRec7[3];
	
 public:
	ReverbDSP() {
	}
	
	ReverbDSP(const ReverbDSP&) = default;
	
	virtual ~ReverbDSP() = default;
	
	ReverbDSP& operator=(const ReverbDSP&) = default;
	
	void metadata(Meta* m) { 
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "1.22.0");
		m->declare("compile_options", "-lang cpp -i -fpga-mem-th 4 -ct 1 -cn ReverbDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.2.0");
		m->declare("filename", "reverb.dsp");
		m->declare("filters.lib/allpass_comb:author", "Julius O. Smith III");
		m->declare("filters.lib/allpass_comb:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/allpass_comb:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "MIT-style STK-4.3 license");
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
		m->declare("name", "reverb");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("reverbs.lib/name", "Faust Reverb Library");
		m->declare("reverbs.lib/version", "1.5.1");
		m->declare("routes.lib/hadamard:author", "Remy Muller, revised by Romain Michon");
		m->declare("routes.lib/name", "Faust Signal Routing Library");
		m->declare("routes.lib/version", "1.3.0");
		m->declare("signals.lib/name", "Faust Routing Library");
		m->declare("signals.lib/version", "1.6.0");
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
		fConst1 = std::floor(0.174713f * fConst0 + 0.5f);
		fConst2 = 6.9077554f * (fConst1 / fConst0);
		fConst3 = 6.2831855f / fConst0;
		fConst4 = 3.1415927f / fConst0;
		fConst5 = std::floor(0.022904f * fConst0 + 0.5f);
		fConst6 = fConst1 - fConst5;
		iConst7 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst6)))))), std::max<float>(0.0f, fConst6)));
		fConst8 = 0.001f * fConst0;
		iConst9 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst5)))))), std::max<float>(0.0f, fConst5 + -1.0f)));
		fConst10 = std::floor(0.153129f * fConst0 + 0.5f);
		fConst11 = 6.9077554f * (fConst10 / fConst0);
		fConst12 = std::floor(0.020346f * fConst0 + 0.5f);
		fConst13 = fConst10 - fConst12;
		iConst14 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst13)))))), std::max<float>(0.0f, fConst13)));
		iConst15 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst12)))))), std::max<float>(0.0f, fConst12 + -1.0f)));
		fConst16 = std::floor(0.127837f * fConst0 + 0.5f);
		fConst17 = 6.9077554f * (fConst16 / fConst0);
		fConst18 = std::floor(0.031604f * fConst0 + 0.5f);
		fConst19 = fConst16 - fConst18;
		iConst20 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst19)))))), std::max<float>(0.0f, fConst19)));
		iConst21 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst18)))))), std::max<float>(0.0f, fConst18 + -1.0f)));
		fConst22 = std::floor(0.125f * fConst0 + 0.5f);
		fConst23 = 6.9077554f * (fConst22 / fConst0);
		fConst24 = std::floor(0.013458f * fConst0 + 0.5f);
		fConst25 = fConst22 - fConst24;
		iConst26 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst25)))))), std::max<float>(0.0f, fConst25)));
		iConst27 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst24)))))), std::max<float>(0.0f, fConst24 + -1.0f)));
		fConst28 = std::floor(0.210389f * fConst0 + 0.5f);
		fConst29 = 6.9077554f * (fConst28 / fConst0);
		fConst30 = std::floor(0.024421f * fConst0 + 0.5f);
		fConst31 = fConst28 - fConst30;
		iConst32 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst31)))))), std::max<float>(0.0f, fConst31)));
		iConst33 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst30)))))), std::max<float>(0.0f, fConst30 + -1.0f)));
		fConst34 = std::floor(0.192303f * fConst0 + 0.5f);
		fConst35 = 6.9077554f * (fConst34 / fConst0);
		fConst36 = std::floor(0.029291f * fConst0 + 0.5f);
		fConst37 = fConst34 - fConst36;
		iConst38 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst37)))))), std::max<float>(0.0f, fConst37)));
		iConst39 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst36)))))), std::max<float>(0.0f, fConst36 + -1.0f)));
		fConst40 = std::floor(0.256891f * fConst0 + 0.5f);
		fConst41 = 6.9077554f * (fConst40 / fConst0);
		fConst42 = std::floor(0.027333f * fConst0 + 0.5f);
		fConst43 = fConst40 - fConst42;
		iConst44 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst43)))))), std::max<float>(0.0f, fConst43)));
		iConst45 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst42)))))), std::max<float>(0.0f, fConst42 + -1.0f)));
		fConst46 = std::floor(0.219991f * fConst0 + 0.5f);
		fConst47 = 6.9077554f * (fConst46 / fConst0);
		fConst48 = std::floor(0.019123f * fConst0 + 0.5f);
		fConst49 = fConst46 - fConst48;
		iConst50 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst49)))))), std::max<float>(0.0f, fConst49)));
		iConst51 = static_cast<int>(std::min<float>(static_cast<float>(static_cast<int>(std::pow(2.0f, std::max<float>(1.0f, std::ceil(1.442695f * std::log(fConst48)))))), std::max<float>(0.0f, fConst48 + -1.0f)));
	}
	
	virtual void instanceResetUserInterface() {
		fEntry0 = static_cast<FAUSTFLOAT>(2.0f);
		fEntry1 = static_cast<FAUSTFLOAT>(6e+03f);
		fEntry2 = static_cast<FAUSTFLOAT>(2e+02f);
		fEntry3 = static_cast<FAUSTFLOAT>(3.0f);
		fEntry4 = static_cast<FAUSTFLOAT>(6e+01f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fRec11[l0] = 0.0f;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fRec10[l1] = 0.0f;
		}
		IOTA0 = 0;
		for (int l2 = 0; l2 < 65536; l2 = l2 + 1) {
			fVec0[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 16384; l3 = l3 + 1) {
			fVec1[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 8192; l4 = l4 + 1) {
			fVec2[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fRec8[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fRec15[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			fRec14[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 32768; l8 = l8 + 1) {
			fVec3[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 4096; l9 = l9 + 1) {
			fVec4[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			fRec12[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fRec19[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			fRec18[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 32768; l13 = l13 + 1) {
			fVec5[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 8192; l14 = l14 + 1) {
			fVec6[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 2; l15 = l15 + 1) {
			fRec16[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 2; l16 = l16 + 1) {
			fRec23[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2; l17 = l17 + 1) {
			fRec22[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 32768; l18 = l18 + 1) {
			fVec7[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 4096; l19 = l19 + 1) {
			fVec8[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 2; l20 = l20 + 1) {
			fRec20[l20] = 0.0f;
		}
		for (int l21 = 0; l21 < 2; l21 = l21 + 1) {
			fRec27[l21] = 0.0f;
		}
		for (int l22 = 0; l22 < 2; l22 = l22 + 1) {
			fRec26[l22] = 0.0f;
		}
		for (int l23 = 0; l23 < 65536; l23 = l23 + 1) {
			fVec9[l23] = 0.0f;
		}
		for (int l24 = 0; l24 < 16384; l24 = l24 + 1) {
			fVec10[l24] = 0.0f;
		}
		for (int l25 = 0; l25 < 8192; l25 = l25 + 1) {
			fVec11[l25] = 0.0f;
		}
		for (int l26 = 0; l26 < 2; l26 = l26 + 1) {
			fRec24[l26] = 0.0f;
		}
		for (int l27 = 0; l27 < 2; l27 = l27 + 1) {
			fRec31[l27] = 0.0f;
		}
		for (int l28 = 0; l28 < 2; l28 = l28 + 1) {
			fRec30[l28] = 0.0f;
		}
		for (int l29 = 0; l29 < 65536; l29 = l29 + 1) {
			fVec12[l29] = 0.0f;
		}
		for (int l30 = 0; l30 < 8192; l30 = l30 + 1) {
			fVec13[l30] = 0.0f;
		}
		for (int l31 = 0; l31 < 2; l31 = l31 + 1) {
			fRec28[l31] = 0.0f;
		}
		for (int l32 = 0; l32 < 2; l32 = l32 + 1) {
			fRec35[l32] = 0.0f;
		}
		for (int l33 = 0; l33 < 2; l33 = l33 + 1) {
			fRec34[l33] = 0.0f;
		}
		for (int l34 = 0; l34 < 65536; l34 = l34 + 1) {
			fVec14[l34] = 0.0f;
		}
		for (int l35 = 0; l35 < 8192; l35 = l35 + 1) {
			fVec15[l35] = 0.0f;
		}
		for (int l36 = 0; l36 < 2; l36 = l36 + 1) {
			fRec32[l36] = 0.0f;
		}
		for (int l37 = 0; l37 < 2; l37 = l37 + 1) {
			fRec39[l37] = 0.0f;
		}
		for (int l38 = 0; l38 < 2; l38 = l38 + 1) {
			fRec38[l38] = 0.0f;
		}
		for (int l39 = 0; l39 < 65536; l39 = l39 + 1) {
			fVec16[l39] = 0.0f;
		}
		for (int l40 = 0; l40 < 4096; l40 = l40 + 1) {
			fVec17[l40] = 0.0f;
		}
		for (int l41 = 0; l41 < 2; l41 = l41 + 1) {
			fRec36[l41] = 0.0f;
		}
		for (int l42 = 0; l42 < 3; l42 = l42 + 1) {
			fRec0[l42] = 0.0f;
		}
		for (int l43 = 0; l43 < 3; l43 = l43 + 1) {
			fRec1[l43] = 0.0f;
		}
		for (int l44 = 0; l44 < 3; l44 = l44 + 1) {
			fRec2[l44] = 0.0f;
		}
		for (int l45 = 0; l45 < 3; l45 = l45 + 1) {
			fRec3[l45] = 0.0f;
		}
		for (int l46 = 0; l46 < 3; l46 = l46 + 1) {
			fRec4[l46] = 0.0f;
		}
		for (int l47 = 0; l47 < 3; l47 = l47 + 1) {
			fRec5[l47] = 0.0f;
		}
		for (int l48 = 0; l48 < 3; l48 = l48 + 1) {
			fRec6[l48] = 0.0f;
		}
		for (int l49 = 0; l49 < 3; l49 = l49 + 1) {
			fRec7[l49] = 0.0f;
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
	
	virtual ReverbDSP* clone() {
		return new ReverbDSP(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("reverb");
		ui_interface->addNumEntry("f1", &fEntry2, FAUSTFLOAT(2e+02f), FAUSTFLOAT(5e+01f), FAUSTFLOAT(1e+03f), FAUSTFLOAT(1.0f));
		ui_interface->addNumEntry("f2", &fEntry1, FAUSTFLOAT(6e+03f), FAUSTFLOAT(1e+03f), FAUSTFLOAT(1e+04f), FAUSTFLOAT(1.0f));
		ui_interface->addNumEntry("rdel", &fEntry4, FAUSTFLOAT(6e+01f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(1e+02f), FAUSTFLOAT(1.0f));
		ui_interface->addNumEntry("t60dc", &fEntry3, FAUSTFLOAT(3.0f), FAUSTFLOAT(0.1f), FAUSTFLOAT(8.0f), FAUSTFLOAT(0.1f));
		ui_interface->addNumEntry("t60m", &fEntry0, FAUSTFLOAT(2.0f), FAUSTFLOAT(0.1f), FAUSTFLOAT(8.0f), FAUSTFLOAT(0.1f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* input1 = inputs[1];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = static_cast<float>(fEntry0);
		float fSlow1 = std::exp(-(fConst2 / fSlow0));
		float fSlow2 = ReverbDSP_faustpower2_f(fSlow1);
		float fSlow3 = 1.0f - fSlow2;
		float fSlow4 = std::cos(fConst3 * static_cast<float>(fEntry1));
		float fSlow5 = 1.0f - fSlow4 * fSlow2;
		float fSlow6 = std::sqrt(std::max<float>(0.0f, ReverbDSP_faustpower2_f(fSlow5) / ReverbDSP_faustpower2_f(fSlow3) + -1.0f));
		float fSlow7 = fSlow5 / fSlow3;
		float fSlow8 = fSlow7 - fSlow6;
		float fSlow9 = 1.0f / std::tan(fConst4 * static_cast<float>(fEntry2));
		float fSlow10 = 1.0f - fSlow9;
		float fSlow11 = 1.0f / (fSlow9 + 1.0f);
		float fSlow12 = static_cast<float>(fEntry3);
		float fSlow13 = std::exp(-(fConst2 / fSlow12)) / fSlow1 + -1.0f;
		float fSlow14 = fSlow1 * (fSlow6 + (1.0f - fSlow7));
		int iSlow15 = static_cast<int>(std::min<float>(8192.0f, std::max<float>(0.0f, fConst8 * static_cast<float>(fEntry4))));
		float fSlow16 = std::exp(-(fConst11 / fSlow0));
		float fSlow17 = ReverbDSP_faustpower2_f(fSlow16);
		float fSlow18 = 1.0f - fSlow17;
		float fSlow19 = 1.0f - fSlow17 * fSlow4;
		float fSlow20 = std::sqrt(std::max<float>(0.0f, ReverbDSP_faustpower2_f(fSlow19) / ReverbDSP_faustpower2_f(fSlow18) + -1.0f));
		float fSlow21 = fSlow19 / fSlow18;
		float fSlow22 = fSlow21 - fSlow20;
		float fSlow23 = std::exp(-(fConst11 / fSlow12)) / fSlow16 + -1.0f;
		float fSlow24 = fSlow16 * (fSlow20 + (1.0f - fSlow21));
		float fSlow25 = std::exp(-(fConst17 / fSlow0));
		float fSlow26 = ReverbDSP_faustpower2_f(fSlow25);
		float fSlow27 = 1.0f - fSlow26;
		float fSlow28 = 1.0f - fSlow4 * fSlow26;
		float fSlow29 = std::sqrt(std::max<float>(0.0f, ReverbDSP_faustpower2_f(fSlow28) / ReverbDSP_faustpower2_f(fSlow27) + -1.0f));
		float fSlow30 = fSlow28 / fSlow27;
		float fSlow31 = fSlow30 - fSlow29;
		float fSlow32 = std::exp(-(fConst17 / fSlow12)) / fSlow25 + -1.0f;
		float fSlow33 = fSlow25 * (fSlow29 + (1.0f - fSlow30));
		float fSlow34 = std::exp(-(fConst23 / fSlow0));
		float fSlow35 = ReverbDSP_faustpower2_f(fSlow34);
		float fSlow36 = 1.0f - fSlow35;
		float fSlow37 = 1.0f - fSlow4 * fSlow35;
		float fSlow38 = std::sqrt(std::max<float>(0.0f, ReverbDSP_faustpower2_f(fSlow37) / ReverbDSP_faustpower2_f(fSlow36) + -1.0f));
		float fSlow39 = fSlow37 / fSlow36;
		float fSlow40 = fSlow39 - fSlow38;
		float fSlow41 = std::exp(-(fConst23 / fSlow12)) / fSlow34 + -1.0f;
		float fSlow42 = fSlow34 * (fSlow38 + (1.0f - fSlow39));
		float fSlow43 = std::exp(-(fConst29 / fSlow0));
		float fSlow44 = ReverbDSP_faustpower2_f(fSlow43);
		float fSlow45 = 1.0f - fSlow44;
		float fSlow46 = 1.0f - fSlow4 * fSlow44;
		float fSlow47 = std::sqrt(std::max<float>(0.0f, ReverbDSP_faustpower2_f(fSlow46) / ReverbDSP_faustpower2_f(fSlow45) + -1.0f));
		float fSlow48 = fSlow46 / fSlow45;
		float fSlow49 = fSlow48 - fSlow47;
		float fSlow50 = std::exp(-(fConst29 / fSlow12)) / fSlow43 + -1.0f;
		float fSlow51 = fSlow43 * (fSlow47 + (1.0f - fSlow48));
		float fSlow52 = std::exp(-(fConst35 / fSlow0));
		float fSlow53 = ReverbDSP_faustpower2_f(fSlow52);
		float fSlow54 = 1.0f - fSlow53;
		float fSlow55 = 1.0f - fSlow4 * fSlow53;
		float fSlow56 = std::sqrt(std::max<float>(0.0f, ReverbDSP_faustpower2_f(fSlow55) / ReverbDSP_faustpower2_f(fSlow54) + -1.0f));
		float fSlow57 = fSlow55 / fSlow54;
		float fSlow58 = fSlow57 - fSlow56;
		float fSlow59 = std::exp(-(fConst35 / fSlow12)) / fSlow52 + -1.0f;
		float fSlow60 = fSlow52 * (fSlow56 + (1.0f - fSlow57));
		float fSlow61 = std::exp(-(fConst41 / fSlow0));
		float fSlow62 = ReverbDSP_faustpower2_f(fSlow61);
		float fSlow63 = 1.0f - fSlow62;
		float fSlow64 = 1.0f - fSlow4 * fSlow62;
		float fSlow65 = std::sqrt(std::max<float>(0.0f, ReverbDSP_faustpower2_f(fSlow64) / ReverbDSP_faustpower2_f(fSlow63) + -1.0f));
		float fSlow66 = fSlow64 / fSlow63;
		float fSlow67 = fSlow66 - fSlow65;
		float fSlow68 = std::exp(-(fConst41 / fSlow12)) / fSlow61 + -1.0f;
		float fSlow69 = fSlow61 * (fSlow65 + (1.0f - fSlow66));
		float fSlow70 = std::exp(-(fConst47 / fSlow0));
		float fSlow71 = ReverbDSP_faustpower2_f(fSlow70);
		float fSlow72 = 1.0f - fSlow71;
		float fSlow73 = 1.0f - fSlow4 * fSlow71;
		float fSlow74 = std::sqrt(std::max<float>(0.0f, ReverbDSP_faustpower2_f(fSlow73) / ReverbDSP_faustpower2_f(fSlow72) + -1.0f));
		float fSlow75 = fSlow73 / fSlow72;
		float fSlow76 = fSlow75 - fSlow74;
		float fSlow77 = std::exp(-(fConst47 / fSlow12)) / fSlow70 + -1.0f;
		float fSlow78 = fSlow70 * (fSlow74 + (1.0f - fSlow75));
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			fRec11[0] = -(fSlow11 * (fSlow10 * fRec11[1] - (fRec4[1] + fRec4[2])));
			fRec10[0] = fSlow14 * (fRec4[1] + fSlow13 * fRec11[0]) + fSlow8 * fRec10[1];
			fVec0[IOTA0 & 65535] = 0.35355338f * fRec10[0] + 1e-20f;
			fVec1[IOTA0 & 16383] = static_cast<float>(input0[i0]);
			float fTemp0 = 0.3f * fVec1[(IOTA0 - iSlow15) & 16383];
			float fTemp1 = fTemp0 + fVec0[(IOTA0 - iConst7) & 65535] - 0.6f * fRec8[1];
			fVec2[IOTA0 & 8191] = fTemp1;
			fRec8[0] = fVec2[(IOTA0 - iConst9) & 8191];
			float fRec9 = 0.6f * fTemp1;
			fRec15[0] = -(fSlow11 * (fSlow10 * fRec15[1] - (fRec0[1] + fRec0[2])));
			fRec14[0] = fSlow24 * (fRec0[1] + fSlow23 * fRec15[0]) + fSlow22 * fRec14[1];
			fVec3[IOTA0 & 32767] = 0.35355338f * fRec14[0] + 1e-20f;
			float fTemp2 = fVec3[(IOTA0 - iConst14) & 32767] + fTemp0 - 0.6f * fRec12[1];
			fVec4[IOTA0 & 4095] = fTemp2;
			fRec12[0] = fVec4[(IOTA0 - iConst15) & 4095];
			float fRec13 = 0.6f * fTemp2;
			float fTemp3 = fRec13 + fRec9;
			fRec19[0] = -(fSlow11 * (fSlow10 * fRec19[1] - (fRec2[1] + fRec2[2])));
			fRec18[0] = fSlow33 * (fRec2[1] + fSlow32 * fRec19[0]) + fSlow31 * fRec18[1];
			fVec5[IOTA0 & 32767] = 0.35355338f * fRec18[0] + 1e-20f;
			float fTemp4 = fVec5[(IOTA0 - iConst20) & 32767] - (fTemp0 + 0.6f * fRec16[1]);
			fVec6[IOTA0 & 8191] = fTemp4;
			fRec16[0] = fVec6[(IOTA0 - iConst21) & 8191];
			float fRec17 = 0.6f * fTemp4;
			fRec23[0] = -(fSlow11 * (fSlow10 * fRec23[1] - (fRec6[1] + fRec6[2])));
			fRec22[0] = fSlow42 * (fRec6[1] + fSlow41 * fRec23[0]) + fSlow40 * fRec22[1];
			fVec7[IOTA0 & 32767] = 0.35355338f * fRec22[0] + 1e-20f;
			float fTemp5 = fVec7[(IOTA0 - iConst26) & 32767] - (fTemp0 + 0.6f * fRec20[1]);
			fVec8[IOTA0 & 4095] = fTemp5;
			fRec20[0] = fVec8[(IOTA0 - iConst27) & 4095];
			float fRec21 = 0.6f * fTemp5;
			float fTemp6 = fRec21 + fRec17 + fTemp3;
			fRec27[0] = -(fSlow11 * (fSlow10 * fRec27[1] - (fRec1[1] + fRec1[2])));
			fRec26[0] = fSlow51 * (fRec1[1] + fSlow50 * fRec27[0]) + fSlow49 * fRec26[1];
			fVec9[IOTA0 & 65535] = 0.35355338f * fRec26[0] + 1e-20f;
			fVec10[IOTA0 & 16383] = static_cast<float>(input1[i0]);
			float fTemp7 = 0.3f * fVec10[(IOTA0 - iSlow15) & 16383];
			float fTemp8 = fTemp7 + 0.6f * fRec24[1] + fVec9[(IOTA0 - iConst32) & 65535];
			fVec11[IOTA0 & 8191] = fTemp8;
			fRec24[0] = fVec11[(IOTA0 - iConst33) & 8191];
			float fRec25 = -(0.6f * fTemp8);
			fRec31[0] = -(fSlow11 * (fSlow10 * fRec31[1] - (fRec5[1] + fRec5[2])));
			fRec30[0] = fSlow60 * (fRec5[1] + fSlow59 * fRec31[0]) + fSlow58 * fRec30[1];
			fVec12[IOTA0 & 65535] = 0.35355338f * fRec30[0] + 1e-20f;
			float fTemp9 = fVec12[(IOTA0 - iConst38) & 65535] + fTemp7 + 0.6f * fRec28[1];
			fVec13[IOTA0 & 8191] = fTemp9;
			fRec28[0] = fVec13[(IOTA0 - iConst39) & 8191];
			float fRec29 = -(0.6f * fTemp9);
			fRec35[0] = -(fSlow11 * (fSlow10 * fRec35[1] - (fRec3[1] + fRec3[2])));
			fRec34[0] = fSlow69 * (fRec3[1] + fSlow68 * fRec35[0]) + fSlow67 * fRec34[1];
			fVec14[IOTA0 & 65535] = 0.35355338f * fRec34[0] + 1e-20f;
			float fTemp10 = 0.6f * fRec32[1] + fVec14[(IOTA0 - iConst44) & 65535];
			fVec15[IOTA0 & 8191] = fTemp10 - fTemp7;
			fRec32[0] = fVec15[(IOTA0 - iConst45) & 8191];
			float fRec33 = 0.6f * (fTemp7 - fTemp10);
			fRec39[0] = -(fSlow11 * (fSlow10 * fRec39[1] - (fRec7[1] + fRec7[2])));
			fRec38[0] = fSlow78 * (fRec7[1] + fSlow77 * fRec39[0]) + fSlow76 * fRec38[1];
			fVec16[IOTA0 & 65535] = 0.35355338f * fRec38[0] + 1e-20f;
			float fTemp11 = 0.6f * fRec36[1] + fVec16[(IOTA0 - iConst50) & 65535];
			fVec17[IOTA0 & 4095] = fTemp11 - fTemp7;
			fRec36[0] = fVec17[(IOTA0 - iConst51) & 4095];
			float fRec37 = 0.6f * (fTemp7 - fTemp11);
			fRec0[0] = fRec36[1] + fRec32[1] + fRec28[1] + fRec24[1] + fRec20[1] + fRec16[1] + fRec8[1] + fRec12[1] + fRec37 + fRec33 + fRec29 + fRec25 + fTemp6;
			fRec1[0] = fRec20[1] + fRec16[1] + fRec8[1] + fRec12[1] + fTemp6 - (fRec36[1] + fRec32[1] + fRec28[1] + fRec24[1] + fRec37 + fRec33 + fRec25 + fRec29);
			float fTemp12 = fRec17 + fRec21;
			fRec2[0] = fRec28[1] + fRec24[1] + fRec8[1] + fRec12[1] + fRec29 + fRec25 + fTemp3 - (fRec36[1] + fRec32[1] + fRec20[1] + fRec16[1] + fRec37 + fRec33 + fTemp12);
			fRec3[0] = fRec36[1] + fRec32[1] + fRec8[1] + fRec12[1] + fRec37 + fRec33 + fTemp3 - (fRec28[1] + fRec24[1] + fRec20[1] + fRec16[1] + fRec29 + fRec25 + fTemp12);
			float fTemp13 = fRec9 + fRec21;
			float fTemp14 = fRec13 + fRec17;
			fRec4[0] = fRec32[1] + fRec24[1] + fRec16[1] + fRec12[1] + fRec33 + fRec25 + fTemp14 - (fRec36[1] + fRec28[1] + fRec20[1] + fRec8[1] + fRec37 + fRec29 + fTemp13);
			fRec5[0] = fRec36[1] + fRec28[1] + fRec16[1] + fRec12[1] + fRec37 + fRec29 + fTemp14 - (fRec32[1] + fRec24[1] + fRec20[1] + fRec8[1] + fRec33 + fRec25 + fTemp13);
			float fTemp15 = fRec9 + fRec17;
			float fTemp16 = fRec13 + fRec21;
			fRec6[0] = fRec36[1] + fRec24[1] + fRec20[1] + fRec12[1] + fRec37 + fRec25 + fTemp16 - (fRec32[1] + fRec28[1] + fRec16[1] + fRec8[1] + fRec33 + fRec29 + fTemp15);
			fRec7[0] = fRec32[1] + fRec28[1] + fRec20[1] + fRec12[1] + fRec33 + fRec29 + fTemp16 - (fRec36[1] + fRec24[1] + fRec16[1] + fRec8[1] + fRec37 + fRec25 + fTemp15);
			output0[i0] = static_cast<FAUSTFLOAT>(0.37f * (fRec1[0] + fRec2[0]));
			output1[i0] = static_cast<FAUSTFLOAT>(0.37f * (fRec1[0] - fRec2[0]));
			fRec11[1] = fRec11[0];
			fRec10[1] = fRec10[0];
			IOTA0 = IOTA0 + 1;
			fRec8[1] = fRec8[0];
			fRec15[1] = fRec15[0];
			fRec14[1] = fRec14[0];
			fRec12[1] = fRec12[0];
			fRec19[1] = fRec19[0];
			fRec18[1] = fRec18[0];
			fRec16[1] = fRec16[0];
			fRec23[1] = fRec23[0];
			fRec22[1] = fRec22[0];
			fRec20[1] = fRec20[0];
			fRec27[1] = fRec27[0];
			fRec26[1] = fRec26[0];
			fRec24[1] = fRec24[0];
			fRec31[1] = fRec31[0];
			fRec30[1] = fRec30[0];
			fRec28[1] = fRec28[0];
			fRec35[1] = fRec35[0];
			fRec34[1] = fRec34[0];
			fRec32[1] = fRec32[0];
			fRec39[1] = fRec39[0];
			fRec38[1] = fRec38[0];
			fRec36[1] = fRec36[0];
			fRec0[2] = fRec0[1];
			fRec0[1] = fRec0[0];
			fRec1[2] = fRec1[1];
			fRec1[1] = fRec1[0];
			fRec2[2] = fRec2[1];
			fRec2[1] = fRec2[0];
			fRec3[2] = fRec3[1];
			fRec3[1] = fRec3[0];
			fRec4[2] = fRec4[1];
			fRec4[1] = fRec4[0];
			fRec5[2] = fRec5[1];
			fRec5[1] = fRec5[0];
			fRec6[2] = fRec6[1];
			fRec6[1] = fRec6[0];
			fRec7[2] = fRec7[1];
			fRec7[1] = fRec7[0];
		}
	}

};

#endif
