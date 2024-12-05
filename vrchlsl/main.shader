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

            uniform uint _Init;, _InitRaw

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

            #define RAM_ADDR(lin) uint2(lin % 320, 1 + (lin / 320))

            uint mem_get_byte(uint addr) {
                addr &= 0xFFFF; // Ensure 16-bit, just in case
                uint bitmask = (addr & 3) * 8;
                uint idx = (addr >> 2) & 3;
                addr >>= 4;
                uint4 raw = _SelfTexture2D[RAM_ADDR(addr)];
                return (raw[idx] & (0xFF >> bitmask)) << bitmask;
            }

            uint mem_get_short(uint addr) {
                addr &= 0xFFFF;
                uint bitmask = (addr & 3) * 8;
                uint idx = (addr >> 2) & 3;
                addr >>= 4;
                uint4 raw = _SelfTexture2D[RAM_ADDR(addr)];
                return (raw[idx] & (0xFFFF >> bitmask)) << bitmask;
            }


            bool pixel_has_state(uint2 pos) {
                return !pos.x && pos.y;
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
                        cpu.writeaddr = 0x10000;
                        cpu.writeval = 0;
                    }
                } else {
                    uint4 raw = _SelfTexture2D[uint2(0, 0)];
                    cpu.axysp = raw.r;
                    cpu.statuspc = raw.g & 0xFFFFFF;
                    cpu.writeaddr = (raw.b & 0xFFFF) | ((raw.g & 0xFF000000) >> 24);
                    cpu.writeval = (raw.b & 0xFFFF0000) >> 16;

                    if(!pixel_has_state(pos)) {
                        return _SelfTexture2D[pos];
                    }

                    // Execute the CPU
                }
                return uint4(cpu.axysp, (cpu.statuspc & 0xFFFFFF) | (cpu.writeaddr << 24), ((cpu.writeaddr >> 8) & 0xFFFF) | cpu.writeval << 16, 0xFFFFFFFF);
            }
            ENDCG
        }

        Pass {
            Name "Commit"

            CGPROGRAM
            #pragma target 5.0

            #include "crt.cginc"
            #include "UnityCG.cginc"

            float _Init;, _InitRaw

            #pragma vertex CustomRenderTextureVertexShader
            #pragma fragment frag

            Texture2D<float4> _Data_CARTROM;

            static uint2 s_dim;

            #define RAM_ADDR(lin) uint2(lin % 320, 1 + (lin / 320))
            #define PIXEL_ADDR(pos) uint()

            uint4 frag(v2f_customrendertexture i) : SV_Target {
                _SelfTexture2D.GetDimensions(s_dim.x, s_dim.y);

                uint2 pos = i.globalTexcoord.xy * s_dim;

                if(_Init && _InitRaw) {
                    if(pos >= RAM_ADDR(0x0600)) {

                    }
                }
            }

            ENDCG
        }
    }
}