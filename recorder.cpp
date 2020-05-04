#include <iostream>
#include <string>
#include "recorder.h"
#include <vector>
#include <functional>

#include <jack/jack.h>

// a pointer to the current JackController instance. Currently only one instance of JackController can run at a time because of this
JackController *controller;

/**
 * Static funtion needed for jack. This is just a proxy for JackController::process.
 * There is probably a better way to do this but I dont know it.
 * This means only 1 instance of JackController can run at a time unless I convert controller to a vector.
 */
int call_process(jack_nframes_t nframes, void *arg)
{
    return controller->process(nframes, arg);
}

/**
 * Initialiser for JackController.
 * Connects to the JACK session and sets up outputs.
 */
JackController::JackController()
{
    controller = this;
    const char *client_name = "JACK audio recorder (JAR)";
    const char *server_name = NULL;
    jack_options_t options = JackNullOption;
    jack_status_t status;

    client = jack_client_open(client_name, options, &status, server_name);
    if (client == NULL) {
        std::cout << "Cant connect to JACK" << std::endl;
        exit(1);
    }

    jack_set_process_callback(client, call_process, 0);
    //TODO: jack_on_shutdown (client, jack_shutdown, 0);
    
    // register inputs
    inputs = std::vector<struct Input>();

    // register outputs
    main_out = jack_port_register(client, "main out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    monitor_out = jack_port_register(client, "monitor out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    // test to make sure all the outputs are working
    if ((main_out == NULL)  || (monitor_out == NULL)) {
		std::cout << "Can't connect. No more JACK ports available" << std::endl;
		exit(1);
	}
    
    // activate jack
    if (jack_activate(client)) {
		std::cout << "Can't activate client" << std::endl;
		exit(1);
	}
}


/*
 * Disconnects from JACK session.
 */
JackController::~JackController()
{
    jack_client_close(client);
}

/*
 * Adds a track to the recording with default properties
 */
bool JackController::add_track()
{
    std::string track_name = "Track " + std::to_string(inputs.size() + 1) + ": [name of track]";
    struct Input new_input = Input(jack_port_register(client, track_name.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0));
    if (new_input.input == NULL) {
        std::cout << "Can't make new input. No more JACK ports available" << std::endl;
        return false;
    }
    inputs.push_back(new_input);
    return true;
}

/**
 * Processes the sound coming from JACK.
 * Takes all the inputs and mixes them and sends the output to all the JACK outputs
 * TODO: also send audio to disk wright buffer for saving
 */
int JackController::process(jack_nframes_t nframes, void *arg)
{
    size_t n_samples = (sizeof(jack_default_audio_sample_t) * nframes) / sizeof(float);

    // get outputs
    float *main_out_buffer = (float *)jack_port_get_buffer(main_out, nframes);
    float *monitor_out_buffer = (float *)jack_port_get_buffer(monitor_out, nframes);
    
    for (int i = 0; i < n_samples; i++) {
        main_out_buffer[i] = 0;
        monitor_out_buffer[i] = 0;
    }

    // get inputs and add them to outputs
    for (struct Input track : inputs) {
        
        if (track.volume > 0 && track.arm && !track.mute) {
            float *in_sample = (float *)jack_port_get_buffer(track.input, nframes);

            // add input buffer to output buffers
            for (int i = 0; i < n_samples; i++) {
                main_out_buffer[i] += in_sample[i] * track.volume;
                monitor_out_buffer[i] += in_sample[i] * track.volume;
            }
        }
    }

    return 0;
}


/**
 * returns the number of tracks (aka inputs)
 */
unsigned int JackController::get_number_of_tracks()
{
    return inputs.size();
}