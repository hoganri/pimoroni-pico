#include "../../../pimoroni-pico/libraries/breakout_3dof/breakout_3dof.hpp"

#define MP_OBJ_TO_PTR2(o, t) ((t *)(uintptr_t)(o))

// SDA/SCL on even/odd pins, I2C0/I2C1 on even/odd pairs of pins.
#define IS_VALID_SCL(i2c, pin) (((pin) & 1) == 1 && (((pin) & 2) >> 1) == (i2c))
#define IS_VALID_SDA(i2c, pin) (((pin) & 1) == 0 && (((pin) & 2) >> 1) == (i2c))


using namespace pimoroni;

enum pins {
    PIN_LED = 25,
};

extern "C" {
#include "breakout_3dof.h"

/***** Variables Struct *****/
typedef struct _breakout_3dof_Breakout3DOF_obj_t {
    mp_obj_base_t base;
    Breakout3DOF *breakout;
} breakout_3dof_Breakout3DOF_obj_t;

/***** Print *****/
void Breakout3DOF_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind; //Unused input parameter    
    breakout_3dof_Breakout3DOF_obj_t *self = MP_OBJ_TO_PTR2(self_in, breakout_3dof_Breakout3DOF_obj_t);
    Breakout3DOF* breakout = self->breakout;
    mp_print_str(print, "Breakout3DOF(");

    mp_print_str(print, "i2c = ");
    mp_obj_print_helper(print, mp_obj_new_int((breakout->get_i2c() == i2c0) ? 0 : 1), PRINT_REPR);

    mp_print_str(print, ", sda = ");
    mp_obj_print_helper(print, mp_obj_new_int(breakout->get_sda()), PRINT_REPR);

    mp_print_str(print, ", scl = ");
    mp_obj_print_helper(print, mp_obj_new_int(breakout->get_scl()), PRINT_REPR);

    mp_print_str(print, ", interrupt = ");
    mp_obj_print_helper(print, mp_obj_new_int(breakout->get_interrupt()), PRINT_REPR);

    mp_print_str(print, ")");
}

/***** Constructor *****/
mp_obj_t Breakout3DOF_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    breakout_3dof_Breakout3DOF_obj_t *self = nullptr;

    if(n_args == 0) {
        mp_arg_check_num(n_args, n_kw, 0, 0, true);
        self = m_new_obj(breakout_3dof_Breakout3DOF_obj_t);
        self->base.type = &breakout_3dof_Breakout3DOF_type;
        self->breakout = new Breakout3DOF();        
    }
    else {
        enum { ARG_i2c, ARG_sda, ARG_scl, ARG_interrupt };
        static const mp_arg_t allowed_args[] = {
            { MP_QSTR_i2c, MP_ARG_REQUIRED | MP_ARG_INT },
            { MP_QSTR_sda, MP_ARG_REQUIRED | MP_ARG_INT },
            { MP_QSTR_scl, MP_ARG_REQUIRED | MP_ARG_INT },
            { MP_QSTR_interrupt, MP_ARG_INT, {.u_int = Breakout3DOF::PIN_UNUSED} },
        };

        // Parse args.
        mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
        mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

        // Get I2C bus.
        int i2c_id = args[ARG_i2c].u_int;
        if(i2c_id < 0 || i2c_id > 1) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("I2C(%d) doesn't exist"), i2c_id);
        }

        int sda = args[ARG_sda].u_int;
        if (!IS_VALID_SDA(i2c_id, sda)) {
            mp_raise_ValueError(MP_ERROR_TEXT("bad SDA pin"));
        }

        int scl = args[ARG_scl].u_int;
        if (!IS_VALID_SCL(i2c_id, scl)) {
            mp_raise_ValueError(MP_ERROR_TEXT("bad SCL pin"));
        }        

        self = m_new_obj(breakout_3dof_Breakout3DOF_obj_t);
        self->base.type = &breakout_3dof_Breakout3DOF_type;
        
        i2c_inst_t *i2c = (i2c_id == 0) ? i2c0 : i2c1;
        self->breakout = new Breakout3DOF(i2c, sda, scl, args[ARG_interrupt].u_int);
    }

    self->breakout->init();

    return MP_OBJ_FROM_PTR(self);
}

/***** Methods *****/
mp_obj_t Breakout3DOF_get_axis(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_axis, ARG_sample_count };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_axis, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_sample_count, MP_ARG_INT, {.u_int = 1} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    breakout_3dof_Breakout3DOF_obj_t *self = MP_OBJ_TO_PTR2(args[ARG_self].u_obj, breakout_3dof_Breakout3DOF_obj_t);

    float value = 0.0f;
    switch(args[ARG_axis].u_int) {
    case AXIS_X:
        value = self->breakout->get_axis(Breakout3DOF::X, args[ARG_sample_count].u_int);
        break;
    case AXIS_Y:
        value = self->breakout->get_axis(Breakout3DOF::Y, args[ARG_sample_count].u_int);
        break;
    case AXIS_Z:
        value = self->breakout->get_axis(Breakout3DOF::Z, args[ARG_sample_count].u_int);
        break;
    default:
        mp_raise_ValueError("axis out of range. Expected 0 to 2");
        break;
    }

    return mp_obj_new_float(value);
}

}