#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <iotctrl/relay.h>

void print_help() {
    printf(
        "Usage: relay-tool\n"
        "    -d, --device-path <device_path>   The path of the device, typically /dev/ttyUSB0\n"
        "    --on, --off                       Turn the switch on/off\n"
    );
}

int main(int argc, char** argv) {
    char* device_path = NULL;
    static int switch_on = 0;
    int c;
    // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    while (1) {
        static struct option long_options[] = {          /* These options set a flag. */
            {"on",           no_argument,           &switch_on, 1},
            {"off",          no_argument,           &switch_on, 0},
            {"device-path",  required_argument,             0, 'd'},
            {"help",         no_argument,                   0, 'h'},
            {NULL,           0,                          NULL,   0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "d:s:h", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;
        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;
            case 'd':
                device_path = optarg;
                break;
            case 's':
                print_help();
                return 0;
            case 'h':
                print_help();
                return 0;
            case '?':
                print_help();
                return 1;
            default:
                print_help();
                return 1;
        }
    }
    /* Print any remaining command line arguments (not options). */
    if (optind < argc) {        
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        putchar ('\n');
        print_help();
        return 1;
    }
    if (device_path == NULL) {
        print_help();
        return 1;
    }

    control_relay("/dev/ttyUSB0", switch_on);
    return 0;
}