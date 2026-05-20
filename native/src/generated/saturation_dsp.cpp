/* ------------------------------------------------------------
name: "saturation"
Code generated with Faust 2.85.5 (https://faust.grame.fr)
Compilation options: -lang cpp -i -fpga-mem-th 4 -ct 1 -cn SaturationDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __SaturationDSP_H__
#define  __SaturationDSP_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

/* link with : "" */
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS SaturationDSP
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


class SaturationDSP : public dsp {
	
 private:
	
	FAUSTFLOAT fEntry0;
	FAUSTFLOAT fEntry1;
	float fVec0[2];
	FAUSTFLOAT fEntry2;
	int fSampleRate;
	float fConst0;
	float fRec0[2];
	float fVec1[2];
	float fRec1[2];
	
 public:
	SaturationDSP() {
	}
	
	SaturationDSP(const SaturationDSP&) = default;
	
	virtual ~SaturationDSP() = default;
	
	SaturationDSP& operator=(const SaturationDSP&) = default;
	
	void metadata(Meta* m) { 
		m->declare("compile_options", "-lang cpp -i -fpga-mem-th 4 -ct 1 -cn SaturationDSP -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
		m->declare("filename", "saturation.dsp");
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
		m->declare("name", "saturation");
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
		fConst0 = 3.1415927f / std::min<float>(1.92e+05f, std::max<float>(1.0f, static_cast<float>(fSampleRate)));
	}
	
	virtual void instanceResetUserInterface() {
		fEntry0 = static_cast<FAUSTFLOAT>(0.5f);
		fEntry1 = static_cast<FAUSTFLOAT>(0.7f);
		fEntry2 = static_cast<FAUSTFLOAT>(0.5f);
	}
	
	virtual void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fVec0[l0] = 0.0f;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fRec0[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fVec1[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec1[l3] = 0.0f;
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
	
	virtual SaturationDSP* clone() {
		return new SaturationDSP(*this);
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("saturation");
		ui_interface->addNumEntry("drive", &fEntry0, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("output", &fEntry1, FAUSTFLOAT(0.7f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->addNumEntry("tone", &fEntry2, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* input1 = inputs[1];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = 1e+01f * static_cast<float>(fEntry0) + 1.0f;
		float fSlow1 = static_cast<float>(fEntry1);
		float fSlow2 = 1.0f / std::tan(fConst0 * (1.5e+04f * static_cast<float>(fEntry2) + 1e+03f));
		float fSlow3 = 1.0f - fSlow2;
		float fSlow4 = 1.0f / (fSlow2 + 1.0f);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			float fTemp0 = fSlow1 * tanhf(fSlow0 * static_cast<float>(input0[i0]));
			fVec0[0] = fTemp0;
			fRec0[0] = -(fSlow4 * (fSlow3 * fRec0[1] - (fTemp0 + fVec0[1])));
			output0[i0] = static_cast<FAUSTFLOAT>(fRec0[0]);
			float fTemp1 = fSlow1 * tanhf(fSlow0 * static_cast<float>(input1[i0]));
			fVec1[0] = fTemp1;
			fRec1[0] = -(fSlow4 * (fSlow3 * fRec1[1] - (fTemp1 + fVec1[1])));
			output1[i0] = static_cast<FAUSTFLOAT>(fRec1[0]);
			fVec0[1] = fVec0[0];
			fRec0[1] = fRec0[0];
			fVec1[1] = fVec1[0];
			fRec1[1] = fRec1[0];
		}
	}

};

#endif
