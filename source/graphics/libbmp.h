#ifndef __LIBBMP_H__
#define __LIBBMP_H__

#define BMP_MAGIC 19778

#define BMP_GET_PADDING(a) ((a) % 4)

//
// BmpError
//

enum BmpError { BMP_FILE_NOT_OPENED = -4, BMP_HEADER_NOT_INITIALIZED, BMP_INVALID_FILE, BMP_ERROR, BMP_OK = 0 };

//
// BmpPixbuf
//

class BmpPixbuf {
    public:
        BmpPixbuf() {};
        BmpPixbuf(const int width, const int height, const uint8_t* pixels) {
            init(width, height);
            if (pixels)
                memcpy(&data[0], pixels, data.size());
        };
        ~BmpPixbuf() { data.clear(); }

        void init(const int width, const int height) {
            len_row = width * len_pixel;
            data.resize(height * len_row);
        }

        void set_pixel(const int x, const int y, const unsigned char r, const unsigned char g, const unsigned char b);

        unsigned char red_at(const int x, const int y) { return data[(x * len_pixel) + (y * len_row) + 2]; }
        unsigned char green_at(const int x, const int y) { return data[(x * len_pixel) + (y * len_row) + 1]; }
        unsigned char blue_at(const int x, const int y) { return data[(x * len_pixel) + (y * len_row)]; }

        void write(const int row, std::ofstream& f, bool fixMagenta = false) { f.write(reinterpret_cast<char*>(&data[row * len_row]), len_row); }
        void read(const int row, std::ifstream& f) { f.read(reinterpret_cast<char*>(&data[row * len_row]), len_row); }

        uint8_t* getData() { return data.data(); }

    private:
        size_t len_row;
        size_t len_pixel = 4;

        std::vector<unsigned char> data;
};

//
// BmpImg
//

class BmpImg : public BmpPixbuf {
    public:
        BmpImg() {};
        BmpImg(const int width, const int height, const uint8_t* pixels = nullptr);
        ~BmpImg() {};

        enum BmpError write(const std::string& filename, bool fixMagenta = false);
        enum BmpError read(const std::string& filename);

        int get_width() { return header.biWidth; }
        int get_height() { return header.biHeight; }

    private:
        // Use a struct to read this in one call
        struct {
            unsigned int bfSize = 0;
            unsigned int bfReserved = 0;
            unsigned int bfOffBits = 54;
            unsigned int biSize = 40;
            int biWidth = 0;
            int biHeight = 0;
            unsigned short biPlanes = 1;
            unsigned short biBitCount = 32;
            unsigned int biCompression = 0;
            unsigned int biSizeImage = 0;
            int biXPelsPerMeter = 0;
            int biYPelsPerMeter = 0;
            unsigned int biClrUsed = 0;
            unsigned int biClrImportant = 0;
        } header;
};

#endif /* __LIBBMP_H__ */
