#include <jack/jack.h>
#include <iostream>

int main() {
    jack_status_t jackStatus;
    // Open a jack client
    jack_client_t* jackClient = jack_client_open("MatiTest", JackUseExactName | JackServerName, &jackStatus, "default");

    if (jackClient == NULL) {
        std::cerr << "Failed to create JACK client: status = " << jackStatus << std::endl;
        return 1;
    }

    // Register an input port
    jack_port_t* input_port = jack_port_register(
        jackClient, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0
    );

    if (input_port == NULL) {
        std::cerr << "Failed to register JACK input port" << std::endl;
        jack_client_close(jackClient);
        return 1;
    }

    // Register an output port
    jack_port_t* output_port = jack_port_register(
        jackClient, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0
    );

    if (output_port == NULL) {
        std::cerr << "Failed to register JACK output port" << std::endl;
        jack_client_close(jackClient);
        return 1;
    }

    if (jack_is_realtime(jackClient)) {
        std::cout << "Jack client is running realtime" << std::endl;
    } else {
        std::cout << "Jack client is NOT running realtime" << std::endl;
    }

    // Activate the jack connections
    jack_activate(jackClient);

    const char* capture = "system:capture_1";
    const char* playback = "system:playback_1";

    if (jack_connect(jackClient, capture, jack_port_name(input_port))) {
        std::cerr << "Failed to connect input port to capture" << std::endl;
    }
    if (jack_connect(jackClient, jack_port_name(output_port), playback)) {
        std::cerr << "Failed to connect output port to playback" << std::endl;
    }

    // Pause
    std::cout << "Press enter to end...";
    std::cin.get();

    // Clean up
    jack_client_close(jackClient);
    return 0;
}
