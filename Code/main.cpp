#include <jack/jack.h>
#include <iostream>

int main() {
    jack_status_t jackStatus;
    jack_client_t* jackClient;

    jackClient = jack_client_open("MatiTest", JackUseExactName, &jackStatus);

    if (jackClient == NULL) {
        std::cout << "Failed to create JACK client: status = " << jackStatus << std::endl;
        return 1;
    }

    // Register an input port
    jack_port_t* input_port = jack_port_register(
        jackClient, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0
    );

    if (input_port == NULL) {
        fprintf(stderr, "Failed to register JACK port\n");
        jack_client_close(jackClient);
        return 1;
    }

    printf("JACK client and port created successfully\n");

    // Clean up
    jack_client_close(jackClient);
    return 0;
}
