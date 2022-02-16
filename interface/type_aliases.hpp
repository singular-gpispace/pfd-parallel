#ifndef TYPE_ALIASES_H
#define TYPE_ALIASES_H

//#include <pnetc/type/config_type.hpp>
#include <pnetc/type/options_type.hpp>
#include <pnetc/type/term.hpp>
#include <pnetc/type/term_count_type.hpp>
#include <we/type/value.hpp>

namespace singular_parallel
{
  using pnet_value = pnet::type::value::value_type;
  using pnet_list = std::list<pnet_value>;
  using pnet_set = std::set<pnet_value>;
  using pnet_map = std::map<pnet_value, pnet_value>;
  using pnet_vector = std::vector<pnet_value>;

  //using pnet_config = pnetc::type::config_type::config_type;
  using pnet_options = pnetc::type::options_type::options_type;
  using pnet_term = pnetc::type::term::term;
  using pnet_term_count_type = pnetc::type::term_count_type::term_count_type;

  // Provides a non-overloaded wrapper for boost::get.
  template <typename T>
  const T& as (const singular_parallel::pnet_value& v)
  {
    return boost::get<T> (v);
  }

  template <typename T, typename U>
  std::map<T, U> unwrap_map (const pnet_map& m)
  {
    std::map<T, U> out;
    for (const auto& [k, v] : m) {
      out.emplace (boost::get<T>(k), boost::get<U>(v));
    }
    return out;
  }
}

#endif
