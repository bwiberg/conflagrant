#pragma once

#include <conflagrant/types.hh>
#include <conflagrant/GL.hh>
#include <conflagrant/serialization/Serialize.hh>

namespace cfl {
namespace comp {
struct Name {
    string value;

    inline static string const GetName() {
        return "Name";
    }

    template<typename TSerializer>
    static bool Serialize(Json::Value &json, Name &name) {
        SERIALIZE(json, name.value);
        return true;
    }
};
} // namespace comp
} // namespace cfl
