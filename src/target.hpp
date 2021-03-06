#ifndef TARGET_HPP_
#define TARGET_HPP_

#include <memory>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include "fwd.hpp"

enum class target_role{
    SRC,
    DST,
};

enum class endian{
    HOST,
    BIG,
    LITTLE,
};

endian to_endian(const std::string& str);

class target{
public:
    target(const std::string& filename, target_role role,
            std::size_t offset = 0ul, std::size_t length = 0ul);
    target(int fd);
    target(const target&) = default;
    ~target(){}

    int transfer_to(const target& dest, const param& prm)const;
    int write_to(const target& dest, const param& prm)const;

    void mmap(int prot);

    // deprecated.
    char* offset()const{return mmapped_data_.get() + page_offset_;}
    std::size_t length()const{return length_;}

private:
    std::shared_ptr<int> ptr_to_fd_;
    std::shared_ptr<char> mmapped_data_;
    const struct stat stat_;
    const std::size_t offset_;
    mutable std::size_t length_;
    const std::size_t page_offset_;

    std::size_t init_length(std::size_t length, target_role role);
    void preprocess(target_role role);
    int passthrough(const target& dest)const;

    static int hexdump(int fd, const char* data, std::size_t offset,
            std::size_t length, std::size_t page_offset, const param& prm);
    static std::uint64_t fetch(const void* p, int width, endian e = endian::HOST);
    static int select_file_flags(target_role r);

    const static long page_size_;

    class iohelper{
    public:
        iohelper(int fd, std::size_t size);
        ~iohelper();

        template <typename... Args>
        int snprintf(const char* format, Args... args); // defined in .cpp file.

    public:
        static int open(const char* pathname, int flags, mode_t mode);
        static ssize_t read(int fd, void* buf, size_t count);
        static ssize_t write(int fd, const void* buf, size_t count);
        static ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset);
        static off_t lseek(int fd, off_t offset, int whence);
        static int ftruncate(int fd, off_t length);
        static void close(int*);

        static struct stat fstat(int fd);

        static void *memcpy(void* dest, const void* src, size_t n,
                int sched_policy, size_t jobs);
        static ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset,
                int sched_policy, size_t jobs);

    private:
        const int fd_;
        const std::size_t size_;
        std::shared_ptr<char[]> buf_;
        std::size_t count_;
    };
};

#endif // TARGET_HPP_

// vim: set expandtab shiftwidth=0 tabstop=4 :
