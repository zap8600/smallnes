Shader "Emu/NES" {
    Properties {
        [ToggleUI] _Init ("Init", float) = 0
        [ToggleUI] _InitRaw ("Init directly from _Data_CARTROM", float) = 0

        _Color ("Tint", Color) = (0, 0, 0, 1)
        _MainTex ("Texture", 2D) = "white" {}
    }

    SubShader {
        Tags{ "RenderType"="Opaque" "Queue"="Geometry" }

        Pass {
            Name "CPU"

            CGPROGRAM
            #pragma target 5.0

            #include "crt.cginc"
            #include "UnityCG.cginc"

            uniform uint _Init, _InitRaw;

            #pragma vertex CustomRenderTextureVertexShader
            #pragma fragment frag

            typedef struct {
                uint axysp;
                uint statuspc;
                uint writeaddr;
                uint writeval;
            } cpu_t;

            static cpu_t cpu;

            static uint2 s_dim;

            #define RAM_ADDR(lin) uint2(1 + (lin % 128), 0)

            uint mem_get_byte(uint addr) {
                addr &= 0x7FF; // Ensure it fits in the NES RAM
                uint bitmask = (addr & 3) * 8;
                uint idx = (addr >> 2) & 3;
                addr >>= 4;
                uint4 raw = _SelfTexture2D[RAM_ADDR(addr)];
                return (raw[idx] & (0xFF000000 >> bitmask)) >> ((3 - (addr & 3)) * 8);
            }

            uint mem_get_short(uint addr) {
                return (mem_get_byte(addr + 1) << 8) | mem_get_byte(addr); // TODO: Find a better way of doing this
            }

            bool pixel_has_state(uint2 pos) {
                return !pos.x && !pos.y;
            }

            uint4 frag(v2f_customrendertexture i) : SV_Target {
                _SelfTexture2D.GetDimensions(s_dim.x, s_dim.y);

                uint2 pos = i.globalTexcoord.xy * s_dim;

                if(_Init) {
                    if(_InitRaw) {
                        return _SelfTexture2D[pos];
                    } else {
                        cpu.axysp = 0;
                        cpu.statuspc = 0x30 | (0x0600 << 8);
                        cpu.writeaddr = 0x800;
                        cpu.writeval = 0;
                    }
                } else {
                    uint4 raw = _SelfTexture2D[uint2(0, 0)];
                    cpu.axysp = raw.r;
                    cpu.statuspc = raw.g & 0xFFFFFF;
                    cpu.writeaddr = ((raw.b & 0xFFFF) << 8) | ((raw.g & 0xFF000000) >> 24);
                    cpu.writeval = (raw.b & 0xFFFF00) >> 8;

                    if(!pixel_has_state(pos)) {
                        return _SelfTexture2D[pos];
                    }

                    // Execute the CPU
                }
                uint4 raw = _SelfTexture2D[uint2(0, 0)];
                return raw;
            }
            ENDCG
        }

        Pass {
            Name "Commit"

            CGPROGRAM
            #pragma target 5.0

            #include "crt.cginc"
            #include "UnityCG.cginc"

            typedef struct {
                uint axysp;
                uint statuspc;
                uint writeaddr;
                uint writeval;
            } cpu_t;

            static cpu_t cpu;

            float _Init, _InitRaw;

            #pragma vertex CustomRenderTextureVertexShader
            #pragma fragment frag

            Texture2D<float4> _Data_CARTROM;

            static uint2 s_dim;
            static uint2 m_dim;

            #define RAM_ADDR(lin) uint2(1 + (lin % 128), 0)

            uint4 mem_write_byte(uint addr, uint value, uint2 pos) {
                addr &= 0x7FF;
                value &= 0xFF;
                uint bitshift = (addr & 3) * 8;
                uint bitmask = 0xFF000000 >> bitshift;
                uint idx = (addr >> 2) & 3;
                addr >>= 4;
                uint4 raw = _SelfTexture2D[pos];
                
                [flatten]
                switch(idx) {
                    case 0:
                        raw.x &= ~(bitmask);
                        raw.x |= (value << ((3 - (addr & 3)) * 8));
                        break;
                    case 1:
                        raw.y &= ~(bitmask);
                        raw.y |= (value << ((3 - (addr & 3)) * 8));
                        break;
                    case 2:
                        raw.z &= ~(bitmask);
                        raw.z |= (value << ((3 - (addr & 3)) * 8));
                        break;
                    default: // 3
                        raw.w &= ~(bitmask);
                        raw.w |= (value << ((3 - (addr & 3)) * 8));
                        break;
                }

                return raw;
            }

            bool pixel_has_state(uint2 pos) {
                return !pos.x && !pos.y;
            }

            #include "helpers.cginc"

            uint4 frag(v2f_customrendertexture i) : SV_Target {
                _SelfTexture2D.GetDimensions(s_dim.x, s_dim.y);
                _Data_CARTROM.GetDimensions(m_dim.x, m_dim.y);

                uint2 pos = i.globalTexcoord.xy * s_dim;

                // Check for _Init and _InitRaw

                uint4 raw = _SelfTexture2D[uint2(0, 0)];
                cpu.axysp = raw.r;
                cpu.statuspc = raw.g & 0xFFFFFF;
                cpu.writeaddr = ((raw.b & 0xFFFF) << 8) | ((raw.g & 0xFF000000) >> 24);
                cpu.writeval = (raw.b & 0xFFFF00) >> 8;

                if(pixel_has_state(pos)) {
                    if(cpu.writeaddr <= 0x7FF) {
                        // This will be written during the commit anyways
                        raw.g &= 0xFFFFFF;
                        raw.b &= 0xFFFF00;
                        raw.b |= 0x08;
                    }

                    return raw;
                }

                if(_Init && _InitRaw) {
                    uint2 progpos = RAM_ADDR(0x600);
                    if(pos >= progpos) {
                        
                    }
                } else {
                    if(cpu.writeaddr <= 0x7FF) {
                        if(cpu.writeval > 0xFF) {
                            if(RAM_ADDR(cpu.writeaddr) == pos) {
                                return mem_write_byte(cpu.writeaddr, cpu.writeval, pos);
                            }
                            if(RAM_ADDR(cpu.writeaddr + 1) == pos) {
                                return mem_write_byte(cpu.writeaddr + 1, cpu.writeval >> 8, pos);
                            }
                        } else {
                            if(RAM_ADDR(cpu.writeaddr) == pos) {
                                return mem_write_byte(cpu.writeaddr, cpu.writeval, pos);
                            }
                        }
                        return raw;
                    }
                    return raw;
                }
            }

            ENDCG
        }
    }
}