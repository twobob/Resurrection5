/* Will 18, 2013. License GPL3 */
/* wj18@talktalk.net */

#include <math.h>
#include <string.h>
#include <sndfile.h>	/* <stdio.h> included by sndfile  */

#define         MAX_CHANNELS    1
#define         BUFFER_LEN      8192
#define		MAXDATASIZE	32767

static sf_count_t expand (sf_count_t endFrameNum, SNDFILE *infile, SNDFILE *outfile, double *data, double maxamp);
static double findMax (sf_count_t frames, SNDFILE *infile, double *data);

int main(int argc, char *argv[])
{
	const char *infilename, *outfilename;
	const char *string;
	char name[256];
	double maxAmp;
	int count, i;
	sf_count_t frames;
	static double data [BUFFER_LEN];
	SNDFILE *infile = NULL;
	SNDFILE *outfile = NULL;
	SF_INFO sfinfo;
	unsigned int flag;

	if (argv[1] == NULL || argc==1)
	{
		printf("usage: %s <soundfile> or %s -h for help\n", argv[0], argv[0]);
		return 0;
	}

	count = 1; 	// opts start at argv[1]

	if (strlen(argv[count])==1)	// only 1st char '-'
	{
		printf ("option unspecified\n");
		return 0;	// no option specified
	}

	flag = 1;	// set 'open file'

	while (count<argc)
	{
		string = argv[count];
		i = 0;

		if (string[i]=='-')
		{
			i++;
			
			if (string[i]=='h')
			{
				printf("dehuman <sndfile_in> <sndfile_out>\n");
				printf("sndfile_out is 16-bit WAV format, default 'out.wav'\n");
				printf("options:\n");
				printf("-a	about this program\n");
				printf("-h	help (this info)\n");
				return 0;
			}
			
			if (string[i]=='a')
			{
				printf("dehuman: eliminate performance variation in a note.\n");
				printf("author:  Will 18, 2013\n");
				printf("contact: wj18@talktalk.net\n");
				printf("license: GPL3\n");
				return 0;
			}
		}

		count++;
	}

	if (!flag)
		return 1;

	if (argc < 3)
		strcpy (name, "out.wav");
	else
		strcpy (name, argv[2]);
	
	sfinfo.format = 0;
	infilename = argv[1];
	infile = sf_open(infilename, SFM_READ, &sfinfo);

	if (!infile)
	{
		printf ("Can't open file %s\n", infilename);
		puts (sf_strerror (NULL));
		return 1;
	}

	if (sfinfo.channels > MAX_CHANNELS)
	{	// we don't suppress this since stereo files fail
		printf ("Mono input files only; channels should be 1\n");
		printf ("%s has %d channels\n", infilename, sfinfo.channels);
		return  1 ;
        } ;

	frames = sfinfo.frames;
	maxAmp = findMax (frames-1, infile, data);
	outfile = sf_open(name, SFM_WRITE, &sfinfo);
	expand (frames-1, infile, outfile, data, maxAmp);
	sf_close (outfile);
	sf_close (infile);
	return 0;
}

static double
findMax (sf_count_t frames, SNDFILE *infile, double *data)
{
	sf_count_t bufSize;
	int readcount, flag, count, i;
	double temp, amp;

	bufSize = (frames<BUFFER_LEN ? frames : BUFFER_LEN);
	count = 0;
	amp = 0;
	flag = 0;
	sf_seek (infile, 0L, SEEK_SET);

	while (readcount = sf_read_double (infile, data, bufSize) && flag == 0)
	{
		for (i=0; i<bufSize; i++)
		{
			temp = sqrt(data[i]*data[i]);

			if (temp>amp)
			{
				if (temp<=1)
					amp = temp;
				else
				{
					i = bufSize;
					flag = 1;
				}
			}

			if (count+i == frames)
			{
				flag = 1;
				i = bufSize;
			}
		}
		count = count+readcount;
	}
	return amp;
}

static sf_count_t
expand (sf_count_t frames, SNDFILE *infile, SNDFILE *outfile, double *data, double maxamp)
{
	sf_count_t bufSize;
	int readcount, count, i;
	double temp, amp;
	short outData;

	bufSize = (frames<BUFFER_LEN? frames : BUFFER_LEN);
	count = 0;
	amp = 0;	// new peak value
	sf_seek (infile, 0L, SEEK_SET);
	sf_seek (outfile, 0L, SEEK_SET);

	while (readcount = sf_read_double (infile, data, bufSize))
	{
		for (i=0; i<bufSize; i++)
		{
			temp = sqrt(data[i]*data[i]);	// amp env
			temp = temp*-1;
			temp = temp+maxamp;
			temp = temp*data[i];

			if (temp > amp)
				amp = temp;
		}
		count = count+readcount;
	}

	sf_seek (infile, 0L, SEEK_SET);
	count = 0;
	
	while (readcount = sf_read_double (infile, data, bufSize))
	{
		for (i=0; i<bufSize; i++)
		{
			temp = sqrt(data[i]*data[i]);	// amp env
			temp = temp*-1;
			temp = temp+maxamp;
			temp = (MAXDATASIZE*temp+1)*data[i];
			temp = temp*maxamp/amp;
			outData = (short) temp;
			sf_writef_short (outfile, &outData, 1);
		}
		count += readcount;
	}
}
