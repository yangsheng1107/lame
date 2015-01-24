// tar zxvf lame-3.99.5.tar.gz
// ./configure
// make

// ./bin/lame -e -i file_mono.pcm -o file_mono.mp3
// ./bin/lame -d -i file_mono.mp3 -o file_mono1.pcm
// ./bin/lame -e -i file_stereo.pcm -o file_stereo.mp3
// ./bin/lame -d -i file_stereo.mp3 -o file_stereo1.pcm
//
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lame.h"

//#define LAME_MONO

typedef int (* CALLBACK) (const char* from, const char* to); 

int encode(const char* from, const char* to)
{
    int read, write;
	FILE *pcm, *mp3;
	lame_t lame;
    const int PCM_SIZE = 8192;
    const int MP3_SIZE = 1.25*8192 + 7200; 
#ifdef LAME_MONO
    short int pcm_buffer[PCM_SIZE];
#else
    short int pcm_buffer[PCM_SIZE*2];
#endif		
    unsigned char mp3_buffer[MP3_SIZE];

	printf ("start encoding from %s to %s.\n", from, to);
    pcm = fopen(from, "rb");
    if (pcm == NULL) {
        printf("open %s failure! \n", from);
		return -1;
    }
    mp3 = fopen(to, "wb");
    if (mp3 == NULL) {
        printf("open %s failure! \n", to);
        fclose(pcm);
		return -1;
    }	

    lame = lame_init();
    lame_set_in_samplerate(lame, 8000);
    lame_set_brate(lame,128);	//8k 16bit => bps = 8k*16=128
#ifdef LAME_MONO
    lame_set_num_channels(lame, 1);
    lame_set_mode(lame, MONO);
#else
    lame_set_num_channels(lame, 2);
    lame_set_mode(lame, STEREO);
#endif		

    lame_init_params(lame);

    do {
#ifdef LAME_MONO
        read = fread(pcm_buffer, sizeof(short int), PCM_SIZE, pcm);
#else
        read = fread(pcm_buffer, 2*sizeof(short int), PCM_SIZE, pcm);
#endif	
        if (read == 0)
            write = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
        else
#ifdef LAME_MONO
			write = lame_encode_buffer(lame, pcm_buffer, NULL, read, mp3_buffer, MP3_SIZE);
#else
            write = lame_encode_buffer_interleaved(lame, pcm_buffer, read, mp3_buffer, MP3_SIZE);
#endif

        fwrite(mp3_buffer, write, 1, mp3);
    } while (read != 0);

    lame_close(lame);
    fclose(mp3);
    fclose(pcm);

    return 0;
}

int decode(const char* from, const char* to)
{
	const int MP3_SIZE = 1024;
	const int PCM_SIZE = 1152;
	int read, samples;
	short int pcm_l[PCM_SIZE], pcm_r[PCM_SIZE];
	unsigned char mp3_buffer[MP3_SIZE];	
	FILE *pcm, *mp3;
	
	printf ("start decoding from %s to %s.\n", from, to);
    mp3 = fopen(from, "rb");
	if(mp3 == NULL)
	{
		printf("failed to open file %s\n", from);
		return -1;
	}
    pcm = fopen(to, "wb");
	if(pcm == NULL)
	{
		printf("failed to open file %s\n", to);
		fclose(mp3);
		return -1;
	}

	//init lame
    lame_t lame = lame_init();
	lame_set_decode_only(lame, 1);
	lame_init_params(lame);

	//init decoder
	hip_t hip = hip_decode_init();
   
    while ((read = fread(mp3_buffer, 1, 210, mp3)) > 0)  
    {  
        samples = hip_decode(hip, mp3_buffer, read, pcm_l, pcm_r);  
        if (samples > 0)  
        {  
#ifdef LAME_MONO
            fwrite(pcm_l, sizeof(short), samples, pcm);
#else
			int i;
			for(i = 0; i < samples; i++)
			{
				fwrite((char*)&pcm_l[i], 1, sizeof(pcm_l[i]), pcm);
				fwrite((char*)&pcm_r[i], 1, sizeof(pcm_r[i]), pcm);
			}
#endif
        }  
    }
	
	hip_decode_exit(hip);
    lame_close(lame);
    fclose(mp3);
    fclose(pcm);
	
	return 0;
}

int main (int argc, char ** argv)
{
	CALLBACK func;
	char *from, *to;
    char c;

	if(argc < 2)
	{
		printf ("Usage: %s [-e:pcm 2 mp3][-d:mp3 2 pcm][-i <inputpath>][-o <outputpath>].\n", argv[0]);
		return 1;
	}
	
    while (1) {
        c = getopt (argc, argv, "edi:o:");
        if (c == -1) {
            /* We have finished processing all the arguments. */
            break;
        }
        switch (c) {
        case 'e':
			func = encode;
            break;
        case 'd':
			func = decode;
            break;			
        case 'i':
			from = optarg;
            break;
        case 'o':
			to = optarg;
            break;			
        case '?':
        default:
			//break;
            printf ("Usage: %s [-e:pcm 2 mp3][-d:mp3 2 pcm][-i <inputpath>][-o <outputpath>].\n", argv[0]);
        }
    }
	
	func(from, to);

    return 0;
}


/*
int decode(const char* from, const char* to)
{
	const int MP3_SIZE = 1024;
	const int PCM_SIZE = 1152;

	int read, i, samples;
	
	printf ("start decoding from %s to %s.\n", from, to);
    FILE *mp3 = fopen(from, "rb");
	if(mp3 == NULL)
	{
		printf("failed to open file %s\n", from);
		return -1;
	}
    FILE *pcm = fopen(to, "wb");
	if(pcm == NULL)
	{
		printf("failed to open file %s\n", to);
		fclose(mp3);
		return -1;
	}

	short int pcm_l[PCM_SIZE], pcm_r[PCM_SIZE];
	unsigned char mp3_buffer[MP3_SIZE];

	//init lame
    lame_t lame = lame_init();
	lame_set_decode_only(lame, 1);
	lame_init_params(lame);

	//init decoder
	hip_t hip = hip_decode_init();

	mp3data_struct mp3data, mp3data_bak;
	memset(&mp3data, 0, sizeof(mp3data));
	memcpy(&mp3data_bak, &mp3data, sizeof(mp3data));

	int nChannels = -1;
	int nSampleRate = -1;
	int mp3_len, debug_count, read_count = 0, total_frames = 0;
    
	while( (read = fread(mp3_buffer, 1, MP3_SIZE, mp3)) > 0 )
	{
		read_count++;
		mp3_len = read;
		debug_count = 0;
		do 
		{
			samples = hip_decode1_headers(hip, mp3_buffer, mp3_len, pcm_l, pcm_r, &mp3data);
			if(mp3data.header_parsed == 1)
			{
				if(nChannels < 0)
				{
					printf("header parsed. channels=%d, samplerate=%d\n", mp3data.stereo, mp3data.samplerate);
				}
				else
				{
					if(nChannels != mp3data.stereo || nSampleRate != mp3data.samplerate)
					{
						printf("channels changed. channels=%d->%d, samplerate=%d->%d\n", 
							nChannels, mp3data.stereo, nSampleRate, mp3data.samplerate);
					}
				}
				nChannels = mp3data.stereo;
				nSampleRate = mp3data.samplerate;			
			}
			if(samples > 0 && mp3data.header_parsed != 1)
			{
				fprintf(stderr, "lame decode error, samples=%d, but header not parsed yet\n", samples);
				break;
			}
			
			if(samples > 0)
			{
				for(i = 0; i < samples; i++)
				{
					fwrite((char*)&pcm_l[i], 1, sizeof(pcm_l[i]), pcm);
					if(nChannels == 2)
					{
						fwrite((char*)&pcm_r[i], 1, sizeof(pcm_r[i]), pcm);
					}
				}
				debug_count++;

				if(samples % mp3data.framesize != 0)
				{
					printf("error: not mod samples. samples=%d, framesize=%d\n", samples, mp3data.framesize);
				}
				total_frames += (samples / mp3data.framesize);
			}


			mp3_len = 0;

		} while (samples > 0);

		if(debug_count >= 10)
		{
			printf("debug_count=%d, read_count=%d\n", debug_count, read_count);
		}
	}

	printf("total frames = %d\n", total_frames);
	
	hip_decode_exit(hip);
    lame_close(lame);
    fclose(mp3);
    fclose(pcm);
	
	return 0;
}
*/
