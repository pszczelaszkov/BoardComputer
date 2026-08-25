#include "adc.h"
#include "sensorsfeed.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define ADC_HW_CHANNEL_COUNT 8
uint16_t ADC;
uint8_t  ADMUX, ADCSRA;

static int adc_fds[ADC_HW_CHANNEL_COUNT];

static const char* bc_dir_path(void)
{
	const char* dir = getenv("BC_DIR");
	if(dir && dir[0] != '\0')
		return dir;
	return ".";
}

static int open_adc_channel(const char* dir, unsigned channel)
{
	char path[512];
	int n = snprintf(path, sizeof(path), "%s/ADC%u", dir, channel);
	if(n < 0 || (size_t)n >= sizeof(path))
		return -1;

	int fd = open(path, O_RDWR | O_CREAT | O_NONBLOCK, 0644);
	if(fd < 0)
		return -1;

	struct stat st;
	if(fstat(fd, &st) == 0 && S_ISREG(st.st_mode) && st.st_size == 0)
		(void)write(fd, "0\n", 2);

	return fd;
}

uint8_t ADC_get_current_channel(void)
{
	return (uint8_t)(ADMUX & 0x0f);
}

void ADC_clear_multiplexer(void)
{
	ADMUX &= (uint8_t)~0x0f;
}

void ADC_increase_multiplexer(void)
{
	ADMUX++;
}

static uint16_t read_adc_file(unsigned channel)
{
	char buf[32];
	ssize_t n;
	char* end;
	unsigned long value;

	if(channel >= ADC_HW_CHANNEL_COUNT || adc_fds[channel] < 0)
		return 0;

	if(lseek(adc_fds[channel], 0, SEEK_SET) < 0)
		return 0;

	n = read(adc_fds[channel], buf, sizeof(buf) - 1);
	if(n <= 0)
		return 0;

	buf[n] = '\0';
	errno = 0;
	value = strtoul(buf, &end, 10);
	if(end == buf || errno == ERANGE)
		return 0;
	if(value > 1023)
		value = 1023;
	return (uint16_t)value;
}

void ADC_start_conversion(void)
{
	uint8_t channel = ADC_get_current_channel();
	ADC = read_adc_file(channel);
	SENSORSFEED_push_adc_value();
}

void ADC_init(void)
{
	const char* dir = bc_dir_path();
	unsigned i;

	printf("ADC sim directory: %s\n", dir);

	if(mkdir(dir, 0755) < 0 && errno != EEXIST)
		perror(dir);

	for(i = 0; i < ADC_HW_CHANNEL_COUNT; i++)
	{
		adc_fds[i] = -1;
		adc_fds[i] = open_adc_channel(dir, i);
		if(adc_fds[i] < 0)
			perror("ADC channel open");
	}

	ADMUX = 0;
	ADCSRA = 0;
	ADC = 0;
}
