LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := 2ndinit
LOCAL_MODULE_STEM  := ext4_resize

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/libarchive/libarchive \
    $(LOCAL_PATH)/libsepol/include \
    $(LOCAL_PATH)/libsepol/src \
    $(LOCAL_PATH)/lzma/src/liblzmadec

LOCAL_CFLAGS := -Wall -Os -DHAVE_CONFIG_H
LOCAL_SRC_FILES := \
    2ndinit.c \
    libarchive/libarchive/archive_acl.c \
    libarchive/libarchive/archive_check_magic.c \
    libarchive/libarchive/archive_entry.c \
    libarchive/libarchive/archive_entry_sparse.c \
    libarchive/libarchive/archive_entry_xattr.c \
    libarchive/libarchive/archive_read.c \
    libarchive/libarchive/archive_read_open_filename.c \
    libarchive/libarchive/archive_read_support_filter_xz.c \
    libarchive/libarchive/archive_read_support_format_cpio.c \
    libarchive/libarchive/archive_string.c \
    libarchive/libarchive/archive_string_sprintf.c \
    libarchive/libarchive/archive_util.c \
    libarchive/libarchive/archive_virtual.c \
    libarchive/libarchive/archive_write.c \
    libarchive/libarchive/archive_write_disk_acl.c \
    libarchive/libarchive/archive_write_disk_posix.c \
    libsepol/src/avrule_block.c \
    libsepol/src/avtab.c \
    libsepol/src/conditional.c \
    libsepol/src/constraint.c \
    libsepol/src/context.c \
    libsepol/src/debug.c \
    libsepol/src/ebitmap.c \
    libsepol/src/expand.c \
    libsepol/src/hashtab.c \
    libsepol/src/mls.c \
    libsepol/src/policydb.c \
    libsepol/src/services.c \
    libsepol/src/sidtab.c \
    libsepol/src/symtab.c \
    libsepol/src/util.c \
    libsepol/src/write.c \
    lzma/src/liblzmadec/main.c
include $(BUILD_EXECUTABLE)
