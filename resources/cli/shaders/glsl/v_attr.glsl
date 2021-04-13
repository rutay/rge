
#define DEF_a_position    layout(location = 0) in vec4 a_position
#define DEF_a_normal      layout(location = 1) in vec4 a_normal
#define DEF_a_tangent     layout(location = 2) in vec4 a_tangent
#define DEF_a_texcoord_0  layout(location = 3) in vec4 a_texcoord_0
#define DEF_a_texcoord_1  layout(location = 4) in vec4 a_texcoord_1
#define DEF_a_color_0     layout(location = 5) in vec4 a_color_0
//#define DEF_a_joints_0  layout(location = 6) in vec4 a_joints_0
//#define DEF_a_weights_0 layout(location = 7) in vec4 a_weights_0

#define DEF_i_model_0  layout(location = 8)  in vec4 i_model_0
#define DEF_i_model_1  layout(location = 9)  in vec4 i_model_1
#define DEF_i_model_2  layout(location = 10) in vec4 i_model_2
#define DEF_i_model_3  layout(location = 11) in vec4 i_model_3
#define DEF_i_model    DEF_i_model_0;DEF_i_model_1;DEF_i_model_2;DEF_i_model_3; \
    mat4 _i_model() \
    { \
        return mat4(i_model_0, i_model_1, i_model_2, i_model_3); \
    }

#define i_model _i_model()
