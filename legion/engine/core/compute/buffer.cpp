#include <core/compute/buffer.hpp>
#include <core/logging/logging.hpp>
#include <CL/cl_gl.h>

namespace legion::core::compute
{
    bool operator&(const buffer_type& lhs, const buffer_type& rhs)
    {
        return static_cast<int>(lhs) & static_cast<int>(rhs);
    }

    Buffer::Buffer(cl_context ctx, void* data, size_type width, size_type height, size_type depth, cl_mem_object_type object_type, cl_image_format* format, buffer_type type, std::string name)
        : m_name(std::move(name))
        , m_data((byte*)data)

    {
        OPTICK_EVENT();
        int channelSize;

        switch (format->image_channel_order)
        {
        case CL_RGBA:
            channelSize = 4;
            break;
        case CL_RGB:
            channelSize = 3;
            break;
        case CL_RA:
            channelSize = 2;
            break;
        case CL_R:
            channelSize = 1;
            break;
        }


        switch (format->image_channel_data_type)
        {
        case CL_UNORM_INT8:

            break;
        case CL_UNORM_INT16:
            channelSize *= 2;
            break;
        case CL_FLOAT:
            channelSize *= 4;
            break;
        }

        m_size = width * height * channelSize;
        //convert buffer_type to cl_mem_flags
        if (type == buffer_type::READ_BUFFER)
            m_type = CL_MEM_READ_ONLY;
        else if (type == buffer_type::WRITE_BUFFER)
            m_type = CL_MEM_WRITE_ONLY;
        else
            m_type = CL_MEM_READ_WRITE;

        m_type |= CL_MEM_USE_HOST_PTR;

        cl_image_desc description;
        description.image_type = object_type;
        description.image_width = width;
        description.image_height = height;
        description.image_depth = depth;


        // as far as I can tell this should be null, It might also be the most important argument here idno
        // https://www.khronos.org/registry/OpenCL/specs/2.2/html/OpenCL_API.html#image-descriptor
        description.mem_object = nullptr;


        // we only support passing one image at a time
        // do not pass CL_IMAGE_2D_ARRAY
        description.image_array_size = 1;

        // when host ptr not 0 then these will be calculated from image_type & image_width
        // https://www.khronos.org/registry/OpenCL/specs/2.2/html/OpenCL_API.html#image-descriptor
        description.image_row_pitch = 0;
        description.image_slice_pitch = 0;

        // num samples & num mip_level must be 0
        // https://www.khronos.org/registry/OpenCL/specs/2.2/html/OpenCL_API.html#image-descriptor
        description.num_samples = 0;
        description.num_mip_levels = 0;

        cl_int ret;

        m_memory_object = common::managed_resource<cl_mem>([](cl_mem& value) {clReleaseMemObject(value); }, clCreateImage(ctx, m_type, format, &description, data, &ret));

        if (ret != CL_SUCCESS)
        {
            log::error("clCreateImage failed for Buffer: {}", m_name);
        }
    }

    Buffer::Buffer(cl_context ctx, cl_uint buffer, buffer_type type, bool is_renderbuffer, std::string name)
        : m_name(std::move(name))
        , m_data(nullptr)
        , m_size(0)
    {
        OPTICK_EVENT();
        //convert buffer_type to cl_mem_flags
        if (type == buffer_type::READ_BUFFER)
            m_type = CL_MEM_READ_ONLY;
        else if (type == buffer_type::WRITE_BUFFER)
            m_type = CL_MEM_WRITE_ONLY;
        else
            m_type = CL_MEM_READ_WRITE;

        cl_int ret;
        if (is_renderbuffer)
        {
            m_memory_object = common::managed_resource<cl_mem>([](cl_mem& value) {clReleaseMemObject(value); }, clCreateFromGLRenderbuffer(ctx, m_type, buffer, &ret));
        }
        else
        {
            m_memory_object = common::managed_resource<cl_mem>([](cl_mem& value) {clReleaseMemObject(value); }, clCreateFromGLBuffer(ctx, m_type, buffer, &ret));
        }
        if (ret != CL_SUCCESS)
        {
            log::error("clCreateFromGL(X?+)Buffer failed for Buffer: {}", m_name);
        }
    }
    Buffer::Buffer(cl_context ctx, byte* data, size_t len, buffer_type type, std::string name) : m_name(std::move(name)), m_data(data), m_size(len)
    {
        OPTICK_EVENT();
        if (!ctx) return;

        //convert buffer_type to cl_mem_flags
        if (type == buffer_type::READ_BUFFER)
            m_type = CL_MEM_READ_ONLY;
        else if (type == buffer_type::WRITE_BUFFER)
            m_type = CL_MEM_WRITE_ONLY;
        else
            m_type = CL_MEM_READ_WRITE;


        cl_int ret;

        //create buffer
        m_memory_object = common::managed_resource<cl_mem>([](cl_mem& value) {clReleaseMemObject(value);}, clCreateBuffer(ctx, m_type, m_size, nullptr, &ret));

        if (ret != CL_SUCCESS)
        {
            log::error("clCreateBuffer failed for Buffer: {}", m_name);
        }

    }

    Buffer::Buffer(cl_context ctx, cl_uint gl_buffer, buffer_type type, std::string name)
        : m_name(std::move(name))
        , m_data(nullptr)
        , m_size(0)
    {
        OPTICK_EVENT();
        //convert buffer_type to cl_mem_flags
        if (type == buffer_type::READ_BUFFER)
            m_type = CL_MEM_READ_ONLY;
        else if (type == buffer_type::WRITE_BUFFER)
            m_type = CL_MEM_WRITE_ONLY;
        else
            m_type = CL_MEM_READ_WRITE;

        cl_int ret;
        m_memory_object = common::managed_resource<cl_mem>([](cl_mem& value) {clReleaseMemObject(value); }, clCreateFromGLBuffer(ctx, m_type, gl_buffer, &ret));
        if (ret != CL_SUCCESS)
        {
            log::error("clCreateFromGLBuffer failed for Buffer: {} with code: {}", m_name, ret);
        }

    }

    Buffer::Buffer(cl_context ctx, cl_uint gl_target, cl_uint gl_texture, cl_uint miplevel, buffer_type type, std::string name)
        : m_name(std::move(name))
        , m_data(nullptr)
        , m_size(0)
    {
        OPTICK_EVENT();
        //convert buffer_type to cl_mem_flags
        if (type == buffer_type::READ_BUFFER)
            m_type = CL_MEM_READ_ONLY;
        else if (type == buffer_type::WRITE_BUFFER)
            m_type = CL_MEM_WRITE_ONLY;
        else
            m_type = CL_MEM_READ_WRITE;

        cl_int ret;
        m_memory_object = common::managed_resource<cl_mem>([](cl_mem& value) {clReleaseMemObject(value); }, clCreateFromGLTexture(ctx, m_type, gl_target, miplevel, gl_texture, &ret));

        if (ret != CL_SUCCESS)
        {
            log::error("clCreateFromGLTexture failed for Buffer: {}", m_name);
        }
    }

    void Buffer::rename(const std::string& name)
    {
        m_name = name;
    }
}
