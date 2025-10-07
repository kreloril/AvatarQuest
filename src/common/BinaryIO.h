#pragma once


namespace Util {

    using Byte = std::uint8_t;

    class BinStream {
    public:
        BinStream();
        explicit BinStream(std::vector<Byte> initial); // takes ownership
        ~BinStream();

        // ---------------- write (append) ----------------
        void writeBytes(const void* data, std::size_t n);

        void writeU8(std::uint8_t v);
        void writeI8(std::int8_t v);
        void writeU16(std::uint16_t v);
        void writeI16(std::int16_t v);
        void writeU32(std::uint32_t v);
        void writeI32(std::int32_t v);
        void writeU64(std::uint64_t v);
        void writeI64(std::int64_t v);

        void writeF32(float v);
        void writeF64(double v);

        // Writes: [u32 length][bytes], no null terminator
        void writeStr(const std::string& s);

        // ---------------- read (from cursor) ----------------
        bool readBytes(void* out, std::size_t n);
        bool skip(std::size_t n);
        std::size_t remaining() const;
        bool eof() const;       // rd_ == buf_.size()
        bool ok() const;        // rd_ <= buf_.size()

        bool readU8(std::uint8_t& v);
        bool readI8(std::int8_t& v);
        bool readU16(std::uint16_t& v);
        bool readI16(std::int16_t& v);
        bool readU32(std::uint32_t& v);
        bool readI32(std::int32_t& v);
        bool readU64(std::uint64_t& v);
        bool readI64(std::int64_t& v);

        bool readF32(float& v);
        bool readF64(double& v);

        bool readStr(std::string& s); // expects [u32 length][bytes]

        // ---------------- buffer management ----------------
        void clear();                     // clears buffer, rd_ = 0
        void reserve(std::size_t cap);
        std::size_t size() const;         // bytes in buffer
        const std::vector<Byte>& buffer() const;
        std::vector<Byte>& buffer();

        // read cursor control
        std::size_t tell() const;         // read pointer
        bool seek(std::size_t pos);       // set read pointer
        void rewind();                    // seek(0)

        // truncate/trim buffer (affects data available for future reads/writes)
        bool truncate(std::size_t newSize); // clamp size; rd_ = min(rd_, newSize)

        // ---------------- file I/O (owning) ----------------
        bool saveFile(const char* path) const;   // writes entire buffer
        bool loadFile(const char* path);         // replaces buffer; rd_ = 0

        // ---------------- adopt/release (owning) ----------------
        void adopt(std::vector<Byte> buf);       // move in
        std::vector<Byte> release();             // move out, rd_=0

    private:
        std::vector<Byte> buf_;
        std::size_t rd_ = 0; // read cursor
    };


}