#ifndef _JACKWRAPPER_H_
#define _JACKWRAPPER_H_

#include <jack/jack.h>
#include <iostream>

int start_jack_wrapper(
    JackProcessCallback process_function,
    JackShutdownCallback jack_shutdown_function,
    jack_port_t *&input_port,
    jack_port_t *&output_port,
    jack_client_t *&jack_client
);

#endif