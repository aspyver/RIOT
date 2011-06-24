#include <shell_commands.h>
#include <stdlib.h>

extern void _id_handler(char* id);

#ifdef MODULE_PS
extern void _ps_handler(char* unused);
#endif

#ifdef MODULE_RTC
extern void _date_handler(char* now);
#endif

#ifdef MODULE_SHT11
extern void _get_temperature_handler(char* unused);
extern void _get_humidity_handler(char* unused);
extern void _get_weather_handler(char* unused);
extern void _set_offset_handler(char* offset);
#endif

#ifdef MODULE_LTC4150
extern void _get_current_handler(char* unused);
extern void _reset_current_handler(char* unused);
#endif

#ifdef MODULE_CC110X
extern void _cc110x_get_set_address_handler(char *addr);
extern void _cc110x_get_set_channel_handler(char *addr);
#endif

#ifdef MODULE_TRANSCEIVER
#ifdef MODULE_CC110X_NG
extern void _cc110x_ng_get_set_address_handler(char *addr);
extern void _cc110x_ng_get_set_channel_handler(char *chan);
extern void _cc110x_ng_send_handler(char *pkt);
extern void _cc110x_ng_monitor_handler(char *mode);
#endif
#endif

const shell_command_t _shell_command_list[] = {
    {"id", "Gets or sets the node's id.", _id_handler},
#ifdef MODULE_PS
    {"ps", "Prints information about running threads.", _ps_handler},
#endif
#ifdef MODULE_RTC
    {"date", "Gets or sets current date and time.", _date_handler},
#endif
#ifdef MODULE_SHT11
    {"temp", "Prints measured temperature.", _get_temperature_handler},
    {"hum", "Prints measured humidity.", _get_humidity_handler},
    {"weather", "Prints measured humidity and temperature.", _get_weather_handler},
	{"offset", "Set temperature offset.", _set_offset_handler},
#endif
#ifdef MODULE_LTC4150
	{"cur", "Prints current and average power consumption.", _get_current_handler},
	{"rstcur", "Resets coulomb counter.", _reset_current_handler},
#endif
#ifdef MODULE_CC110X
    {"addr", "Gets or sets the address for the CC1100 transceiver", _cc110x_get_set_address_handler},
    {"chan", "Gets or sets the channel for the CC1100 transceiver", _cc110x_get_set_channel_handler},
#endif
#ifdef MODULE_TRANSCEIVER
#ifdef MODULE_CC110X_NG
    {"addr", "Gets or sets the address for the CC1100 transceiver", _cc110x_ng_get_set_address_handler},
    {"chan", "Gets or sets the channel for the CC1100 transceiver", _cc110x_ng_get_set_channel_handler},
    {"txtsnd", "Sends a text message to a given node via the CC1100 transceiver", _cc110x_ng_send_handler},
    {"monitor", "Enables or disables address checking for the CC1100 transceiver", _cc110x_ng_monitor_handler},
#endif
#endif
    {NULL, NULL, NULL}
};

