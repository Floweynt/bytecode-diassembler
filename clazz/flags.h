#pragma once

enum class_access_flags
{
    CL_ACC_PUBLIC = 0x0001,
    CL_ACC_FINAL = 0x0010,
    CL_ACC_SUPER = 0x0020,
    CL_ACC_INTERFACE = 0x0200,
    CL_ACC_ABSTRACT = 0x0400,
    CL_ACC_SYNTHETIC = 0x1000,
    CL_ACC_ANNOTATION = 0x2000,
    CL_ACC_ENUM = 0x4000,
};

enum field_access_flags
{
    FIELD_ACC_PUBLIC = 0x0001,
    FIELD_ACC_PRIVATE = 0x0002,
    FIELD_ACC_PROTECTED = 0x0004,
    FIELD_ACC_STATIC = 0x0008,
    FIELD_ACC_FINAL = 0x0010,
    FIELD_ACC_VOLATILE = 0x0040,
    FIELD_ACC_TRANSIENT = 0x0080,
    FIELD_ACC_SYNTHETIC = 0x1000,
    FIELD_ACC_ENUM = 0x4000,
};

enum method_access_flags
{
    METHOD_ACC_PUBLIC = 0x0001,
    METHOD_ACC_PRIVATE = 0x0002,
    METHOD_ACC_PROTECTED = 0x0004,
    METHOD_ACC_STATIC = 0x0008,
    METHOD_ACC_FINAL = 0x0010,
    METHOD_ACC_SYNCHRONIZED = 0x0020,
    METHOD_ACC_BRIDGE = 0x0040,
    METHOD_ACC_VARARGS = 0x0080,
    METHOD_ACC_NATIVE = 0x0100,
    METHOD_ACC_ABSTRACT = 0x0400,
    METHOD_ACC_STRICT = 0x0800,
    METHOD_ACC_SYNTHETIC = 0x1000,
};

enum inner_class_access_flags
{
    IC_ACC_PUBLIC = 0x0001,
    IC_ACC_PRIVATE = 0x0002,
    IC_ACC_PROTECTED = 0x0004,
    IC_ACC_STATIC = 0x0008,
    IC_ACC_FINAL = 0x0010,
    IC_ACC_INTERFACE = 0x0200,
    IC_ACC_ABSTRACT = 0x0400,
    IC_ACC_SYNTHETIC = 0x1000,
    IC_ACC_ANNOTATION = 0x2000,
    IC_ACC_ENUM = 0x4000,
};