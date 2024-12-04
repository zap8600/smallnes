Shader "Emu/NES" {
    Properties {
        [ToggleUI] _Init ("Init", float) = 0

        _ROM ("Program", 2D) = "black" {}

        _Color ("Tint", Color) = (0, 0, 0, 1)
        _MainTex ("Texture", 2D) = "white" {}
    }

    SubShader {
        Tags{ "RenderType"="Opaque" "Queue"="Geometry" }

        Pass {
            CGPROGRAM

            #include "crt.cginc"
            #include "UnityCG.cginc"

            #pragma vertex CustomRenderTextureVertexShader
            #pragma fragment frag

            typedef struct {
                uint axysp;
                uint statuspc;
            } cpu_t;

            Texture2D<uint4> _ROM;

            static uint2 s_dim;
            static uint2 m_dim;

            uint4 frag(v2f_customrendertexture i) : SV_Target {
                _SelfTexture2D.GetDimensions(s_dim.x, s_dim.y);
                _ROM.GetDimensions(m_dim.x, m_dim.y);

                uint2 pos = i.globalTexcoord.xy * s_dim;

                if(_Init) {
                    // Probably write the program into memory
                    
                }
            }
        }
    }
}