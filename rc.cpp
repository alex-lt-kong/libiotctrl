#include <fstream>
#include <iostream>

using namespace std;

bool is_device_exist(const char *fileName)
{
  ifstream infile(fileName);
  return infile.good();
}

int main(int argc, char *argv[])
{
  if (argc != 3) {
    cerr << "Usage: rc.out [Device Path] [0/1]" << endl;
    return 1;
  }
  if (is_device_exist(argv[1]) == false) {
    // Must have this, otherwise the below code will create a normal file instead of
    // writing bytes to serial port TTY device as expected.
    cerr << "Device [" << argv[1] << "] does not exist" << endl;
    return 1;
  }
  if (*argv[2] != '0' && *argv[2] != '1') {
    cerr << "The 2nd option has to be 0 or 1" << endl;
    return 1;
  }
  int ops_idx = (int)(*argv[2] - 48);

  // The definitions of bytes series can be found here:
  // https://detail.tmall.com/item.htm?id=41202173031&spm=a1z09.2.0.0.139f2e8dWpxNdU&_u=lb08hajdb33
  unsigned char ops[][4] = {{0xA0, 0x01, 0x00, 0xA1}, {0xA0, 0x01, 0x01, 0xA2}};

  ofstream fileout;
  fileout.open(argv[1], ios_base::binary);
  if (fileout.is_open()) {
    for(int i = 0; i < sizeof(ops[ops_idx]); i++) {
      fileout.write((char*)(ops[ops_idx] + i), 1);
    }
    fileout.close();
    return 0;
  } else {
    cerr << "Failed to open device: " << argv[1] << endl;
    return 1;
  }
}
