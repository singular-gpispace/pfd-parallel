#ifndef TYPE_ALIASES_H
#define TYPE_ALIASES_H

#include <we/type/value.hpp>

namespace singular_parallel
{
  using pnet_value = pnet::type::value::value_type;
  using pnet_list = std::list<pnet_value>;
  using pnet_set = std::set<pnet_value>;
  using pnet_map = std::map<pnet_value, pnet_value>;

  // Provides a non-overloaded wrapper for boost::get.
  template <typename T>
  const T& as (const singular_parallel::pnet_value& v)
  {
    return boost::get<T> (v);
  }
}

#endif
