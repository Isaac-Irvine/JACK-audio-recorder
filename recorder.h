#include <jack/jack.h>
#include <vector>
#include <string>

class JackController
{
    private:
    struct Input
    {
        Input(jack_port_t *input) : volume(1), arm(true), mute(false), input(input) {}
        jack_port_t *input;
        float volume;
        bool arm;
        bool mute;
    };

    std::vector<struct Input> inputs;
    jack_port_t *main_out;
    jack_port_t *monitor_out;
    jack_client_t *client;

    public:
    JackController();
    ~JackController();
    bool start_recording(std::string file_name);
    bool stop_recording();
    
    struct Input get_track(unsigned int track);
    bool add_track();
    unsigned int get_number_of_tracks();

    int process(jack_nframes_t nframes, void *arg);

};