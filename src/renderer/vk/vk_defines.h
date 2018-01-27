#pragma once

#define FWRD_DEVICE_PTR struct Device;

#define SET_INTERNAL_DEVICE_PTR(o, d) o.internal_device_ptr = &d;
#define GET_INTERNAL_DEVICE_PTR(o) o.internal_device_ptr
#define GET_NATIVE_INTERNAL_DEVICE(o) o.internal_device_ptr->device
#define GET_NATIVE_INTERNAL_CMD_POOL(o) o.internal_device_ptr->cmd_pool

#define INTERNAL_DEVICE_PTR Device* internal_device_ptr;

