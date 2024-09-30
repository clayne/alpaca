#pragma once
#ifndef ALPACA_EXCLUDE_SUPPORT_STD_DEQUE
#include <alpaca/detail/to_bytes.h>
#include <alpaca/detail/type_info.h>
#include <deque>
#include <system_error>

namespace alpaca {

namespace detail {

template <typename T>
typename std::enable_if<is_specialization<T, std::deque>::value, void>::type
type_info(
    std::vector<uint8_t> &typeids,
    std::unordered_map<std::string_view, std::size_t> &struct_visitor_map) {
  typeids.push_back(to_byte<field_type::deque>());
  using value_type = typename T::value_type;
  type_info<value_type>(typeids, struct_visitor_map);
}

template <options O, typename T, typename Container>
void to_bytes_router(const T &input, Container &bytes, std::size_t &byte_index);

template <options O, typename T, typename Container>
void to_bytes_from_deque_type(const T &input, Container &bytes,
                              std::size_t &byte_index) {
  // save deque size
  to_bytes_router<O, size_t_serialized_type>(input.size(), bytes, byte_index);

  // value of each element in deque
  for (const auto &v : input) {
    // check if the value_type is a nested deque type
    to_bytes_router<O>(v, bytes, byte_index);
  }
}

template <options O, typename Container, typename U>
void to_bytes(Container &bytes, std::size_t &byte_index,
              const std::deque<U> &input) {
  to_bytes_from_deque_type<O>(input, bytes, byte_index);
}

template <options O, typename T, typename Container>
void from_bytes_router(T &output, Container &bytes, std::size_t &byte_index,
                       std::size_t &end_index, std::error_code &error_code);

template <options O, typename T, typename Container>
bool from_bytes_to_deque(std::deque<T> &value, Container &bytes,
                         std::size_t &current_index, std::size_t &end_index,
                         std::error_code &error_code) {

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

  // current byte is the size of the vector
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
  for (size_t_serialized_type i = 0; i < size; ++i) {
    T v{};
    from_bytes_router<O>(v, bytes, current_index, end_index, error_code);
    if (error_code) {
      // something went wrong
      return false;
    }
    value.push_back(v);
  }

  return true;
}

template <options O, typename T, typename Container>
bool from_bytes(std::deque<T> &output, Container &bytes,
                std::size_t &byte_index, std::size_t &end_index,
                std::error_code &error_code) {
  return from_bytes_to_deque<O>(output, bytes, byte_index, end_index,
                                error_code);
}

} // namespace detail

} // namespace alpaca
#endif
