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
#include <cmath>
#include <vector>
#include <algorithm> // For std::max_element
#include <fftw3.h>   // FFTW library
#include "jack_wrapper.h"
#include "lpf.h"

#define BUFFER_SIZE 4096

float volume_value = 1;
float pitch_value = 1;
double lpf_cutoffFrequency = 1000;

LowPassFilter lpf;

jack_port_t *input_port;
jack_port_t *output_port;
jack_client_t *jack_client;

static void signal_handler ( int sig )
{
    jack_client_close ( jack_client );
    fprintf ( stderr, "signal received, exiting ...\n\r" );
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

    // Write data into the ring buffer and process for output
    for (unsigned int i = 0; i < nframes; ++i) {
        ring_buffer[write_index] = in[i] * volume_value;
        write_index = (write_index + 1) % BUFFER_SIZE;
    }

    // Output from ring buffer
    for (unsigned int i = 0; i < nframes; i++) {
        out[i] = lpf.next(ring_buffer[int(read_index)]);
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
        case int('a'):
        case int('A'):
            lpf_cutoffFrequency += 100;
            lpf_cutoffFrequency = lpf_cutoffFrequency > 20000 ? 20000 : lpf_cutoffFrequency;
        case int('z'):
        case int('Z'):
            lpf_cutoffFrequency -= 100;
            lpf_cutoffFrequency = lpf_cutoffFrequency <= 10 ? 10 : lpf_cutoffFrequency;
            lpf.set_parameters(jack_get_sample_rate(jack_client),lpf_cutoffFrequency);
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

    int jack_status = start_jack_wrapper(process, jack_shutdown, input_port, output_port, jack_client);
    if (jack_status != 0) return jack_status;

    lpf.set_parameters(jack_get_sample_rate(jack_client),500);

    // Start the key press listener in a separate thread
    std::thread key_listener_thread(key_press_listener);

    // Main loop - wait indefinitely
    while (1)
    {
        sleep ( 1 );
    }

    jack_client_close ( jack_client );
    return 0;
}
