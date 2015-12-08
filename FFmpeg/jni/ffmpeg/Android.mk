LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

LOCAL_MODULE := encoder

LOCAL_SRC_FILES := encodeh264.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)/ffmpeg-build/include

LOCAL_LDLIBS += -L$(LOCAL_PATH)/ffmpeg-build/lib \
				-lavformat \
				-lavfilter \
				-lavcodec \
				-lswscale \
				-lavutil \
				-lavresample \
				-lpostproc \
				-lswresample \
				-lx264

LOCAL_LDLIBS +=-L$(SYSROOT)/usr/lib -llog -lz -ljnigraphics -lc -lgcc -ldl -lm

#LOCAL_CFLAGS += -D_cpusplus -mfloat-abi=softfp -mfpu=neon -march=armv7-a
LOCAL_CFLAGS += -march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon -D__ARM_ARCH_7__ -D__ARM_ARCH_7A__
LOCAL_ARM_MODE := arm
LOCAL_ARM_NEON := true
TARGET_ARCH_ABI := armeabi-v7a

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := decoder

LOCAL_SRC_FILES := decodeh264.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)/ffmpeg-build/include

LOCAL_LDLIBS += -L$(LOCAL_PATH)/ffmpeg-build/lib \
				-lavformat \
				-lavfilter \
				-lavcodec \
				-lswscale \
				-lavutil \
				-lavresample \
				-lpostproc \
				-lswresample \
				-lx264

LOCAL_LDLIBS +=-L$(SYSROOT)/usr/lib -llog -lz -ljnigraphics -lc -lgcc -ldl -lm

#LOCAL_CFLAGS += -D_cpusplus -mfloat-abi=softfp -mfpu=neon -march=armv7-a
LOCAL_CFLAGS += -march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon -D__ARM_ARCH_7__ -D__ARM_ARCH_7A__
LOCAL_ARM_MODE := arm
LOCAL_ARM_NEON := true
TARGET_ARCH_ABI := armeabi-v7a

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := bitmap

LOCAL_SRC_FILES := android_bitmap_info.cpp


LOCAL_LDLIBS +=-L$(SYSROOT)/usr/lib -llog -lz -ljnigraphics -lc -lgcc -ldl -lm

#LOCAL_CFLAGS += -D_cpusplus -mfloat-abi=softfp -mfpu=neon -march=armv7-a
LOCAL_CFLAGS += -march=armv7-a -mtune=cortex-a9 -mfloat-abi=softfp -mfpu=neon -D__ARM_ARCH_7__ -D__ARM_ARCH_7A__
LOCAL_ARM_MODE := arm
LOCAL_ARM_NEON := true
TARGET_ARCH_ABI := armeabi-v7a

include $(BUILD_SHARED_LIBRARY)
