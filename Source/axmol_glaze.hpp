#include <math/Vec2.h>
#include <array>
#include <glaze/glaze.hpp>

namespace glz::detail
{
   template <>
   struct from_json<ax::Vec2>
   {
        template <auto Opts>
        static void op(ax::Vec2& value, auto&&... args)
        {
            std::array<float, 2> vec;
            read<json>::op<Opts>(vec, args...);
            value = ax::Vec2{vec[0], vec[1]};
        }
    };
};