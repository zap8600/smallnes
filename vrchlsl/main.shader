Shader "Emu/NES" {
    Properties {
        [ToggleUI] _Init ("Init", float) = 0
        [ToggleUI] _InitRaw ("Init directly from _Data_RAM_RAW", float) = 0

        _Color ("Tint", Color) = (0, 0, 0, 1)
        _MainTex ("Texture", 2D) = "white" {}
    }

    SubShader {
        Tags{ "RenderType"="Opaque" "Queue"="Geometry" }

        Pass {
            CGPROGRAM

            #include "crt.cginc"
            #include "UnityCG.cginc"

            uniform uint _Init;

            #pragma vertex CustomRenderTextureVertexShader
            #pragma fragment frag

            typedef struct {
                uint axysp;
                uint statuspc;
            } cpu_t;

            static cpu_t cpu;

            Texture2D<uint4> _Data_CARTROM;

            static uint2 s_dim;

            #define RAM_ADDR(lin) uint2(lin % 320, 1 + (lin / 2048))

            uint4 frag(v2f_customrendertexture i) : SV_Target {
                _SelfTexture2D.GetDimensions(s_dim.x, s_dim.y);

                uint2 pos = i.globalTexcoord.xy * s_dim;

                if(_Init) {
                    if(_InitRaw) {
                        return _SelfTexture2D[pos];
                    } else {
                        cpu.axysp = 0;
                        cpu.statuspc = (0x30 << 16) | _SelfTexture2D[RAM_ADDR(0xFFFC)];
                    }
                }
            }
        }
    }
}