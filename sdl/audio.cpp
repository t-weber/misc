/**
 * sdl audio test
 * @author Tobias Weber
 * @date 19-feb-2023
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *  * https://wiki.libsdl.org/SDL2/SDL_AudioSpec
 *  * https://wiki.libsdl.org/SDL2/SDL_OpenAudioDevice
 *
 * g++ -std=c++20 -Wall -Wextra -Weffc++ -o audio audio.cpp -lSDL2
 */

#include <SDL2/SDL.h>

#include <cmath>
#include <numbers>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>


using t_audio = float;


/**
 * pythagorean tuning
 * @see https://en.wikipedia.org/wiki/Pythagorean_tuning
 */
template<class t_real = double>
std::vector<t_real> get_pythagorean_tuning(t_real base_freq)
{
	// octave
	const t_real order2_freq = t_real(2) * base_freq;

	std::vector<t_real> tuning;
	tuning.push_back(base_freq);

	// up from base frequency
	t_real freq = base_freq;
	for(int i=0; i<5; ++i)
	{
		freq *= t_real(3)/t_real(2);
		if(freq > order2_freq)
			freq *= t_real(0.5);

		tuning.push_back(freq);
	}

	// down from second order
	freq = order2_freq;
	for(int i=0; i<1; ++i)
	{
		freq *= t_real(2)/t_real(3);
		if(freq < base_freq)
			freq *= t_real(2);

		tuning.push_back(freq);
	}

	tuning.push_back(order2_freq);

	std::sort(tuning.begin(), tuning.end());
	return tuning;
}


/**
 * equal tuning
 * @see https://en.wikipedia.org/wiki/Equal_temperament
 */
template<class t_real = double>
std::vector<t_real> get_equal_tuning(t_real base_freq, bool all_keys = false)
{
	// halftone step
	const t_real step = std::pow(t_real(2), 1./12.);
	// octave
	const t_real order2_freq = t_real(2) * base_freq;

	std::vector<t_real> tuning;
	tuning.push_back(base_freq);

	t_real freq = base_freq;
	for(int i=0; i<11; ++i)
	{
		freq *= step;

		// skip black piano keys?
		if(!all_keys && (i==0 || i==2 || i==5 || i==7 || i==9))
			continue;

		tuning.push_back(freq);
	}

	tuning.push_back(order2_freq);
	return tuning;
}



/**
 * prints the audio data format
 * @see https://wiki.libsdl.org/SDL2/SDL_AudioFormat
 */
std::string print_audioformat(SDL_AudioFormat fmt)
{
	std::ostringstream ostr;

	if(fmt & (1<<15))
		ostr << "signed ";
	else
		ostr << "unsigned ";

	if(fmt & (1<<12))
		ostr << "big endian ";
	else
		ostr << "little endian ";

	if(fmt & (1<<8))
		ostr << "float";
	else
		ostr << "int";

	ostr << (fmt & 0xff);

	return ostr.str();
}


/**
 * adds a sine sample
 */
bool queue_sine_samples(SDL_AudioDeviceID audio_dev, const SDL_AudioSpec* audio_spec, std::size_t num_samples,
	t_audio freq, t_audio* _last_phase = nullptr)
{
	t_audio *samples = new t_audio[num_samples];
	//memset(samples, 0, num_samples*sizeof(t_audio));

	t_audio last_phase = _last_phase ? *_last_phase : 0;
	int num_channels = audio_spec->channels;

	for(std::size_t idx=0; idx<num_samples; idx+=num_channels)
	{
		t_audio amp = 0.75;
		t_audio phi = t_audio(2) * std::numbers::pi_v<t_audio> * t_audio(idx) / t_audio(num_channels);
		t_audio phase = phi * freq / t_audio(audio_spec->freq) + last_phase;

		for(int chan=0; chan<num_channels; ++chan)
			samples[idx + chan] = amp * std::sin(phase);

		// save last phase to avoid phase jumps for multiple queued sine waves
		if(_last_phase && idx == num_samples-num_channels)
			*_last_phase = phase;
	}

	int ret = SDL_QueueAudio(audio_dev, samples, sizeof(t_audio)*num_samples);
	delete[] samples;

	return ret >= 0;
}


/**
 * opens an audio device
 */
std::pair<SDL_AudioDeviceID, SDL_AudioSpec> create_audio_dev(int freq, int channels, SDL_AudioFormat fmt)
{
	SDL_AudioSpec audiospec, audiospec_req
	{
		.freq = freq,
		.format = fmt,
		.channels = Uint8(channels),
		.silence = 0,
		.samples = 1<<12,
		.padding = 0,
		.size = 0,
		.callback = nullptr,
		.userdata = nullptr,
	};

	SDL_AudioDeviceID audiodev = SDL_OpenAudioDevice(nullptr, 0, &audiospec_req, &audiospec, SDL_AUDIO_ALLOW_ANY_CHANGE);

	// clear previous buffers
	SDL_PauseAudioDevice(audiodev, SDL_TRUE);
	SDL_ClearQueuedAudio(audiodev);

	return std::make_pair(audiodev,std::move(audiospec));
}


int main()
{
	if(SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		std::cerr << SDL_GetError() << std::endl;
		return -1;
	}

	auto [audiodev, audiospec] = create_audio_dev(44100, 2, AUDIO_F32);
	if(audiodev == 0)
	{
		std::cerr << SDL_GetError() << std::endl;
		return -1;
	}

	std::cout << audiospec.freq << " Hz, "
		<< int(audiospec.channels) << " channels, "
		<< audiospec.samples << " samples, "
		<< "size: " << audiospec.size << ", "
		<< "silence: " << int(audiospec.silence) << ", "
		<< "padding: " << int(audiospec.padding) << ", "
		<< "format: " << print_audioformat(audiospec.format)
		<< std::endl;

	// play tuning tones
	//std::vector<t_audio> tuning = get_pythagorean_tuning<t_audio>(330);
	std::vector<t_audio> tuning = get_equal_tuning<t_audio>(330);

	t_audio last_phase = 0;
	t_audio seconds = 0.5;
	for(t_audio freq : tuning)
	{
		if(!queue_sine_samples(audiodev, &audiospec, audiospec.freq*seconds*audiospec.channels, freq, &last_phase))
		{
			std::cerr << SDL_GetError() << std::endl;
			return -1;
		}
	}

	SDL_PauseAudioDevice(audiodev, SDL_FALSE);
	for(std::size_t idx=0; idx<tuning.size(); ++idx)
	{
		t_audio freq = tuning[idx];
		std::cout << idx << ": " << freq << " Hz";
		if(idx > 0)
			std::cout << " = freq[" << idx-1 << "] * " << freq / tuning[idx-1];
		if(idx > 1)
			std::cout << " = freq[0] * " << freq / tuning[0];
		std::cout << std::endl;
		SDL_Delay(seconds * 1000);
	}

	// clean up
	SDL_PauseAudioDevice(audiodev, SDL_TRUE);
	SDL_CloseAudioDevice(audiodev);
	SDL_Quit();

	return 0;
}
