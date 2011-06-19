#ifndef _audio_captcha1_
#define _audio_captcha1_

#ifdef AUDIO_CAPTCHA

#include "icaptcha.h"
#include <speak_lib.h>

class AudioCaptcha1:public ICaptcha
{
public:
	AudioCaptcha1();

	const bool Generate();

	const bool GetPuzzle(std::vector<unsigned char> &puzzle);
	const bool GetSolution(std::vector<unsigned char> &solution);
	const std::string GetMimeType()			{ return "audio/x-wav"; }
	const std::string GetCaptchaType()		{ return "audiocaptcha1"; }

private:

	class AudioStream
	{
	public:
		
		static int Callback(short *samples, int numsamples, espeak_EVENT *ev)
		{
			AudioStream *as=(AudioStream *)(ev->user_data);
			if(samples && numsamples>0)
			{
				as->m_buffer.insert(as->m_buffer.end(),&samples[0],&samples[numsamples]);
			}
			return 0;
		}
		
		std::vector<short> &GetSamples()			{ return m_buffer; }
		
		void SetSampleRate(const long samplerate)	{ m_samplerate=samplerate; }
		const long GetSampleRate() const			{ return m_samplerate; }
		
	private:
		long m_samplerate;
		std::vector<short> m_buffer;
	};

	const unsigned long ULongRand() const;
	void GenerateNoise(int numsamples, AudioStream &result);
	void CombineStreams(std::vector<AudioStream *> &streams, AudioStream &result);

	void CreateWave(AudioStream &stream);

	std::vector<unsigned char> m_puzzle;
	std::vector<unsigned char> m_solution;

};

#endif	// AUDIO_CAPTCHA

#endif	// _audio_captcha1_
