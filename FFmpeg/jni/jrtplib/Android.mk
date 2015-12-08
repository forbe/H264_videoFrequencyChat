#LOCAL_PATH := $(call my-dir)
#
#include $(CLEAR_VARS)
#
#LOCAL_MODULE := rtpsend
#
#LOCAL_SRC_FILES := rtpsend.cpp
#
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/jrtplib3/include/jrtplib3 \
#					$(LOCAL_PATH)/jrtplib3/include/jthread
#
#LOCAL_LDLIBS += -L$(LOCAL_PATH)/jrtplib3/libs \
#				-ljrtplib \
#				-ljthread
#
#LOCAL_LDLIBS +=-L$(SYSROOT)/usr/lib -llog -lz -ljnigraphics -lc -lgcc -ldl -lm
#
#LOCAL_CFLAGS += -D_cpusplus -mfloat-abi=softfp -mfpu=neon -march=armv7-a
#LOCAL_CFLAGS += -march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon -D__ARM_ARCH_7__ -D__ARM_ARCH_7A__
#LOCAL_ARM_MODE := arm
#LOCAL_ARM_NEON := true
#TARGET_ARCH_ABI := armeabi-v7a
#
#include $(BUILD_SHARED_LIBRARY)


LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE  := jthread
LOCAL_SRC_FILES := libs/libjthread.a
LOCAL_C_INCLUDES += $(LOCAL_PATH)/jthread
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE  := jrtplib
LOCAL_SRC_FILES := libs/libjrtplib.a
LOCAL_STATIC_LIBRARIES := jthread

LOCAL_C_INCLUDES += $(LOCAL_PATH)/jrtplib3 \
					$(LOCAL_PATH)/jthread

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := jrtp
LOCAL_SRC_FILES := rtpmanage.cpp com_jni_rtp.cpp
					

LOCAL_STATIC_LIBRARIES := jthread jrtplib

LOCAL_C_INCLUDES += $(LOCAL_PATH)/jthread \
					$(LOCAL_PATH)/jrtplib3

LOCAL_LDLIBS := -llog

#LOCAL_CFLAGS += -D_cpusplus -mfloat-abi=softfp -mfpu=neon -march=armv7-a
LOCAL_CFLAGS += -march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon -D__ARM_ARCH_7__ -D__ARM_ARCH_7A__
LOCAL_ARM_MODE := arm
LOCAL_ARM_NEON := true
TARGET_ARCH_ABI := armeabi-v7a

include $(BUILD_SHARED_LIBRARY)
