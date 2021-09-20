#pragma once
#include <core/core.hpp>

#include <stack>
#include <nlohmann/json.hpp>

#include <iostream>

namespace legion::core::serialization
{
    using json = nlohmann::ordered_json;

    struct serializer_view
    {
        serializer_view() = default;
        virtual ~serializer_view() = default;

        virtual void start_object(std::string name) = 0;
        virtual void start_object() = 0;
        virtual void end_object() = 0;

        virtual void start_container(std::string name) = 0;
        virtual void start_container() = 0;
        virtual void end_container() = 0;

        template<typename Type>
        bool serialize(std::string name, Type&& value);

        virtual void serialize_int(std::string& name, int serializable) LEGION_PURE;
        virtual void serialize_float(std::string& name, float serializable) = 0;
        virtual void serialize_double(std::string& name, double serializable) = 0;
        virtual void serialize_bool(std::string& name, bool serializable) = 0;
        virtual void serialize_string(std::string& name, const std::string_view& serializable) = 0;
        virtual void serialize_id_type(std::string& name, id_type serializable) = 0;

        virtual common::result<void, fs_error> write(fs::view& file) = 0;

        virtual bool load(fs::view& file) = 0;

        template<typename Type>
        common::result<Type> deserialize(std::string_view& name) {}

        virtual common::result<int, exception> deserialize_int(std::string_view& name) = 0;
        virtual common::result<float, exception> deserialize_float(std::string_view& name) = 0;
        virtual common::result<double, exception> deserialize_double(std::string_view& name) = 0;
        virtual bool deserialize_bool(std::string_view& name) = 0;
        virtual common::result<std::string, exception> deserialize_string(std::string_view& name) = 0;
        virtual common::result<id_type, exception> deserialize_id_type(std::string_view& name) = 0;
    };

    struct json_view : public serializer_view
    {
        nlohmann::json root;

        std::stack<nlohmann::json> current_writing;

        json_view() = default;
        ~json_view() = default;

        virtual void start_object(std::string name) override;
        virtual void end_object() override;

        virtual void serialize_int(std::string& name, int serializable) override;
        virtual void serialize_float(std::string& name, float serializable) override;
        virtual void serialize_double(std::string& name, double serializable) override;
        virtual void serialize_bool(std::string& name, bool serializable) override;
        virtual void serialize_string(std::string& name, const std::string_view& serializable) override;
        virtual void serialize_id_type(std::string& name, id_type serializable) override;

        virtual common::result<void, fs_error> write(fs::view& file) override;

        virtual common::result<int, exception> deserialize_int(std::string_view& name) override;
        virtual common::result<float, exception> deserialize_float(std::string_view& name) override;
        virtual common::result<double, exception> deserialize_double(std::string_view& name) override;
        virtual bool deserialize_bool(std::string_view& name) override;
        virtual common::result<std::string, exception> deserialize_string(std::string_view& name) override;
        virtual common::result<id_type, exception> deserialize_id_type(std::string_view& name) override;
    };

    struct bson_view : serializer_view
    {

        bson_view() = default;
        ~bson_view() = default;

        virtual void start_object(std::string name) override
        {

        }
        virtual void end_object() override
        {

        }

        virtual void start_container(std::string name) override
        {

        }
        virtual void end_container() override
        {

        }

        virtual void serialize_int(std::string& name, int serializable) override;
        virtual void serialize_float(std::string& name, float serializable) override;
        virtual void serialize_double(std::string& name, double serializable) override;
        virtual void serialize_bool(std::string& name, bool serializable) override;
        virtual void serialize_string(std::string& name, const std::string_view& serializable) override;
        virtual void serialize_id_type(std::string& name, id_type serializable) override;

        virtual common::result<int, exception> deserialize_int(std::string_view& name) override;
        virtual common::result<float, exception> deserialize_float(std::string_view& name) override;
        virtual common::result<double, exception> deserialize_double(std::string_view& name) override;
        virtual bool deserialize_bool(std::string_view& name) override;
        virtual common::result<std::string, exception> deserialize_string(std::string_view& name) override;
        virtual common::result<id_type, exception> deserialize_id_type(std::string_view& name) override;
    };

    struct yaml_view : serializer_view
    {

        yaml_view() = default;
        ~yaml_view() = default;

        virtual void start_object(std::string name) override
        {

        }
        virtual void end_object() override
        {

        }

        virtual void start_container(std::string name) override
        {

        }
        virtual void end_container() override
        {

        }

        virtual void serialize_int(std::string& name, int serializable) override;
        virtual void serialize_float(std::string& name, float serializable) override;
        virtual void serialize_double(std::string& name, double serializable) override;
        virtual void serialize_bool(std::string& name, bool serializable) override;
        virtual void serialize_string(std::string& name, const std::string_view& serializable) override;
        virtual void serialize_id_type(std::string& name, id_type serializable) override;

        virtual common::result<int, exception> deserialize_int(std::string_view& name) override;
        virtual common::result<float, exception> deserialize_float(std::string_view& name) override;
        virtual common::result<double, exception> deserialize_double(std::string_view& name) override;
        virtual bool deserialize_bool(std::string_view& name) override;
        virtual common::result<std::string, exception> deserialize_string(std::string_view& name) override;
        virtual common::result<id_type, exception> deserialize_id_type(std::string_view& name) override;
    };

   
}

#include <core/serialization/serializer_view.inl>

