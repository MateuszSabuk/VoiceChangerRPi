#include <jack/jack.h>
#include <iostream>
#include <csignal>
#include <memory>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>

// Encapsulation of resources in a struct
struct JackContext {
    jack_client_t* client = nullptr;
    jack_port_t* input_port = nullptr;
    jack_port_t* output_port = nullptr;

    ~JackContext() {
        if (client != nullptr) {
            if (input_port != nullptr) {
                jack_port_unregister(client, input_port);
            }
            if (output_port != nullptr) {
                jack_port_unregister(client, output_port);
            }
            jack_client_close(client);
        }
    }
};


int main() {
    // Singleton instance of the JackContext
    std::unique_ptr<JackContext> jack_context;
    auto cleanup [&jack_context](int signum) {
        if (g_jack_context) {
            std::cout << "Cleaning up JACK resources..." << std::endl;
            g_jack_context.reset();  // Automatically cleans up via the destructor
        }
        exit(0);
    }
    // Set up signal handlers
    signal(SIGTERM, cleanup);
    signal(SIGINT, cleanup);

    // Create and initialize the JackContext
    g_jack_context = std::make_unique<JackContext>();

    jack_status_t jackStatus;
    jack_options_t jackOptions = (jack_options_t)(JackUseExactName | JackServerName);

    g_jack_context->client = jack_client_open("MatiTest", jackOptions, &jackStatus, "default");

    if (g_jack_context->client == nullptr) {
        std::cerr << "Failed to create JACK client: status = " << jackStatus << std::endl;
        return 1;
    }

    // Register an input port
    g_jack_context->input_port = jack_port_register(
        g_jack_context->client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0
    );

    if (g_jack_context->input_port == nullptr) {
        std::cerr << "Failed to register JACK input port" << std::endl;
        return 1;
    }

    // Register an output port
    g_jack_context->output_port = jack_port_register(
        g_jack_context->client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0
    );

    if (g_jack_context->output_port == nullptr) {
        std::cerr << "Failed to register JACK output port" << std::endl;
        return 1;
    }

    if (jack_is_realtime(g_jack_context->client)) {
        std::cout << "Jack client is running realtime" << std::endl;
    } else {
        std::cout << "Jack client is NOT running realtime" << std::endl;
    }

    // Activate the jack client
    jack_activate(g_jack_context->client);

    const char* capture = "system:capture_1";
    const char* playback = "system:playback_1";

    if (jack_connect(g_jack_context->client, capture, jack_port_name(g_jack_context->input_port))) {
        std::cerr << "Failed to connect input port to capture" << std::endl;
    }
    if (jack_connect(g_jack_context->client, jack_port_name(g_jack_context->output_port), playback)) {
        std::cerr << "Failed to connect output port to playback" << std::endl;
    }

    // Main loop - wait indefinitely
    while (true) {
        sleep(60);  // Simulate workload
    }

    // Cleanup handled via signal handler
    return 0;
}
