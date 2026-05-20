/* ------------------------------------------------------------
name: "noise"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
Compilation options: -lang cpp -i -fpga-mem-th 4 -ct 1 -cn NoiseDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __NoiseDSP_H__
#define  __NoiseDSP_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS NoiseDSP
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


class NoiseDSP : public dsp {
	
 private:
	
	int iRec0[2];
	FAUSTFLOAT fEntry0;
	FAUSTFLOAT fEntry1;
	FAUSTFLOAT fEntry2;
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fRec1[2];
	float fConst2;
	float fConst3;
	FAUSTFLOAT fEntry3;
	float fRec2[2];
	
 public:
	NoiseDSP() {
	}
	
	NoiseDSP(const NoiseDSP&) = default;
	
	virtual ~NoiseDSP() = default;
	
	NoiseDSP& operator=(const NoiseDSP&) = default;
	
	void metadata(Meta* m) { 
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "1.22.0");
		m->declare("compile_options", "-lang cpp -i -fpga-mem-th 4 -ct 1 -cn NoiseDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("envelopes.lib/are:author", "Julius O. Smith III, revised by Stephane Letz");
		m->declare("envelopes.lib/are:licence", "STK-4.3");
		m->declare("envelopes.lib/author", "GRAME");
		m->declare("envelopes.lib/copyright", "GRAME");
		m->declare("envelopes.lib/license", "LGPL with exception");
		m->declare("envelopes.lib/name", "Faust Envelope Library");
		m->declare("envelopes.lib/version", "1.3.0");
		m->declare("filename", "noise.dsp");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.9.0");
		m->declare("name", "noise");
		m->declare("noises.lib/name", "Faust Noise Generator Library");
		m->declare("noises.lib/version", "1.5.0");
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
		fConst1 = 1.0f / fConst0;
		fConst2 = 44.1f / fConst0;
		fConst3 = 1.0f - fConst2;
	}
	
	virtual void instanceResetUserInterface() {
		fEntry0 = static_cast<FAUSTFLOAT>(0.0f);
		fEntry1 = static_cast<FAUSTFLOAT>(1e+01f);
		fEntry2 = static_cast<FAUSTFLOAT>(0.05f);
		fEntry3 = static_cast<FAUSTFLOAT>(0.0f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iRec0[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fRec1[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec2[l2] = 0.0f;
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
	
	virtual NoiseDSP* clone() {
		return new NoiseDSP(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("noise");
		ui_interface->addNumEntry("amp", &fEntry3, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.001f));
		ui_interface->addNumEntry("attack", &fEntry2, FAUSTFLOAT(0.05f), FAUSTFLOAT(0.001f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.001f));
		ui_interface->addNumEntry("gate", &fEntry0, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(1.0f));
		ui_interface->addNumEntry("release", &fEntry1, FAUSTFLOAT(1e+01f), FAUSTFLOAT(0.01f), FAUSTFLOAT(3e+01f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		int iSlow0 = static_cast<float>(fEntry0) > 0.0f;
		float fSlow1 = 0.1447178f * ((iSlow0) ? static_cast<float>(fEntry2) : static_cast<float>(fEntry1));
		int iSlow2 = std::fabs(fSlow1) < 1.1920929e-07f;
		float fSlow3 = ((iSlow2) ? 0.0f : std::exp(-(fConst1 / ((iSlow2) ? 1.0f : fSlow1))));
		float fSlow4 = (1.0f - fSlow3) * ((iSlow0) ? static_cast<float>(iSlow0) : 0.0f);
		float fSlow5 = fConst2 * static_cast<float>(fEntry3);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iRec0[0] = 1103515245 * iRec0[1] + 12345;
			fRec1[0] = fSlow4 + fSlow3 * fRec1[1];
			fRec2[0] = fSlow5 + fConst3 * fRec2[1];
			output0[i0] = static_cast<FAUSTFLOAT>(4.656613e-10f * fRec2[0] * fRec1[0] * static_cast<float>(iRec0[0]));
			iRec0[1] = iRec0[0];
			fRec1[1] = fRec1[0];
			fRec2[1] = fRec2[0];
		}
	}

};

#endif
