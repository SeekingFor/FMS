#include "../../../include/freenet/captcha/audiocaptcha1.h"

#ifdef AUDIO_CAPTCHA

#include <cstdlib>
#include <Poco/Path.h>
#include <Poco/File.h>

AudioCaptcha1::AudioCaptcha1()
{
	
}

void AudioCaptcha1::CombineStreams(std::vector<AudioStream *> &streams, AudioStream &result)
{
	if(streams.size()>0)
	{
		std::vector<short>::size_type longest=0;
		for(std::vector<AudioStream *>::iterator i=streams.begin(); i!=streams.end(); i++)
		{
			if(longest<(*i)->GetSamples().size())
			{
				longest=(*i)->GetSamples().size();
			}
		}

		result.GetSamples().resize(longest);

		for(std::vector<short>::size_type i=0; i<longest; i++)
		{
			long val=0;
			long count=0;
			for(std::vector<AudioStream *>::iterator j=streams.begin(); j!=streams.end(); j++)
			{
				if(i<(*j)->GetSamples().size())
				{
					val+=(*j)->GetSamples()[i];
					count++;
				}
			}
			result.GetSamples()[i]=val/count;
		}

	}
}

void AudioCaptcha1::CreateWave(AudioStream &stream)
{
	long temp;

	m_puzzle.clear();

	m_puzzle.push_back('R');
	m_puzzle.push_back('I');
	m_puzzle.push_back('F');
	m_puzzle.push_back('F');

	temp=36+stream.GetSamples().size();
	m_puzzle.push_back(temp & 0xff);
	m_puzzle.push_back((temp >> 8) & 0xff);
	m_puzzle.push_back((temp >> 16) & 0xff);
	m_puzzle.push_back((temp >> 24) & 0xff);

	m_puzzle.push_back('W');
	m_puzzle.push_back('A');
	m_puzzle.push_back('V');
	m_puzzle.push_back('E');

	m_puzzle.push_back('f');
	m_puzzle.push_back('m');
	m_puzzle.push_back('t');
	m_puzzle.push_back(' ');

	m_puzzle.push_back(16);
	m_puzzle.push_back(0);
	m_puzzle.push_back(0);
	m_puzzle.push_back(0);

	m_puzzle.push_back(1);
	m_puzzle.push_back(0);

	m_puzzle.push_back(1);
	m_puzzle.push_back(0);

	m_puzzle.push_back(stream.GetSampleRate() & 0xff);
	m_puzzle.push_back((stream.GetSampleRate() >> 8) & 0xff);
	m_puzzle.push_back((stream.GetSampleRate() >> 16) & 0xff);
	m_puzzle.push_back((stream.GetSampleRate() >> 24) & 0xff);

	// byte rate = samplerate * numchannels * bitspersample/8
	temp=stream.GetSampleRate()*1*sizeof(short);
	m_puzzle.push_back(temp & 0xff);
	m_puzzle.push_back((temp >> 8) & 0xff);
	m_puzzle.push_back((temp >> 16) & 0xff);
	m_puzzle.push_back((temp >> 24) & 0xff);

	// block align = numchannels * bitspersample/8
	temp=1*sizeof(short);
	m_puzzle.push_back(temp & 0xff);
	m_puzzle.push_back((temp >> 8) & 0xff);

	//bitspersample
	temp=sizeof(short)*8;
	m_puzzle.push_back(temp & 0xff);
	m_puzzle.push_back((temp >> 8) & 0xff);

	m_puzzle.push_back('d');
	m_puzzle.push_back('a');
	m_puzzle.push_back('t');
	m_puzzle.push_back('a');

	temp=sizeof(short)*stream.GetSamples().size();
	m_puzzle.push_back(temp & 0xff);
	m_puzzle.push_back((temp >> 8) & 0xff);
	m_puzzle.push_back((temp >> 16) & 0xff);
	m_puzzle.push_back((temp >> 24) & 0xff);

	m_puzzle.reserve(m_puzzle.size()+(stream.GetSamples().size()*sizeof(short)));
	for(std::vector<short>::iterator i=stream.GetSamples().begin(); i!=stream.GetSamples().end(); i++)
	{
		m_puzzle.push_back((*i) & 0xff);
		m_puzzle.push_back(((*i) >> 8) & 0xff);
	}

}

const bool AudioCaptcha1::Generate()
{
	AudioStream noise;
	AudioStream background;
	AudioStream puzzle;
	espeak_ERROR err=EE_OK;
	unsigned int id=0;
	Poco::Path espeakpath("espeak-data");
	std::string savelocale("");

	try
	{
		Poco::File es(espeakpath);
		if(es.exists()==false)
		{
			return false;
		}
	}
	catch(...)
	{
		return false;
	}

	if(setlocale(LC_ALL,0))
	{
		savelocale=std::string(setlocale(LC_ALL,0));
	}

#ifdef _WIN32
	int rate=espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS,100,".\\",0);
#else
	int rate=espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS,100,"./",0);
#endif

	noise.SetSampleRate(rate);
	background.SetSampleRate(rate);
	puzzle.SetSampleRate(rate);

	err=espeak_SetVoiceByName("english-us");
	if(err!=EE_OK)
	{
		if(savelocale!="")
		{
			setlocale(LC_ALL,savelocale.c_str());
		}
		return false;
	}
	espeak_SetSynthCallback(AudioStream::Callback);

	// do puzzle letters
	m_solution.clear();
	int count=(rand()%4)+5;

	puzzle.GetSamples().resize(puzzle.GetSamples().size()+(rate/10));
	for(int i=0; i<count; i++)
	{
		int samplesbefore=ULongRand()%(rate/2);
		puzzle.GetSamples().resize(puzzle.GetSamples().size()+samplesbefore);

		char newchar=(rand()%26)+97;
		std::string charstr(1,newchar);
		m_solution.push_back(newchar);
		espeak_SetParameter(espeakPITCH,(rand()%20)+40,0);
		espeak_SetParameter(espeakRANGE,(rand()%20)+40,0);
		espeak_Synth(charstr.c_str(),1,0,POS_CHARACTER,0,0,&id,&puzzle);
		if(err!=EE_OK)
		{
			if(savelocale!="")
			{
				setlocale(LC_ALL,savelocale.c_str());
			}
			return false;
		}

		int samplesafter=ULongRand()%(rate/2);
		puzzle.GetSamples().resize(puzzle.GetSamples().size()+samplesafter);
	}
	puzzle.GetSamples().resize(puzzle.GetSamples().size()+(rate/10));

	// do background noise now
	espeak_SetParameter(espeakVOLUME,30,0);
	while(background.GetSamples().size()<puzzle.GetSamples().size())
	{
		std::string speak="";
		speak+=(rand()%26)+97;
		espeak_SetParameter(espeakPITCH,(rand()%20)+40,0);
		espeak_SetParameter(espeakRANGE,(rand()%20)+40,0);
		espeak_Synth(speak.c_str(),speak.size(),0,POS_CHARACTER,0,0,&id,&background);
		if(err!=EE_OK)
		{
			if(savelocale!="")
			{
				setlocale(LC_ALL,savelocale.c_str());
			}
			return false;
		}
	}
	
	GenerateNoise(puzzle.GetSamples().size(),noise);
	
	std::vector<AudioStream *> streams;
	streams.push_back(&puzzle);
	streams.push_back(&background);
	streams.push_back(&noise);
	AudioStream result;
	CombineStreams(streams,result);

	result.SetSampleRate(puzzle.GetSampleRate());
	CreateWave(result);

	if(savelocale!="")
	{
		setlocale(LC_ALL,savelocale.c_str());
	}

	return true;

}

void AudioCaptcha1::GenerateNoise(const int numsamples, AudioStream &result)
{
	std::vector<short> &samples=result.GetSamples();
	samples.resize(numsamples,0);
	for(int i=0; i<samples.size(); i++)
	{
		samples[i]=rand();
		samples[i]=samples[i]-(samples[i]*.95);	// attenuate
	}
}

const bool AudioCaptcha1::GetPuzzle(std::vector<unsigned char> &puzzle)
{
	puzzle=m_puzzle;
	return true;
}

const bool AudioCaptcha1::GetSolution(std::vector<unsigned char> &solution)
{
	solution=m_solution;
	return true;
}

const unsigned long AudioCaptcha1::ULongRand() const
{
	unsigned long rnum=0;
	// low bytes don't have good randomness so offset by 4
	rnum|=((rand() >> 4) & 0xff);
	rnum|=((rand() << 4) & 0xff00);
	rnum|=((rand() << 12) & 0xff0000);
	rnum|=((rand() << 20) & 0xff000000);

	return rnum;
}

#endif	// AUDIO_CAPTCHA
