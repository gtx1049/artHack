/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_gtx_arthack_MainActivity */

#ifndef _Included_com_gtx_arthack_MainActivity
#define _Included_com_gtx_arthack_MainActivity
#ifdef __cplusplus
extern "C" {
#endif
#undef com_gtx_arthack_MainActivity_BIND_ABOVE_CLIENT
#define com_gtx_arthack_MainActivity_BIND_ABOVE_CLIENT 8L
#undef com_gtx_arthack_MainActivity_BIND_ADJUST_WITH_ACTIVITY
#define com_gtx_arthack_MainActivity_BIND_ADJUST_WITH_ACTIVITY 128L
#undef com_gtx_arthack_MainActivity_BIND_ALLOW_OOM_MANAGEMENT
#define com_gtx_arthack_MainActivity_BIND_ALLOW_OOM_MANAGEMENT 16L
#undef com_gtx_arthack_MainActivity_BIND_AUTO_CREATE
#define com_gtx_arthack_MainActivity_BIND_AUTO_CREATE 1L
#undef com_gtx_arthack_MainActivity_BIND_DEBUG_UNBIND
#define com_gtx_arthack_MainActivity_BIND_DEBUG_UNBIND 2L
#undef com_gtx_arthack_MainActivity_BIND_IMPORTANT
#define com_gtx_arthack_MainActivity_BIND_IMPORTANT 64L
#undef com_gtx_arthack_MainActivity_BIND_NOT_FOREGROUND
#define com_gtx_arthack_MainActivity_BIND_NOT_FOREGROUND 4L
#undef com_gtx_arthack_MainActivity_BIND_WAIVE_PRIORITY
#define com_gtx_arthack_MainActivity_BIND_WAIVE_PRIORITY 32L
#undef com_gtx_arthack_MainActivity_CONTEXT_IGNORE_SECURITY
#define com_gtx_arthack_MainActivity_CONTEXT_IGNORE_SECURITY 2L
#undef com_gtx_arthack_MainActivity_CONTEXT_INCLUDE_CODE
#define com_gtx_arthack_MainActivity_CONTEXT_INCLUDE_CODE 1L
#undef com_gtx_arthack_MainActivity_CONTEXT_RESTRICTED
#define com_gtx_arthack_MainActivity_CONTEXT_RESTRICTED 4L
#undef com_gtx_arthack_MainActivity_MODE_APPEND
#define com_gtx_arthack_MainActivity_MODE_APPEND 32768L
#undef com_gtx_arthack_MainActivity_MODE_ENABLE_WRITE_AHEAD_LOGGING
#define com_gtx_arthack_MainActivity_MODE_ENABLE_WRITE_AHEAD_LOGGING 8L
#undef com_gtx_arthack_MainActivity_MODE_MULTI_PROCESS
#define com_gtx_arthack_MainActivity_MODE_MULTI_PROCESS 4L
#undef com_gtx_arthack_MainActivity_MODE_PRIVATE
#define com_gtx_arthack_MainActivity_MODE_PRIVATE 0L
#undef com_gtx_arthack_MainActivity_MODE_WORLD_READABLE
#define com_gtx_arthack_MainActivity_MODE_WORLD_READABLE 1L
#undef com_gtx_arthack_MainActivity_MODE_WORLD_WRITEABLE
#define com_gtx_arthack_MainActivity_MODE_WORLD_WRITEABLE 2L
#undef com_gtx_arthack_MainActivity_DEFAULT_KEYS_DIALER
#define com_gtx_arthack_MainActivity_DEFAULT_KEYS_DIALER 1L
#undef com_gtx_arthack_MainActivity_DEFAULT_KEYS_DISABLE
#define com_gtx_arthack_MainActivity_DEFAULT_KEYS_DISABLE 0L
#undef com_gtx_arthack_MainActivity_DEFAULT_KEYS_SEARCH_GLOBAL
#define com_gtx_arthack_MainActivity_DEFAULT_KEYS_SEARCH_GLOBAL 4L
#undef com_gtx_arthack_MainActivity_DEFAULT_KEYS_SEARCH_LOCAL
#define com_gtx_arthack_MainActivity_DEFAULT_KEYS_SEARCH_LOCAL 3L
#undef com_gtx_arthack_MainActivity_DEFAULT_KEYS_SHORTCUT
#define com_gtx_arthack_MainActivity_DEFAULT_KEYS_SHORTCUT 2L
#undef com_gtx_arthack_MainActivity_RESULT_CANCELED
#define com_gtx_arthack_MainActivity_RESULT_CANCELED 0L
#undef com_gtx_arthack_MainActivity_RESULT_FIRST_USER
#define com_gtx_arthack_MainActivity_RESULT_FIRST_USER 1L
#undef com_gtx_arthack_MainActivity_RESULT_OK
#define com_gtx_arthack_MainActivity_RESULT_OK -1L
/*
 * Class:     com_gtx_arthack_MainActivity
 * Method:    testCode
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_example_tomek_notepad_NoteActivity_init(JNIEnv *, jobject);
JNIEXPORT jstring JNICALL Java_com_example_tomek_notepad_NoteActivity_initPwd(JNIEnv *, jobject, jstring);

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

typedef struct oatheader
{
  uint8_t magic_[4];
  uint8_t version_[4];
  uint32_t adler32_checksum_;

  uint32_t instruction_set_;
  uint32_t instruction_set_features_;

  uint32_t dex_file_count_;
  uint32_t executable_offset_;
  uint32_t interpreter_to_interpreter_bridge_offset_;
  uint32_t interpreter_to_compiled_code_bridge_offset_;
  uint32_t jni_dlsym_lookup_offset_;
  uint32_t portable_imt_conflict_trampoline_offset_;
  uint32_t portable_resolution_trampoline_offset_;
  uint32_t portable_to_interpreter_bridge_offset_;
  uint32_t quick_generic_jni_trampoline_offset_;
  uint32_t quick_imt_conflict_trampoline_offset_;
  uint32_t quick_resolution_trampoline_offset_;
  uint32_t quick_to_interpreter_bridge_offset_;

  // The amount that the image this oat is associated with has been patched.
  uint32_t image_patch_delta_;

  uint32_t image_file_location_oat_checksum_;
  uint32_t image_file_location_oat_data_begin_;

  uint32_t key_value_store_size_;

}oatheader;

typedef struct oatmethodheader
{
  // The offset in bytes from the start of the mapping table to the end of the header.
  uint32_t mapping_table_offset_;
  // The offset in bytes from the start of the vmap table to the end of the header.
  uint32_t vmap_table_offset_;
  // The offset in bytes from the start of the gc map to the end of the header.
  uint32_t gc_map_offset_;
  // The stack frame information.
  uint32_t frame_info_frame_size_inbyte;
  uint32_t frame_info_core_spill_mask;
  uint32_t frame_info_fp_spill_mask;
  // The code size in bytes.
  uint32_t code_size_;
} oatmethodheader;

typedef struct oatmethodheader2
{
  // The offset in bytes from the start of the mapping table to the end of the header.
  uint32_t mapping_table_offset_;
  // The offset in bytes from the start of the vmap table to the end of the header.
  uint32_t vmap_table_offset_;

  // The stack frame information.
  uint32_t frame_info_frame_size_inbyte;
  uint32_t frame_info_core_spill_mask;
  uint32_t frame_info_fp_spill_mask;

} oatmethodheader2;

typedef struct codepack
{
  char* oatcode;
  char* dexcode;
  int dexcode_size;

  char* maping_table;
  int maping_size;

  char* vmap_table;
  int vmap_size;

  unsigned char* diff_record;
  int diff_len;

  oatmethodheader2 header2;
} codepack;

#ifdef __cplusplus
}
#endif
#endif
