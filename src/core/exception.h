#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <string>

namespace evnt
{
class Exception : public std::exception
{
public:
    //! Creates exception object
    Exception(const char * str, int line, const char * file) noexcept :
        eLine_(line),
        eFile_(file),
        eText_(str)
    {}
    Exception(std::string str, int line, const char * file) noexcept :
        eLine_(line),
        eFile_(file),
        eText_(std::move(str))
    {}
    ~Exception() noexcept override = default;

    Exception(const Exception & other) noexcept :
        eLine_(other.eLine_),
        eFile_(other.eFile_),
        eText_(other.eText_)
    {}
    // Unimplemented and unaccessible due to const members.
    Exception & operator=(const Exception & other) = delete;

    const char * what() const noexcept override;

private:
    //! The line number it occurred on
    const int eLine_;
    //! The source file
    std::string eFile_;
    //! A message passed along when the exception was raised
    std::string eText_;
    //! Final text message
    mutable std::string message_;
};

}   // namespace evnt

//! Use this macro to handle exceptions easily
#define EV_EXCEPT(str) throw(evnt::Exception(str, __LINE__, __FILE__))

#endif   // EXCEPTION_H
