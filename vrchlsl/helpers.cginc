uint4 unpack_uint4(in float3 data[6]) {
    uint4 result;
    result.x = uint(data[0].r * 255.0f + 0.5f) |
               (uint(data[1].r * 255.0f + 0.5f) << 8) |
               (uint(data[2].r * 255.0f + 0.5f) << 16) |
               (uint(data[3].r * 255.0f + 0.5f) << 24);
    result.y = uint(data[0].g * 255.0f + 0.5f) |
               (uint(data[1].g * 255.0f + 0.5f) << 8) |
               (uint(data[2].g * 255.0f + 0.5f) << 16) |
               (uint(data[3].g * 255.0f + 0.5f) << 24);
    result.z = uint(data[0].b * 255.0f + 0.5f) |
               (uint(data[1].b * 255.0f + 0.5f) << 8) |
               (uint(data[2].b * 255.0f + 0.5f) << 16) |
               (uint(data[3].b * 255.0f + 0.5f) << 24);
    result.w = uint(data[4].r * 255.0f + 0.5f) |
               (uint(data[4].g * 255.0f + 0.5f) << 8) |
               (uint(data[4].b * 255.0f + 0.5f) << 16) |
               (uint(data[5].r * 255.0f + 0.5f) << 24);
    return result;
}