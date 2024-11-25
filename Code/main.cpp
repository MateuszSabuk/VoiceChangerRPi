#include <jack/jack.h>
#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <vector>
#include <thread> // For capturing key presses
#include <ncurses.h> // For capturing key presses

#define BUFFER_SIZE 8192

float volume_value = 1;
float pitch_value = 1;

jack_port_t *input_port;
jack_port_t *output_port;
jack_client_t *client;

static void signal_handler ( int sig )
{
    jack_client_close ( client );
    fprintf ( stderr, "signal received, exiting ...\n" );
    exit ( 0 );
}

void jack_shutdown ( void *arg )
{// For capturing key presses
    free ( input_port );
    free ( output_port );
    exit ( 1 );
}

std::vector<jack_default_audio_sample_t> ring_buffer(BUFFER_SIZE);
std::mutex ring_mutex; // Mutex to ensure thread-safe access to the ring buffer
size_t write_index = 0;
float read_index = 0;

int process(jack_nframes_t nframes, void *arg)
{
    // Lock the mutex to safely access the ring buffer
    std::lock_guard<std::mutex> lock(ring_mutex);

    // Access the input and output ports
    jack_default_audio_sample_t *in, *out;
    in = static_cast<jack_default_audio_sample_t *>(jack_port_get_buffer(input_port, nframes));
    out = static_cast<jack_default_audio_sample_t *>(jack_port_get_buffer(output_port, nframes));

    // Write data into the ring buffer
    for (unsigned int i = 0; i < nframes; ++i) {
        ring_buffer[write_index] = in[i] * volume_value;
        write_index = (write_index + 1) % BUFFER_SIZE;
    }

    // Read data from the ring buffer and write it to the output
    for (unsigned int i = 0; i < nframes; i++) {
        out[i] = ring_buffer[int(read_index)];
        read_index += pitch_value;
        if (read_index >= BUFFER_SIZE) read_index -= BUFFER_SIZE;
    }

    return 0;
}

// Thread function to handle key presses
void key_press_listener()
{
    initscr();             // Initialize ncurses
    cbreak();              // Disable line buffering
    noecho();              // Disable echoing of characters
    keypad(stdscr, TRUE);  // Enable function and arrow keys
    timeout(100);          // Non-blocking input

    while (true) {
        int ch = getch();
        switch (ch) {
        case KEY_UP:
            volume_value += 0.05;
            break;
        case KEY_DOWN:
            volume_value -= 0.05;
            break;
        case KEY_LEFT:
            pitch_value -= 0.01;
            break;
        case KEY_RIGHT:
            pitch_value += 0.01;
            break;
        default:
            break;
        }
    }
}

int main ( int argc, char *argv[] )
{   
   /* install a signal handler to properly quits jack client */
    signal ( SIGQUIT, signal_handler );
    signal ( SIGTERM, signal_handler );
    signal ( SIGHUP, signal_handler );
    signal ( SIGINT, signal_handler );


    jack_status_t jack_status;
    jack_options_t jack_options = (jack_options_t)(JackUseExactName | JackServerName);

    client = jack_client_open("MatiTest", jack_options, &jack_status, "default");

    if (client == nullptr) {
        std::cerr << "Failed to create JACK client: status = " << jack_status << std::endl;
        return 1;
    }

    jack_set_process_callback ( client, process, 0 );

    jack_on_shutdown ( client, jack_shutdown, 0 );

    // Start the key press listener in a separate thread
    std::thread key_listener_thread(key_press_listener);

    // Register an input port
    input_port = jack_port_register(
        client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0
    );

    if (input_port == nullptr) {
        std::cerr << "Failed to register JACK input port" << std::endl;
        return 1;
    }

    // Register an output port
    output_port = jack_port_register(
        client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0
    );

    if (output_port == nullptr) {
        std::cerr << "Failed to register JACK output port" << std::endl;
        return 1;
    }

    if (jack_is_realtime(client)) {
        std::cout << "Jack client is running realtime" << std::endl;
    } else {
        std::cout << "Jack client is NOT running realtime" << std::endl;
    }

    // Activate the jack client
    jack_activate(client);

    const char* capture = "system:capture_1";
    const char* playback = "system:playback_1";

    if (jack_connect(client, capture, jack_port_name(input_port))) {
        std::cerr << "Failed to connect input port to capture" << std::endl;
    }
    if (jack_connect(client, jack_port_name(output_port), playback)) {
        std::cerr << "Failed to connect output port to playback" << std::endl;
    }

    // Main loop - wait indefinitely
    while (1)
    {
    #ifdef WIN32
            Sleep ( 1000 );
    #else
            sleep ( 1 );
    #endif
    }

    jack_client_close ( client );
    return 0;
}
