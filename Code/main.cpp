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

// Singleton instance of the JackContext
std::unique_ptr<JackContext> g_jack_context;

void cleanup(int signum) {
    if (g_jack_context) {
        std::cout << "Cleaning up JACK resources..." << std::endl;
        g_jack_context.reset();  // Automatically cleans up via the destructor
    }
    exit(0);
}

void daemonize() {
    // First fork: parent exits, child continues as the daemon
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Failed to fork process" << std::endl;
        exit(1);
    }
    if (pid > 0) {
        exit(0);  // Parent exits
    }

    // Create a new session and detach from the controlling terminal
    if (setsid() < 0) {
        std::cerr << "Failed to create a new session" << std::endl;
        exit(1);
    }

    // Second fork: to prevent the daemon from acquiring a controlling terminal
    pid = fork();
    if (pid < 0) {
        std::cerr << "Failed to fork process" << std::endl;
        exit(1);
    }
    if (pid > 0) {
        exit(0);  // Parent exits again
    }

    // Change the working directory to root, or somewhere safe
    if (chdir("/") < 0) {
        std::cerr << "Failed to change directory to /" << std::endl;
        exit(1);
    }

    // Close standard file descriptors and redirect them to /dev/null
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int null_fd = open("/dev/null", O_RDWR);
    if (null_fd < 0) {
        std::cerr << "Failed to open /dev/null" << std::endl;
        exit(1);
    }

    // Redirect stdin, stdout, stderr to /dev/null
    dup2(null_fd, STDIN_FILENO);
    dup2(null_fd, STDOUT_FILENO);
    dup2(null_fd, STDERR_FILENO);
}

int main() {
    daemonize();  // Daemonize the process

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
