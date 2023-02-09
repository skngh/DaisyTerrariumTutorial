#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisy::seed;
using namespace daisysp;

DaisySeed hw;
// -------------------------- FUNCTION DECS -------------------------------

void InitializeADC();
void ProcessADC();

// ----------------------------- ENUMS ------------------------------------

enum AdcChannel {
	knobOne,
	knobTwo,
	knobThree,
	NUM_ADC_CHANNELS
};

// ----------------------- INPUTS AND OUTPUTS ----------------------------

#define KnobOneInput A0
#define KnobTwoInput A1
#define KnobThreeInput A2
#define BypassButtonInput A3

// -----------------------  GLOBAL VARIABLES -------------------------------

// Flanger
Flanger flanger;
float gFlangerDepth = 0.5f;
float gFlangerFreq = 0.33f;
float gFlangerFeedback = 0.83f;

bool gBypass = false;

// Switches
Switch bypassButton;

// -------------------------------------------------------------------------

void InitializeADC()
{
	AdcChannelConfig adc_config[NUM_ADC_CHANNELS];
	adc_config[knobOne].InitSingle(KnobOneInput);
	adc_config[knobTwo].InitSingle(KnobTwoInput);
	adc_config[knobThree].InitSingle(KnobThreeInput);
	hw.adc.Init(adc_config, NUM_ADC_CHANNELS);
	hw.adc.Start();
}

void ProcessADC()
{
	gFlangerDepth = hw.adc.GetFloat(knobOne);
	gFlangerFreq = hw.adc.GetFloat(knobTwo);
	gFlangerFeedback = hw.adc.GetFloat(knobThree);

	flanger.SetLfoDepth(gFlangerDepth);
	flanger.SetLfoFreq(gFlangerFreq);
	flanger.SetFeedback(gFlangerFeedback);

	gBypass ^= bypassButton.RisingEdge();
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	ProcessADC();
	for (size_t i = 0; i < size; i++)
	{
		if(!gBypass)
			out[0][i] = flanger.Process(in[0][i]);
		else
			out[0][i] = in[0][i];
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	// Initialize flanger
	flanger.Init(hw.AudioSampleRate());
	flanger.SetLfoFreq(gFlangerFreq);
	flanger.SetLfoDepth(gFlangerDepth);
	flanger.SetFeedback(gFlangerFeedback);

	bypassButton.Init(BypassButtonInput, 1000);

	InitializeADC();

	hw.StartAudio(AudioCallback);
	while(1) {}
}
