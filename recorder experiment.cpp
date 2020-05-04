#include <iostream>
#include <unistd.h> // using sleep
#include <string>

#include <jack/jack.h>

struct Input {
    Input(jack_port_t *input) : volume(1), arm(true), mute(false), input(input) {}
    jack_port_t *input;
    float volume;
    bool arm;
    bool mute;
};

struct Input **inputs;
jack_port_t *main_out;
jack_port_t *monitor_out;
jack_client_t *client;

const unsigned int n_tracks = 5;

/**
 * Gets called every time audio needs to be processed.
 * Copys all audio inputs to the outputs
 */
int process(jack_nframes_t nframes, void *arg) {
    size_t n_samples = (sizeof(jack_default_audio_sample_t) * nframes) / sizeof(float);

    // get outputs
    float *main_out_buffer = (float *)jack_port_get_buffer(main_out, nframes);
    float *monitor_out_buffer = (float *)jack_port_get_buffer(monitor_out, nframes);
    
    for (int i = 0; i < n_samples; i++) {
        main_out_buffer[i] = 0;
        monitor_out_buffer[i] = 0;
    }

    // get inputs and add them to outputs
    for (int track = 0; track < n_tracks; track++) {
        
        if (inputs[track]->volume > 0 && inputs[track]->arm && !inputs[track]->smute) {
            float *in_sample = (float *)jack_port_get_buffer(inputs[track]->input, nframes);

            // add input buffer to output buffers
            for (int i = 0; i < n_samples; i++) {
                main_out_buffer[i] += in_sample[i] * inputs[track]->volume;
                monitor_out_buffer[i] += in_sample[i] * inputs[track]->volume;
            }
        }
    }

    return 0;
}


/**
 * If JACK shuts down, exit the program. TODO: don't exit the program
 */
void jack_shutdown(void *arg)
{
    exit(1);
}

int main(int argc, char *argv[])
{
    //const char **ports;
    const char *client_name = "JACK audio recorder (JAR)";
    const char *server_name = NULL;
    jack_options_t options = JackNullOption;
    jack_status_t status;

    client = jack_client_open(client_name, options, &status, server_name);
    if (client == NULL) {
        std::cout << "Cant connect to JACK" << std::endl;
        exit(1);
    }

    jack_set_process_callback(client, process, 0);
    jack_on_shutdown (client, jack_shutdown, 0);
    
    // register inputs
    bool all_inputs_connected = true;
    inputs = new struct Input*[n_tracks];
    for (unsigned int i = 0; i < n_tracks; i++) {
        std::string track_name = "track " + std::to_string(i + 1) + ": [name of track]";
        inputs[i] = new struct Input(jack_port_register(client, track_name.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0));
        if (inputs[i]->input == NULL) {
            all_inputs_connected = false;
        }
    }

    // register outputs
    main_out = jack_port_register(client, "main out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    monitor_out = jack_port_register(client, "monitor out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    // test to naje sure all inputs and outputs are working
    if (!all_inputs_connected || (main_out == NULL)  || (monitor_out == NULL)) {
		std::cout << "Can't connect. No more JACK ports available" << std::endl;
		exit(1);
	}
    
    // activate jack
    if (jack_activate(client)) {
		std::cout << "Can't activate client" << std::endl;
		exit(1);
	}

    /* keep running until stopped by the user */
	sleep (-1);

    jack_client_close(client);
    
    return 0;
}