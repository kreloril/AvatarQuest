#include "Common.h"
#include <fstream>

namespace Util{

    // ---------------- helpers (LE encode/decode) ----------------
    static inline void append(std::vector<Byte>& b, const void* p, std::size_t n) {
        const Byte* s = static_cast<const Byte*>(p);
        b.insert(b.end(), s, s + n);
    }
    template <class T>
    static inline void write_le(std::vector<Byte>& b, T v) {
        static_assert(std::is_trivially_copyable_v<T>, "write_le requires POD");
        append(b, &v, sizeof(T));
    }
    template <class T>
    static inline bool read_le(const std::vector<Byte>& b, std::size_t& rd, T& v) {
        static_assert(std::is_trivially_copyable_v<T>, "read_le requires POD");
        if (rd + sizeof(T) > b.size()) return false;
        std::memcpy(&v, b.data() + rd, sizeof(T));
        rd += sizeof(T);
        return true;
    }

    // ---------------- BinStream ----------------
    BinStream::BinStream() = default;
    BinStream::BinStream(std::vector<Byte> initial) : buf_(std::move(initial)), rd_(0) {}
    BinStream::~BinStream() = default;

    // write
    void BinStream::writeBytes(const void* data, std::size_t n) { append(buf_, data, n); }
    void BinStream::writeU8(std::uint8_t v) { write_le(buf_, v); }
    void BinStream::writeI8(std::int8_t v) { write_le(buf_, v); }
    void BinStream::writeU16(std::uint16_t v) { write_le(buf_, v); }
    void BinStream::writeI16(std::int16_t v) { write_le(buf_, v); }
    void BinStream::writeU32(std::uint32_t v) { write_le(buf_, v); }
    void BinStream::writeI32(std::int32_t v) { write_le(buf_, v); }
    void BinStream::writeU64(std::uint64_t v) { write_le(buf_, v); }
    void BinStream::writeI64(std::int64_t v) { write_le(buf_, v); }
    void BinStream::writeF32(float v) { write_le(buf_, v); }
    void BinStream::writeF64(double v) { write_le(buf_, v); }
    void BinStream::writeStr(const std::string& s) {
        writeU32(static_cast<std::uint32_t>(s.size()));
        if (!s.empty()) writeBytes(s.data(), s.size());
    }

    // read
    bool BinStream::readBytes(void* out, std::size_t n) {
        if (rd_ + n > buf_.size()) return false;
        std::memcpy(out, buf_.data() + rd_, n);
        rd_ += n;
        return true;
    }
    bool BinStream::skip(std::size_t n) {
        if (rd_ + n > buf_.size()) return false;
        rd_ += n;
        return true;
    }
    std::size_t BinStream::remaining() const { return buf_.size() - rd_; }
    bool BinStream::eof() const { return rd_ == buf_.size(); }
    bool BinStream::ok()  const { return rd_ <= buf_.size(); }

    bool BinStream::readU8(std::uint8_t& v) { return read_le(buf_, rd_, v); }
    bool BinStream::readI8(std::int8_t& v) { return read_le(buf_, rd_, v); }
    bool BinStream::readU16(std::uint16_t& v) { return read_le(buf_, rd_, v); }
    bool BinStream::readI16(std::int16_t& v) { return read_le(buf_, rd_, v); }
    bool BinStream::readU32(std::uint32_t& v) { return read_le(buf_, rd_, v); }
    bool BinStream::readI32(std::int32_t& v) { return read_le(buf_, rd_, v); }
    bool BinStream::readU64(std::uint64_t& v) { return read_le(buf_, rd_, v); }
    bool BinStream::readI64(std::int64_t& v) { return read_le(buf_, rd_, v); }
    bool BinStream::readF32(float& v) { return read_le(buf_, rd_, v); }
    bool BinStream::readF64(double& v) { return read_le(buf_, rd_, v); }

    bool BinStream::readStr(std::string& s) {
        std::uint32_t n = 0;
        if (!readU32(n)) return false;
        s.resize(n);
        if (n == 0) return true;
        return readBytes(s.data(), n);
    }

    // buffer mgmt
    void BinStream::clear() { buf_.clear(); rd_ = 0; }
    void BinStream::reserve(std::size_t cap) { buf_.reserve(cap); }
    std::size_t BinStream::size() const { return buf_.size(); }
    const std::vector<Byte>& BinStream::buffer() const { return buf_; }
    std::vector<Byte>& BinStream::buffer() { return buf_; }

    // read cursor control
    std::size_t BinStream::tell() const { return rd_; }
    bool BinStream::seek(std::size_t pos) { if (pos > buf_.size()) return false; rd_ = pos; return true; }
    void BinStream::rewind() { rd_ = 0; }

    bool BinStream::truncate(std::size_t newSize) {
        if (newSize > buf_.size()) return false;
        buf_.resize(newSize);
        if (rd_ > newSize) rd_ = newSize;
        return true;
    }

    // file I/O
    bool BinStream::saveFile(const char* path) const {
        std::ofstream f(path, std::ios::binary | std::ios::trunc);
        if (!f) return false;
        if (!buf_.empty()) {
            f.write(reinterpret_cast<const char*>(buf_.data()),
                static_cast<std::streamsize>(buf_.size()));
            if (!f.good()) return false;
        }
        f.flush();
        return f.good();
    }

    bool BinStream::loadFile(const char* path) {
        std::ifstream f(path, std::ios::binary);
        if (!f) return false;

        f.seekg(0, std::ios::end);
        std::streamoff len = f.tellg();
        if (len < 0) return false;
        f.seekg(0, std::ios::beg);

        std::vector<Byte> tmp;
        tmp.resize(static_cast<std::size_t>(len));
        if (len > 0) {
            if (!f.read(reinterpret_cast<char*>(tmp.data()), len)) return false;
        }

        buf_.swap(tmp);
        rd_ = 0;
        return true;
    }

    // adopt/release
    void BinStream::adopt(std::vector<Byte> buf) {
        buf_ = std::move(buf);
        rd_ = 0;
    }

    std::vector<Byte> BinStream::release() {
        rd_ = 0;
        std::vector<Byte> out;
        out.swap(buf_);
        return out;
    }


}