#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <iotctrl/temp-sensor.h>

void print_help() {
    printf(
        "Usage: temp-sensor-tool\n"
        "    -d, --device-path <device_path>   The path of the device, typically /dev/ttyUSB0\n"
        "    [-v, --verbose]                    Enable verbose mode\n"
    );
}

int main(int argc, char** argv) {
    char* device_path = NULL;
    bool verbose_mode = false;
    int c;
    // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
    while (1) {
        static struct option long_options[] = {       
            {"verbose",      no_argument,                   0, 'v'},
            {"device-path",  required_argument,             0, 'd'},            
            {"help",         no_argument,                   0, 'h'},
            {NULL,           0,                          NULL,   0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "d:hv", long_options, &option_index);

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
            case 'h':
                print_help();
                return 0;
            case 'v':
                verbose_mode = true;
                break;
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

    int temp_raw = get_temperature(device_path, verbose_mode);
    if (temp_raw == INVALID_TEMP) {
        fprintf(stderr, "failed to read from sensor.\n");
    } else {
        float temp_parsed = temp_raw / 10.0;
        printf("%.1f °C\n", temp_parsed);
    }
    return 0;
}