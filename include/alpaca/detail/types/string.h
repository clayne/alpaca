#pragma once
#ifndef ALPACA_EXCLUDE_SUPPORT_STD_STRING
#include <alpaca/detail/from_bytes.h>
#include <alpaca/detail/to_bytes.h>
#include <alpaca/detail/type_info.h>
#include <string>
#include <system_error>
#include <vector>

namespace alpaca {

namespace detail {

template <typename T>
typename std::enable_if<is_specialization<T, std::basic_string>::value,
                        void>::type
type_info(std::vector<uint8_t> &typeids,
          std::unordered_map<std::string_view, std::size_t> &) {
  typeids.push_back(to_byte<field_type::string>());
}

template <options O, typename T, typename Container>
void to_bytes_router(const T &input, Container &bytes, std::size_t &byte_index);

template <options O, typename Container, typename CharType>
void to_bytes(Container &bytes, std::size_t &byte_index,
              const std::basic_string<CharType> &input) {
  // save string length
  to_bytes_router<O>((size_t_serialized_type) input.size(), bytes, byte_index);

  for (const auto &c : input) {
    to_bytes<O>(bytes, byte_index, c);
  }
}

template <options O, typename Container, typename CharType>
typename std::enable_if<!std::is_same_v<Container, std::ifstream>, bool>::type
from_bytes(std::basic_string<CharType> &value, Container &bytes,
           std::size_t &current_index, std::size_t &end_index,
           std::error_code &error_code) {
  // clear out the value - this ensures that value will be only what is read
  // from the stream, and not any previous data that may have been set during
  // the construction of the containing object T().
  value.clear();
  if (current_index >= end_index) {
    // end of input
    // return true for forward compatibility, unless strict mode is enabled
    if constexpr (detail::strict<O>()) {
      error_code = std::make_error_code(std::errc::bad_message);
      return false;
    } else {
      return true;
    }
  }

  // current byte is the length of the string
  size_t_serialized_type size = 0;
  detail::from_bytes<O, size_t_serialized_type>(size, bytes, current_index, end_index,
                                     error_code);

  if (size > end_index - current_index) {
    // size is greater than the number of bytes remaining
    error_code = std::make_error_code(std::errc::value_too_large);

    // stop here
    return false;
  }

  // read `size` bytes and save to value
  value.reserve(size * sizeof(CharType));
  for (std::size_t i = 0; i < size; ++i) {
    CharType character{};
    from_bytes<O>(character, bytes, current_index, end_index, error_code);
    value += character;
  }

  return true;
}

// ifstream version
template <options O, typename Container, typename CharType>
typename std::enable_if<std::is_same_v<Container, std::ifstream>, bool>::type
from_bytes(std::basic_string<CharType> &value, Container &bytes,
           std::size_t &current_index, std::size_t &end_index,
           std::error_code &error_code) {
  // clear out the value - this ensures that value will be only what is read
  // from the stream, and not any previous data that may have been set during
  // the construction of the containing object T().
  value.clear();
  if (current_index >= end_index) {
    // end of input
    // return true for forward compatibility, unless strict mode is enabled
    if constexpr (detail::strict<O>()) {
      error_code = std::make_error_code(std::errc::bad_message);
      return false;
    } else {
      return true;
    }
  }

  // current byte is the length of the string
  size_t_serialized_type size = 0;
  detail::from_bytes<O, size_t_serialized_type>(size, bytes, current_index, end_index,
                                     error_code);

  if (size > end_index - current_index) {
    // size is greater than the number of bytes remaining
    error_code = std::make_error_code(std::errc::value_too_large);

    // stop here
    return false;
  }

  // read `size` bytes and save to value
  value.reserve(size * sizeof(CharType));
  for (std::size_t i = 0; i < size; ++i) {
    CharType character;
    from_bytes<O>(character, bytes, current_index, end_index, error_code);
    value += character;
  }

  return true;
}

} // namespace detail

} // namespace alpaca
#endif
