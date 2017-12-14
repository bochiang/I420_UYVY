#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <iostream>
#include <vector>
#include "cxxopts.hpp"

using namespace std;

typedef struct config
{
    size_t width;
    size_t height;
    size_t frames;
    string input_file_name;
    string output_file_name;
};

config arg_config;

vector<string> split(string &Input, const char* Regex)
{
    int pos = 0;
    int npos = 0;
    vector<string> Result;
    int regexlen = strlen(Regex);
    while ((npos = Input.find(Regex, pos)) != -1) {
        string tmp = Input.substr(pos, npos - pos);
        Result.push_back(tmp);
        pos = npos + regexlen;
    }
    Result.push_back(Input.substr(pos, Input.length() - pos));
    return Result;
}

void print_usage()
{
    cout << "Usage:" << endl;
    cout << "    I420_UYVY [options] [input_file_name] [output_file_name]" << endl;
    cout << "Options:" << endl;
    cout << "    -s   width and height of YUV image, example: -s 640x480" << endl;
    cout << "    -f   frames to be transformed" << endl;
    cout << "    -i   input file name" << endl;
    cout << "    -o   output file name" << endl;
}

void print_args()
{
    cout << "image size:                       " << arg_config.width << "x" << arg_config.height << endl;
    cout << "frames to be transformed:         " << arg_config.frames << endl;
    cout << "input file name:                  " << arg_config.input_file_name.c_str() << endl;
    cout << "output file name:                 " << arg_config.output_file_name.c_str() << endl;
}

int parse_args(int argc, char *argv[])
{
    int i;
    string str;
    int type;
    try {
        cxxopts::Options options(argv[0], " - I420 to UYVY");
        options.add_options()
            ("s,size", "image size", cxxopts::value<string>(str))
            ("f,frames", "frames to be transformed", cxxopts::value<int>(type))
            ("i,input", "input file name", cxxopts::value<string>(arg_config.input_file_name))
            ("o,output", "output file name", cxxopts::value<string>(arg_config.output_file_name));
        options.parse(argc, argv);
        vector<string> v = split(str, "x");
        arg_config.width = strtol(v[0].c_str(), NULL, 10);
        arg_config.height = strtol(v[1].c_str(), NULL, 10);
        arg_config.frames = type;
        print_args();
    }
    catch (const std::exception&) {
        return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    if (argc < 9) {
        print_usage();
        return 0;
    }
    parse_args(argc, argv);
    FILE *in_fp = fopen(arg_config.input_file_name.c_str(), "rb");
    FILE *out_fp = fopen(arg_config.output_file_name.c_str(), "wb");
    int width = arg_config.width;
    int height = arg_config.height;
    int frames = arg_config.frames;

    int in_size = width * height * 3 / 2;
    int out_size = width * height * 2;

    uint8_t *in_image = (uint8_t *)malloc(in_size * sizeof(uint8_t));
    uint8_t *out_image = (uint8_t *)malloc(out_size * sizeof(uint8_t));

    uint8_t *Y = in_image;
    uint8_t *U = in_image + width * height;
    uint8_t *V = in_image + width * height * 5 / 4;

    int frame_cnt, r, c;
    for (frame_cnt = 0; frame_cnt < frames; frame_cnt++) {
        fread(in_image, sizeof(uint8_t), in_size, in_fp);
        for(r = 0; r < height; r++)
            for (c = 0; c < width; c++) {
                if((c & 0x01) == 0)
                {
                    out_image[r * width * 2 + c * 2] = U[(r >> 1) * (width >> 1)+ (c >> 1)];
                }
                else
                {
                    out_image[r * width * 2 + c * 2] = V[(r >> 1) * (width >> 1) + (c >> 1)];
                }
                out_image[r * width * 2 + c * 2 + 1] = Y[r * width + c];
            }
        fwrite(out_image, sizeof(uint8_t), out_size, out_fp);
    }

    free(in_image);
    free(out_image);
    fclose(in_fp);
    fclose(out_fp);
}