#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"

using namespace terrarium;
using namespace daisy;
using namespace daisysp;

DaisyPetal hw;
Flanger flanger;

float flangerDepth = 0.5f;
float flangerFreq = 0.33f;
float flangerFeedback = 0.83f;

bool bypass = false;

void ProcessADC();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	ProcessADC();

	for (size_t i = 0; i < size; i++)
	{
		if(!bypass)
			out[0][i] = flanger.Process(in[0][i]);
		else
			out[0][i] = in[0][i];
	}
} 
void ProcessADC()
{
	bypass ^= hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge();

	flangerDepth = hw.knob[Terrarium::KNOB_1].Process();
	flangerFreq = hw.knob[Terrarium::KNOB_2].Process();
	flangerFeedback = hw.knob[Terrarium::KNOB_3].Process();

	flanger.SetLfoDepth(flangerDepth);
	flanger.SetLfoFreq(flangerFreq);
	flanger.SetFeedback(flangerFeedback);
}
int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAdc();

	flanger.Init(48000);
	flanger.SetLfoFreq(0.33f);
	flanger.SetLfoDepth(0.5f);
	flanger.SetFeedback(0.83f);

	hw.StartAudio(AudioCallback);
	while(1) {}
}
