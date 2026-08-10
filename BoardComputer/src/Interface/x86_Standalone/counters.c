#include "counters.h"
#include "countersfeed.h"
#include <signal.h>
#include <string.h>

volatile uint8_t COUNTERS_signal_enable = 0;

static void fuel_signal_handler(int sig, siginfo_t* info, void* context)
{
    (void)sig;
    (void)context;
    if(info)
        COUNTERSFEED_count_fuelusage((uint16_t)info->si_value.sival_int);
}

static void speed_signal_handler(int sig, siginfo_t* info, void* context)
{
    (void)sig;
    (void)context;
    if(info)
        COUNTERSFEED_count_speed((uint16_t)info->si_value.sival_int);
}

void COUNTERS_enable_signals(void)
{

}

void COUNTERS_disable_signals(void)
{

}

void COUNTERS_init(void)
{
    struct sigaction sa_fuel;
    struct sigaction sa_speed;

    memset(&sa_fuel, 0, sizeof(sa_fuel));
    sa_fuel.sa_sigaction = fuel_signal_handler;
    sa_fuel.sa_flags = SA_SIGINFO;
    sigemptyset(&sa_fuel.sa_mask);
    sigaction(COUNTERS_SIG_FUEL, &sa_fuel, NULL);

    memset(&sa_speed, 0, sizeof(sa_speed));
    sa_speed.sa_sigaction = speed_signal_handler;
    sa_speed.sa_flags = SA_SIGINFO;
    sigemptyset(&sa_speed.sa_mask);
    sigaction(COUNTERS_SIG_SPEED, &sa_speed, NULL);
}
