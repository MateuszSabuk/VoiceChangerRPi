#include "jack_wrapper.h"

int start_jack_wrapper(
    JackProcessCallback process_function,
    JackShutdownCallback jack_shutdown_function,
    jack_port_t *&input_port,
    jack_port_t *&output_port,
    jack_client_t *&jack_client
) {
    jack_status_t jack_status;
    jack_options_t jack_options = (jack_options_t)(JackUseExactName | JackServerName);

    jack_client = jack_client_open("MatiSoundModulator", jack_options, &jack_status, "default");

    if (jack_client == nullptr) {
        std::cerr << "Failed to create JACK client: status = " << jack_status << std::endl;
        return 1;
    }

    jack_set_process_callback ( jack_client, process_function, 0 );

    jack_on_shutdown ( jack_client, jack_shutdown_function, 0 );

    // Register an input port
    input_port = jack_port_register(
        jack_client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0
    );

    if (input_port == nullptr) {
        std::cerr << "Failed to register JACK input port" << std::endl;
        return 1;
    }

    // Register an output port
    output_port = jack_port_register(
        jack_client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0
    );

    if (output_port == nullptr) {
        std::cerr << "Failed to register JACK output port" << std::endl;
        return 1;
    }

    if (jack_is_realtime(jack_client)) {
        std::cout << "Jack client is running realtime" << std::endl;
    } else {
        std::cout << "Jack client is NOT running realtime" << std::endl;
    }

    // Activate the jack client
    jack_activate(jack_client);

    const char* capture = "system:capture_1";
    const char* playback = "system:playback_1";

    if (jack_connect(jack_client, capture, jack_port_name(input_port))) {
        std::cerr << "Failed to connect input port to capture" << std::endl;
    }
    if (jack_connect(jack_client, jack_port_name(output_port), playback)) {
        std::cerr << "Failed to connect output port to playback" << std::endl;
    }

    return 0;
}
