#include <unistd.h> // using sleep
#include "recorder.h"


int main()
{
    JackController controler = JackController();

    for (int i = 0; i < 5; i++) {
        controler.add_track();
    }

    sleep(-1);
}
